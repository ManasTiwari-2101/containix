#include <stdio.h>
#include <seccomp.h>
#include <unistd.h>

int main() {

    scmp_filter_ctx ctx;

    ctx = seccomp_init(SCMP_ACT_ALLOW);

    seccomp_rule_add(
        ctx,
        SCMP_ACT_KILL,
        SCMP_SYS(getpid),
        0
    );

    seccomp_load(ctx);

    printf("About to call getpid()\n");

    pid_t pid = getpid();

    printf("PID: %d\n", pid);

    seccomp_release(ctx);

    return 0;
}
