//
// Created by hubert on 09.03.2021.
//

#ifndef UNTITLED_LIBRARY_H
#define UNTITLED_LIBRARY_H

#include <stdio.h>
#include <sys/times.h>

struct main_array {
    int number_of_blocks;
    struct block *array;
};

struct pair_of_files {
    char *a;
    char *b;
};

struct block {
    char **content;
    int number_of_lines;
};

clock_t times(struct tms *buf);


struct main_array *create_main_array(int is_static, long int size);

struct pair_of_files *create_sequence_of_pairs(int argc, char **argv);

int number_of_lines(FILE *file);

void merge_pair(struct main_array *main_array, const char *a, const char *b);

struct block *create_block(const char *file, int number_of_iterations, char **pointers_1, char **pointers_2);

void static_add_block_to_main_array(struct main_array *main_array, struct block *x);

void add_tmp_to_main_array(struct main_array *main_array, struct block *block);

int get_number_of_lines_in_block(int i);

void delete_block(struct main_array *array, long int i);

void delete_line(struct main_array *array, long int i, long int j);

void print_lines(struct block x);

void print_blocks(struct main_array *x);

#endif //UNTITLED_LIBRARY_H
