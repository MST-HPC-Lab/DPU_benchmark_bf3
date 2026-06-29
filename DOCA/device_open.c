#include <stdio.h>
#include <stdlib.h>

#include <doca_error.h>
#include <doca_log.h>
#include <doca_dev.h>

DOCA_LOG_REGISTER(DEVICE_OPEN);

int main(void)
{
    doca_error_t result;
    struct doca_devinfo **dev_list;
    struct doca_dev *dev;
    uint32_t nb_devs;

    result = doca_log_backend_create_standard();
    if (result != DOCA_SUCCESS) {
        printf("Failed to create DOCA log backend\n");
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("Starting DOCA device open program");

    result = doca_devinfo_create_list(&dev_list, &nb_devs);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to create device list: %s",
                     doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("Number of DOCA devices found: %u", nb_devs);

    if (nb_devs == 0) {
        DOCA_LOG_ERR("No DOCA devices found");
        doca_devinfo_destroy_list(dev_list);
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("Opening device 0");

    result = doca_dev_open(dev_list[0], &dev);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to open device: %s",
                     doca_error_get_descr(result));
        doca_devinfo_destroy_list(dev_list);
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("Device opened successfully");

    doca_dev_close(dev);
    DOCA_LOG_INFO("Device closed successfully");

    doca_devinfo_destroy_list(dev_list);
    DOCA_LOG_INFO("Device list destroyed");

    DOCA_LOG_INFO("Finished program");

    return EXIT_SUCCESS;
}