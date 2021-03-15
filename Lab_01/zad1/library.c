//
// Created by hubert on 09.03.2021.
//

#include "library.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct block global_array[100000];
const int LINE_LENGTH = 1000;
const char *TMP = "tmp.txt";

struct main_array *create_main_array(int is_static, long int size) {

    if (!is_static && size == 0) {
        perror("You can not request dynamic array with size 0");
    }
    struct main_array *created_array = malloc(sizeof(struct main_array));
    created_array->number_of_blocks = 0;
    if (is_static) {
//        printf("Using static array\n");
        created_array->array = global_array;
    } else {
        struct block *dynamic_array = calloc(size, sizeof(struct block));
        created_array->array = dynamic_array;
//        printf("Created dynamic array with size of %ld\n", size);
    }
    return created_array;
}

struct pair_of_files *create_sequence_of_pairs(int argc, char **argv) {
    const int n = (int) (argc / 2);
    struct pair_of_files *sequence = calloc(n, sizeof(struct pair_of_files));
    for (int i = 0; i < n; i++) {
        sequence[i].a = argv[2 * i];
        sequence[i].b = argv[2 * i + 1];
    }
    return sequence;
}


int number_of_lines(FILE *file) {
    int lines = 0;
    int ch = 0;
    if (file == NULL) perror("Error while opening the file");

    lines++;
    while (!feof(file)) {
        ch = fgetc(file);
        if (ch == '\n') {
            lines++;
        }
    }
    fclose(file);
    return lines;
}

void add_tmp_to_main_array(struct main_array *main_array, struct block *block) {
    static_add_block_to_main_array(main_array, block);
}

// create block from TMP file - after merging
struct block *create_block(const char *file, int number_of_iterations, char **pointers_1, char **pointers_2) {
    FILE *tmp = fopen(file, "r");
    struct block *result = malloc(sizeof(struct block));
    char **array = calloc(number_of_iterations * 2, sizeof(char *));
    result->number_of_lines = 2 * number_of_iterations;

    for (int i = 0; i < number_of_iterations; i++) {
        array[2 * i] = calloc(strlen(pointers_1[i]) + 1, sizeof(char));
        fgets(array[2 * i], strlen(pointers_1[i]) + 1, tmp);
        array[2 * i + 1] = calloc(strlen(pointers_2[i]) + 1, sizeof(char));
        fgets(array[2 * i + 1], strlen(pointers_2[i]) + 1, tmp);
    }
    result->content = array;
    return result;
}

void merge_pair(struct main_array *main_array, const char *a, const char *b) {
    const char *FILENAME = "tmp.txt";

    FILE *file_1 = fopen(a, "r");
    FILE *file_2 = fopen(b, "r");
    FILE *tmp = fopen(FILENAME, "w");

    fseek(file_1, 0, SEEK_END);
    long fsize_1 = ftell(file_1);
    fseek(file_1, 0, SEEK_SET);

    char *content_1 = malloc(fsize_1 + 1);
    fread(content_1, 1, fsize_1, file_1);
    fclose(file_1);

    fseek(file_2, 0, SEEK_END);
    long fsize_2 = ftell(file_2);
    fseek(file_2, 0, SEEK_SET);

    char *content_2 = malloc(fsize_2 + 1);
    fread(content_2, 1, fsize_2, file_2);
    fclose(file_2);

    content_1[fsize_1] = '\0';
    content_2[fsize_2] = '\0';

    int number_of_lines_1 = 1;
    int number_of_lines_2 = 1;
    for (int i = 0; i < fsize_1; i++) {
        if (content_1[i] == '\n') {
            number_of_lines_1++;
            content_1[i] = '\0';
        }
    }
    for (int i = 0; i < fsize_2; i++) {
        if (content_2[i] == '\n') {
            number_of_lines_2++;
            content_2[i] = '\0';
        }
    }

    char **pointers_1 = calloc(number_of_lines_1, sizeof(char *));
    char **pointers_2 = calloc(number_of_lines_2, sizeof(char *));

    int i = 1;
    int j = 0;
    int step = 0;
    pointers_1[0] = content_1;
    while (i < number_of_lines_1 && j < fsize_1 + 1) {
        step = strlen(pointers_1[i - 1]);
        j += step + 1;
        pointers_1[i] = &content_1[j];
        i++;
    }
    i = 1;
    j = 0;
    step = 0;
    pointers_2[0] = content_2;
    while (i < number_of_lines_2 && j < fsize_2 + 1) {
        step = strlen(pointers_2[i - 1]);
        j += step + 1;
        pointers_2[i] = &content_2[j];
        i++;
    }

    int number_of_iterations = 0;
    if (number_of_lines_1 > number_of_lines_2) {
        number_of_iterations = number_of_lines_2;
    } else {
        number_of_iterations = number_of_lines_1;
    }
    for (i = 0; i < number_of_iterations - 1; i++) {
        char *line;
        line = pointers_1[i];
        fputs(line, tmp);
        line = pointers_2[i];
        fputs(line, tmp);
    }
    fclose(tmp);
    struct block *result = create_block(FILENAME, number_of_iterations, pointers_1, pointers_2);
    add_tmp_to_main_array(main_array, result);

    // cleaning memory
    free(pointers_1);
    free(pointers_2);
    free(content_1);
    free(content_2);
    fclose(tmp);
}


void static_add_block_to_main_array(struct main_array *main_array, struct block *x) {
    int i = 0;
    while (main_array->array[i].content != NULL) {
        i++;
        if (i >= 100000) {
            perror("Array is full!\n");
            break;
        }
    }
    main_array->array[i] = *x;
    main_array->number_of_blocks++;
}


int get_number_of_lines_in_block(int i) {
    return global_array[i].number_of_lines;
}

void delete_block(struct main_array *array, long int i) {
    for (int j = 0; j < array->array[i].number_of_lines; j++) {
        free(array->array[i].content[j]);
    }
    free(array->array[i].content);
    array->array[i].content = NULL;
    array->array[i].number_of_lines = 0;
    array->number_of_blocks--;
//    printf("Removed block: %ld\n", i);
}

void delete_line(struct main_array *array, long int i, long int j) {
    free(array->array[i].content[j]);
    array->array[i].content[j] = NULL;
    array->array[i].number_of_lines--;
//    printf("Removed line : %ld in block: %ld\n", j, i);
}

void print_lines(struct block x) {
    int i = 0;
    int j = 0;
    while (j < x.number_of_lines) {
        if (x.content[i] != NULL) {
            printf("%s\n", x.content[i]);
            i++;
            j++;
        } else {
            i++;
        }
    }
}

void print_blocks(struct main_array *x) {
    int i = 0;
    int j = 0;
    while (j < x->number_of_blocks) {
        if (x->array[i].number_of_lines != 0) {
            printf("Block: %d\n", i);
            print_lines(x->array[i]);
            i++;
            j++;
        } else {
            i++;
        }
    }
}