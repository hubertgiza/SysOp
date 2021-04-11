#include <stdio.h>
#include <signal.h>
#include <string.h>

int main(int argc, char **argv) {
    printf("\nanother.c\n");
    if (strcmp(argv[1], "pending") != 0) {
        raise(SIGUSR1);
    }
    if (strcmp(argv[1], "pending") == 0) {
        sigset_t mask;
        sigpending(&mask);
        printf("Checking if signal is pending...");
        if (sigismember(&mask, SIGUSR1)) {
            printf(" True!\n");
        } else {
            printf(" False!\n");
        }
    } else if (strcmp(argv[1], "ignore") == 0) {
        struct sigaction act;
        sigaction(SIGUSR1, NULL, &act);
        printf("Checking if signal is being ignored...");
        if (act.sa_handler == SIG_IGN) {
            printf(" True\n");
        } else {
            printf(" False\n");
        }
    } else if (strcmp(argv[1], "mask") == 0) {
        sigset_t mask;
        sigpending(&mask);
        printf("Checking if signal is pending...");
        if (sigismember(&mask, SIGUSR1)) {
            printf(" True!\n");
        } else {
            printf(" False!\n");
        }
    }
    return 0;
}
