#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LENGTH 51

int main(int argc, char **argv) {
    if (argc != 3) {
        perror("Wrong number of arguments!\n");
        printf("Provided %d arguments, when needed number is 4!\n", argc - 1);
        exit(-1);
    }
    FILE *a;
    if ((a = fopen(argv[1], "r")) == NULL) {
        perror("Error during opening the first file");
        exit(-1);
    }
    FILE *b;
    if ((b = fopen(argv[2], "r")) == NULL) {
        perror("Error during opening the second file");
        exit(-1);
    }
    b = fopen(argv[2], "a+");
    char* line = calloc(51, sizeof(char));
    while (fgets(line, LENGTH, a) != NULL) {
        line[50] = '\n';
        fputs(line, b);
    }
    free(line);
    fclose(a);
    fclose(b);
}