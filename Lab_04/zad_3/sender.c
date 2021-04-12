#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>


int received_back = 0;
int last_index;
bool catching = true;

void sigusr1_handle() {
    received_back++;
}

void sigusr2_handle() {
    catching = false;
}

void sigrtmin_handle() {
    received_back++;
}

void sigrtmax_handle() {
    catching = false;
}

void sigusr2_handle_sigaction(int sig, siginfo_t *info, void *ucontext) {
    catching = false;
    last_index = info->si_value.sival_int;
}


int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Wrong number of arguments!\n");
        printf("Got %d, when required is 3\n", argc - 1);
        return 0;
    }

    long int catcher_pid = strtol(argv[1], NULL, 10);
    long int signals_to_send = strtol(argv[2], NULL, 10);
    char *mode = argv[3];

    if (strcmp(mode, "kill") == 0) {
        printf("Sender is in kill mode\n");
        signal(SIGUSR1, sigusr1_handle);
        signal(SIGUSR2, sigusr2_handle);
        for (int i = 0; i < signals_to_send; i++) {
            kill((int) catcher_pid, SIGUSR1);
        }
        printf("Sender is done!\n");
        kill((int) catcher_pid, SIGUSR2);

    } else if (strcmp(mode, "sigqueue") == 0) {
        printf("Sender is in sigqueue mode\n");
        signal(SIGUSR1, sigusr1_handle);
        struct sigaction act;
        act.sa_flags = SA_SIGINFO;
        act.sa_sigaction = sigusr2_handle_sigaction;
        sigaction(SIGUSR2, &act, NULL);

        union sigval value;
        for (int i = 0; i < signals_to_send; i++) {
            value.sival_int = i;
            sigqueue((int) catcher_pid, SIGUSR1, value);
        }
        value.sival_int = (int) signals_to_send;
        sigqueue((int) catcher_pid, SIGUSR2, value);
        printf("Sender is done!\n");

    } else if (strcmp(mode, "sigrt") == 0) {
        printf("Sender is in sigrt mode\n");
        for (int i = 0; i < signals_to_send; i++) {
            kill((int) catcher_pid, SIGRTMIN);
        }
        kill((int) catcher_pid, SIGRTMAX);
        signal(SIGRTMIN, sigrtmin_handle);
        signal(SIGRTMAX, sigrtmax_handle);
        printf("Sender is done!\n");
    } else {
        printf("Provided not valid mode\n");
        exit(0);
    }
    catching = true;
    while (catching) {
        pause();
    }
    if (strcmp(mode, "sigqueue") == 0) {
        printf("Received %d signals, when expected %d\n", received_back, (int) signals_to_send);
        printf("Catcher caught actually %d signals\n", last_index);
    } else {
        printf("Received %d signals, when expected %d\n", received_back, (int) signals_to_send);
    }
    printf("Exiting sender\n");
}