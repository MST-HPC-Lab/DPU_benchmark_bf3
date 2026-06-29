#include <stdio.h>
#include <stdlib.h>

#include <doca_error.h>
#include <doca_log.h>
#include <doca_dev.h>
#include <doca_dma.h>

DOCA_LOG_REGISTER(CAPABILITY_DISCOVERY);

int main(void)
{
    doca_error_t result;
    struct doca_devinfo **dev_list;
    uint32_t nb_devs;
    uint32_t i;

    result = doca_log_backend_create_standard();
    if (result != DOCA_SUCCESS) {
        printf("Failed to create DOCA log backend\n");
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("Starting DOCA capability discovery");

    result = doca_devinfo_create_list(&dev_list, &nb_devs);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to create device list: %s",
                     doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("Number of DOCA devices found: %u", nb_devs);

    for (i = 0; i < nb_devs; i++) {
        DOCA_LOG_INFO("Checking device %u", i);

        result = doca_dma_cap_task_memcpy_is_supported(dev_list[i]);

        if (result == DOCA_SUCCESS) {
            DOCA_LOG_INFO("Device %u supports DOCA DMA memcpy", i);
        } else {
            DOCA_LOG_INFO("Device %u does NOT support DOCA DMA memcpy: %s",
                          i, doca_error_get_descr(result));
        }
    }

    doca_devinfo_destroy_list(dev_list);

    DOCA_LOG_INFO("Finished capability discovery");

    return EXIT_SUCCESS;
}