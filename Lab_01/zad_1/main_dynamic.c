#include "library.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/times.h>
#include <time.h>
#include <zconf.h>
#include <dlfcn.h>

#define REMOVE_BLOCK "remove_block"
#define MERGE_FILES "merge_files"
#define CREATE_TABLE "create_table"
#define REMOVE_ROW "remove_row"

double calculate_time(clock_t start, clock_t end) {
    return (double) (end - start) / sysconf(_SC_CLK_TCK);
}

int take_request(struct main_array *array, char **argv, int curr_index) {
    void *handle = dlopen("./library.so", RTLD_NOW);
    if (!handle) {
        printf("!!! %s\n", dlerror());
        exit(1);
    }
    void (*delete_block)(struct main_array *, int) = dlsym(handle, "delete_block");
    void (*delete_line)(struct main_array *, int, int) =  dlsym(handle, "delete_line");
    void (*merge_pair)(struct main_array *, char *, char *) = dlsym(handle, "merge_pair");
    struct pair_of_files *(*create_sequence_of_pairs)(int, char **) = dlsym(handle, "create_sequence_of_pairs");
    char *curr_value = argv[curr_index];
    int arguments_taken = 0;
    if (strcmp(curr_value, REMOVE_BLOCK) == 0) {
        arguments_taken++;
        long int index = strtol(argv[curr_index + arguments_taken], NULL, 10);
        arguments_taken++;
        delete_block(array, index);
        return arguments_taken;
    } else if (strcmp(curr_value, REMOVE_ROW) == 0) {
        arguments_taken++;
        long int block_n = strtol(argv[curr_index + arguments_taken], NULL, 10);
        arguments_taken++;
        long int line_n = strtol(argv[curr_index + arguments_taken], NULL, 10);
        arguments_taken++;
        delete_line(array, block_n, line_n);
        return arguments_taken;
    } else if (strcmp(curr_value, MERGE_FILES) == 0) {
        arguments_taken++;
        int number_of_files = 0;
        struct pair_of_files *sequence;
        char **files_to_merge;
        regex_t regex;
        int val;
        val = regcomp(&regex, "[.]txt$", 0);
        if (val) {
            printf("Could not compile regex\n");
            exit(1);
        }
        char *line = argv[curr_index + number_of_files + 1];
        val = regexec(&regex, line, 0, NULL, 0);
        if (val) {
            perror("no files to merge\n");
        }
        number_of_files++;
        do {
            if (argv[curr_index + number_of_files + 1] != NULL) {
                line = argv[curr_index + number_of_files + 1];
                val = regexec(&regex, line, 0, NULL, 0);
                if (!val) {
                    number_of_files++;
                } else break;
            } else break;
        } while (1);
        if (number_of_files % 2 == 1) {
            perror("Provide even number of files to merge");
            exit(1);
        }
        files_to_merge = calloc(number_of_files, sizeof(char *));
        while (arguments_taken <= number_of_files) {
            files_to_merge[arguments_taken - 1] = argv[curr_index + arguments_taken];
            arguments_taken++;
        }
        sequence = create_sequence_of_pairs(number_of_files, files_to_merge);
        for (int i = 0; i < number_of_files / 2; i++) {
            merge_pair(array, sequence[i].a, sequence[i].b);
        }
        free(files_to_merge);
        free(sequence);
        return arguments_taken;
    } else {
        perror("Expected instruction, got value, probably\n");
        exit(1);
        return 0;
    }
}

int main(int argc, char **argv) {
    struct main_array *main_array;
    struct tms **tms_time = malloc(5 * sizeof(struct tms *));
    clock_t real_time[5];
    const int n_test = 50;
    void *handle = dlopen("./library.so", RTLD_NOW);
    if (!handle) {
        printf("!!! %s\n", dlerror());
        exit(1);
    }
    struct main_array *(*create_main_array)(int, int) = dlsym(handle, "create_main_array");
    double r_c = 0.0, u_c = 0.0, s_c = 0.0, r_m = 0.0, u_m = 0.0, s_m = 0.0, r_rb = 0.0, u_rb = 0.0, s_rb = 0.0, r_rl = 0.0, u_rl = 0.0, s_rl = 0.0;

    for (int i = 0; i < 5; i++) {
        tms_time[i] = (struct tms *) malloc(sizeof(struct tms *));
    }

    for (int i = 0; i < n_test; i++) {
        int curr_index;
        real_time[0] = times(tms_time[0]);
        if (strcmp(argv[1], CREATE_TABLE) == 0) {
            main_array = create_main_array(0, strtol(argv[2], NULL, 10));
            curr_index = 3;
        } else {
            main_array = create_main_array(1, 0);
            curr_index = 1;
        }
        real_time[1] = times(tms_time[1]);
        curr_index += take_request(main_array, argv, curr_index);
        real_time[2] = times(tms_time[2]);
        curr_index += take_request(main_array, argv, curr_index);
        real_time[3] = times(tms_time[3]);
        curr_index += take_request(main_array, argv, curr_index);
        real_time[4] = times(tms_time[4]);

        r_c += calculate_time(real_time[0], real_time[1]);
        u_c += calculate_time(tms_time[0]->tms_utime, tms_time[1]->tms_utime);
        s_c += calculate_time(tms_time[0]->tms_stime, tms_time[1]->tms_stime);
        r_m += calculate_time(real_time[1], real_time[2]);
        u_m += calculate_time(tms_time[1]->tms_utime, tms_time[2]->tms_utime);
        s_m += calculate_time(tms_time[1]->tms_stime, tms_time[2]->tms_stime);
        r_rb += calculate_time(real_time[2], real_time[3]);
        u_rb += calculate_time(tms_time[2]->tms_utime, tms_time[3]->tms_utime);
        s_rb += calculate_time(tms_time[2]->tms_stime, tms_time[3]->tms_stime);
        r_rl += calculate_time(real_time[3], real_time[4]);
        u_rl += calculate_time(tms_time[3]->tms_utime, tms_time[4]->tms_utime);
        s_rl += calculate_time(tms_time[3]->tms_stime, tms_time[4]->tms_stime);
    }
    r_c /= n_test;
    u_c /= n_test;
    s_c /= n_test;
    r_m /= n_test;
    u_m /= n_test;
    s_m /= n_test;
    r_rb /= n_test;
    u_rb /= n_test;
    s_rb /= n_test;
    r_rl /= n_test;
    u_rl /= n_test;
    s_rl /= n_test;
    printf("             Real         User        System\n");
    printf("arr        ");
    printf("%lf     %lf     %lf\n", r_c, u_c, s_c);
    printf("merge      ");
    printf("%lf     %lf     %lf\n", r_m, u_m, s_m);
    printf("rm block   ");
    printf("%lf     %lf     %lf\n", r_rb, u_rb, s_rb);
    printf("rm line    ");
    printf("%lf     %lf     %lf\n", r_rl, u_rl, s_rl);
}
