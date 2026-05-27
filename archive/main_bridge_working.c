#define _GNU_SOURCE

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <sys/capability.h>
#include <fcntl.h>
#include <string.h>

#define STACK_SIZE (1024 * 1024)

char container_stack[STACK_SIZE];


// =========================
// DROP CAPABILITIES
// =========================

void drop_capabilities()
{
    cap_t caps;

    caps = cap_init();

    if(cap_set_proc(caps) == -1)
    {
        perror("cap_set_proc");
        exit(1);
    }

    cap_free(caps);
}


// =========================
// SETUP ROOT FILESYSTEM
// =========================

void setup_rootfs()
{
    char rootfs_path[] =
        "/home/manastiwari/container_in_c/myruntime/rootfs";

    // Prevent mount propagation to host
    if(mount(
        NULL,
        "/",
        NULL,
        MS_REC | MS_PRIVATE,
        NULL
    ) == -1)
    {
        perror("mount private");
        exit(1);
    }

    // Bind mount rootfs onto itself
    if(mount(
        rootfs_path,
        rootfs_path,
        NULL,
        MS_BIND | MS_REC,
        NULL
    ) == -1)
    {
        perror("bind mount");
        exit(1);
    }

    // Create oldroot
    char oldroot[512];

    snprintf(
        oldroot,
        sizeof(oldroot),
        "%s/oldroot",
        rootfs_path
    );

    mkdir(oldroot, 0777);

    // pivot_root
    if(syscall(
        SYS_pivot_root,
        rootfs_path,
        oldroot
    ) == -1)
    {
        perror("pivot_root");
        exit(1);
    }

    // Move into new root
    if(chdir("/") == -1)
    {
        perror("chdir");
        exit(1);
    }

    // Unmount old root
    if(umount2("/oldroot", MNT_DETACH) == -1)
    {
        perror("umount oldroot");
        exit(1);
    }

    rmdir("/oldroot");
}


// =========================
// MOUNT /proc
// =========================

void setup_proc()
{
    mkdir("/proc", 0555);

    if(mount(
        "proc",
        "/proc",
        "proc",
        0,
        NULL
    ) == -1)
    {
        perror("mount proc");
        exit(1);
    }
}


// =========================
// SETUP BRIDGE
// =========================

void setup_bridge()
{
    printf("[*] Setting up bridge...\n");

    system(
        "ip link show br0 > /dev/null 2>&1 || "
        "ip link add br0 type bridge"
    );

    system(
        "ip addr show br0 | grep 10.0.0.1 > /dev/null 2>&1 || "
        "ip addr add 10.0.0.1/24 dev br0"
    );

    system("ip link set br0 up");

    printf("[+] Bridge ready\n");
}


// =========================
// SETUP VETH
// =========================

void setup_veth(pid_t pid)
{
    char command[512];

    printf("[*] Creating veth pair...\n");

    system(
        "ip link add veth-host "
        "type veth peer name veth-cont"
    );

    printf("[*] Moving veth-cont into container namespace...\n");

    snprintf(
        command,
        sizeof(command),
        "ip link set veth-cont netns %d",
        pid
    );

    system(command);

    printf("[*] Attaching veth-host to bridge...\n");

    system("ip link set veth-host master br0");

    system("ip link set veth-host up");

    printf("[+] Veth setup complete\n");
}


// =========================
// CONTAINER PROCESS
// =========================

int container_main(void *arg)
{
    printf("Inside container\n");

    // Hostname isolation
    if(sethostname("myruntime", 9) == -1)
    {
        perror("sethostname");
        exit(1);
    }

    // Filesystem isolation
    setup_rootfs();

    // Mount proc
    setup_proc();

    // Wait for parent networking setup
    sleep(1);

    // Bring loopback UP
    system("/bin/ip link set lo up");

    // Configure container veth
    system(
        "/bin/ip addr add "
        "10.0.0.2/24 "
        "dev veth-cont"
    );

    system("/bin/ip link set veth-cont up");

    // Add default route
    system(
        "/bin/ip route add "
        "default via 10.0.0.1"
    );

    // Prevent privilege escalation
    if(prctl(
        PR_SET_NO_NEW_PRIVS,
        1,
        0,
        0,
        0
    ) == -1)
    {
        perror("prctl");
        exit(1);
    }

    // Drop all capabilities
    drop_capabilities();

    // Minimal PATH
    setenv("PATH", "/bin", 1);

    // Launch shell
    execl("/bin/sh", "sh", NULL);

    perror("execl");

    return 1;
}


// =========================
// MAIN
// =========================

int main()
{
    printf("Starting myruntime...\n");

    // Setup bridge
    setup_bridge();

    int flags =
        CLONE_NEWUTS  |
        CLONE_NEWPID  |
        CLONE_NEWNS   |
        CLONE_NEWNET;

    pid_t pid = clone(
        container_main,
        container_stack + STACK_SIZE,
        flags | SIGCHLD,
        NULL
    );

    if(pid == -1)
    {
        perror("clone");
        return 1;
    }

    // Parent sets up networking
    setup_veth(pid);

    waitpid(pid, NULL, 0);

    printf("Container exited\n");

    return 0;
}
