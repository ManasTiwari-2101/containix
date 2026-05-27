#include <stdio.h>
#include <unistd.h>

int main() {

    int count = 0;

    while(1) {

        pid_t pid = fork();

        if(pid < 0) {

            printf("Fork failed at count = %d\n", count);
            break;
        }

        if(pid == 0) {

            while(1)
                sleep(1);
        }

        count++;
    }

    return 0;
}
