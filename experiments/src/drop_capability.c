#include <stdio.h>
#include <stdlib.h>
#include <sys/capability.h>
#include <sys/mount.h>

int main() {

    cap_t caps;

    // Get current process capabilities
    caps = cap_get_proc();

    // Capability we want to remove
    cap_value_t cap_list[1] = {CAP_SYS_ADMIN};

    // Remove from EFFECTIVE set
    cap_set_flag(
        caps,
        CAP_EFFECTIVE,
        1,
        cap_list,
        CAP_CLEAR
    );

    // Remove from PERMITTED set
    cap_set_flag(
        caps,
        CAP_PERMITTED,
        1,
        cap_list,
        CAP_CLEAR
    );

    // Apply modified capabilities to process
    cap_set_proc(caps);

    // Free capability object
    cap_free(caps);

    printf("Dropped CAP_SYS_ADMIN\n");

    // Try privileged mount operation
    int result = mount(
        "none",
        "/tmp",
        "tmpfs",
        0,
        ""
    );

    if(result == 0)
        printf("Mount successful\n");
    else
        perror("Mount failed");

    return 0;
}
