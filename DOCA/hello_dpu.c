#include <stdio.h>
#include <stdlib.h>
#include <doca_log.h>
#include <doca_error.h>

DOCA_LOG_REGISTER(HELLO_WORLD);

int main(int argc, char **argv)
{
    doca_error_t result;

    result = doca_log_backend_create_standard();
    if (result != DOCA_SUCCESS) {
        printf("Failed to register standard log backend\n");
        return EXIT_FAILURE;
    }

    DOCA_LOG_INFO("Hello, DOCA World!");

    return EXIT_SUCCESS;
}