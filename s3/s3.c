// Operating Systems Lab - Session 3. Threads
// Guillermo Nebra Aljama <guillermo.nebra>
// Spencer Johnson <spencerjames.johnson>


// AS WE ARE USING MATH.H, WE KINDLY REQUEST FOR THE FOLLOWING COMPILATION FLAG: -lm 

#define _GNU_SOURCE
#define _XOPEN_SOURCE 500
#define _POSIX_C_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>

// In case we lazy :)
#define customWrite(x) write(1, x, strlen(x))

int getArraySize(float *data) {
    int size = 0;
    while (data[size] != '\0') {  
        size++;
    }
    return size;
}


void * doMean(void * arg) {    
    float* info = (float *) arg; 
    float sum = 0;
    int size = getArraySize(info);


    float *returnable = malloc(1 * sizeof(float));
    for (int i = 0; i < size; i++) {
        sum += info[i];
    }
    returnable[0] = sum / size;
    return (void *) returnable;
}


int compare(const void *a, const void *b) {
    float fa = *(const float*)a;
    float fb = *(const float*)b;
    return (fa > fb) - (fa < fb);
}

void *doMedian(void *arg) {
    float* info = (float *) arg;
    int size = getArraySize(info);  

    float* arrCopy = malloc(size * sizeof(float));
    if (arrCopy == NULL) {
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < size; i++) {
        arrCopy[i] = info[i];
    }

    qsort(arrCopy, size, sizeof(float), compare);  

    float median;
    int size2 = size / 2;

    if (size % 2 == 0) {
        median = (arrCopy[size2 - 1] + arrCopy[size2]) / 2.0;
    } else {
        median = arrCopy[size2];
    }

    float *returnable = malloc(1 * sizeof(float));
    if (returnable == NULL) {
        perror("Malloc failed");
        free(arrCopy);  
        exit(EXIT_FAILURE);
    }

    *returnable = median;

    free(arrCopy);

    return (void *) returnable;
}

void *doMax(void * arg) {
    float* info = (float *) arg; 
     float max = info[0];
    int size = getArraySize(info);

    float *returnable = malloc(1 * sizeof(float));
    for (int i = 1; i < size; i++) {
        if (info[i] > max) {
            max = info[i]; 
        }
    }
    *returnable = max;
    return (void *) returnable;
}

void *doMin(void * arg){
    float* info = (float *) arg; 
    float max = info[0];
    int size = getArraySize(info);

    float *returnable = malloc(1 * sizeof(float));
    for (int i = 1; i < size; i++) {
        if (info[i] < max) {
            max = info[i]; 
        }
    }
    *returnable = max;
    return (void *) returnable;
}

void *doVariance(void *arg) {
    float* info = (float *)arg;
    int size = getArraySize(info);
    float sum = 0;
    for (int i = 0; i < size; i++) {  
        sum += info[i];
    }
    float mean = sum / size;
    float variance = 0;
    for (int i = 0; i < size; i++) { 
        variance += pow((info[i] - mean), 2);
    }
    variance /= size;
    float *returnable = malloc(sizeof(float)); 
    if (returnable == NULL) {
        perror("Malloc failed");
        exit(EXIT_FAILURE);
    }
    *returnable = variance;
    return (void *)returnable;
}

// used to read from file
char *read_until(int fd, char end) {
    char *string = NULL;
    char c;
    int i = 0, size;

    while (1) {
        size = read(fd, &c, sizeof(char));
        if (string == NULL) {
            string = (char *)malloc(sizeof(char));
        }
        if (c != end && size > 0) {
            string = (char *)realloc(string, sizeof(char) * (i + 2));
            string[i++] = c;
        } else {
            break;
        }
    }
    string[i] = '\0';
    return string;
}

// ignore the sigint

void sigint_handler() {
	return;
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        customWrite("Usage: ./g <file>\n");
        return 1;
    }

    float *data = NULL;
    int size = 0;

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        customWrite("Error opening file\n");
        exit(1);
    }

    signal(SIGINT, sigint_handler);

    while (1) {
        char *line = read_until(fd, '\n');
        if (line[0] == '\0') {
            free(line);
            break;
        }

        data = (float *)realloc(data, sizeof(float) * (size + 1));
        data[size++] = atof(line);  // Store the float value
        free(line);
    }

    close(fd);  // Close the file descriptor

int s1;
    pthread_t t1;
    s1 = pthread_create(&t1, NULL, doMean, data);
    if(s1 != 0){
        
        char *buffer;
        asprintf(&buffer, "bad creation :(\n");
        write(1, buffer, strlen(buffer));
        free(buffer);
        exit(EXIT_FAILURE);
    }

    int s2;
    pthread_t t2;
    s2 = pthread_create(&t2, NULL, doMedian, data);
    if(s2 != 0){
        char *buffer;
        asprintf(&buffer, "bad creation :(\n");
        write(1, buffer, strlen(buffer));
        free(buffer);
        exit(EXIT_FAILURE);
    }

    int s3;
    pthread_t t3;
    s3 = pthread_create(&t3, NULL, doMax, data);
    if(s3 != 0){
        char *buffer;
        asprintf(&buffer, "bad creation :(\n");
        write(1, buffer, strlen(buffer));
        free(buffer);
        exit(EXIT_FAILURE);
    }

    int s4;
    pthread_t t4;
    s4 = pthread_create(&t4, NULL, doMin, data);
    if(s4 != 0){
        char *buffer;
        asprintf(&buffer, "bad creation :(\n");
        write(1, buffer, strlen(buffer));
        free(buffer);
        exit(EXIT_FAILURE);
    }

    int s5;
    pthread_t t5;
    s5 = pthread_create(&t5, NULL, doVariance, data);
    if(s5 != 0){
        char *buffer;
        asprintf(&buffer, "bad creation :(\n");
        write(1, buffer, strlen(buffer));
        free(buffer);
        exit(EXIT_FAILURE);
    }

    void *res1;
    void *res2;
    void *res3;
    void *res4;
    void *res5;


    s1 = pthread_join(t1, &res1); 
    if(s1 != 0){
        exit (EXIT_FAILURE); 
    }

    s2 = pthread_join(t2, &res2); 
    if(s2 != 0){
        exit (EXIT_FAILURE); 
    }

        s3 = pthread_join(t3, &res3); 
    if(s3 != 0){
        exit (EXIT_FAILURE); 
    }

        s4 = pthread_join(t4, &res4); 
    if(s4 != 0){
        exit (EXIT_FAILURE); 
    }

        s5 = pthread_join(t5, &res5); 
    if(s5 != 0){
        exit (EXIT_FAILURE); 
    }


	// DO NOT REMOVE!! WE NEED TO FREEEEEEE
	free(data);

    // print all the res variables
    char *buffer;
    asprintf(&buffer, "Mean: %.5f\n", *(float *)res1);
    write(1, buffer, strlen(buffer));
    free(buffer);

    asprintf(&buffer, "Median: %.5f\n", *(float *)res2);
    write(1, buffer, strlen(buffer));
    free(buffer);

    asprintf(&buffer, "Maximum value: %.5f\n", *(float *)res3);
    write(1, buffer, strlen(buffer));
    free(buffer);

    asprintf(&buffer, "Minimum value: %.5f\n", *(float *)res4);
    write(1, buffer, strlen(buffer));
    free(buffer);

    asprintf(&buffer, "Variance: %f\n", *(float *)res5);
    write(1, buffer, strlen(buffer));
    free(buffer);




    exit (EXIT_SUCCESS);


    return 0;
}