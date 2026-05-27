#define _GNU_SOURCE

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define STACK_SIZE 1024 * 1024

char child_stack[STACK_SIZE];

int child_function() {

    printf("Inside container\n");

    chroot("./rootfs");

    chdir("/");

    execl("/bin/sh", "sh", NULL);

    return 0;
}

int main() {

    pid_t pid = clone(
        child_function,
        child_stack + STACK_SIZE,
        SIGCHLD,
        NULL
    );

    wait(NULL);

    return 0;
}
