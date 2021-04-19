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
    raise(SIGSTOP);
    printf("Exit code %d\n", info->si_status);
}

void handle_status(int sig, siginfo_t *info, void *ucontext) {
    printf("I am handling a status\n");
    printf("Signal number %d\n", info->si_signo);
    printf("Sending PID %d\n", info->si_pid);
}

void handle_reset(int sig, siginfo_t *info, void *ucontext) {
    printf("I am handling reset\n");
    exit(42);
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
        raise(SIGINT);
    } else if (strcmp(argv[1], "child") == 0) {
        act.sa_flags = SA_NOCLDSTOP;
        act.sa_sigaction = handle_child;
        sigaction(SIGCHLD, &act, NULL);
        pid_t child_pid = fork();
        if (child_pid == 0) {
            raise(SIGCHLD);
        }
        //    wait(NULL); //odkomentowac kiedy sie chce porownywac dzialanie z flaga albo bez
    } else if (strcmp(argv[1], "reset") == 0) {
        act.sa_flags = SA_RESETHAND;
        act.sa_sigaction = handle_reset;
        sigaction(SIGCHLD, &act, NULL);
        raise(SIGCHLD);
        raise(SIGCHLD);
    }
}
