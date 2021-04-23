#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Consumer.c\n");
        printf("Wrong number of arguments!\n");
        printf("Expected 3, but was given: %d\n", argc - 1);
        exit(1);
    }
    char *pipe_path = argv[1];
    char *file_path = argv[2];
    int N = atoi(argv[3]);
    FILE *write_file = fopen(file_path, "w");
    if (write_file == NULL) {
        printf("Error while opening file: %s", file_path);
        exit(1);
    }
    FILE *pipe_file = fopen(pipe_path, "r");
    if (pipe_file == NULL) {
        printf("Error while opening pipe: %s", pipe_path);
        exit(1);
    }
    char buffer[N];
    while (fgets(buffer, N, pipe_file) != NULL) {
        fprintf(write_file, buffer, strlen(buffer));
    }
    fclose(pipe_file);
    fclose(write_file);
    return 0;
}
