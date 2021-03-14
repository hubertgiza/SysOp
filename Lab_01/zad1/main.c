//
// Created by hubert on 13.03.2021.
//

#include "library.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define REMOVE_BLOCK "remove_block"
#define MERGE_FILES "merge_files"
#define CREATE_TABLE "create_table"
#define REMOVE_ROW "remove_row"

int take_request(struct main_array *array, char **argv, int curr_index) {
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
        files_to_merge = calloc(number_of_files, sizeof(char *));
        while (arguments_taken <= number_of_files) {
            files_to_merge[arguments_taken - 1] = argv[curr_index + arguments_taken];
            arguments_taken++;
        }
        sequence = create_sequence_of_pairs(number_of_files, files_to_merge);
        for (int i = 0; i < number_of_files / 2; i++) {
            merge_pair(sequence[i].a, sequence[i].b);
            add_tmp_to_main_array(array, 1);
        }
        free(files_to_merge);
        free(sequence);
        return arguments_taken;
    } else {
        perror("Expected instruction, got value, probably\n");
        return 0;
    }
}

int main(int argc, char **argv) {
    struct main_array *main_array;
    int curr_index;
    if (strcmp(argv[1], CREATE_TABLE) == 0) {
        main_array = create_main_array(0, strtol(argv[2], NULL, 10));
        curr_index = 3;
    } else {
        main_array = create_main_array(1, 0);
        curr_index = 1;
    }
    while (curr_index < argc) {
        curr_index += take_request(main_array, argv, curr_index);
    }
    print_blocks(main_array);
}
