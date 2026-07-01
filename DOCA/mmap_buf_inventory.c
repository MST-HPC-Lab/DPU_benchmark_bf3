#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <doca_error.h>
#include <doca_log.h>
#include <doca_dev.h>
#include <doca_mmap.h>
#include <doca_buf.h>
#include <doca_dma.h>

DOCA_LOG_REGISTER(MMAP_BUF_INVENTORY);

#define BUFFER_SIZE 1024
#define BUF_INVENTORY_SIZE 8

int main(void)
{
    doca_error_t result;
    struct doca_devinfo **dev_list;
    struct doca_dev *dev = NULL;
    struct doca_mmap *mmap = NULL;
    struct doca_buf_inventory *buf_inv = NULL;
    struct doca_buf *buf = NULL;
    char *data_buffer = NULL;
    uint32_t nb_devs;
    uint32_t i;

    result = doca_log_backend_create_standard();
    if (result != DOCA_SUCCESS) {
        printf("Failed to create DOCA log backend\n");
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("Starting mmap and buffer inventory program");

    data_buffer = malloc(BUFFER_SIZE);
    if (data_buffer == NULL) {
        DOCA_LOG_ERR("Failed to allocate buffer");
        return EXIT_FAILURE;
    }

    memset(data_buffer, 'A', BUFFER_SIZE);

    result = doca_devinfo_create_list(&dev_list, &nb_devs);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to create device list: %s",
                     doca_error_get_descr(result));
        free(data_buffer);
        return EXIT_FAILURE;
    }

    for (i = 0; i < nb_devs; i++) {
        result = doca_dma_cap_task_memcpy_is_supported(dev_list[i]);
        if (result == DOCA_SUCCESS) {
            result = doca_dev_open(dev_list[i], &dev);
            if (result == DOCA_SUCCESS) {
                DOCA_LOG_INFO("Opened DMA-capable device %u", i);
                break;
            }
        }
    }

    if (dev == NULL) {
        DOCA_LOG_ERR("No DMA-capable device could be opened");
        doca_devinfo_destroy_list(dev_list);
        free(data_buffer);
        return EXIT_FAILURE;
    }

    result = doca_mmap_create(&mmap);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to create mmap: %s",
                     doca_error_get_descr(result));
        doca_dev_close(dev);
        doca_devinfo_destroy_list(dev_list);
        free(data_buffer);
        return EXIT_FAILURE;
    }

    result = doca_mmap_set_memrange(mmap, data_buffer, BUFFER_SIZE);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to set mmap memory range: %s",
                     doca_error_get_descr(result));
        doca_mmap_destroy(mmap);
        doca_dev_close(dev);
        doca_devinfo_destroy_list(dev_list);
        free(data_buffer);
        return EXIT_FAILURE;
    }

    result = doca_mmap_add_dev(mmap, dev);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to add device to mmap: %s",
                     doca_error_get_descr(result));
        doca_mmap_destroy(mmap);
        doca_dev_close(dev);
        doca_devinfo_destroy_list(dev_list);
        free(data_buffer);
        return EXIT_FAILURE;
    }

    result = doca_mmap_start(mmap);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to start mmap: %s",
                     doca_error_get_descr(result));
        doca_mmap_destroy(mmap);
        doca_dev_close(dev);
        doca_devinfo_destroy_list(dev_list);
        free(data_buffer);
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("Memory map started successfully");

    result = doca_buf_inventory_create(BUF_INVENTORY_SIZE, &buf_inv);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to create buffer inventory: %s",
                     doca_error_get_descr(result));
        doca_mmap_destroy(mmap);
        doca_dev_close(dev);
        doca_devinfo_destroy_list(dev_list);
        free(data_buffer);
        return EXIT_FAILURE;
    }

    result = doca_buf_inventory_start(buf_inv);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to start buffer inventory: %s",
                     doca_error_get_descr(result));
        doca_buf_inventory_destroy(buf_inv);
        doca_mmap_destroy(mmap);
        doca_dev_close(dev);
        doca_devinfo_destroy_list(dev_list);
        free(data_buffer);
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("Buffer inventory started successfully");

    result = doca_buf_inventory_buf_get_by_addr(buf_inv,
                                                mmap,
                                                data_buffer,
                                                BUFFER_SIZE,
                                                &buf);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to create DOCA buffer: %s",
                     doca_error_get_descr(result));
        doca_buf_inventory_destroy(buf_inv);
        doca_mmap_destroy(mmap);
        doca_dev_close(dev);
        doca_devinfo_destroy_list(dev_list);
        free(data_buffer);
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("DOCA buffer created successfully");

    doca_buf_dec_refcount(buf, NULL);
    doca_buf_inventory_destroy(buf_inv);
    doca_mmap_destroy(mmap);
    doca_dev_close(dev);
    doca_devinfo_destroy_list(dev_list);
    free(data_buffer);

    DOCA_LOG_INFO("Finished mmap and buffer inventory program");

    return EXIT_SUCCESS;
}