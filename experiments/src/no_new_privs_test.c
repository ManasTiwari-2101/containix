#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>

int main() {

    int result;

    result = prctl(
        PR_SET_NO_NEW_PRIVS,
        1,
        0,
        0,
        0
    );

    if(result == 0)
        printf("no_new_privs enabled\n");
    else
        perror("prctl failed");

    execl("/bin/sh", "sh", NULL);

    return 0;
}
