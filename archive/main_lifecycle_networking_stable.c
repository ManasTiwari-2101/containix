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
#include <signal.h>

#define STACK_SIZE (1024 * 1024)

char container_stack[STACK_SIZE];


// =========================
// CLEANUP NETWORK
// =========================

void cleanup_network()
{
    printf("[*] Cleaning up network...\n");

    system("ip link delete veth-host 2>/dev/null");

    printf("[+] Network cleanup complete\n");
}


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
        "/home/manastiwari/containix/myruntime/rootfs";

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

    // Setup DNS
    mkdir("/etc", 0755);

    FILE *resolv = fopen("/etc/resolv.conf", "w");

    if(resolv == NULL)
    {
        perror("fopen resolv.conf");
        exit(1);
    }

    fprintf(resolv, "nameserver 8.8.8.8\n");

    fclose(resolv);
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
// SETUP NAT
// =========================

void setup_nat()
{
    printf("[*] Setting up NAT...\n");

    // Enable IP forwarding
    system(
        "sysctl -w net.ipv4.ip_forward=1 > /dev/null"
    );

    // NAT masquerading
    system(
        "iptables -t nat -C POSTROUTING "
        "-s 10.0.0.0/24 "
        "-o eth0 "
        "-j MASQUERADE "
        "2>/dev/null || "
        "iptables -t nat -A POSTROUTING "
        "-s 10.0.0.0/24 "
        "-o eth0 "
        "-j MASQUERADE"
    );

    // Forward bridge -> internet
    system(
        "iptables -C FORWARD "
        "-i br0 -o eth0 "
        "-j ACCEPT "
        "2>/dev/null || "
        "iptables -A FORWARD "
        "-i br0 -o eth0 "
        "-j ACCEPT"
    );

    // Allow established connections back
    system(
        "iptables -C FORWARD "
        "-i eth0 -o br0 "
        "-m state "
        "--state RELATED,ESTABLISHED "
        "-j ACCEPT "
        "2>/dev/null || "
        "iptables -A FORWARD "
        "-i eth0 -o br0 "
        "-m state "
        "--state RELATED,ESTABLISHED "
        "-j ACCEPT"
    );

    printf("[+] NAT configured\n");
}


// =========================
// SETUP VETH
// =========================

void setup_veth(pid_t pid)
{
    char command[512];

    // Cleanup stale interfaces
    cleanup_network();

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

    // Setup NAT
    setup_nat();

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

    int status;

    waitpid(pid, &status, 0);

    printf("Container exited\n");

    // Cleanup networking
    cleanup_network();

    printf("[+] Runtime cleanup complete\n");

    return 0;
}