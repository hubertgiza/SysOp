#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LENGTH 256

int check_word(char *line, int index, char *n1) {
    int n = strlen(n1);
    for (int i = 0; i < n; i++) {
        if ((line[index + i] == NULL) || (line[index + i] != n1[i])) {
            return 0;
        }
    }
    return 1;
}


int main(int argc, char **argv) {
    if (argc != 5) {
        perror("Wrong number of arguments!\n");
        printf("Provided %d arguments, when needed number is 4!\n", argc - 1);
        exit(-1);
    }
    FILE *input = fopen(argv[1], "r");
    FILE *output = fopen(argv[2], "w");
    char *n1 = argv[3];
    char *n2 = argv[4];
    char *line = malloc(LENGTH * sizeof(char));

    while ((line = fgets(line, LENGTH, input)) != NULL) {
        char *swapped_line = calloc(LENGTH, sizeof(char));
        int j = 0;
        for (int i = 0; i < LENGTH; i++) {
            if (line[i] == '\n') {
                swapped_line[j] = '\n';
                break;
            }
            if (line[i] == n1[0]) {
                if (check_word(line, i, n1)) {
                    i += strlen(n1) - 1;
                    for (int k = 0; k < strlen(n2); k++) {
                        swapped_line[j] = n2[k];
                        j++;
                    }
                } else {
                    swapped_line[j] = line[i];
                    j++;
                }
            } else {
                swapped_line[j] = line[i];
                j++;
            }
        }
        fputs(swapped_line, output);
        free(swapped_line);
    }
}
