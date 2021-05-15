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


typedef struct PGMImage {
    char pgmType[3];
    int **int_data;
    unsigned int width;
    unsigned int height;
    unsigned int maxValue;
} PGMImage;

PGMImage *image;
int mode;
int step;


void *threadFunc(void *arg) {
    int *my_id = (int *) arg;
    printf("%d\n", *my_id);
    if (mode == 0) {
        for (int i = 0; i < image->height; i++) {
            for (int j = *my_id; j < image->width; j += step) {
                image->int_data[i][j] = 255 - image->int_data[i][j];
            }
        }
    } else {
        printf("here will be blocked code\n");
    }
    return NULL;
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
            result[i] = calloc(pgm->height, sizeof(int));
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
                    if (j == 512) {
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

void printImageDetails(PGMImage *pgm, const char *filename) {
    FILE *pgmfile = fopen(filename, "rb");

    // Retreiving the file extension
    char *ext = strrchr(filename, '.');

    if (!ext)
        printf("No extension found"
               "in file %s",
               filename);
    else
        printf("File format"
               "    : %s\n",
               ext + 1);

    printf("PGM File type  : %s\n",
           pgm->pgmType);

    // Print type of PGM file, in ascii
    // and binary format
    if (!strcmp(pgm->pgmType, "P2"))
        printf("PGM File Format:"
               "ASCII\n");
    else if (!strcmp(pgm->pgmType,
                     "P5"))
        printf("PGM File Format:"
               " Binary\n");

    printf("Width of img   : %d px\n",
           pgm->width);
    printf("Height of img  : %d px\n",
           pgm->height);
    printf("Max Gray value : %d\n",
           pgm->maxValue);

    // close file
    fclose(pgmfile);
}


int main(int argc, char **argv) {
    int number_of_threads;
    char *input_name;
    char *output_name;
    if (argc != 5) {
        printf("Wrong number of arguments\nExpected 4, got %d!\n", argc - 1);
    } else {
        number_of_threads = atoi(argv[1]);
        if (strcmp("distributed", argv[2]) == 0) {
            mode = 0;
        } else {
            mode = 1;
        }
        input_name = argv[3];
        output_name = argv[4];
        image = malloc(sizeof(PGMImage));
        if (openPGM(image, input_name)) printImageDetails(image, input_name);
    }
    pthread_t threads[number_of_threads];
    int *ids = calloc(number_of_threads, sizeof(int));
    step = number_of_threads;
    for (int i = 0; i < number_of_threads; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, threadFunc, &ids[i]);
    }
    for (int i = 0; i < number_of_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    savePGM(image, output_name);
}