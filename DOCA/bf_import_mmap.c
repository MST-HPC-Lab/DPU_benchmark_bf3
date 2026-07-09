#include <stdio.h>
#include <stdlib.h>

#include <doca_error.h>
#include <doca_log.h>
#include <doca_dev.h>
#include <doca_mmap.h>

DOCA_LOG_REGISTER(BF_IMPORT_MMAP);

#define EXPORT_FILE "mmap_export.bin"

int main(void)
{
    doca_error_t result;
    struct doca_devinfo **dev_list;
    struct doca_dev *dev = NULL;
    struct doca_mmap *remote_mmap = NULL;
    uint32_t nb_devs;
    uint32_t i;
    FILE *fp = NULL;
    void *export_desc = NULL;
    size_t export_desc_len;

    result = doca_log_backend_create_standard();
    if (result != DOCA_SUCCESS) {
        printf("Failed to create log backend\n");
        return EXIT_FAILURE;
    }

    fp = fopen(EXPORT_FILE, "rb");
    if (fp == NULL) {
        DOCA_LOG_ERR("Failed to open export file");
        return EXIT_FAILURE;
    }

    fseek(fp, 0, SEEK_END);
    export_desc_len = ftell(fp);
    rewind(fp);

    export_desc = malloc(export_desc_len);
    if (export_desc == NULL) {
        DOCA_LOG_ERR("Failed to allocate export descriptor buffer");
        fclose(fp);
        return EXIT_FAILURE;
    }

    fread(export_desc, 1, export_desc_len, fp);
    fclose(fp);

    DOCA_LOG_INFO("Read export descriptor, size = %zu bytes", export_desc_len);

    result = doca_devinfo_create_list(&dev_list, &nb_devs);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to create device list: %s", doca_error_get_descr(result));
        free(export_desc);
        return EXIT_FAILURE;
    }

    for (i = 0; i < nb_devs; i++) {
        uint8_t supported = 0;

        result = doca_mmap_cap_is_create_from_export_pci_supported(dev_list[i], &supported);
        if (result == DOCA_SUCCESS && supported) {
            result = doca_dev_open(dev_list[i], &dev);
            if (result == DOCA_SUCCESS) {
                DOCA_LOG_INFO("Opened PCI import-capable device %u", i);
                break;
            }
        }
    }

    if (dev == NULL) {
        DOCA_LOG_ERR("No PCI import-capable device found");
        doca_devinfo_destroy_list(dev_list);
        free(export_desc);
        return EXIT_FAILURE;
    }

    result = doca_mmap_create_from_export(NULL,
                                          export_desc,
                                          export_desc_len,
                                          dev,
                                          &remote_mmap);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to create mmap from export: %s",
                     doca_error_get_descr(result));
        doca_dev_close(dev);
        doca_devinfo_destroy_list(dev_list);
        free(export_desc);
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("Successfully imported host mmap on BlueField");

    doca_mmap_destroy(remote_mmap);
    doca_dev_close(dev);
    doca_devinfo_destroy_list(dev_list);
    free(export_desc);

    return EXIT_SUCCESS;
}