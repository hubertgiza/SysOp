#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <signal.h>
#include <sys/shm.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/mman.h>

typedef l;

int main() {
    char a[100] = {"112 341 214 11 23 112 234"};
    char b[4] = {'\0', '\0'};
    printf("%d\n", strlen(b));
//    int j = 0;
//    int numbers_written = 0;
//    char c_number[4];
//    c_number[3] = '\0';
//    int c = 0;
//    while (numbers_written < 7) {
//        while (a[j] != NULL && a[j] != ' ') {
//            c_number[c] = a[j];
//            c++;
//            j++;
//        }
//        j++;
//        printf("%d\n", atoi(c_number));
//        numbers_written++;
//        c = 0;
//        c_number[0] = '\0';
//        c_number[1] = '\0';
//        c_number[2] = '\0';
//    }
}

