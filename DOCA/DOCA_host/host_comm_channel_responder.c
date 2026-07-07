#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include <doca_error.h>
#include <doca_log.h>
#include <doca_dev.h>
#include <doca_mmap.h>
#include <doca_types.h>
#include <doca_comm_channel.h>

DOCA_LOG_REGISTER(HOST_CC_RESPONDER);

#define BUFFER_SIZE 1024
#define SERVICE_NAME "dma_blob_service"

struct export_blob_header {
    uint64_t remote_addr;
    uint64_t remote_len;
    uint64_t export_desc_len;
};

static doca_error_t open_export_pci_device(struct doca_dev **dev)
{
    doca_error_t result;
    struct doca_devinfo **dev_list;
    uint32_t nb_devs;

    result = doca_devinfo_create_list(&dev_list, &nb_devs);
    if (result != DOCA_SUCCESS)
        return result;

    for (uint32_t i = 0; i < nb_devs; i++) {
        uint8_t supported = 0;

        result = doca_mmap_cap_is_export_pci_supported(dev_list[i], &supported);
        if (result == DOCA_SUCCESS && supported) {
            result = doca_dev_open(dev_list[i], dev);
            if (result == DOCA_SUCCESS) {
                DOCA_LOG_INFO("Opened PCI export-capable device %u", i);
                doca_devinfo_destroy_list(dev_list);
                return DOCA_SUCCESS;
            }
        }
    }

    doca_devinfo_destroy_list(dev_list);
    return DOCA_ERROR_NOT_FOUND;
}

static doca_error_t open_device_representor(struct doca_dev *dev,
                                            struct doca_dev_rep **dev_rep)
{
    doca_error_t result;
    struct doca_devinfo_rep **rep_list;
    uint32_t nb_reps;

    result = doca_devinfo_rep_create_list(dev,
                                          DOCA_DEVINFO_REP_FILTER_NET,
                                          &rep_list,
                                          &nb_reps);
    if (result != DOCA_SUCCESS)
        return result;

    if (nb_reps == 0) {
        doca_devinfo_rep_destroy_list(rep_list);
        return DOCA_ERROR_NOT_FOUND;
    }

    result = doca_dev_rep_open(rep_list[0], dev_rep);
    if (result == DOCA_SUCCESS)
        DOCA_LOG_INFO("Opened device representor 0");

    doca_devinfo_rep_destroy_list(rep_list);
    return result;
}

int main(void)
{
    doca_error_t result;

    struct doca_dev *dev = NULL;
    struct doca_dev_rep *dev_rep = NULL;
    struct doca_mmap *mmap = NULL;

    struct doca_comm_channel_ep_t *ep = NULL;
    struct doca_comm_channel_addr_t *peer_addr = NULL;

    char *buffer = NULL;
    const void *export_desc = NULL;
    size_t export_desc_len = 0;

    struct export_blob_header hdr;

    result = doca_log_backend_create_standard();
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    buffer = malloc(BUFFER_SIZE);
    if (buffer == NULL)
        return EXIT_FAILURE;

    memset(buffer, 'H', BUFFER_SIZE);
    buffer[BUFFER_SIZE - 1] = '\0';

    result = open_export_pci_device(&dev);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to open export-capable device: %s",
                     doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = open_device_representor(dev, &dev_rep);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to open device representor: %s",
                     doca_error_get_descr(result));
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

    hdr.remote_addr = (uint64_t)(uintptr_t)buffer;
    hdr.remote_len = BUFFER_SIZE;
    hdr.export_desc_len = export_desc_len;

    result = doca_comm_channel_ep_create(&ep);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to create comm channel endpoint: %s",
                     doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    result = doca_comm_channel_ep_set_device(ep, dev);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_comm_channel_ep_set_device_rep(ep, dev_rep);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    result = doca_comm_channel_ep_listen(ep, SERVICE_NAME);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to listen on service: %s",
                     doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("Waiting for BlueField connection using DOCA Comm Channel");

    char hello[64];
    size_t hello_len = sizeof(hello);

    while (1) {
        result = doca_comm_channel_ep_recvfrom(ep,
                                               hello,
                                               &hello_len,
                                               DOCA_CC_MSG_FLAG_NONE,
                                               &peer_addr);
        if (result == DOCA_SUCCESS)
            break;

        if (result != DOCA_ERROR_AGAIN) {
            DOCA_LOG_ERR("Failed while waiting for peer: %s",
                         doca_error_get_descr(result));
            return EXIT_FAILURE;
        }
    }

    DOCA_LOG_INFO("BlueField connected");
    DOCA_LOG_INFO("Sending export header");

    result = doca_comm_channel_ep_sendto(ep,
                                         &hdr,
                                         sizeof(hdr),
                                         DOCA_CC_MSG_FLAG_NONE,
                                         peer_addr);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to send header: %s", doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("Sending export descriptor");

    result = doca_comm_channel_ep_sendto(ep,
                                         export_desc,
                                         export_desc_len,
                                         DOCA_CC_MSG_FLAG_NONE,
                                         peer_addr);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to send export descriptor: %s",
                     doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("Export descriptor sent");
    DOCA_LOG_INFO("Host buffer address: %p", buffer);
    DOCA_LOG_INFO("Host buffer length : %lu", hdr.remote_len);
    DOCA_LOG_INFO("Keep host running while BlueField performs DMA");
    DOCA_LOG_INFO("Press Enter after BlueField finishes");

    getchar();

    doca_comm_channel_ep_disconnect(ep, peer_addr);
    doca_comm_channel_ep_destroy(ep);

    doca_mmap_destroy(mmap);
    doca_dev_rep_close(dev_rep);
    doca_dev_close(dev);
    free(buffer);

    return EXIT_SUCCESS;
}