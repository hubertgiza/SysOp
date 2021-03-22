#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LENGTH 256

void print_given_lines(FILE *file, char c) {
    char *line = calloc(LENGTH, sizeof(char));
    while (fgets(line, LENGTH, file) != NULL) {
        for (int i = 0; i < LENGTH; i++) {
            if (line[i] == c) {
                printf("%s", line);
                break;
            }
        }
    }
    free(line);
}

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        perror("Provide acceptable number of arguments - 2");
        exit(-1);
    }
    char c = *argv[1];
    char *name = argv[2];
    FILE *file = fopen(name, "r");
    print_given_lines(file, c);
    fclose(file);
}

