#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <doca_error.h>
#include <doca_log.h>
#include <doca_dev.h>
#include <doca_mmap.h>
#include <doca_buf.h>
#include <doca_buf_inventory.h>
#include <doca_ctx.h>
#include <doca_dma.h>
#include <doca_pe.h>
#include <doca_types.h>

DOCA_LOG_REGISTER(BF_DMA_INITIATOR);

#define LOCAL_BUFFER_SIZE 1024
#define TCP_PORT 5555
#define BUFFER_INVENTORY_SIZE 2

struct export_file_header {
    uint64_t remote_addr;
    uint64_t remote_len;
    uint64_t export_desc_len;
};

struct dma_state {
    bool done;
    doca_error_t result;
};

static int recv_all(int fd, void *buf, size_t len)
{
    char *ptr = buf;

    while (len > 0) {
        ssize_t received = recv(fd, ptr, len, 0);
        if (received <= 0)
            return -1;

        ptr += received;
        len -= received;
    }

    return 0;
}

static int connect_to_host(const char *host_ip)
{
    int fd;
    struct sockaddr_in addr;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(TCP_PORT);

    if (inet_pton(AF_INET, host_ip, &addr.sin_addr) != 1) {
        perror("inet_pton");
        close(fd);
        return -1;
    }

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(fd);
        return -1;
    }

    return fd;
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

int main(int argc, char **argv)
{
    doca_error_t result;

    struct doca_devinfo **dev_list;
    struct doca_dev *dev = NULL;
    uint32_t nb_devs;

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

    char *local_buffer = NULL;
    void *export_desc = NULL;

    struct export_file_header hdr;

    int sockfd = -1;

    if (argc != 2) {
        printf("Usage: %s <host_ip>\n", argv[0]);
        printf("Example: %s 192.168.100.1\n", argv[0]);
        return EXIT_FAILURE;
    }

    result = doca_log_backend_create_standard();
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    sockfd = connect_to_host(argv[1]);
    if (sockfd < 0)
        return EXIT_FAILURE;

    DOCA_LOG_INFO("Connected to host");

    if (recv_all(sockfd, &hdr, sizeof(hdr)) < 0) {
        perror("recv header");
        return EXIT_FAILURE;
    }

    export_desc = malloc(hdr.export_desc_len);
    if (export_desc == NULL)
        return EXIT_FAILURE;

    if (recv_all(sockfd, export_desc, hdr.export_desc_len) < 0) {
        perror("recv export descriptor");
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("Received export descriptor");
    DOCA_LOG_INFO("Remote address: 0x%lx", hdr.remote_addr);
    DOCA_LOG_INFO("Remote length : %lu", hdr.remote_len);
    DOCA_LOG_INFO("Export length : %lu", hdr.export_desc_len);

    local_buffer = malloc(LOCAL_BUFFER_SIZE);
    if (local_buffer == NULL)
        return EXIT_FAILURE;

    memset(local_buffer, 0, LOCAL_BUFFER_SIZE);

    result = doca_devinfo_create_list(&dev_list, &nb_devs);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    for (uint32_t i = 0; i < nb_devs; i++) {
        uint8_t supported = 0;

        result = doca_dma_cap_task_memcpy_is_supported(dev_list[i], &supported);
        if (result == DOCA_SUCCESS && supported) {
            result = doca_dev_open(dev_list[i], &dev);
            if (result == DOCA_SUCCESS) {
                DOCA_LOG_INFO("Opened DMA-capable device %u", i);
                break;
            }
        }
    }

    if (dev == NULL) {
        DOCA_LOG_ERR("No DMA-capable device found");
        return EXIT_FAILURE;
    }

    result = doca_mmap_create(&local_mmap);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_mmap_set_memrange(local_mmap, local_buffer, LOCAL_BUFFER_SIZE);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_mmap_add_dev(local_mmap, dev);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_mmap_set_permissions(local_mmap, DOCA_ACCESS_FLAG_LOCAL_READ_WRITE);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_mmap_start(local_mmap);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_mmap_create_from_export(NULL,
                                          export_desc,
                                          hdr.export_desc_len,
                                          dev,
                                          &remote_mmap);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to create remote mmap from export: %s",
                     doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = doca_buf_inventory_create(BUFFER_INVENTORY_SIZE, &buf_inv);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_buf_inventory_start(buf_inv);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_buf_inventory_buf_get_by_addr(buf_inv,
                                                remote_mmap,
                                                (void *)(uintptr_t)hdr.remote_addr,
                                                hdr.remote_len,
                                                &src_buf);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to create source buffer: %s",
                     doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = doca_buf_inventory_buf_get_by_addr(buf_inv,
                                                local_mmap,
                                                local_buffer,
                                                LOCAL_BUFFER_SIZE,
                                                &dst_buf);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to create destination buffer: %s",
                     doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = doca_dma_create(dev, &dma);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to create DMA context: %s",
                     doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    dma_ctx = doca_dma_as_ctx(dma);

    union doca_data ctx_user_data;
    ctx_user_data.ptr = &state;

    result = doca_ctx_set_user_data(dma_ctx, ctx_user_data);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_dma_task_memcpy_set_conf(dma,
                                           dma_completed_callback,
                                           dma_error_callback,
                                           1);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_pe_create(&pe);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_pe_connect_ctx(pe, dma_ctx);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_ctx_start(dma_ctx);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    union doca_data task_user_data;
    memset(&task_user_data, 0, sizeof(task_user_data));

    result = doca_dma_task_memcpy_alloc_init(dma,
                                             src_buf,
                                             dst_buf,
                                             task_user_data,
                                             &memcpy_task);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to allocate DMA memcpy task: %s",
                     doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    state.done = false;
    state.result = DOCA_SUCCESS;

    result = doca_task_submit(doca_dma_task_memcpy_as_task(memcpy_task));
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to submit DMA task: %s",
                     doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    while (!state.done)
        doca_pe_progress(pe);

    if (state.result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("DMA task failed: %s", doca_error_get_descr(state.result));
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("DMA completed successfully");
    DOCA_LOG_INFO("Local BlueField buffer content:");
    printf("%s\n", local_buffer);

    doca_ctx_stop(dma_ctx);

    doca_buf_dec_refcount(src_buf, NULL);
    doca_buf_dec_refcount(dst_buf, NULL);

    doca_buf_inventory_stop(buf_inv);
    doca_buf_inventory_destroy(buf_inv);

    doca_mmap_destroy(remote_mmap);
    doca_mmap_destroy(local_mmap);

    doca_dma_destroy(dma);
    doca_pe_destroy(pe);

    doca_dev_close(dev);
    doca_devinfo_destroy_list(dev_list);

    free(export_desc);
    free(local_buffer);
    close(sockfd);

    return EXIT_SUCCESS;
}