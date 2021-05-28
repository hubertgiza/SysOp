#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/sem.h>
#include <signal.h>
#include <errno.h>

#define REINDEER 9
#define ELVES 10
#define SEM_ID 's'
#define PERMISSIONS 0660
#define N 3
int sem_id;
int santa_sleep = 1;
int deer_ready = 0;
pthread_cond_t deer_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t elves_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t santa_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t santa_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t deer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t elves_mutex = PTHREAD_MUTEX_INITIALIZER;


void handle_INT() {
    exit(0);
}

void *santa_thread_func(void *arg) {
    struct sembuf inc_reindeer_sem;
    inc_reindeer_sem.sem_num = 0;
    inc_reindeer_sem.sem_op = 9;
    for (int i = 0; i < N; i++) {
        pthread_mutex_lock(&deer_mutex);
        while (!deer_ready) {
            pthread_cond_wait(&santa_cond, &deer_mutex);
        }
        pthread_mutex_unlock(&deer_mutex);
        deer_ready = 0;
        pthread_mutex_lock(&santa_mutex);
        santa_sleep = 0;
        pthread_cond_broadcast(&deer_cond);
        pthread_mutex_unlock(&santa_mutex);

        printf("Św. Mikołaj: dostarczam zabawki!\n");
        sleep(rand() % 3 + 2);
        printf("Św. Mikołaj: zasypiam!\n");
        semop(sem_id, &inc_reindeer_sem, 1);
        pthread_mutex_lock(&santa_mutex);
        santa_sleep = 1;
        pthread_cond_broadcast(&deer_cond);
        pthread_mutex_unlock(&santa_mutex);
    }
    return NULL;
}

void *reindeer_thread_func(void *arg) {
    int *my_id = (int *) arg;
    int sem_val;
    int sleep_time;
    struct sembuf dec_reindeer_sem;
    dec_reindeer_sem.sem_num = 0;
    dec_reindeer_sem.sem_op = -1;
    for (int i = 0; i < N; i++) {
        sleep_time = rand() % 6 + *my_id % 6;
        sleep(sleep_time);
        pthread_mutex_lock(&elves_mutex);
        semop(sem_id, &dec_reindeer_sem, 1);
        sem_val = semctl(sem_id, 0, GETVAL, NULL);
        pthread_mutex_unlock(&elves_mutex);
        printf("Renifer %d: czeka %d reniferów na Świętego Mikołaja\n", *my_id, 9 - sem_val);
        if (sem_val == 0) {
            pthread_mutex_lock(&deer_mutex);
            deer_ready = 1;
            pthread_cond_broadcast(&santa_cond);
            pthread_mutex_unlock(&deer_mutex);
        }
        pthread_mutex_lock(&santa_mutex);
        while (santa_sleep) {
            pthread_cond_wait(&deer_cond, &santa_mutex);
        }
        pthread_mutex_unlock(&santa_mutex);
        pthread_mutex_lock(&santa_mutex);
        while (!santa_sleep) {
            pthread_cond_wait(&deer_cond, &santa_mutex);
        }
        pthread_mutex_unlock(&santa_mutex);
        printf("Renifer %d: lece na wakacje!\n", *my_id);
    }
    return NULL;
}

void *elves_thread_func(void *arg) {
    return NULL;
}


int main() {
    srand(time(NULL));
    signal(SIGINT, handle_INT);
    sem_id = semget(ftok(getenv("HOME"), SEM_ID), 2, IPC_CREAT | IPC_EXCL | PERMISSIONS);
    if (sem_id == -1) {
        printf("%s\n", strerror(errno));
        raise(SIGINT);
    }
    union semun val_to_set;
    val_to_set.val = REINDEER;
    semctl(sem_id, 0, SETVAL, val_to_set);
    val_to_set.val = 3;
    semctl(sem_id, 1, SETVAL, val_to_set);
    if (semctl(sem_id, 0, GETVAL, NULL) != REINDEER || semctl(sem_id, 1, GETVAL, NULL) != 3) {
        printf("Could not initialize semaphores correctly\n");
        raise(SIGINT);
    }
    pthread_t santa_thread;
    pthread_t reindeer_thread[REINDEER];
    int *reindeer_id = calloc(REINDEER, sizeof(int));
    pthread_t elves_thread[ELVES];
    int *elves_id = calloc(ELVES, sizeof(int));
    pthread_create(&santa_thread, NULL, santa_thread_func, NULL);
    for (int i = 0; i < REINDEER; i++) {
        reindeer_id[i] = i;
        pthread_create(&reindeer_thread[i], NULL, reindeer_thread_func, &reindeer_id[i]);
    }
    for (int i = 0; i < REINDEER; i++) {
        pthread_join(reindeer_thread[i], NULL);
    }
//    for (int i = 0; i < ELVES; i++) {
//        elves_id[i] = i;
//        pthread_create(&elves_thread[i], NULL, elves_thread_func, &elves_id[i]);
//    }
    for (int i = 0; i < REINDEER; i++) {
        pthread_join(reindeer_thread[i], NULL);
    }
    semctl(sem_id, 0, IPC_RMID, NULL);
}