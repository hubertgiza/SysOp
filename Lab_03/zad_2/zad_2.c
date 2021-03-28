#include "library.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/times.h>
#include <zconf.h>

double calculate_time(clock_t start, clock_t end) {
    return (double) (end - start) / sysconf(_SC_CLK_TCK);
}

int main(int argc, char **argv) {
    if (argc % 2 == 0) {
        perror("Provide even number of files to merge\n");
        exit(-1);
    }
    struct main_array *main_array = create_main_array(1, 0);
    struct tms **tms_time = malloc(2 * sizeof(struct tms *));
    clock_t real_time[2];
    const int n_test = 50;
    double r_m = 0.0, u_m = 0.0, s_m = 0.0;
    for (int i = 0; i < 2; i++) {
        tms_time[i] = (struct tms *) malloc(sizeof(struct tms *));
    }
    int n = (argc - 1) / 2;

    for (int k = 0; k < n_test; k++) {
        real_time[0] = times(tms_time[0]);
        for (int i = 1; i <= n; i++) {
            if (fork() == 0) {
                merge_pair(main_array, argv[2 * i - 1], argv[2 * i]);
                exit(0);
            }
        }
        real_time[1] = times(tms_time[1]);
        while (wait(NULL) > 0);
        r_m += calculate_time(real_time[0], real_time[1]);
        u_m += calculate_time(tms_time[0]->tms_utime, tms_time[1]->tms_utime);
        s_m += calculate_time(tms_time[0]->tms_stime, tms_time[1]->tms_stime);
    }
    r_m /= n_test;
    u_m /= n_test;
    s_m /= n_test;
    printf("             Real         User        System\n");
    printf("merge      ");
    printf("%lf     %lf     %lf\n", r_m, u_m, s_m);
}

