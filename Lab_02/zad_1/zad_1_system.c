#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

void print_files(int file_1, int file_2) {
    char c_1;
    char c_2;
    int v_1 = 1;
    int v_2 = 1;
    while ((v_1 != 0) || (v_2 != 0)) {
        while (v_1 != 0) {
            v_1 = read(file_1, &c_1, 1);
            if (v_1 != 0) {
                printf("%c", c_1);
            } else {
                printf("\n");
            }
            if (c_1 == '\n')break;
        }
        while (v_2 != 0) {
            v_2 = read(file_2, &c_2, 1);
            if (v_2 != 0) {
                printf("%c", c_2);
            } else {
                printf("\n");
            }
            if (c_2 == '\n')break;
        }

    }
}

int main(int argc, char **argv) {
    char *name_1 = malloc(100 * sizeof(char));
    char *name_2 = malloc(100 * sizeof(char));
    int file_1;
    int file_2;
    if (argc > 3) {
        perror("provide only 2 names of files!");
        exit(-1);
    }
    if (argc < 3) {
        if (argc == 2) {
            name_1 = argv[1];
            printf("Provided only one name of file to open\n");
            printf("Please enter the name of second file here: ");
            scanf("%s", name_2);
        } else {
            printf("Provided 0 files\n");
            printf("Please provide names of the files: ");
            scanf("%s %s", name_1, name_2);
            printf("\n");
        }
        if (((file_1 = open(name_1, O_RDONLY)) == -1) || ((file_2 = open(name_2, O_RDONLY)) == -1)) {
            perror("one of given files does not exist");
            exit(-1);
        }
    } else {
        if (((file_1 = open(argv[1], O_RDONLY)) == -1) || ((file_2 = open(argv[2], O_RDONLY)) == -1)) {
            perror("one of given files does not exist");
            exit(-1);
        }
    }

    print_files(file_1, file_2);
    close(file_1);
    close(file_2);
}