#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <doca_error.h>
#include <doca_log.h>
#include <doca_dev.h>
#include <doca_mmap.h>
#include <doca_buf.h>
#include <doca_buf_inventory.h>
#include <doca_dma.h>
#include <doca_pe.h>

DOCA_LOG_REGISTER(BF_TCP_DMA);

#define LOCAL_BUFFER_SIZE 4096

struct dma_state {
    bool done;
    doca_error_t result;
};

static int send_all(int fd, const void *buf, size_t len)
{
    const char *p = buf;
    while (len > 0) {
        ssize_t n = send(fd, p, len, 0);
        if (n <= 0)
            return -1;
        p += n;
        len -= n;
    }
    return 0;
}

static int recv_all(int fd, void *buf, size_t len)
{
    char *p = buf;
    while (len > 0) {
        ssize_t n = recv(fd, p, len, 0);
        if (n <= 0)
            return -1;
        p += n;
        len -= n;
    }
    return 0;
}

static int tcp_connect_to_host(const char *ip, int port)
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
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1) {
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

static doca_error_t open_doca_device(const char *pci_addr, struct doca_dev **dev)
{
    struct doca_devinfo **devinfo_list;
    uint32_t nb_devs;
    doca_error_t result;

    result = doca_devinfo_create_list(&devinfo_list, &nb_devs);
    if (result != DOCA_SUCCESS)
        return result;

    for (uint32_t i = 0; i < nb_devs; i++) {
        char found_pci[DOCA_DEVINFO_PCI_ADDR_SIZE];

        result = doca_devinfo_get_pci_addr_str(devinfo_list[i], found_pci);
        if (result != DOCA_SUCCESS)
            continue;

        if (strcmp(found_pci, pci_addr) == 0) {
            result = doca_dev_open(devinfo_list[i], dev);
            doca_devinfo_destroy_list(devinfo_list);
            return result;
        }
    }

    doca_devinfo_destroy_list(devinfo_list);
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

int main(int argc, char **argv)
{
    const char *host_ip;
    const char *pci_addr;
    int port;

    int sockfd = -1;

    uint64_t remote_addr;
    uint64_t remote_len;
    uint64_t blob_len;

    void *export_blob = NULL;
    char *local_buffer = NULL;

    struct doca_dev *dev = NULL;

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

    doca_error_t result;

    if (argc < 4) {
        printf("Usage: %s <host_ip> <bf_pci_addr> <port>\n", argv[0]);
        printf("Example: %s 192.168.100.1 03:00.0 5555\n", argv[0]);
        return EXIT_FAILURE;
    }

    host_ip = argv[1];
    pci_addr = argv[2];
    port = atoi(argv[3]);

    result = doca_log_backend_create_standard();
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    sockfd = tcp_connect_to_host(host_ip, port);
    if (sockfd < 0)
        return EXIT_FAILURE;

    printf("Connected to host TCP server.\n");

    recv_all(sockfd, &remote_addr, sizeof(remote_addr));
    recv_all(sockfd, &remote_len, sizeof(remote_len));
    recv_all(sockfd, &blob_len, sizeof(blob_len));

    export_blob = malloc(blob_len);
    if (export_blob == NULL) {
        perror("malloc export_blob");
        return EXIT_FAILURE;
    }

    recv_all(sockfd, export_blob, blob_len);

    printf("Received export blob from host.\n");
    printf("Remote host VA: 0x%lx\n", remote_addr);
    printf("Remote length : %lu\n", remote_len);
    printf("Blob length   : %lu\n", blob_len);

    result = open_doca_device(pci_addr, &dev);
    if (result != DOCA_SUCCESS) {
        printf("Failed to open DOCA device: %s\n", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    local_buffer = aligned_alloc(4096, LOCAL_BUFFER_SIZE);
    if (local_buffer == NULL) {
        perror("aligned_alloc");
        return EXIT_FAILURE;
    }

    memset(local_buffer, 0, LOCAL_BUFFER_SIZE);

    result = doca_mmap_create(&local_mmap);
    if (result != DOCA_SUCCESS) {
        printf("doca_mmap_create local failed: %s\n", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = doca_mmap_add_dev(local_mmap, dev);
    if (result != DOCA_SUCCESS) {
        printf("doca_mmap_add_dev local failed: %s\n", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = doca_mmap_set_memrange(local_mmap, local_buffer, LOCAL_BUFFER_SIZE);
    if (result != DOCA_SUCCESS) {
        printf("doca_mmap_set_memrange local failed: %s\n", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = doca_mmap_start(local_mmap);
    if (result != DOCA_SUCCESS) {
        printf("doca_mmap_start local failed: %s\n", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = doca_mmap_create_from_export(NULL, export_blob, blob_len, dev, &remote_mmap);
    if (result != DOCA_SUCCESS) {
        printf("doca_mmap_create_from_export failed: %s\n", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = doca_buf_inventory_create(2, &buf_inv);
    if (result != DOCA_SUCCESS) {
        printf("doca_buf_inventory_create failed: %s\n", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = doca_buf_inventory_start(buf_inv);
    if (result != DOCA_SUCCESS) {
        printf("doca_buf_inventory_start failed: %s\n", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = doca_buf_inventory_buf_get_by_addr(
        buf_inv,
        remote_mmap,
        (void *)(uintptr_t)remote_addr,
        remote_len,
        &src_buf
    );

    if (result != DOCA_SUCCESS) {
        printf("Failed to create remote source buffer: %s\n", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = doca_buf_inventory_buf_get_by_addr(
        buf_inv,
        local_mmap,
        local_buffer,
        LOCAL_BUFFER_SIZE,
        &dst_buf
    );

    if (result != DOCA_SUCCESS) {
        printf("Failed to create local destination buffer: %s\n", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = doca_dma_create(dev, &dma);
    if (result != DOCA_SUCCESS) {
        printf("doca_dma_create failed: %s\n", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    dma_ctx = doca_dma_as_ctx(dma);

    union doca_data ctx_user_data;
    ctx_user_data.ptr = &state;

    result = doca_ctx_set_user_data(dma_ctx, ctx_user_data);
    if (result != DOCA_SUCCESS) {
        printf("doca_ctx_set_user_data failed: %s\n", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = doca_dma_task_memcpy_set_conf(
        dma,
        dma_completed_callback,
        dma_error_callback,
        1
    );

    if (result != DOCA_SUCCESS) {
        printf("doca_dma_task_memcpy_set_conf failed: %s\n", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = doca_pe_create(&pe);
    if (result != DOCA_SUCCESS) {
        printf("doca_pe_create failed: %s\n", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = doca_pe_connect_ctx(pe, dma_ctx);
    if (result != DOCA_SUCCESS) {
        printf("doca_pe_connect_ctx failed: %s\n", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = doca_ctx_start(dma_ctx);
    if (result != DOCA_SUCCESS) {
        printf("doca_ctx_start failed: %s\n", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    union doca_data task_user_data;
    memset(&task_user_data, 0, sizeof(task_user_data));

    result = doca_dma_task_memcpy_alloc_init(
        dma,
        src_buf,
        dst_buf,
        task_user_data,
        &memcpy_task
    );

    if (result != DOCA_SUCCESS) {
        printf("doca_dma_task_memcpy_alloc_init failed: %s\n", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    state.done = false;
    state.result = DOCA_SUCCESS;

    result = doca_task_submit(doca_dma_task_memcpy_as_task(memcpy_task));
    if (result != DOCA_SUCCESS) {
        printf("doca_task_submit failed: %s\n", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    while (!state.done)
        doca_pe_progress(pe);

    if (state.result != DOCA_SUCCESS) {
        printf("DMA memcpy failed: %s\n", doca_error_get_descr(state.result));
        return EXIT_FAILURE;
    }

    printf("DMA memcpy completed successfully.\n");
    printf("BlueField local buffer says:\n%s\n", local_buffer);

    char done = 1;
    send_all(sockfd, &done, 1);

    doca_ctx_stop(dma_ctx);
    doca_pe_destroy(pe);

    doca_buf_dec_refcount(src_buf, NULL);
    doca_buf_dec_refcount(dst_buf, NULL);

    doca_buf_inventory_stop(buf_inv);
    doca_buf_inventory_destroy(buf_inv);

    doca_dma_destroy(dma);

    doca_mmap_destroy(remote_mmap);
    doca_mmap_destroy(local_mmap);

    doca_dev_close(dev);

    free(export_blob);
    free(local_buffer);

    close(sockfd);

    return EXIT_SUCCESS;
}