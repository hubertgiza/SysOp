#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char **argv) {
    if (argc != 2) {
        perror("Wrong number of arguments!\n");
        printf("Provided %d arguments, when required is: 1\n", argc - 1);
        exit(-1);
    }
    long int n = strtol(argv[1], NULL, 10);

    for (int i = 0; i < n; i++) {
        if (fork() == 0) {
            printf("Child PID: %d, parent PID: %d\n", (int) getpid(), (int) getppid());
            exit(0);
        } else {
            wait(NULL);
        }
    }
    return 0;
}