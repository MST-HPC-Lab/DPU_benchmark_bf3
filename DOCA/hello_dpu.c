#include <stdio.h>
#include <stdlib.h>
#include <doca_log.h>
#include <doca_error.h>

/* Define application log name */
DOCA_LOG_REGISTER(HELLO_WORLD);

int main(int argc, char **argv) {
    doca_error_t result;

    /* Register a logger to display DOCA messages */
    result = doca_log_backend_create_standard();
    if (result != DOCA_SUCCESS) {
        printf("Failed to register standard log backend\n");
        return EXIT_FAILURE;
    }

    /* Set global log level */
    doca_log_global_set_level(DOCA_LOG_LEVEL_INFO);

    DOCA_LOG_INFO("Hello, DOCA World!");

    /* Standard cleanup */
    // Free resources here...

    return EXIT_SUCCESS;
}