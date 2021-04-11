#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>

void handle_child(int sig, siginfo_t *info, void *ucontext) {
    printf("Signal number %d\n", info->si_signo);
    printf("Sending PID %d\n", info->si_pid);
    // kod wyjscia procesu dziecka (sygnal SIGCHLD)
    raise(SIGSTOP);
    printf("Child exit code %d\n", info->si_status);
}

void handle_status(int sig, siginfo_t *info, void *ucontext) {
    printf("Signal number %d\n", info->si_signo);
    printf("Sending PID %d\n", info->si_pid);
    if (info->si_code == SI_USER) {
        printf("Send by user\n");
    } else {
        printf("Send by kernel\n");
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Wrong number of arguments!\n");
        printf("Got %d, when required is 1\n", argc - 1);
        return 0;
    }

    struct sigaction act;
    sigemptyset(&act.sa_mask);


    if (strcmp(argv[1], "status") == 0) {
        act.sa_flags = SA_SIGINFO;
        act.sa_sigaction = handle_status;
        sigaction(SIGINT, &act, NULL);
        pause();
    } else if (strcmp(argv[1], "child") == 0) {
        act.sa_flags = SA_NOCLDSTOP;
        act.sa_sigaction = handle_child;
        sigaction(SIGCHLD, &act, NULL);
        pid_t child_pid = fork();
        if (child_pid == 0) {
            raise(SIGCHLD);
        }
    }
    https://linux.die.net/man/2/waitpid
    wait(NULL);
}