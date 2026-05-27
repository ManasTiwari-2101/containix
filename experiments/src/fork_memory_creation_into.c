#include <stdio.h>
#include <unistd.h>

int main() {

    int x = 10;

    pid_t pid = fork();

    if(pid == 0) {
        x = 50;
        printf("CHILD: x = %d\n", x);
	printf("%p\n", &x);
    }
    else {
        x = 20;
        printf("PARENT: x = %d\n", x);
	printf("%p\n", &x);
    }

    return 0;
}
