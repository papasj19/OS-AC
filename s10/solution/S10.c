/*
 * Solution S9 Operating Systems - Semaphores2
 * Curs 2024-25
 * @author: Ferran Castañé
 *
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>
#include "semaphore_v2.h"

// Number of stations in the assembly line
#define N_STATIONS 5

// Maximum number of cars that can be assembled concurrently
#define MAX_CARS 2

// Messages for various stations and actions
#define MSG_BODYWORK "Car bodywork of car %d starting\n"
#define MSG_ENGINE_STATION  "Engine station of car %d starting\n"
#define MSG_WHEEL_STATION "Wheel station of car %d starting\n"
#define MSG_PAINT_STATION  "Paint station of car %d starting\n"
#define MSG_QUALITY_CONTROL_STATION  "Quality control station of car %d starting\n"
#define MSG_BODYWORK_ASSEMBLED "Bodywork of car %d assembled\n"
#define MSG_ENGINE_ASSEMBLED "Engine of car %d assembled\n"
#define MSG_WHEELS_ASSEMBLED "Wheels of car %d assembled\n"
#define MSG_PAINT_APPLIED "Paint of car %d assembled\n"
#define MSG_QUALITY_CONTROL_PASSED "Quality control of car %d at station %s passed\n"
#define MSG_QUALITY_CONTROL_FAILED "Quality control of car %d at station %s failed\n"
#define MSG_QUALITY_CONTROL_OVERALL_PASSED "Quality control of car %d passed\n"
#define ERR_MSG_ARGS "Usage: ./S9 <number of cars>\n"

// Mutex for screen output synchronization
pthread_mutex_t mutexScreen;
// Number of cars to be assembled
int n_cars =  0;

/**
 * Struct to hold thread arguments.
 */
typedef struct {
    int numCar;    // Car number
    char *station; // Station name
} ThreadArgs;

// Semaphores for each station and synchronization
semaphore *semWheels, *semEngine, *semBody, *semPaint, *semQualityControl;
semaphore semCars, semSync, semSyncQualityControl, semSyncProduction;

// Arrays to hold thread IDs
pthread_t *threads = NULL;
pthread_t *threadsQualityControl = NULL;

/**
 * Initializes the screen mutex.
 */
void initScreenMutex() {
    pthread_mutex_init(&mutexScreen, NULL);
}

/**
 * Destroys the screen mutex.
 */
void destroyScreenMutex() {
    pthread_mutex_destroy(&mutexScreen);
}

/**
 * Prints a string to the screen with thread safety.
 * @param string The string to print.
 */
void printF(char *string) {
    pthread_mutex_lock(&mutexScreen);
    write(STDOUT_FILENO, string, strlen(string));
    pthread_mutex_unlock(&mutexScreen);
}

/**
 * Function representing the wheel station.
 * @param arg The car number.
 * @return NULL
 */
void *wheelStation(void *arg) {
    int numCar = *((int *)arg);
    char *buffer = NULL;
    int random = rand() % 5 + 1;
    SEM_signal(&semSync);
    SEM_wait(&(semEngine[numCar]));
    asprintf(&buffer, MSG_WHEEL_STATION, numCar + 1);
    printF(buffer);
    free(buffer);
    sleep(random);
    asprintf(&buffer, MSG_WHEELS_ASSEMBLED, numCar + 1);
    printF(buffer);
    free(buffer);
    SEM_signal(&(semWheels[numCar]));
    return NULL;
}

/**
 * Function representing the engine station.
 * @param arg The car number.
 * @return NULL
 */
void *engineStation(void *arg) {
    char *buffer = NULL;
    int numCar = *((int *)arg);
    int random = rand() % 5 + 1;
    SEM_signal(&semSync);
    SEM_wait(&(semBody[numCar]));
    asprintf(&buffer, MSG_ENGINE_STATION, numCar + 1);
    printF(buffer);
    free(buffer);
    sleep(random);
    asprintf(&buffer, MSG_ENGINE_ASSEMBLED, numCar + 1);
    printF(buffer);
    free(buffer);
    SEM_signal(&(semEngine[numCar]));
    return NULL;
}

/**
 * Function representing the bodywork station.
 * @param arg The car number.
 * @return NULL
 */
void *bodyworkStation(void *arg) {
    char *buffer = NULL;
    int numCar = *((int *)arg);
    int random = rand() % 5 + 1;
    SEM_signal(&semSync);
    asprintf(&buffer, MSG_BODYWORK, numCar + 1);
    printF(buffer);
    free(buffer);
    sleep(random);
    asprintf(&buffer, MSG_BODYWORK_ASSEMBLED, numCar + 1);
    printF(buffer);
    free(buffer);
    SEM_signal(&(semBody[numCar]));
    return NULL;
}

/**
 * Function representing the paint station.
 * @param arg The car number.
 * @return NULL
 */
void *paintStation(void *arg) {
    char *buffer = NULL;
    int numCar = *((int *)arg);
    int random = rand() % 5 + 1;
    SEM_signal(&semSync);
    SEM_wait(&(semWheels[numCar]));
    asprintf(&buffer, MSG_PAINT_STATION, numCar + 1);
    printF(buffer);
    free(buffer);
    sleep(random);
    asprintf(&buffer, MSG_PAINT_APPLIED, numCar + 1);
    printF(buffer);
    free(buffer);
    SEM_signal(&(semPaint[numCar]));
    return NULL;
}

/**
 * Function to check quality control.
 * @param arg Thread arguments.
 * @return NULL
 */
void *checkQualityControl(void *arg) {
    ThreadArgs threadArgs = *(ThreadArgs *)arg;
    char *buffer = NULL;
    char *station = strdup(threadArgs.station);
    int random = 0;
    SEM_signal(&semSyncQualityControl);

    do {
        random = rand() % 10;
        if (random > 5) {
            asprintf(&buffer, MSG_QUALITY_CONTROL_PASSED, threadArgs.numCar + 1, station);
            printF(buffer);
            free(buffer);
            SEM_signal(&(semQualityControl[threadArgs.numCar]));
            break;
        } else {
            asprintf(&buffer, MSG_QUALITY_CONTROL_FAILED, threadArgs.numCar + 1, station);
            printF(buffer);
            free(buffer);
            sleep(1);
        }
    } while (random <= 5);

    free(station);

    return NULL;
}

/**
 * Function representing the quality control station.
 * @param arg The car number.
 * @return NULL
 */
void *qualityControlStation(void *arg) {
    char *buffer = NULL;
    int numCar = *((int *)arg);
    SEM_signal(&semSync);
    SEM_wait(&(semPaint[numCar]));
    asprintf(&buffer, MSG_QUALITY_CONTROL_STATION, numCar + 1);
    printF(buffer);
    free(buffer);
    int posThread = numCar * N_STATIONS;
    for (int i = 0; i < N_STATIONS - 1; i++) {
        char *station = NULL;
        switch (i) {
            case 0:
                station = strdup("Bodywork");
                break;
            case 1:
                station = strdup("Engine");
                break;
            case 2:
                station = strdup("Wheels");
                break;
            case 3:
                station = strdup("Paint");
                break;
        }
        ThreadArgs *threadArgs = (ThreadArgs *)malloc(sizeof(ThreadArgs));
        threadArgs->numCar = numCar;
        threadArgs->station = station;
        pthread_create(&(threadsQualityControl[i + posThread]), NULL, checkQualityControl, (void *)threadArgs);
        SEM_wait(&semSyncQualityControl);
        free(threadArgs->station);
        free(threadArgs);
    }
    for (int i = 0; i < N_STATIONS - 1; i++) {
        SEM_wait(&(semQualityControl[numCar]));
    }
    for (int i = 0; i < N_STATIONS - 1; i++) {
        pthread_join(threadsQualityControl[i + posThread], NULL);
    }
    asprintf(&buffer, MSG_QUALITY_CONTROL_OVERALL_PASSED, numCar + 1);
    printF(buffer);
    free(buffer);
    return NULL;
}

/**
 * Function to initialize car production.
 * @return NULL
 */
void *initProduction() {
    for (int x = 0; x < n_cars; x++) {
        SEM_wait(&semCars);
        int posThread = x * N_STATIONS;
        for (int i = 0; i < N_STATIONS; i++) {
            switch (i) {
                case 0:
                    pthread_create(&(threads[i + posThread]), NULL, bodyworkStation, (void *)&x);
                    break;
                case 1:
                    pthread_create(&(threads[i + posThread]), NULL, engineStation, (void *)&x);
                    break;
                case 2:
                    pthread_create(&(threads[i + posThread]), NULL, wheelStation, (void *)&x);
                    break;
                case 3:
                    pthread_create(&(threads[i + posThread]), NULL, paintStation, (void *)&x);
                    break;
                case 4:
                    pthread_create(&(threads[i + posThread]), NULL, qualityControlStation, (void *)&x);
                    break;
            }
            SEM_wait(&semSync);
        }
        SEM_signal(&semSyncProduction);
    }
    return NULL;
}

/**
 * Main function to initialize the system and create threads for each task.
 * @param argc Number of arguments.
 * @param argv Array of arguments.
 */

int main(int argc, char *argv[]) {
    pthread_t thread;

    if(argc != 2) {
        printF(ERR_MSG_ARGS);
        return -1;
    }
    n_cars = atoi(argv[1]);
    signal(SIGINT, SIG_IGN);

    // Initialize the screen mutex
    initScreenMutex();

    // Allocate memory for semaphores for each station
    semEngine = (semaphore *)malloc(sizeof(semaphore) * n_cars);
    semWheels = (semaphore *)malloc(sizeof(semaphore) * n_cars);
    semBody = (semaphore *)malloc(sizeof(semaphore) * n_cars);
    semPaint = (semaphore *)malloc(sizeof(semaphore) * n_cars);
    semQualityControl = (semaphore *)malloc(sizeof(semaphore) * n_cars);

    //Initialize semaphores
    SEM_constructor(&semCars);
    SEM_init(&semCars, MAX_CARS);

    SEM_constructor(&semSync);
    SEM_init(&semSync, 0);
    SEM_constructor(&semSyncQualityControl);
    SEM_init(&semSyncQualityControl, 0);
    SEM_constructor(&semSyncProduction);
    SEM_init(&semSyncProduction, 0);

    for (int i = 0; i < n_cars; i++) {
        SEM_constructor(&semEngine[i]);
        SEM_constructor(&semWheels[i]);
        SEM_constructor(&semBody[i]);
        SEM_constructor(&semPaint[i]);
        SEM_constructor(&semQualityControl[i]);
        SEM_init(&semEngine[i], 0);
        SEM_init(&semWheels[i], 0);
        SEM_init(&semBody[i], 0);
        SEM_init(&semPaint[i], 0);
        SEM_init(&semQualityControl[i], 0);
    }

    // Allocate memory for thread arrays
    threads = (pthread_t *)malloc((N_STATIONS * n_cars) * sizeof(pthread_t));
    threadsQualityControl = (pthread_t *)malloc((N_STATIONS * n_cars) * sizeof(pthread_t));

    // Create production initialization thread
    pthread_create(&thread, NULL, initProduction, NULL);
    SEM_wait(&semSyncProduction);
    for (int x = 0; x < n_cars; x++) {
        int posThread = x * N_STATIONS;
        for (int i = 0; i < N_STATIONS; i++) {
            pthread_join(threads[i + posThread], NULL);
        }
        SEM_signal(&semCars);
    }
    pthread_join(thread, NULL);

    // Clean up semaphores
    for (int i = 0; i < n_cars; i++) {
        SEM_destructor(&semEngine[i]);
        SEM_destructor(&semWheels[i]);
        SEM_destructor(&semBody[i]);
        SEM_destructor(&semPaint[i]);
        SEM_destructor(&semQualityControl[i]);
    }
    free(semEngine);
    free(semWheels);
    free(semBody);
    free(semPaint);
    free(semQualityControl);
    SEM_destructor(&semCars);
    SEM_destructor(&semSync);
    SEM_destructor(&semSyncQualityControl);
    SEM_destructor(&semSyncProduction);
    
    // Destroy the screen mutex
    destroyScreenMutex();
    
    // Free thread arrays
    free(threads);
    free(threadsQualityControl);

    return 0;
}