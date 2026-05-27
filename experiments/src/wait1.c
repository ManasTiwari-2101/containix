#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {

    pid_t pid = fork();

    if(pid == 0) {

        printf("CHILD running\n");

        sleep(3);

        printf("CHILD exiting\n");
    }
    else {

        printf("PARENT waiting\n");

        wait(NULL);

        printf("PARENT resumed\n");
    }

    return 0;
}
