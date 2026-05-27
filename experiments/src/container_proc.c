#define _GNU_SOURCE

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mount.h>

#define STACK_SIZE 1024 * 1024

char child_stack[STACK_SIZE];

int child_function() {

    printf("Inside container\n");

    printf("Container PID: %d\n", getpid());

    mount("proc", "/proc", "proc", 0, NULL);

    system("ps");

    umount("/proc");

    return 0;
}

int main() {

    printf("Parent PID: %d\n", getpid());

    pid_t pid = clone(
        child_function,
        child_stack + STACK_SIZE,
        CLONE_NEWPID | CLONE_NEWNS | SIGCHLD,
        NULL
    );

    wait(NULL);

    printf("Container exited\n");

    return 0;
}
