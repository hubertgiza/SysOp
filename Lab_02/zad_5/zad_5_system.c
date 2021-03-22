#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define LENGTH 51

int main(int argc, char **argv) {
    if (argc != 3) {
        perror("Wrong number of arguments!\n");
        printf("Provided %d arguments, when needed number is 4!\n", argc - 1);
        exit(-1);
    }
    int a;
    if ((a = open(argv[1], O_RDONLY)) == -1) {
        perror("Error during opening the first file");
        exit(-1);
    }
    int b;
    if ((b = open(argv[2], O_RDONLY)) == -1) {
        perror("Error during opening the second file");
        exit(-1);
    }
    b = open(argv[2], O_WRONLY | O_APPEND);
    char *line = calloc(51, sizeof(char));
    char x;
    int i = 0;
    while (read(a, &x, 1) != 0) {
        if (x != '\n' && i < LENGTH - 1) {
            line[i] = x;
            i++;
        } else {
            if (i == LENGTH - 1) {
                if (x != '\n') {
                    line[i] = '\n';
                    write(b, line, LENGTH);
                    line = calloc(LENGTH, sizeof(char));
                    line[0] = x;
                    i = 1;
                }
            } else {
                line[i] = '\n';
                write(b, line, i + 1);
                i = 0;
                line = calloc(LENGTH, sizeof(char));
            }
        }
    }
    line[i] = '\n';
    write(b, line, i + 1);
    free(line);
    close(a);
    close(b);
}