#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

int signals_received = 0;
int sender_pid = 0;
bool catching = true;

void sigusr1_handle(int sig, siginfo_t *info, void *ucontext) {
    signals_received++;
    if (!sender_pid)
        sender_pid = info->si_pid;
}

void sigusr2_handle() {
    catching = false;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Wrong number of arguments!\n");
        printf("Got %d, when required is 1\n", argc - 1);
        return 0;
    }
    char *mode = argv[1];
    printf("Catcher PID: %d\n", getpid());

    if (strcmp(mode, "kill") == 0 || strcmp(mode, "sigqueue") == 0) {
        struct sigaction act;
        act.sa_flags = SA_SIGINFO;
        act.sa_sigaction = sigusr1_handle;
        sigaction(SIGUSR1, &act, NULL);
        signal(SIGUSR2, sigusr2_handle);
    } else if (strcmp(mode, "sigrt") == 0) {
        sigset_t mask;
        sigfillset(&mask);
        sigdelset(&mask, SIGRTMIN);
        sigdelset(&mask, SIGRTMAX);
        sigprocmask(SIG_SETMASK, &mask, NULL);

        struct sigaction act;
        act.sa_flags = SA_SIGINFO;
        act.sa_sigaction = sigusr1_handle;
        sigaction(SIGRTMIN, &act, NULL);
        signal(SIGRTMAX, sigusr2_handle);
    } else {
        printf("Provided not valid mode\n");
        exit(0);
    }

    printf("Catcher is waiting in mode: %s\n", mode);
    while (catching) {
        pause();
    }

    printf("Caught signals: %d\n", signals_received);

    if (strcmp(mode, "kill") == 0) {
        printf("Now catcher is sending back\n");
        for (int i = 0; i < signals_received; i++) {
            kill(sender_pid, SIGUSR1);
        }
        kill(sender_pid, SIGUSR2);
    } else if (strcmp(mode, "sigqueue") == 0) {
        union sigval value;
        for (int i = 0; i < signals_received; i++) {
            value.sival_int = i;
            sigqueue(sender_pid, SIGUSR1, value);
        }
        value.sival_int = signals_received;
        sigqueue(sender_pid, SIGUSR2, value);
    } else if (strcmp(mode, "sigrt") == 0) {
        for (int i = 0; i < signals_received; i++) {
            kill(sender_pid, SIGRTMIN);
        }
        kill(sender_pid, SIGRTMAX);
    }

    printf("Exiting catcher\n");
}