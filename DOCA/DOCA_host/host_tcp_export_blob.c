#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <doca_error.h>
#include <doca_log.h>
#include <doca_dev.h>
#include <doca_mmap.h>

DOCA_LOG_REGISTER(HOST_TCP_EXPORT);

#define DEFAULT_PORT 5555
#define BUFFER_SIZE 4096

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

static int tcp_listen(int port)
{
    int server_fd;
    struct sockaddr_in addr;
    int opt = 1;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return -1;
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 1) < 0) {
        perror("listen");
        close(server_fd);
        return -1;
    }

    return server_fd;
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

int main(int argc, char **argv)
{
    const char *pci_addr;
    int port = DEFAULT_PORT;

    struct doca_dev *dev = NULL;
    struct doca_mmap *mmap = NULL;

    char *host_buffer = NULL;
    const void *export_blob = NULL;
    size_t export_blob_len = 0;

    int server_fd = -1;
    int client_fd = -1;

    doca_error_t result;

    if (argc < 2) {
        printf("Usage: %s <host_pci_addr> [port]\n", argv[0]);
        printf("Example: %s 03:00.0 5555\n", argv[0]);
        return EXIT_FAILURE;
    }

    pci_addr = argv[1];
    if (argc >= 3)
        port = atoi(argv[2]);

    result = doca_log_backend_create_standard();
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = open_doca_device(pci_addr, &dev);
    if (result != DOCA_SUCCESS) {
        printf("Failed to open DOCA device: %s\n", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    host_buffer = aligned_alloc(4096, BUFFER_SIZE);
    if (host_buffer == NULL) {
        perror("aligned_alloc");
        return EXIT_FAILURE;
    }

    snprintf(host_buffer, BUFFER_SIZE,
             "Hello from HOST memory. This message was fetched by BlueField using DOCA DMA over exported mmap.");

    printf("Host buffer VA: %p\n", host_buffer);
    printf("Host buffer content: %s\n", host_buffer);

    result = doca_mmap_create(&mmap);
    if (result != DOCA_SUCCESS) {
        printf("doca_mmap_create failed: %s\n", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = doca_mmap_add_dev(mmap, dev);
    if (result != DOCA_SUCCESS) {
        printf("doca_mmap_add_dev failed: %s\n", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = doca_mmap_set_memrange(mmap, host_buffer, BUFFER_SIZE);
    if (result != DOCA_SUCCESS) {
        printf("doca_mmap_set_memrange failed: %s\n", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = doca_mmap_start(mmap);
    if (result != DOCA_SUCCESS) {
        printf("doca_mmap_start failed: %s\n", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = doca_mmap_export_dpu(mmap, dev, &export_blob, &export_blob_len);
    if (result != DOCA_SUCCESS) {
        printf("doca_mmap_export_dpu failed: %s\n", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    server_fd = tcp_listen(port);
    if (server_fd < 0)
        return EXIT_FAILURE;

    printf("Waiting for BlueField TCP connection on port %d...\n", port);

    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
        perror("accept");
        return EXIT_FAILURE;
    }

    printf("BlueField connected. Sending export blob...\n");

    uint64_t remote_addr = (uint64_t)(uintptr_t)host_buffer;
    uint64_t remote_len = BUFFER_SIZE;
    uint64_t blob_len = export_blob_len;

    send_all(client_fd, &remote_addr, sizeof(remote_addr));
    send_all(client_fd, &remote_len, sizeof(remote_len));
    send_all(client_fd, &blob_len, sizeof(blob_len));
    send_all(client_fd, export_blob, export_blob_len);

    printf("Export blob sent successfully.\n");
    printf("Keeping host memory alive. Waiting for BlueField completion...\n");

    char done;
    recv_all(client_fd, &done, 1);

    printf("BlueField signaled completion.\n");

    close(client_fd);
    close(server_fd);

    doca_mmap_destroy(mmap);
    doca_dev_close(dev);
    free(host_buffer);

    return EXIT_SUCCESS;
}