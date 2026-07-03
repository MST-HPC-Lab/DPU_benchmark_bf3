#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <doca_error.h>
#include <doca_log.h>
#include <doca_dev.h>
#include <doca_pe.h>
#include <doca_ctx.h>
#include <doca_dma.h>
#include <doca_mmap.h>
#include <doca_buf.h>
#include <doca_buf_inventory.h>

DOCA_LOG_REGISTER(BF_DMA_INITIATOR);

#define EXPORT_FILE "dma_export.bin"
#define BUF_INVENTORY_SIZE 8

struct export_file_header {
    uint64_t remote_addr;
    uint64_t remote_len;
    uint64_t export_desc_len;
};

static bool task_done = false;
static bool task_success = false;

static void dma_done_cb(struct doca_dma_task_memcpy *task,
                        union doca_data task_user_data,
                        union doca_data ctx_user_data)
{
    (void)task;
    (void)task_user_data;
    (void)ctx_user_data;
    task_done = true;
    task_success = true;
}

static void dma_error_cb(struct doca_dma_task_memcpy *task,
                         union doca_data task_user_data,
                         union doca_data ctx_user_data)
{
    (void)task;
    (void)task_user_data;
    (void)ctx_user_data;
    task_done = true;
    task_success = false;
}

int main(void)
{
    doca_error_t result;
    struct export_file_header hdr;
    void *export_desc = NULL;
    FILE *fp = NULL;

    struct doca_devinfo **dev_list;
    struct doca_dev *dev = NULL;
    uint32_t nb_devs;

    struct doca_pe *pe = NULL;
    struct doca_dma *dma = NULL;
    struct doca_ctx *ctx = NULL;

    struct doca_mmap *remote_mmap = NULL;
    struct doca_mmap *local_mmap = NULL;
    struct doca_buf_inventory *buf_inv = NULL;
    struct doca_buf *remote_src_buf = NULL;
    struct doca_buf *local_dst_buf = NULL;
    struct doca_dma_task_memcpy *task = NULL;

    char *local_dst = NULL;

    result = doca_log_backend_create_standard();
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    fp = fopen(EXPORT_FILE, "rb");
    if (fp == NULL) {
        DOCA_LOG_ERR("Failed to open export file");
        return EXIT_FAILURE;
    }

    fread(&hdr, sizeof(hdr), 1, fp);

    export_desc = malloc(hdr.export_desc_len);
    fread(export_desc, 1, hdr.export_desc_len, fp);
    fclose(fp);

    DOCA_LOG_INFO("Remote host address: 0x%lx", hdr.remote_addr);
    DOCA_LOG_INFO("Remote length: %lu", hdr.remote_len);

    local_dst = malloc(hdr.remote_len);
    memset(local_dst, 0, hdr.remote_len);

    result = doca_devinfo_create_list(&dev_list, &nb_devs);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    for (uint32_t i = 0; i < nb_devs; i++) {
        uint8_t import_supported = 0;

        result = doca_mmap_cap_is_create_from_export_pci_supported(dev_list[i], &import_supported);
        if (result != DOCA_SUCCESS || !import_supported)
            continue;

        result = doca_dma_cap_task_memcpy_is_supported(dev_list[i]);
        if (result != DOCA_SUCCESS)
            continue;

        result = doca_dev_open(dev_list[i], &dev);
        if (result == DOCA_SUCCESS) {
            DOCA_LOG_INFO("Opened DMA/import-capable device %u", i);
            break;
        }
    }

    if (dev == NULL) {
        DOCA_LOG_ERR("No suitable device found");
        return EXIT_FAILURE;
    }

    result = doca_mmap_create_from_export(NULL,
                                          export_desc,
                                          hdr.export_desc_len,
                                          dev,
                                          &remote_mmap);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to import host mmap: %s", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = doca_mmap_create(&local_mmap);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_mmap_set_memrange(local_mmap, local_dst, hdr.remote_len);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_mmap_add_dev(local_mmap, dev);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_mmap_start(local_mmap);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_buf_inventory_create(BUF_INVENTORY_SIZE, &buf_inv);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_buf_inventory_start(buf_inv);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_buf_inventory_buf_get_by_data(buf_inv,
                                                remote_mmap,
                                                (void *)(uintptr_t)hdr.remote_addr,
                                                hdr.remote_len,
                                                &remote_src_buf);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to create remote source buf: %s", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = doca_buf_inventory_buf_get_by_addr(buf_inv,
                                                local_mmap,
                                                local_dst,
                                                hdr.remote_len,
                                                &local_dst_buf);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to create local destination buf: %s", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = doca_pe_create(&pe);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_dma_create(dev, &dma);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_dma_task_memcpy_set_conf(dma, dma_done_cb, dma_error_cb, 1);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    ctx = doca_dma_as_ctx(dma);

    result = doca_pe_connect_ctx(pe, ctx);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_ctx_start(ctx);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_dma_task_memcpy_alloc_init(dma,
                                             remote_src_buf,
                                             local_dst_buf,
                                             NULL,
                                             &task);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to allocate DMA task: %s", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = doca_task_submit(doca_dma_task_memcpy_as_task(task));
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to submit DMA task: %s", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    while (!task_done)
        doca_pe_progress(pe);

    if (task_success && local_dst[0] == 'H') {
        DOCA_LOG_INFO("DMA host-to-BlueField copy succeeded");
        DOCA_LOG_INFO("First byte copied: %c", local_dst[0]);
    } else {
        DOCA_LOG_ERR("DMA failed or copied data is wrong");
    }

    if (task != NULL)
        doca_task_free(doca_dma_task_memcpy_as_task(task));

    if (remote_src_buf != NULL)
        doca_buf_dec_refcount(remote_src_buf, NULL);

    if (local_dst_buf != NULL)
        doca_buf_dec_refcount(local_dst_buf, NULL);

    if (buf_inv != NULL)
        doca_buf_inventory_destroy(buf_inv);

    if (remote_mmap != NULL)
        doca_mmap_destroy(remote_mmap);

    if (local_mmap != NULL)
        doca_mmap_destroy(local_mmap);

    if (ctx != NULL)
        doca_ctx_stop(ctx);

    if (dma != NULL)
        doca_dma_destroy(dma);

    if (pe != NULL)
        doca_pe_destroy(pe);

    if (dev != NULL)
        doca_dev_close(dev);

    doca_devinfo_destroy_list(dev_list);
    free(export_desc);
    free(local_dst);

    return EXIT_SUCCESS;
}