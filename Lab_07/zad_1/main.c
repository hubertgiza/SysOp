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

#define SEM_ID 's'
#define KITCHEN_ID 'm'
#define SHIPPING_ID 'S'
#define PERMISSIONS 0660

const int pizza_slots = 5;
int furnace_size = pizza_slots * sizeof(int);
int semaphore_id;
int kitchen_shm_des;
int *kitchen_memory;
int shipping_shm_des;
int *shipping_memory;
int *cooks_pid;
int *couriers_pid;
int n;
int m;
struct timeval start_time;

union semun {
    int              val;
    struct semid_ds *buf;
    unsigned short  *array;
    struct seminfo  *__buf;
};


void print_table() {
    for (int i = 0; i < pizza_slots; i++) {
        printf("%d ", shipping_memory[i]);
    }
    printf("\n");
}

void clean_up() {
    printf("*******************************************************************************************\n");
    printf("Cleaning up\n");
    printf("Killing workers\n");
    for (int i = 0; i < n; i++) {
        kill(cooks_pid[i], SIGINT);
    }
    for (int i = 0; i < m; i++) {
        kill(couriers_pid[i], SIGINT);
    }
    sleep(1);
    printf("Closing semaphore\n");
    if (semaphore_id != 0) {
        int err = semctl(semaphore_id, 0, IPC_RMID, NULL);
        if (err == -1) {
            printf("Could not close the semaphore\n");
        }
    }
    printf("Closing kitchen's memory\n");
    if (kitchen_shm_des != 0) {
        int err = shmdt(kitchen_memory);
        if (err == -1) {
            printf("Could not disconnect kitchen's shared memory from process\n");
        }
        err = shmctl(kitchen_shm_des, IPC_RMID, NULL);
        if (err == -1) {
            printf("Could not close the kitchen's shared memory\n");
        }
    }
    printf("Closing shipping's memory\n");
    if (shipping_shm_des != 0) {
        int err = shmdt(shipping_memory);
        if (err == -1) {
            printf("Could not disconnect shipping's shared memory from process\n");
        }
        err = shmctl(shipping_shm_des, IPC_RMID, NULL);
        if (err == -1) {
            printf("Could not close the shipping's shared memory\n");
        }
    }
    printf("Cleaning done. Exiting the program\n");
    exit(0);
}

void handleINT() {
    printf("Received SIGINT signal\nProceeding to exit the program...\n");
    clean_up();
    exit(0);
}

void clean_up_child() {
    if (kitchen_shm_des != 0) {
        int err = shmdt(kitchen_memory);
        if (err == -1) {
            printf("Could not disconnect kitchen's shared memory from process\n");
        }
    }
    if (shipping_shm_des != 0) {
        int err = shmdt(shipping_memory);
        if (err == -1) {
            printf("Could not disconnect shipping's shared memory from process\n");
        }
    }
    printf("Child %d is exiting\n", getpid());
    exit(0);
}

void handleINTCHILD() {
    printf("Child %d got SIGINT signal\n", getpid());
    clean_up_child();
    exit(0);
}

void initialize() {
    signal(SIGINT, handleINT);
    printf("Initializing semaphore\n");
    semaphore_id = semget(ftok(getenv("HOME"), SEM_ID), 2, IPC_CREAT | IPC_EXCL | PERMISSIONS);
    if (semaphore_id == -1) {
        printf("%s\n", strerror(errno));
        raise(SIGINT);
    }
    union semun val_to_set;
    val_to_set.val = 1;
    semctl(semaphore_id, 0, SETVAL, val_to_set);
    semctl(semaphore_id, 1, SETVAL, val_to_set);
    if (semctl(semaphore_id, 0, GETVAL, NULL) != 1 || semctl(semaphore_id, 1, GETVAL, NULL) != 1) {
        printf("Could not initialize semaphores correctly\n");
        raise(SIGINT);
    }
    printf("Initializing kitchen's shared memory\n");
    kitchen_shm_des = shmget(ftok(getenv("HOME"), KITCHEN_ID), furnace_size, IPC_CREAT | IPC_EXCL | PERMISSIONS);
    if (kitchen_shm_des == -1) {
        printf("%s\n", strerror(errno));
        raise(SIGINT);
    }
    kitchen_memory = shmat(kitchen_shm_des, NULL, SHM_W);
    if (*kitchen_memory == -1) {
        printf("%s\n", strerror(errno));
        raise(SIGINT);
    }
    printf("Initializing shipping's shared memory\n");
    shipping_shm_des = shmget(ftok(getenv("HOME"), SHIPPING_ID), furnace_size, IPC_CREAT | IPC_EXCL | PERMISSIONS);
    if (shipping_shm_des == -1) {
        printf("%s\n", strerror(errno));
        raise(SIGINT);
    }
    shipping_memory = shmat(shipping_shm_des, NULL, SHM_W);
    if (*shipping_memory == -1) {
        printf("%s\n", strerror(errno));
        raise(SIGINT);
    }
    printf("Initialization done\n");
    gettimeofday(&start_time, NULL);
}

int get_free_furnace_slot() {
    for (int i = 0; i < pizza_slots; i++) {
        if (kitchen_memory[i] == 0) {
            return i;
        }
    }
    return -1;
}

int get_free_shipping_slot() {
    for (int i = 0; i < pizza_slots; i++) {
        if (shipping_memory[i] == 0) {
            return i;
        }
    }
    return -1;
}

int get_pizza_to_delivery() {
    for (int i = 0; i < pizza_slots; i++) {
        if (shipping_memory[i] != 0) {
            return i;
        }
    }
    return -1;
}

int pizzas_in_furnace() {
    int pizzas = 0;
    for (int i = 0; i < pizza_slots; i++) {
        if (kitchen_memory[i] != 0) pizzas++;
    }
    return pizzas;
}

int pizzas_on_stack() {
    int pizzas = 0;
    for (int i = 0; i < pizza_slots; i++) {
        if (shipping_memory[i] != 0) pizzas++;
    }
    return pizzas;
}


struct timeval get_curr_timestamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    if ((tv.tv_usec - start_time.tv_usec) > 0) {
        tv.tv_sec = tv.tv_sec - start_time.tv_sec;
        tv.tv_usec = tv.tv_usec - start_time.tv_usec;
    } else {
        tv.tv_usec = tv.tv_sec - start_time.tv_sec - 1;
        tv.tv_usec = abs(tv.tv_usec - start_time.tv_usec);
    }
    return tv;
}

void cook_code() {
    struct sembuf dec_furnace;
    dec_furnace.sem_num = 0;
    dec_furnace.sem_op = -1;

    struct sembuf inc_furnace;
    inc_furnace.sem_num = 0;
    inc_furnace.sem_op = 1;

    struct sembuf dec_shipping;
    dec_shipping.sem_num = 1;
    dec_shipping.sem_op = -1;

    struct sembuf inc_shipping;
    inc_shipping.sem_num = 1;
    inc_shipping.sem_op = 1;

    struct timeval curr_time;
    int free_furnace_slot;
    int free_shipping_slot;
    int pizzas_in_preparations;
    int pizzas_to_ship;
    int type;
    while (1) {
        curr_time = get_curr_timestamp();
        type = curr_time.tv_usec % 10;
        printf("PID: %d TIME: %ld:%d -> Przygotowuje pizze %d\n", getpid(), curr_time.tv_sec, curr_time.tv_usec, type);
        sleep(1);
        free_furnace_slot = -1;
        while (free_furnace_slot == -1) {
            semop(semaphore_id, &dec_furnace, 1);
            free_furnace_slot = get_free_furnace_slot();
            if (free_furnace_slot != -1) break;
            semop(semaphore_id, &inc_furnace, 1);
        }
        kitchen_memory[free_furnace_slot] = type;
        pizzas_in_preparations = pizzas_in_furnace();
        semop(semaphore_id, &inc_furnace, 1);
        curr_time = get_curr_timestamp();
        printf("PID: %d TIME: %ld:%d -> Dodalem pizze %d. Liczba pizz w piecu: %d\n", getpid(), curr_time.tv_sec,
               curr_time.tv_usec, type, pizzas_in_preparations);
        sleep(4);
        semop(semaphore_id, &dec_furnace, 1);
        kitchen_memory[free_furnace_slot] = 0;
        pizzas_in_preparations = pizzas_in_furnace();
        semop(semaphore_id, &inc_furnace, 1);
        free_shipping_slot = -1;
        while (free_shipping_slot == -1) {
            semop(semaphore_id, &dec_shipping, 1);
            free_shipping_slot = get_free_shipping_slot();
            if (free_shipping_slot != -1) break;
            semop(semaphore_id, &inc_shipping, 1);
        }
        shipping_memory[free_shipping_slot] = type;
        pizzas_to_ship = pizzas_on_stack();
        semop(semaphore_id, &inc_shipping, 1);
        curr_time = get_curr_timestamp();
        printf("PID: %d TIME: %ld:%d -> Wyjmuje pizze %d. Liczba pizz w piecu: %d. Liczba pizz na stole: %d\n",
               getpid(),
               curr_time.tv_sec,
               curr_time.tv_usec, type, pizzas_in_preparations, pizzas_to_ship);
    }
}

void courier_code() {
    struct sembuf dec_shipping;
    dec_shipping.sem_num = 1;
    dec_shipping.sem_op = -1;

    struct sembuf inc_shipping;
    inc_shipping.sem_num = 1;
    inc_shipping.sem_op = 1;

    struct timeval curr_time;
    int delivery_slot;
    int pizzas_to_ship;
    int type;
    while (1) {
        delivery_slot = -1;
        while (delivery_slot == -1) {
            semop(semaphore_id, &dec_shipping, 1);
            delivery_slot = get_pizza_to_delivery();
            if (delivery_slot != -1) break;
            semop(semaphore_id, &inc_shipping, 1);
        }
        type = shipping_memory[delivery_slot];
        shipping_memory[delivery_slot] = 0;
        pizzas_to_ship = pizzas_on_stack();
        semop(semaphore_id, &inc_shipping, 1);
        curr_time = get_curr_timestamp();
        printf("PID: %d TIME: %ld:%d -> Pobieram pizze %d. Liczba pizz na stole: %d\n",
               getpid(),
               curr_time.tv_sec,
               curr_time.tv_usec, type, pizzas_to_ship);
        sleep(4);
        curr_time = get_curr_timestamp();
        printf("PID: %d TIME: %ld:%d -> Dostarczylem pizze %d\n", getpid(), curr_time.tv_sec, curr_time.tv_usec, type);
        sleep(4);
    }
}

int main(int argc, char **argv) {
    srand(time(NULL));
    if (argc != 3) {
        printf("Wrong number of arguments!\n");
        printf("Received %d, when expected 2!\n", argc - 1);
        exit(0);
    }
    n = atoi(argv[1]);
    m = atoi(argv[2]);
    initialize();
    cooks_pid = calloc(n, sizeof(int));
    couriers_pid = calloc(m, sizeof(int));
    for (int i = 0; i < n; i++) {
        pid_t tmp_pid = fork();
        if (tmp_pid != 0) {
            cooks_pid[i] = tmp_pid;
        } else {
            signal(SIGINT, handleINTCHILD);
            cook_code();
            exit(0);
        }
    }
    printf("Main has created cooks\n");
    for (int i = 0; i < m; i++) {
        pid_t tmp_pid = fork();
        if (tmp_pid != 0) {
            couriers_pid[i] = tmp_pid;
        } else {
            signal(SIGINT, handleINTCHILD);
            courier_code();
            exit(0);
        }
    }
    printf("Main has created couriers\n");
    wait(NULL);
}
