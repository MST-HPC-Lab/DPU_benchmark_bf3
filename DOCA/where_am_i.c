#include <stdio.h>
#include <unistd.h>
#include <sys/utsname.h>

int main(void)
{
    struct utsname u;
    uname(&u);

    printf("Hostname: %s\n", u.nodename);
    printf("Machine : %s\n", u.machine);

    return 0;
}