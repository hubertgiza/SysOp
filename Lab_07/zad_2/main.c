#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#define PERMISSIONS 0660

int main(int argc, char **argv) {
    sem_t *sem = sem_open("moj", O_CREAT | O_EXCL | O_RDWR, PERMISSIONS, 1);
    int val;
    sem_getvalue(sem, &val);
    printf("%d\n", val);
    sem_post(sem);
    sem_getvalue(sem, &val);
    printf("%d\n", val);
    sem_unlink("moj");
    sem_close(sem);
}