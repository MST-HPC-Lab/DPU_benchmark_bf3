#include <stdio.h>
#include <unistd.h>
#include <sys/utsname.h>

int main(void) {
    struct utsname info;

    printf("Hello from BlueField DPU Arm cores!\n");

    if (uname(&info) == 0) {
        printf("System:  %s\n", info.sysname);
        printf("Node:    %s\n", info.nodename);
        printf("Release: %s\n", info.release);
        printf("Machine: %s\n", info.machine);
    }

    printf("PID: %d\n", getpid());

    return 0;
}
