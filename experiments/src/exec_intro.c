#include <stdio.h>
#include <unistd.h>

int main() {

    printf("PID before exec: %d\n", getpid());

    execl("/bin/ls", "ls", "-l", NULL);

    return 0;
}
