#include <stdio.h>
#include <sys/mount.h>

int main() {

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
