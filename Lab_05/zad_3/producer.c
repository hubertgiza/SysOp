#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>

#define LINE_LENGTH 128

int main(int argc, char *argv[]) {

    if (argc != 5) {
        printf("Producer.c\n");
        printf("Wrong number of arguments!\n");
        printf("Expected 4, but was given: %d\n", argc - 1);
        exit(1);
    }
    char *pipe_path = argv[1];
    char *file_path = argv[2];
    int X = atoi(argv[3]);
    int N = atoi(argv[4]);

    FILE *read_file = fopen(file_path, "r");
    if (read_file == NULL) {
        printf("Error while opening file: %s", file_path);
        exit(1);
    }
    int pipe_file = open(pipe_path, O_WRONLY);
    if (pipe_file < 0) {
        printf("Error while opening pipe: %d", pipe_file);
        exit(1);
    }
    char buff[LINE_LENGTH];
    for (int i = 0; i < X; i++) {
        if (fgets(buff, LINE_LENGTH, read_file) == NULL) {
            printf("One of the consumers was given a too short file\n");
            exit(0);
        }
    }
    char buffer[N];

    while (fgets(buffer, N, read_file) != NULL) {
        char message[N + 20];
        sprintf(message, "PID of producer:%d, message: %s\n", getpid(), buffer);
        write(pipe_file, message, strlen(message));
        sleep(2);
    }
    close(pipe_file);
    fclose(read_file);
    return 0;
}