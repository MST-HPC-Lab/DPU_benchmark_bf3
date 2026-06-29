#include <stdio.h>
#include <stdlib.h>

#include <doca_error.h>
#include <doca_log.h>
#include <doca_pe.h>

DOCA_LOG_REGISTER(PROGRESS_ENGINE);

int main(void)
{
    doca_error_t result;
    struct doca_pe *pe;

    result = doca_log_backend_create_standard();
    if (result != DOCA_SUCCESS) {
        printf("Failed to create DOCA log backend\n");
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("Starting DOCA progress engine program");

    result = doca_pe_create(&pe);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to create progress engine: %s",
                     doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("Progress engine created successfully");

    result = doca_pe_progress(pe);
    if (result < 0) {
        DOCA_LOG_ERR("Progress engine failed to progress");
        doca_pe_destroy(pe);
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("Progress engine progressed once");

    result = doca_pe_destroy(pe);
    if (result != DOCA_SUCCESS) {
        DOCA_LOG_ERR("Failed to destroy progress engine: %s",
                     doca_error_get_descr(result));
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("Progress engine destroyed successfully");

    return EXIT_SUCCESS;
}