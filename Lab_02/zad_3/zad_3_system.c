#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>

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
    int data;
    if ((data = open("dane.txt", O_RDONLY)) == -1) {
        perror("Provide dane.txt file!");
        exit(-1);
    }
    int a = open("a.txt", O_TRUNC | O_WRONLY | O_CREAT, 0777);
    int b = open("b.txt", O_TRUNC | O_WRONLY | O_CREAT, 0777);
    int c = open("c.txt", O_TRUNC | O_WRONLY | O_CREAT, 0777);
    char *line = calloc(LENGTH, sizeof(char));
    char x;
    int i = 0;
    int even = 0;
    while (read(data, &x, 1) != 0) {
        if (x != '\n') {
            line[i] = x;
            i++;
        } else {
            long int number = strtol(line, NULL, 10);
            line[i] = '\n';
            if (number % 2 == 0) even++;
            if (number >= 10 && (((int) (number % 100) / 10) == 0 || ((int) (number % 100) / 10) == 7)) {
                write(b, line, i + 1);
            }
            if (number > 0 && is_square(number)) {
                write(c, line, i + 1);
            }
            i = 0;
            line = calloc(LENGTH, sizeof(char));
        }
    }
    int len = number_of_digits(even);
    char *string_number = malloc(len * sizeof(char));
    sprintf(string_number, "%d", even);
    write(a, string_number, len);
    free(string_number);
    close(data);
    close(a);
    close(b);
    close(c);
    free(line);
}
