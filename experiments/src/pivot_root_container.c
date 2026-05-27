#define _GNU_SOURCE
#include <sys/stat.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/syscall.h>

#define STACK_SIZE 1024 * 1024

char child_stack[STACK_SIZE];

int child_function() {

    printf("Inside container\n");

    mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL);

    mount("rootfs", "rootfs", NULL, MS_BIND | MS_REC, NULL);

    mkdir("rootfs/oldrootfs", 0777);

    syscall(SYS_pivot_root, "rootfs", "rootfs/oldrootfs");

    chdir("/");

    umount2("/oldrootfs", MNT_DETACH);

    rmdir("/oldrootfs");

    execl("/bin/sh", "sh", NULL);

    return 0;
}

int main() {

    pid_t pid = clone(
        child_function,
        child_stack + STACK_SIZE,
        CLONE_NEWNS | CLONE_NEWPID | SIGCHLD,
        NULL
    );

    wait(NULL);

    return 0;
}
