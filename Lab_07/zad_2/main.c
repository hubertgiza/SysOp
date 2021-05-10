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
#include <sys/mman.h>

const int pizza_slots = 5;
int furnace_size = pizza_slots * sizeof(int);
sem_t *kitchen_semaphore;
sem_t *shipping_semaphore;
int kitchen_shm_des;
int *kitchen_memory;
int shipping_shm_des;
int *shipping_memory;
int *cooks_pid;
int *couriers_pid;
int n;
int m;
struct timeval start_time;

#define PERMISSIONS 0660


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
    printf("Closing kitchen semaphore\n");
    if (kitchen_semaphore > 0) {
        int err = sem_close(kitchen_semaphore);
        if (err == -1) {
            printf("Could not close the kitchen semaphore\n");
        }
        err = sem_unlink("kitchen");
        if (err == -1) {
            printf("Could not unlink the kitchen semaphore\n");
        }
    }

    printf("Closing shipping semaphore\n");
    if (shipping_semaphore > 0) {
        int err = sem_close(shipping_semaphore);
        if (err == -1) {
            printf("Could not close the shipping semaphore\n");
        }
        err = sem_unlink("shipping");
        if (err == -1) {
            printf("Could not unlink the shipping semaphore\n");
        }
    }

    printf("Closing kitchen's memory\n");
    if (kitchen_shm_des > 0) {
        int err = munmap(kitchen_memory, furnace_size);
        if (err == -1) {
            printf("%s\n", strerror(errno));
        }
        err = shm_unlink("kitchen_shm");
        if (err == -1) {
            printf("%s\n", strerror(errno));
        }
    }
    printf("Closing kitchen's memory\n");
    if (shipping_shm_des > 0) {
        int err = munmap(shipping_memory, furnace_size);
        if (err == -1) {
            printf("%s\n", strerror(errno));
        }
        err = shm_unlink("shipping_shm");
        if (err == -1) {
            printf("%s\n", strerror(errno));
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
    if (kitchen_semaphore > 0) {
        int err = sem_close(kitchen_semaphore);
        if (err == -1) {
            printf("Could not disconnect kitchen's semaphore from process\n");
        }
    }
    if (shipping_semaphore > 0) {
        int err = sem_close(shipping_semaphore);
        if (err == -1) {
            printf("Could not disconnect kitchen's semaphore from process\n");
        }
    }

    if (kitchen_shm_des > 0) {
        int err = munmap(kitchen_memory, furnace_size);
        if (err == -1) {
            printf("Could not disconnect kitchen's shared memory from process\n");
        }
    }
    if (shipping_shm_des > 0) {
        int err = munmap(shipping_memory, furnace_size);
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
    printf("Initializing semaphores\n");
    kitchen_semaphore = sem_open("kitchen", O_CREAT | O_EXCL, O_RDWR, 1);
    if ((int) kitchen_semaphore == -1) {
        printf("%s\n", strerror(errno));
        raise(SIGINT);
    }
    shipping_semaphore = sem_open("shipping", O_CREAT | O_EXCL, O_RDWR, 1);
    if ((int) shipping_semaphore == -1) {
        printf("%s\n", strerror(errno));
        raise(SIGINT);
    }

    printf("Initializing kitchen's shared memory\n");
    kitchen_shm_des = shm_open("kitchen_shm", O_RDWR | O_CREAT | O_EXCL, PERMISSIONS);
    if (kitchen_shm_des == -1) {
        printf("%s\n", strerror(errno));
        raise(SIGINT);
    }

    int err = ftruncate(kitchen_shm_des, furnace_size);
    if (err == -1) {
        printf("%s\n", strerror(errno));
    }
    kitchen_memory = mmap(NULL, furnace_size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED, kitchen_shm_des, 0);
    if (*kitchen_memory == -1) {
        printf("%s\n", strerror(errno));
    }

    printf("Initializing shipping's shared memory\n");
    shipping_shm_des = shm_open("shipping_shm", O_RDWR | O_CREAT | O_EXCL, PERMISSIONS);
    if (shipping_shm_des == -1) {
        printf("%s\n", strerror(errno));
        raise(SIGINT);
    }

    err = ftruncate(shipping_shm_des, furnace_size);
    if (err == -1) {
        printf("%s\n", strerror(errno));
    }
    shipping_memory = mmap(NULL, furnace_size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED, shipping_shm_des, 0);
    if (*shipping_memory == -1) {
        printf("%s\n", strerror(errno));
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
            sem_wait(kitchen_semaphore);
            free_furnace_slot = get_free_furnace_slot();
            if (free_furnace_slot != -1) break;
            sem_post(kitchen_semaphore);
        }
        kitchen_memory[free_furnace_slot] = type;
        pizzas_in_preparations = pizzas_in_furnace();
        sem_post(kitchen_semaphore);
        curr_time = get_curr_timestamp();
        printf("PID: %d TIME: %ld:%d -> Dodalem pizze %d. Liczba pizz w piecu: %d\n", getpid(), curr_time.tv_sec,
               curr_time.tv_usec, type, pizzas_in_preparations);
        sleep(4);
        sem_wait(kitchen_semaphore);
        kitchen_memory[free_furnace_slot] = 0;
        pizzas_in_preparations = pizzas_in_furnace();
        sem_post(kitchen_semaphore);
        free_shipping_slot = -1;
        while (free_shipping_slot == -1) {
            sem_wait(shipping_semaphore);
            free_shipping_slot = get_free_shipping_slot();
            if (free_shipping_slot != -1) break;
            sem_post(shipping_semaphore);
        }
        shipping_memory[free_shipping_slot] = type;
        pizzas_to_ship = pizzas_on_stack();
        sem_post(shipping_semaphore);
        curr_time = get_curr_timestamp();
        printf("PID: %d TIME: %ld:%d -> Wyjmuje pizze %d. Liczba pizz w piecu: %d. Liczba pizz na stole: %d\n",
               getpid(),
               curr_time.tv_sec,
               curr_time.tv_usec, type, pizzas_in_preparations, pizzas_to_ship);
    }
}

void courier_code() {
    struct timeval curr_time;
    int delivery_slot;
    int pizzas_to_ship;
    int type;
    while (1) {
        delivery_slot = -1;
        while (delivery_slot == -1) {
            sem_wait(shipping_semaphore);
            delivery_slot = get_pizza_to_delivery();
            if (delivery_slot != -1) break;
            sem_post(shipping_semaphore);
        }
        type = shipping_memory[delivery_slot];
        shipping_memory[delivery_slot] = 0;
        pizzas_to_ship = pizzas_on_stack();
        sem_post(shipping_semaphore);
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
