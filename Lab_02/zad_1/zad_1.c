#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void print_files(FILE *file_1, FILE *file_2) {
    char c_1;
    char c_2;
    while ((c_1 != EOF) && (c_2 != EOF)) {
        while ((c_1 = fgetc(file_1)) != EOF && (c_1 != '\n')) {
            printf("%c", c_1);
        }
        printf("\n");
        while ((c_2 = fgetc(file_2)) != EOF && (c_2 != '\n')) {
            printf("%c", c_2);
        }
        printf("\n");
    }
}

int main(int argc, char **argv) {
    char *name_1 = malloc(100 * sizeof(char));
    char *name_2 = malloc(100 * sizeof(char));
    FILE *file_1;
    FILE *file_2;
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
        if (((file_1 = fopen(name_1, "r")) == NULL) || ((file_2 = fopen(name_2, "r")) == NULL)) {
            perror("one of given files does not exist");
            exit(-1);
        }
    } else {
        if (((file_1 = fopen(argv[1], "r")) == NULL) || ((file_2 = fopen(argv[2], "r")) == NULL)) {
            perror("one of given files does not exist");
            exit(-1);
        }
    }

    print_files(file_1, file_2);
    fclose(file_1);
    fclose(file_2);

}