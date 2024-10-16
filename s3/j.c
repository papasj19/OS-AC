/*
    Unit 3 
    Spencer Johnson spencerjames.johnson    
    Guillermo Nebra guillermo.nebra
*/

#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>


void * doMean(void * arg) {    
    float* info = (float *) arg; // Initialize max to the first element of the array
    float sum = 0;
    float *returnable = malloc(1 * sizeof(float));
    for (int i = 0; i < sizeof(info); i++) {
        sum += info[i];
    }
    returnable[0] = sum / sizeof(info);
    return (void *) returnable;
}


int compare(const void *a, const void *b) {
    return (*(float*)a - *(float*)b);
}

void *doMedian(void *arg) {
    // Cast the void pointer to a float pointer
    float* info = (float *) arg;

    // First element contains the size of the array (assuming you pass the size in the array)
    int size = (int)info[0];  // info[0] stores the size of the array

    // Allocate memory for a copy of the array to sort
    float* arrCopy = malloc(size * sizeof(float));
    if (arrCopy == NULL) {
        perror("Malloc failed");
        exit(EXIT_FAILURE);
    }

    // Copy the original array elements to arrCopy
    for (int i = 0; i < size; i++) {
        arrCopy[i] = info[i + 1];
    }

    // Sort the copied array
    qsort(arrCopy, size, sizeof(float), compare);

    // Calculate the median
    float median;
    if (size % 2 != 0) {
        median = arrCopy[size / 2];  // Odd number of elements
    } else {
        median = (arrCopy[size / 2 - 1] + arrCopy[size / 2]) / 2.0;  // Even number of elements
    }

    // Allocate memory for the return value
    float *returnable = malloc(1 * sizeof(float));
    if (returnable == NULL) {
        perror("Malloc failed");
        free(arrCopy); // Free the sorted array in case of failure
        exit(EXIT_FAILURE);
    }

    // Store the median in returnable and return
    *returnable = median;

    // Free the copied array
    free(arrCopy);

    return (void *) returnable;
}




void *doMax(void * arg) {
    float* info = (float *) arg; // Initialize max to the first element of the array
    float max = info[0];
    float *returnable = malloc(1 * sizeof(float));
    for (int i = 1; i < sizeof(arg); i++) {
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
    float *returnable = malloc(1 * sizeof(float));
    for (int i = 1; i < sizeof(arg); i++) {
        if (info[i] < max) {
            max = info[i]; 
        }
    }
    *returnable = max;
    return (void *) returnable;
}

void *doVariance(void *arg) {
    // Cast the void pointer to a float pointer
    float* info = (float *) arg;

    // First element contains the size of the array (assuming you pass size in the array)
    int size = (int)info[0];  // info[0] stores the size of the array

    // Calculate the mean
    float sum = 0;
    for (int i = 1; i <= size; i++) {
        sum += info[i];
    }
    float mean = sum / size;

    // Calculate the variance
    float variance = 0;
    for (int i = 1; i <= size; i++) {
        variance += pow((info[i] - mean), 2);
    }
    variance /= size;

    // Allocate memory for the return value
    float *returnable = malloc(1 * sizeof(float));
    if (returnable == NULL) {
        perror("Malloc failed");
        exit(EXIT_FAILURE);
    }

    // Store the variance in returnable and return
    *returnable = variance;
    return (void *) returnable;
}


int main ( int argc, char* argv[]){


    float info[100];

    if (argc != 2) {
        char *buffer;
        asprintf(&buffer, "Arguments wrong.\n");
        write(1, buffer, strlen(buffer));
        free(buffer);
    }

    int s1;
    pthread_t t1;
    s1 = pthread_create(&t1, NULL, doMean, NULL);
    if(s1 != 0){
        
        char *buffer;
        asprintf(&buffer, "bad creation :(\n");
        write(1, buffer, strlen(buffer));
        free(buffer);
        exit(EXIT_FAILURE);
    }

    int s2;
    pthread_t t2;
    s2 = pthread_create(&t2, NULL, doMedian, NULL);
    if(s2 != 0){
        char *buffer;
        asprintf(&buffer, "bad creation :(\n");
        write(1, buffer, strlen(buffer));
        free(buffer);
        exit(EXIT_FAILURE);
    }

    int s3;
    pthread_t t3;
    s3 = pthread_create(&t3, NULL, doMax, info);
    if(s3 != 0){
        char *buffer;
        asprintf(&buffer, "bad creation :(\n");
        write(1, buffer, strlen(buffer));
        free(buffer);
        exit(EXIT_FAILURE);
    }

    int s4;
    pthread_t t4;
    s4 = pthread_create(&t4, NULL, doMin, NULL);
    if(s4 != 0){
        char *buffer;
        asprintf(&buffer, "bad creation :(\n");
        write(1, buffer, strlen(buffer));
        free(buffer);
        exit(EXIT_FAILURE);
    }

    int s5;
    pthread_t t5;
    s5 = pthread_create(&t5, NULL, doVar, NULL);
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

    



   //Make sure join is fine 
    s1 = pthread_join(t1, &res1); 
    if(s1 != 0){
        printf("pthread_join\n");
        exit (EXIT_FAILURE); 
    }

    s2 = pthread_join(t2, &res2); 
    if(s2 != 0){
        printf("pthread_join\n");
        exit (EXIT_FAILURE); 
    }

        s3 = pthread_join(t3, &res3); 
    if(s3 != 0){
        printf("pthread_join\n");
        exit (EXIT_FAILURE); 
    }

        s4 = pthread_join(t4, &res4); 
    if(s4 != 0){
        printf("pthread_join\n");
        exit (EXIT_FAILURE); 
    }

        s5 = pthread_join(t5, &res5); 
    if(s5 != 0){
        printf("pthread_join\n");
        exit (EXIT_FAILURE); 
    }




    exit (EXIT_SUCCESS);
}