#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/time.h>


typedef struct PGMImage {
    char pgmType[3];
    int **int_data;
    unsigned int width;
    unsigned int height;
    unsigned int maxValue;
} PGMImage;

PGMImage *image;
int mode;
int threads_number;
int **output_array;

struct timeval get_curr_timestamp(struct timeval start_time) {
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


void handleINT() {
    printf("Thread %lu: Received SIGINT signal\nThread is proceeding to exit\n", pthread_self());
    pthread_exit(0);
}

void ignoreComments(FILE *fp) {
    int ch;
    char line[100];
    while ((ch = fgetc(fp)) != EOF
           && isspace(ch));
    if (ch == '#') {
        fgets(line, sizeof(line), fp);
        ignoreComments(fp);
    } else
        fseek(fp, -1, SEEK_CUR);
}

int openPGM(PGMImage *pgm, const char *filename) {
    FILE *pgmfile = fopen(filename, "rb");
    if (pgmfile == NULL) {
        printf("File does not exist\n");
        return 0;
    }
    ignoreComments(pgmfile);
    fscanf(pgmfile, "%s",
           pgm->pgmType);
    if (strcmp(pgm->pgmType, "P2")) {
        fprintf(stderr, "Wrong file type!\n");
        exit(EXIT_FAILURE);
    }
    ignoreComments(pgmfile);
    fscanf(pgmfile, "%d %d",
           &(pgm->width),
           &(pgm->height));
    ignoreComments(pgmfile);
    fscanf(pgmfile, "%d", &(pgm->maxValue));
    ignoreComments(pgmfile);
    if (pgm->pgmType[1] == '2') {
        int **result = calloc(pgm->height, sizeof(int *));
        for (int i = 0; i < pgm->height; i++) {
            result[i] = calloc(pgm->width, sizeof(int));
        }
        fgetc(pgmfile);
        int i = 0;
        int j = 0;
        char buff;
        char c_number[4] = {'\0', '\0', '\0', '\0'};
        int c = 0;
        while ((buff = fgetc(pgmfile)) != EOF) {
            if (isalnum(buff)) {
                c_number[c] = buff;
                c++;
            } else {
                if (isalnum(c_number[0])) {
                    result[i][j] = atoi(c_number);
                    j++;
                    if (j == image->width) {
                        i++;
                        j = 0;
                    }
                }
                c_number[0] = '\0';
                c_number[1] = '\0';
                c_number[2] = '\0';
                c_number[3] = '\0';
                c = 0;
            }
        }
        pgm->int_data = result;
    }
    fclose(pgmfile);
    return 1;
}

int savePGM(PGMImage *pgm, const char *filename) {
    FILE *output = fopen(filename, "w");
    char sep[2] = "  ";
    fputs(pgm->pgmType, output);
    char line[40];
    sprintf(line, "\n%d  %d\n%d\n", pgm->height, pgm->width, pgm->maxValue);
    fputs(line, output);
    for (int i = 0; i < pgm->height; i++) {
        for (int j = 0; j < pgm->width; j++) {
            sprintf(line, "%d  ", pgm->int_data[i][j]);
            fputs(line, output);
        }
        if (i != 511) {
            fputs("\n", output);
        }
    }
    fclose(output);
    return 1;
}

void *threadFunc(void *arg) {
    struct timeval my_start_time;
    struct timeval my_curr_time;
    gettimeofday(&my_start_time, NULL);
    int *my_id = (int *) arg;
    if (mode == 0) {
        int my_values_start = 256 / threads_number * (*my_id);
        int my_values_end = 256 / threads_number * (*my_id + 1);
        for (int i = 0; i < image->height; i++) {
            for (int j = 0; j < image->width; j++) {
                if (image->int_data[i][j] >= my_values_start && image->int_data[i][j] < my_values_end)
                    output_array[i][j] = 255 - image->int_data[i][j];
            }
        }
    } else {
        int my_block_start = (*my_id) * (image->width / threads_number);
        int my_block_end = (*my_id + 1) * (image->width / threads_number);
        for (int i = 0; i < image->height; i++) {
            for (int j = my_block_start; j < my_block_end; j++) {
                output_array[i][j] = 255 - image->int_data[i][j];
            }
        }
    }
    my_curr_time = get_curr_timestamp(my_start_time);
    printf("Thread %d done in: %ld,%ld\n", *my_id, my_curr_time.tv_sec, my_curr_time.tv_usec);
    pthread_exit((void *) 0);
}

int main(int argc, char **argv) {
    struct timeval start_time;
    struct timeval curr_time;
    char *input_name;
    char *output_name;
    gettimeofday(&start_time, NULL);
    if (argc != 5) {
        printf("Wrong number of arguments\nExpected 4, got %d!\n", argc - 1);
    } else {
        threads_number = atoi(argv[1]);
        if (strcmp("number", argv[2]) == 0) {
            mode = 0;
        } else if (strcmp("block", argv[2]) == 0) {
            mode = 1;
        } else {
            printf("Wrong mode type\n");
            exit(0);
        }
        input_name = argv[3];
        output_name = argv[4];
        image = malloc(sizeof(PGMImage));
        openPGM(image, input_name);
        output_array = calloc(image->height, sizeof(int *));
        for (int i = 0; i < image->height; i++) {
            output_array[i] = calloc(image->width, sizeof(int));
        }
    }
    pthread_t threads[threads_number];
    int *ids = calloc(threads_number, sizeof(int));
    threads_number = threads_number;
    for (int i = 0; i < threads_number; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, threadFunc, &ids[i]);
    }
    for (int i = 0; i < threads_number; i++) {
        int err;
        pthread_join(threads[i], &err);
        if (err != 0) {
            printf("%d\n", err);
        }
    }
    image->int_data = output_array;
    savePGM(image, output_name);
    curr_time = get_curr_timestamp(start_time);
    printf("Main done in: %ld, %ld\n", curr_time.tv_sec, curr_time.tv_usec);
}