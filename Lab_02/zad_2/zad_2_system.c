#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define LENGTH 256

void print_given_lines(int file, char c) {
    char *line = calloc(LENGTH, sizeof(char));
    char x;
    int i = 0;
    int was_c = 0;
    while (read(file, &x, 1) != 0) {
        if (x == c) {
            was_c = 1;
        }
        if (x != '\n') {
            line[i] = x;
            i++;
        } else {
            if (was_c) {
                line[i] = '\n';
                printf("%s", line);
                line = calloc(LENGTH, sizeof(char));
                i = 0;
                was_c = 0;
            } else {
                was_c = 0;
                line = calloc(LENGTH, sizeof(char));
                i = 0;
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
    int file = open(name, O_RDONLY);
    print_given_lines(file, c);
    close(file);
}


