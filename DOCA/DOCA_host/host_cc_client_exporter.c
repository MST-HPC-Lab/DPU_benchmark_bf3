#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include <doca_error.h>
#incl:contentReference[oaicite:0]{index=0}e <doca_dev.h>
#include <doca_mmap.h>
#include <doca_types.h>
#include <doca_comm_channel.h>

DOCA_LOG_REGISTER(HOST_CC_CLIENT);

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

static void wait_success(doca_error_t result, const char *msg)
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
    wait_success(result, "Failed to open export-capable device");

    result = doca_mmap_create(&mmap);
    wait_success(result, "doca_mmap_create failed");

    result = doca_mmap_set_memrange(mmap, buffer, BUFFER_SIZE);
    wait_success(result, "doca_mmap_set_memrange failed");

    result = doca_mmap_add_dev(mmap, dev);
    wait_success(result, "doca_mmap_add_dev failed");

    result = doca_mmap_set_permissions(mmap, DOCA_ACCESS_FLAG_PCI_READ_WRITE);
    wait_success(result, "doca_mmap_set_permissions failed");

    result = doca_mmap_start(mmap);
    wait_success(result, "doca_mmap_start failed");

    result = doca_mmap_export_pci(mmap, dev, &export_desc, &export_desc_len);
    wait_success(result, "doca_mmap_export_pci failed");

    hdr.remote_addr = (uint64_t)(uintptr_t)buffer;
    hdr.remote_len = BUFFER_SIZE;
    hdr.export_desc_len = export_desc_len;

    result = doca_comm_channel_ep_create(&ep);
    wait_success(result, "doca_comm_channel_ep_create failed");

    result = doca_comm_channel_ep_set_device(ep, dev);
    wait_success(result, "doca_comm_channel_ep_set_device failed");

    DOCA_LOG_INFO("Connecting to BlueField service: %s", SERVICE_NAME);

    while (1) {
        result = doca_comm_channel_ep_connect(ep, SERVICE_NAME, &peer_addr);
        if (result == DOCA_SUCCESS)
            break;

        if (result != DOCA_ERROR_AGAIN) {
            DOCA_LOG_ERR("Failed to connect to service: %s",
                         doca_error_get_descr(result));
            return EXIT_FAILURE;
        }

        usleep(1000);
    }

    DOCA_LOG_INFO("Connected to BlueField service");

    result = doca_comm_channel_ep_sendto(ep,
                                         &hdr,
                                         sizeof(hdr),
                                         DOCA_CC_MSG_FLAG_NONE,
                                         peer_addr);
    wait_success(result, "Failed to send export header");

    result = doca_comm_channel_ep_sendto(ep,
                                         export_desc,
                                         export_desc_len,
                                         DOCA_CC_MSG_FLAG_NONE,
                                         peer_addr);
    wait_success(result, "Failed to send export descriptor");

    DOCA_LOG_INFO("Export descriptor sent to BlueField");
    DOCA_LOG_INFO("Host buffer address: %p", buffer);
    DOCA_LOG_INFO("Host buffer length : %lu", hdr.remote_len);
    DOCA_LOG_INFO("Keep this program running while BlueField performs DMA");
    DOCA_LOG_INFO("Press Enter after BlueField finishes");

    getchar();

    doca_comm_channel_ep_disconnect(ep, peer_addr);
    doca_comm_channel_ep_destroy(ep);

    doca_mmap_destroy(mmap);
    doca_dev_close(dev);
    free(buffer);

    return EXIT_SUCCESS;
}