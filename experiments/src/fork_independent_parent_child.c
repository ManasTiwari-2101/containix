#include <stdio.h>
#include <unistd.h>

int main() {

    pid_t pid = fork();

    for(int i = 0; i < 5; i++) {

        if(pid == 0) {
            printf("CHILD %d\n", i);
        }
        else {
            printf("PARENT %d\n", i);
        }

        sleep(1);
    }

    return 0;
}
