#include <stdio.h>
#include <stdlib.h>

#include <doca_error.h>
#include <doca_log.h>
#include <doca_dev.h>
#include <doca_pe.h>
#include <doca_ctx.h>
#include <doca_dma.h>

DOCA_LOG_REGISTER(DMA_CONTEXT_LIFECYCLE);

int main(void)
{
    doca_error_t result;
    struct doca_devinfo **dev_list;
    struct doca_dev *dev = NULL;
    struct doca_pe *pe = NULL;
    struct doca_dma *dma = NULL;
    struct doca_ctx *ctx = NULL;
    uint32_t nb_devs;
    uint32_t i;

    result = doca_log_backend_create_standard();
    if (result != DOCA_SUCCESS) {
        printf("Failed to create DOCA log backend\n");
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("Starting DMA context lifecycle program");

    result = doca_devinfo_create_list(&dev_list, &nb_devs);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to create device list: %s",
                     doca_error_get_descr(result));
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
        return EXIT_FAILURE;
    }

    result = doca_pe_create(&pe);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to create progress engine: %s",
                     doca_error_get_descr(result));
        doca_dev_close(dev);
        doca_devinfo_destroy_list(dev_list);
        return EXIT_FAILURE;
    }

    result = doca_dma_create(dev, &dma);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to create DMA object: %s",
                     doca_error_get_descr(result));
        doca_pe_destroy(pe);
        doca_dev_close(dev);
        doca_devinfo_destroy_list(dev_list);
        return EXIT_FAILURE;
    }

    ctx = doca_dma_as_ctx(dma);
    if (ctx == NULL) {
        DOCA_LOG_ERR("Failed to convert DMA object to DOCA context");
        doca_dma_destroy(dma);
        doca_pe_destroy(pe);
        doca_dev_close(dev);
        doca_devinfo_destroy_list(dev_list);
        return EXIT_FAILURE;
    }

    result = doca_pe_connect_ctx(pe, ctx);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to connect context to PE: %s",
                     doca_error_get_descr(result));
        doca_dma_destroy(dma);
        doca_pe_destroy(pe);
        doca_dev_close(dev);
        doca_devinfo_destroy_list(dev_list);
        return EXIT_FAILURE;
    }

    result = doca_ctx_start(ctx);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to start context: %s",
                     doca_error_get_descr(result));
        doca_dma_destroy(dma);
        doca_pe_destroy(pe);
        doca_dev_close(dev);
        doca_devinfo_destroy_list(dev_list);
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("DMA context started successfully");

    result = doca_ctx_stop(ctx);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to stop context: %s",
                     doca_error_get_descr(result));
    } else {
        DOCA_LOG_INFO("DMA context stopped successfully");
    }

    doca_dma_destroy(dma);
    doca_pe_destroy(pe);
    doca_dev_close(dev);
    doca_devinfo_destroy_list(dev_list);

    DOCA_LOG_INFO("Finished DMA context lifecycle program");

    return EXIT_SUCCESS;
}