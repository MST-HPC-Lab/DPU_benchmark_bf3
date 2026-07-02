#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <doca_error.h>
#include <doca_log.h>
#include <doca_dev.h>
#include <doca_mmap.h>
#include <doca_types.h>

DOCA_LOG_REGISTER(HOST_EXPORT_MMAP);

#define BUFFER_SIZE 1024
#define EXPORT_FILE "mmap_export.bin"

int main(void)
{
    doca_error_t result;
    struct doca_devinfo **dev_list;
    struct doca_dev *dev = NULL;
    struct doca_mmap *mmap = NULL;
    uint32_t nb_devs;
    uint32_t i;
    char *buffer = NULL;
    const void *export_desc = NULL;
    size_t export_desc_len = 0;
    FILE *fp = NULL;

    result = doca_log_backend_create_standard();
    if (result != DOCA_SUCCESS) {
        printf("Failed to create log backend\n");
        return EXIT_FAILURE;
    }

    buffer = malloc(BUFFER_SIZE);
    if (buffer == NULL) {
        DOCA_LOG_ERR("Failed to allocate buffer");
        return EXIT_FAILURE;
    }

    memset(buffer, 'H', BUFFER_SIZE);

    result = doca_devinfo_create_list(&dev_list, &nb_devs);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to create device list: %s", doca_error_get_descr(result));
        free(buffer);
        return EXIT_FAILURE;
    }

    for (i = 0; i < nb_devs; i++) {
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
        doca_devinfo_destroy_list(dev_list);
        free(buffer);
        return EXIT_FAILURE;
    }

    result = doca_mmap_create(&mmap);
    if (result != DOCA_SUCCESS)
        goto cleanup;

    result = doca_mmap_set_memrange(mmap, buffer, BUFFER_SIZE);
    if (result != DOCA_SUCCESS)
        goto cleanup;

    result = doca_mmap_add_dev(mmap, dev);
    if (result != DOCA_SUCCESS)
        goto cleanup;

    result = doca_mmap_set_permissions(mmap, DOCA_ACCESS_FLAG_PCI_READ_WRITE);
    if (result != DOCA_SUCCESS)
        goto cleanup;

    result = doca_mmap_start(mmap);
    if (result != DOCA_SUCCESS)
        goto cleanup;

    result = doca_mmap_export_pci(mmap, dev, &export_desc, &export_desc_len);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to export mmap: %s", doca_error_get_descr(result));
        goto cleanup;
    }

    fp = fopen(EXPORT_FILE, "wb");
    if (fp == NULL) {
        DOCA_LOG_ERR("Failed to open export file");
        goto cleanup;
    }

    fwrite(export_desc, 1, export_desc_len, fp);
    fclose(fp);
    fp = NULL;


    DOCA_LOG_INFO("Exported mmap descriptor to %s, size = %zu bytes",
              EXPORT_FILE, export_desc_len);

    DOCA_LOG_INFO("Keep this host program running.");
    DOCA_LOG_INFO("Now copy mmap_export.bin to BlueField and run bf_import_mmap.");
    DOCA_LOG_INFO("Press Enter here only after BlueField import is done.");

    getchar();

cleanup:
    if (fp != NULL)
        fclose(fp);

    if (mmap != NULL)
        doca_mmap_destroy(mmap);

    if (dev != NULL)
        doca_dev_close(dev);

    doca_devinfo_destroy_list(dev_list);
    free(buffer);

    return EXIT_SUCCESS;
}