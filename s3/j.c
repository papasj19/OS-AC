/*
    Unit 3 
    Spencer Johnson spencerjames.johnson    
    Guillermo Nebra guillermo.nebra
*/

#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>


int y;

void *threadFunc (void *arg) {
    int *number = (int *)arg;
    *number = *number + 5;
    y = y + 1;
    printf("From thread y=%d\n", y);
    return (void *) arg; 
}


int main (){
    pthread_t t1;
    void *res; int s, x;
    x = 7;
    y = 5;
    printf("[main] Initial value x=%d\n", x); 
    printf("[main] Initial value y=%d\n", y);
    s = pthread_create(&t1, NULL, threadFunc, &x);
    if(s != 0){
        printf("pthread_create\n");
        exit(EXIT_FAILURE);
    }
    y = y + 2;
    printf("[main] Middle value y=%d\n", y);
    s = pthread_join(t1, &res); 
    if(s != 0){
        printf("pthread_join\n");
        exit (EXIT_FAILURE); 
    }
    printf("Thread returned %lX\n", (long)res); 
    printf("[main] Final value x=%d\n", x); 
    printf("[main] Final value y=%d\n", y);
    exit (EXIT_SUCCESS);
}