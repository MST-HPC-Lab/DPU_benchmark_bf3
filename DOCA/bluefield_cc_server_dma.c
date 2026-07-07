#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#include <doca_error.h>
#include <doca_log.h>
#include <doca_dev.h>
#include <doca_mmap.h>
#include <doca_buf.h>
#include <doca_buf_inventory.h>
#include <doca_dma.h>
#include <doca_pe.h>
#include <doca_types.h>
#include <doca_comm_channel.h>

DOCA_LOG_REGISTER(BF_CC_SERVER);

#define LOCAL_BUFFER_SIZE 1024
#define BUFFER_INVENTORY_SIZE 2
#define SERVICE_NAME "dma_blob_service"

struct export_blob_header {
    uint64_t remote_addr;
    uint64_t remote_len;
    uint64_t export_desc_len;
};

struct dma_state {
    bool done;
    doca_error_t result;
};

static doca_error_t open_dma_device(struct doca_dev **dev)
{
    doca_error_t result;
    struct doca_devinfo **dev_list;
    uint32_t nb_devs;

    result = doca_devinfo_create_list(&dev_list, &nb_devs);
    if (result != DOCA_SUCCESS)
        return result;

    for (uint32_t i = 0; i < nb_devs; i++) {
        result = doca_dma_cap_task_memcpy_is_supported(dev_list[i]);
        if (result == DOCA_SUCCESS) {
            result = doca_dev_open(dev_list[i], dev);
            if (result == DOCA_SUCCESS) {
                DOCA_LOG_INFO("Opened DMA-capable device %u", i);
                doca_devinfo_destroy_list(dev_list);
                return DOCA_SUCCESS;
            }
        }
    }

    doca_devinfo_destroy_list(dev_list);
    return DOCA_ERROR_NOT_FOUND;
}

static doca_error_t open_first_representor(struct doca_dev *dev,
                                           struct doca_dev_rep **dev_rep)
{
    doca_error_t result;
    struct doca_devinfo_rep **rep_list;
    uint32_t nb_reps;

    result = doca_devinfo_rep_create_list(dev,
                                          DOCA_DEVINFO_REP_FILTER_ALL,
                                          &rep_list,
                                          &nb_reps);
    if (result != DOCA_SUCCESS)
        return result;

    for (uint32_t i = 0; i < nb_reps; i++) {
        result = doca_dev_rep_open(rep_list[i], dev_rep);
        if (result == DOCA_SUCCESS) {
            DOCA_LOG_INFO("Opened representor %u", i);
            doca_devinfo_rep_destroy_list(rep_list);
            return DOCA_SUCCESS;
        }
    }

    doca_devinfo_rep_destroy_list(rep_list);
    return DOCA_ERROR_NOT_FOUND;
}

static void dma_completed_callback(struct doca_dma_task_memcpy *task,
                                   union doca_data task_user_data,
                                   union doca_data ctx_user_data)
{
    struct dma_state *state = (struct dma_state *)ctx_user_data.ptr;

    state->done = true;
    state->result = DOCA_SUCCESS;

    doca_task_free(doca_dma_task_memcpy_as_task(task));
}

static void dma_error_callback(struct doca_dma_task_memcpy *task,
                               union doca_data task_user_data,
                               union doca_data ctx_user_data)
{
    struct dma_state *state = (struct dma_state *)ctx_user_data.ptr;

    state->done = true;
    state->result = doca_task_get_status(doca_dma_task_memcpy_as_task(task));

    doca_task_free(doca_dma_task_memcpy_as_task(task));
}

static void check_result(doca_error_t result, const char *msg)
{
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("%s: %s", msg, doca_error_get_descr(result));
        exit(EXIT_FAILURE);
    }
}

int main(void)
{
    doca_error_t result;

    struct doca_dev *dev = NULL;
    struct doca_dev_rep *dev_rep = NULL;

    struct doca_comm_channel_ep_t *ep = NULL;
    struct doca_comm_channel_addr_t *peer_addr = NULL;

    struct export_blob_header hdr;
    void *export_desc = NULL;

    char *local_buffer = NULL;

    struct doca_mmap *local_mmap = NULL;
    struct doca_mmap *remote_mmap = NULL;

    struct doca_buf_inventory *buf_inv = NULL;
    struct doca_buf *src_buf = NULL;
    struct doca_buf *dst_buf = NULL;

    struct doca_dma *dma = NULL;
    struct doca_ctx *dma_ctx = NULL;
    struct doca_pe *pe = NULL;
    struct doca_dma_task_memcpy *memcpy_task = NULL;

    struct dma_state state = {0};

    result = doca_log_backend_create_standard();
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = open_dma_device(&dev);
    check_result(result, "Failed to open DMA-capable device");

    result = open_first_representor(dev, &dev_rep);
    check_result(result, "Failed to open host representor");

    result = doca_comm_channel_ep_create(&ep);
    check_result(result, "doca_comm_channel_ep_create failed");

    result = doca_comm_channel_ep_set_device(ep, dev);
    check_result(result, "doca_comm_channel_ep_set_device failed");

    result = doca_comm_channel_ep_set_device_rep(ep, dev_rep);
    check_result(result, "doca_comm_channel_ep_set_device_rep failed");

    result = doca_comm_channel_ep_listen(ep, SERVICE_NAME);
    check_result(result, "doca_comm_channel_ep_listen failed");

    DOCA_LOG_INFO("BlueField listening on service: %s", SERVICE_NAME);

    size_t hdr_len = sizeof(hdr);

    while (1) {
        hdr_len = sizeof(hdr);

        result = doca_comm_channel_ep_recvfrom(ep,
                                               &hdr,
                                               &hdr_len,
                                               DOCA_CC_MSG_FLAG_NONE,
                                               &peer_addr);
        if (result == DOCA_SUCCESS)
            break;

        if (result != DOCA_ERROR_AGAIN) {
            DOCA_LOG_ERR("Failed to receive header: %s",
                         doca_error_get_descr(result));
            return EXIT_FAILURE;
        }

        usleep(1000);
    }

    DOCA_LOG_INFO("Received export header from host");
    DOCA_LOG_INFO("Remote address: 0x%lx", hdr.remote_addr);
    DOCA_LOG_INFO("Remote length : %lu", hdr.remote_len);
    DOCA_LOG_INFO("Export length : %lu", hdr.export_desc_len);

    if (hdr.remote_len > LOCAL_BUFFER_SIZE) {
        DOCA_LOG_ERR("Remote buffer larger than local buffer");
        return EXIT_FAILURE;
    }

    export_desc = malloc(hdr.export_desc_len);
    if (export_desc == NULL)
        return EXIT_FAILURE;

    size_t desc_len = hdr.export_desc_len;

    while (1) {
        desc_len = hdr.export_desc_len;

        result = doca_comm_channel_ep_recvfrom(ep,
                                               export_desc,
                                               &desc_len,
                                               DOCA_CC_MSG_FLAG_NONE,
                                               &peer_addr);
        if (result == DOCA_SUCCESS)
            break;

        if (result != DOCA_ERROR_AGAIN) {
            DOCA_LOG_ERR("Failed to receive export descriptor: %s",
                         doca_error_get_descr(result));
            return EXIT_FAILURE;
        }

        usleep(1000);
    }

    DOCA_LOG_INFO("Received export descriptor");

    local_buffer = malloc(LOCAL_BUFFER_SIZE);
    if (local_buffer == NULL)
        return EXIT_FAILURE;

    memset(local_buffer, 0, LOCAL_BUFFER_SIZE);

    result = doca_mmap_create(&local_mmap);
    check_result(result, "doca_mmap_create local failed");

    result = doca_mmap_set_memrange(local_mmap, local_buffer, LOCAL_BUFFER_SIZE);
    check_result(result, "doca_mmap_set_memrange local failed");

    result = doca_mmap_add_dev(local_mmap, dev);
    check_result(result, "doca_mmap_add_dev local failed");

    result = doca_mmap_set_permissions(local_mmap, DOCA_ACCESS_FLAG_LOCAL_READ_WRITE);
    check_result(result, "doca_mmap_set_permissions local failed");

    result = doca_mmap_start(local_mmap);
    check_result(result, "doca_mmap_start local failed");

    result = doca_mmap_create_from_export(NULL,
                                          export_desc,
                                          hdr.export_desc_len,
                                          dev,
                                          &remote_mmap);
    check_result(result, "doca_mmap_create_from_export failed");

    result = doca_buf_inventory_create(BUFFER_INVENTORY_SIZE, &buf_inv);
    check_result(result, "doca_buf_inventory_create failed");

    result = doca_buf_inventory_start(buf_inv);
    check_result(result, "doca_buf_inventory_start failed");

    result = doca_buf_inventory_buf_get_by_addr(buf_inv,
                                                remote_mmap,
                                                (void *)(uintptr_t)hdr.remote_addr,
                                                hdr.remote_len,
                                                &src_buf);
    check_result(result, "Failed to create source buffer");

    result = doca_buf_inventory_buf_get_by_addr(buf_inv,
                                                local_mmap,
                                                local_buffer,
                                                hdr.remote_len,
                                                &dst_buf);
    check_result(result, "Failed to create destination buffer");

    result = doca_buf_set_data(src_buf,
                               (void *)(uintptr_t)hdr.remote_addr,
                               hdr.remote_len);
    check_result(result, "doca_buf_set_data source failed");

    result = doca_buf_set_data(dst_buf,
                               local_buffer,
                               hdr.remote_len);
    check_result(result, "doca_buf_set_data destination failed");

    result = doca_dma_create(dev, &dma);
    check_result(result, "doca_dma_create failed");

    dma_ctx = doca_dma_as_ctx(dma);

    union doca_data ctx_user_data;
    ctx_user_data.ptr = &state;

    result = doca_ctx_set_user_data(dma_ctx, ctx_user_data);
    check_result(result, "doca_ctx_set_user_data failed");

    result = doca_dma_task_memcpy_set_conf(dma,
                                           dma_completed_callback,
                                           dma_error_callback,
                                           1);
    check_result(result, "doca_dma_task_memcpy_set_conf failed");

    result = doca_pe_create(&pe);
    check_result(result, "doca_pe_create failed");

    result = doca_pe_connect_ctx(pe, dma_ctx);
    check_result(result, "doca_pe_connect_ctx failed");

    result = doca_ctx_start(dma_ctx);
    check_result(result, "doca_ctx_start failed");

    union doca_data task_user_data;
    memset(&task_user_data, 0, sizeof(task_user_data));

    result = doca_dma_task_memcpy_alloc_init(dma,
                                             src_buf,
                                             dst_buf,
                                             task_user_data,
                                             &memcpy_task);
    check_result(result, "doca_dma_task_memcpy_alloc_init failed");

    state.done = false;
    state.result = DOCA_SUCCESS;

    result = doca_task_submit(doca_dma_task_memcpy_as_task(memcpy_task));
    check_result(result, "doca_task_submit failed");

    while (!state.done)
        doca_pe_progress(pe);

    if (state.result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("DMA task failed: %s", doca_error_get_descr(state.result));
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("DMA completed successfully");
    printf("BlueField local buffer content:\n%s\n", local_buffer);

    doca_ctx_stop(dma_ctx);

    doca_buf_dec_refcount(src_buf, NULL);
    doca_buf_dec_refcount(dst_buf, NULL);

    doca_buf_inventory_stop(buf_inv);
    doca_buf_inventory_destroy(buf_inv);

    doca_dma_destroy(dma);
    doca_pe_destroy(pe);

    doca_mmap_destroy(remote_mmap);
    doca_mmap_destroy(local_mmap);

    doca_comm_channel_ep_disconnect(ep, peer_addr);
    doca_comm_channel_ep_destroy(ep);

    doca_dev_rep_close(dev_rep);
    doca_dev_close(dev);

    free(export_desc);
    free(local_buffer);

    return EXIT_SUCCESS;
}