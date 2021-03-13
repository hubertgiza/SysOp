//
// Created by hubert on 09.03.2021.
//

#include "library.h"
#include <stdio.h>
#include <stdlib.h>


struct block global_array[100000];
const int LINE_LENGTH = 100;
const char *TMP = "tmp.txt";

struct main_array *create_main_array(int is_static, long int size) {

    if (!is_static && size == 0) {
        perror("You can not request dynamic array with size 0");
    }
    struct main_array *created_array = malloc(sizeof(struct main_array));
    created_array->number_of_blocks = 0;
    if (is_static) {
        printf("Using static array\n");
        created_array->array = global_array;
    } else {
        free(global_array);
        struct block *dynamic_array = calloc(size, sizeof(struct block));
        created_array->array = dynamic_array;
        printf("Created dynamic array with size of %ld", size);
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

void merge_pair(const char *a, const char *b) {
    const char *FILENAME = "tmp.txt";


    FILE *file_1 = fopen(a, "r");
    FILE *file_2 = fopen(b, "r");
    FILE *tmp = fopen(FILENAME, "w");

    char line_1[LINE_LENGTH];
    char line_2[LINE_LENGTH];

    while ((fgets(line_1, LINE_LENGTH, file_1) != NULL) && (fgets(line_2, LINE_LENGTH, file_2) != NULL)) {
        fputs(line_1, tmp);
        fputs(line_2, tmp);
    }

    fclose(file_1);
    fclose(file_2);
    fclose(tmp);
}

// create block from TMP file - after merging
struct block *create_block(const char *file) {
    struct block *result = malloc(sizeof(struct block));
    int len = number_of_lines(fopen(file, "r"));
    result->number_of_lines = len - 1;

    FILE *tmp = fopen(file, "r");
    char **block = calloc(len, sizeof(char *));
    int i = 0;
    char *line = calloc(LINE_LENGTH, sizeof(char));

    if (fgets(line, LINE_LENGTH, tmp) == NULL) {
        perror("bad file is given");
    } else {
        do {
            block[i] = line;
            line = calloc(LINE_LENGTH, sizeof(char));
            i++;
        } while (fgets(line, LINE_LENGTH, tmp) != NULL);
    }
    fclose(tmp);
    free(line);
    result->content = block;
    return result;
}

int static_add_block_to_main_array(struct main_array *main_array, struct block *x) {
    int i = 0;
    while (main_array->array[i].content != NULL) {
        i++;
    }
    main_array->array[i] = *x;
    main_array->number_of_blocks++;
    return i;
}

int add_tmp_to_main_array(struct main_array *main_array, int is_static) {
    struct block *block = create_block(TMP);
    if (is_static) {
        int index = static_add_block_to_main_array(main_array, block);
        return index;
    } else {//TODO dynamicznie
        return 0;
    }
}

int get_number_of_lines_in_block(int i) {
    return global_array[i].number_of_lines;
}

void delete_block(struct main_array *array, long int i) {
    for (int j = 0; j < array->array[i].number_of_lines; j++) {
        free(array->array[i].content[j]);
    }
    free(array->array[i].content);
    array->array[i].number_of_lines = 0;
    array->number_of_blocks--;
    printf("Removed block: %ld\n", i);
}

void delete_line(struct main_array *array, long int i, long int j) {
    free(array->array[i].content[j]);
    array->array[i].number_of_lines--;
    printf("Removed line : %ld in block: %ld\n", j, i);
}

void print_lines(struct block x) {
    int i = 0;
    int j = 0;
    while (j < x.number_of_lines) {
        if (x.content[i] != NULL) {
            printf("%s", x.content[i]);
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

//int main(int argc, char **argv) {
//    struct main_array *array = create_main_array(1, 0);
//    struct pair_of_files *files_to_merge = create_sequence_of_pairs(argc - 1, argv);
//    for (int i = 0; i < (argc - 1) / 2; i++) {
//        merge_pair(files_to_merge[i].a, files_to_merge[i].b);
//        int j = add_tmp_to_main_array(array, 1);
//    }
//    delete_block(array, 1);
//    print_blocks(array);
//}