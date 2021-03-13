#include <stdio.h>
#include <stdlib.h>

int main() {
//    char **a = calloc(5, sizeof(char *));
//    char *b = calloc(3, sizeof(char));
//    b = "12\0";
//    a[0] = b;
//    printf("%s\n", a[0]);
//    b = "ab\0";
//    printf("%s\n", b);
//    printf("%s\n", a[0]);
    char *c = "10sfgd 1";
    long int a = strtol(c, NULL, 10);
    printf("%d", a);
}
