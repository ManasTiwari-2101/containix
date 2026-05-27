#include <stdio.h>
#include <unistd.h>

int main() {

    printf("Program started\n");

    pid_t pid = fork();

    if(pid == 0) {
        printf("I am CHILD\n");
	printf("My PID: %d\n", getpid());
    }
    else {
        printf("I am PARENT\n");
	printf("My PID: %d\n", getpid());
    }

    return 0;
}
