#define _GNU_SOURCE

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define STACK_SIZE 1024 * 1024

char container_stack[STACK_SIZE];

int container_main(void *arg) {

    printf("Container PID: %d\n", getpid());

    sethostname("mycontainer", 11);

    system("hostname");

    return 0;
}

int main() {

    printf("Parent PID: %d\n", getpid());

    pid_t pid = clone(
        container_main,
        container_stack + STACK_SIZE,
        CLONE_NEWUTS | SIGCHLD,
        NULL
    );

    waitpid(pid, NULL, 0);

    printf("Container exited\n");

    return 0;
}
