#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
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
#include <doca_types.h>

DOCA_LOG_REGISTER(HOST_DMA_RESPONDER);

#define BUFFER_SIZE 1024
#define TCP_PORT 5555

struct export_file_header {
    uint64_t remote_addr;
    uint64_t remote_len;
    uint64_t export_desc_len;
};

static int send_all(int fd, const void *buf, size_t len)
{
    const char *ptr = buf;

    while (len > 0) {
        ssize_t sent = send(fd, ptr, len, 0);
        if (sent <= 0)
            return -1;

        ptr += sent;
        len -= sent;
    }

    return 0;
}

static int create_server_socket(void)
{
    int server_fd;
    int opt = 1;
    struct sockaddr_in addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return -1;
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(TCP_PORT);

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

int main(void)
{
    doca_error_t result;
    struct doca_devinfo **dev_list;
    struct doca_dev *dev = NULL;
    struct doca_mmap *mmap = NULL;
    uint32_t nb_devs;

    char *buffer = NULL;
    const void *export_desc = NULL;
    size_t export_desc_len = 0;

    int server_fd = -1;
    int client_fd = -1;

    result = doca_log_backend_create_standard();
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    buffer = malloc(BUFFER_SIZE);
    if (buffer == NULL)
        return EXIT_FAILURE;

    memset(buffer, 'H', BUFFER_SIZE);
    buffer[BUFFER_SIZE - 1] = '\0';

    result = doca_devinfo_create_list(&dev_list, &nb_devs);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    for (uint32_t i = 0; i < nb_devs; i++) {
        uint8_t supported = 0;

        result = doca_mmap_cap_is_export_pci_supported(dev_list[i], &supported);
        if (result == DOCA_SUCCESS && supported) {
            result = doca_dev_open(dev_list[i], &dev);
            if (result == DOCA_SUCCESS) {
                DOCA_LOG_INFO("Opened PCI export-capable device %u", i);
                break;
            }
        }
    }

    if (dev == NULL) {
        DOCA_LOG_ERR("No PCI export-capable device found");
        return EXIT_FAILURE;
    }

    result = doca_mmap_create(&mmap);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_mmap_set_memrange(mmap, buffer, BUFFER_SIZE);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_mmap_add_dev(mmap, dev);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_mmap_set_permissions(mmap, DOCA_ACCESS_FLAG_PCI_READ_WRITE);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_mmap_start(mmap);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_mmap_export_pci(mmap, dev, &export_desc, &export_desc_len);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to export mmap: %s", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    struct export_file_header hdr;
    hdr.remote_addr = (uint64_t)(uintptr_t)buffer;
    hdr.remote_len = BUFFER_SIZE;
    hdr.export_desc_len = export_desc_len;

    server_fd = create_server_socket();
    if (server_fd < 0)
        return EXIT_FAILURE;

    DOCA_LOG_INFO("Waiting for BlueField connection on TCP port %d", TCP_PORT);

    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
        perror("accept");
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("BlueField connected");

    if (send_all(client_fd, &hdr, sizeof(hdr)) < 0) {
        perror("send header");
        return EXIT_FAILURE;
    }

    if (send_all(client_fd, export_desc, export_desc_len) < 0) {
        perror("send export descriptor");
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("Export descriptor sent over TCP");
    DOCA_LOG_INFO("Host buffer address: %p", buffer);
    DOCA_LOG_INFO("Host buffer length : %lu", hdr.remote_len);
    DOCA_LOG_INFO("Keep this program running while BlueField performs DMA");
    DOCA_LOG_INFO("Press Enter only after BlueField finishes");

    getchar();

    close(client_fd);
    close(server_fd);

    doca_mmap_destroy(mmap);
    doca_dev_close(dev);
    doca_devinfo_destroy_list(dev_list);
    free(buffer);

    return EXIT_SUCCESS;
}