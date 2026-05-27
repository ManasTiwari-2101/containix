#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {

    size_t mb = 1024 * 1024;

    int count = 0;

    while(1) {

        void *ptr = malloc(mb);

        if(ptr == NULL) {

            printf("malloc failed\n");
            break;
        }

        memset(ptr, 0, mb);

        count++;

        printf("Allocated %d MB\n", count);

        sleep(1);
    }

    return 0;
}
