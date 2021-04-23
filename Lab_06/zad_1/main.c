#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

int main(int argc, char **argv) {
    key_t key = ftok("/Users/hubertgiza/CLionProjects/SysOp/Lab_06/zad_1/main.c", 50);
    printf("%d", key);
}