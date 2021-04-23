#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define X "0"
#define X_1 "2"
#define N "5"
#define N_1 "10"

int main(int argc, char *argv[]) {
    char *consumer[] = {"./consumer", "fif", "./consumer_data/output.txt", N_1, NULL};
    char *producer_1[] = {"./producer", "fif", "./producer_data/a.txt", X, N, NULL};
    char *producer_2[] = {"./producer", "fif", "./producer_data/b.txt", X, N, NULL};
    char *producer_3[] = {"./producer", "fif", "./producer_data/c.txt", X, N, NULL};
    char *producer_4[] = {"./producer", "fif", "./producer_data/d.txt", X, N, NULL};
    char *producer_5[] = {"./producer", "fif", "./producer_data/e.txt", X, N, NULL};

    mkfifo("fif", S_IRUSR | S_IWUSR);

    pid_t pid_tab[6];
    if ((pid_tab[0] = fork()) == 0)
        execvp(consumer[0], consumer);
    if ((pid_tab[1] = fork()) == 0)
        execvp(producer_2[0], producer_2);
    if ((pid_tab[2] = fork()) == 0)
        execvp(producer_3[0], producer_3);
    if ((pid_tab[3] = fork()) == 0)
        execvp(producer_1[0], producer_1);
    if ((pid_tab[4] = fork()) == 0)
        execvp(producer_4[0], producer_4);
    if ((pid_tab[5] = fork()) == 0)
        execvp(producer_5[0], producer_5);
    for (int i = 0; i < 6; i++)
        waitpid(pid_tab[i], NULL, 0);

    printf("Main is done!\n");
    printf("You can now check consumer_data/output.txt file\n");
    return 0;
}