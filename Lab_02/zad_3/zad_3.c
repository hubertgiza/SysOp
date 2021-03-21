#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define LENGTH 256

int is_square(int number) {
    int int_val;
    float float_val;
    float_val = sqrt((double) number);
    int_val = float_val;
    if (int_val == float_val) {
        return 1;
    } else {
        return 0;
    }
}

int number_of_digits(int number) {
    int digits = 0;
    while (number != 0) {
        number /= 10;
        digits++;
    }
    return digits;
}

int main() {
    FILE *data;
    if ((data = fopen("dane.txt", "r")) == NULL) {
        perror("Provide dane.txt file!");
        exit(-1);
    }
    FILE *a = fopen("a.txt", "w");
    FILE *b = fopen("b.txt", "w");
    FILE *c = fopen("c.txt", "w");
    char *line = malloc(LENGTH * sizeof(char));
    int even = 0;
    while (fgets(line, LENGTH, data) != NULL) {
        long int number = strtol(line, NULL, 10);
        if (number % 2 == 0) even++;
        if (number >= 10 && (((int) (number % 100) / 10) == 0 || ((int) (number % 100) / 10) == 7)) {
            fputs(line, b);
        }
        if (number > 0 && is_square(number)) {
            fputs(line, c);
        }
    }
    int len = number_of_digits(even);
    char *string_number = malloc(len * sizeof(char));
    sprintf(string_number, "%d", even);
    fputs(string_number, a);
    free(string_number);
    fclose(data);
    fclose(a);
    fclose(b);
    fclose(c);
    free(line);
}