/*
    Unit 3 
    Spencer Johnson spencerjames.johnson    
    Guillermo Nebra guillermo.nebra
*/

#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>


void * doMean(void *arg){



    return (void *) arg; 
}

void doMedian(){

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

void doVar(){

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
    s = pthread_join(t1, &res); 
    if(s != 0){
        printf("pthread_join\n");
        exit (EXIT_FAILURE); 
    }


    exit (EXIT_SUCCESS);
}