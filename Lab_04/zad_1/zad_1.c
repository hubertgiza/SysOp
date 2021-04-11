#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

void signal_handle(int sigint) {
    printf("Signal received\n");
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Wrong number of arguments!\n");
        printf("Got %d, when required is 2\n", argc-1);
        return 0;
    }
    if (strcmp(argv[1], "handler") == 0) {
        signal(SIGUSR1, signal_handle);
    } else if (strcmp(argv[1], "ignore") == 0) {
        signal(SIGUSR1, SIG_IGN);
    } else if (strcmp(argv[1], "mask") == 0 || strcmp(argv[1], "pending") == 0) {
        sigset_t mask;
        sigemptyset (&mask);
        sigaddset (&mask, SIGUSR1);
        sigprocmask(SIG_BLOCK, &mask, NULL);
    }

    raise(SIGUSR1);

    sigset_t mask;

    if (strcmp(argv[1], "mask") == 0 || strcmp(argv[1], "pending") == 0) {
        sigpending(&mask);
        printf("Checking if signal is pending in parent...");
        if (sigismember(&mask, SIGUSR1)) {
            printf(" True!\n");
        } else {
            printf(" False!\n");
        }

    }

    if (strcmp(argv[2], "fork") == 0) {
        if (fork() == 0) {
            if (strcmp(argv[1], "pending") != 0) {
                raise(SIGUSR1);
            }
            if (strcmp(argv[1], "pending") == 0) {
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
                printf("Is ignored %d\n", act.sa_handler == SIG_IGN);
            } else if (strcmp(argv[1], "mask") == 0) {
                sigpending(&mask);
                printf("Checking if signal is pending...");
                if (sigismember(&mask, SIGUSR1)) {
                    printf(" True!\n");
                } else {
                    printf(" False!\n");
                }
            }
        }
    } else {
        execl("./another","./another", argv[1], NULL);
    }
    return 0;
}