/*
    OPERATING SYSTEMS: SESSION 10
    Guillermo Nebra Aljama <guillermo.nebra>
    Spencer Johnson <spencerjames.johnson>

    DISCLAIMER: 
    As in the previous session, the error message shown when compiling the code is related to a conflict with the _GNU_SOURCE macro and the provided semaphore library.
    Aaron told us to write this disclaimer, as the error originates from "semaphore_v2.h" and not from the code itself :)
    Pease be kind and ignore the error message. 
    
    Thank you!
        - Guillermo and Spencer

*/

#define _GNU_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include "semaphore_v2.h"

#define Write(x) write(1, x, strlen(x))
#define NUM_STATIONS 5
#define MAX_CARS_IN_PROCESS 2

#define ERR_ARGS "Usage: ./S10.exe <number of cars>\n"
#define STATION_START "\033[0;32m%s of car %d starting\n\033[0m"
#define STATION_COMPLETE "\033[0;34m%s of car %d assembled\n\033[0m"
#define QC_CHECK "\033[0;33mQuality control of car %d at %s \033[0m"
#define QC_STATUS "\033[0;%dm%s\033[0m\n"
#define QC_PASSED "\033[0;32mQuality control of car %d passed\n\033[0m"

const char* STATION_NAMES_START[] = {"Car bodywork", "Engine station", "Wheels station", "Paint station", "Quality Control"};
const char* STATION_NAMES_END[] = {"Bodywork", "Engine", "Wheels", "Paint", "Quality Control"};

semaphore sem_screen;
semaphore sem_process_limit;
semaphore sem_stations[NUM_STATIONS];

typedef struct {
    int num_cars;
    int* car_station;
    int* car_complete;
    int total_complete;
} AssemblyState;

typedef struct {
    int station_idx;
    AssemblyState* state;
} ThreadArgs;

void init_semaphores() {
    SEM_constructor_with_name(&sem_screen, 1000);
    SEM_constructor_with_name(&sem_process_limit, 1001);
    
    for(int i = 0; i < NUM_STATIONS; i++) {
        SEM_constructor_with_name(&sem_stations[i], 1002 + i);
    }
    
    SEM_init(&sem_screen, 1);
    SEM_init(&sem_process_limit, MAX_CARS_IN_PROCESS);
    
    for(int i = 0; i < NUM_STATIONS; i++) {
        SEM_init(&sem_stations[i], 1);
    }
}

void cleanup_semaphores() {
    SEM_destructor(&sem_screen);
    SEM_destructor(&sem_process_limit);
    for(int i = 0; i < NUM_STATIONS; i++) {
        SEM_destructor(&sem_stations[i]);
    }
}

int get_random_time() {
    return (rand() % 5) + 1;
}

void do_station_work(int car_num, int station_idx) {
    int work_time = get_random_time();
    char* aux;
    
    SEM_wait(&sem_screen);
    asprintf(&aux, STATION_START, STATION_NAMES_START[station_idx], car_num + 1);
    Write(aux);
    free(aux);
    SEM_signal(&sem_screen);
    
    sleep(work_time);
    
    SEM_wait(&sem_screen);
    asprintf(&aux, STATION_COMPLETE, STATION_NAMES_END[station_idx], car_num + 1);
    Write(aux);
    free(aux);
    SEM_signal(&sem_screen);
}

int check_quality() {
    return (rand() % 10) > 5;
}

void do_quality_control(int car_num) {
    const char* aspects[] = {"Body", "Engine", "Wheels", "Paint"};
    int all_passed;
    char* aux;
    
    do {
        all_passed = 1;
        for(int i = 0; i < 4; i++) {
            SEM_wait(&sem_screen);
            asprintf(&aux, QC_CHECK, car_num + 1, aspects[i]);
            Write(aux);
            
            int passed = check_quality();
            asprintf(&aux, QC_STATUS, passed ? 32 : 31, passed ? "passed" : "failed");
            Write(aux);
            free(aux);
            SEM_signal(&sem_screen);
            
            if(!passed) {
                all_passed = 0;
                sleep(1);
                break;
            }
        }
    } while(!all_passed);
    
    SEM_wait(&sem_screen);
    asprintf(&aux, QC_PASSED, car_num + 1);
    Write(aux);
    free(aux);
    SEM_signal(&sem_screen);
}

void* station_worker(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    int station_idx = args->station_idx;
    AssemblyState* state = args->state;
    free(args);
    
    while(state->total_complete < state->num_cars) {
        SEM_wait(&sem_stations[station_idx]);
        
        for(int car = 0; car < state->num_cars; car++) {
            if(state->car_complete[car]) continue;
            
            if(state->car_station[car] == station_idx) {
                if(station_idx == NUM_STATIONS - 1) {
                    do_quality_control(car);
                } else {
                    do_station_work(car, station_idx);
                }
                
                state->car_station[car]++;
                if(state->car_station[car] == NUM_STATIONS) {
                    state->car_complete[car] = 1;
                    SEM_signal(&sem_process_limit);
                    state->total_complete++;
                }
                
                if(state->car_station[car] < NUM_STATIONS) {
                    SEM_signal(&sem_stations[state->car_station[car]]);
                }
            }
        }
        
        SEM_signal(&sem_stations[station_idx]);
        usleep(100000);
    }
    
    return NULL;
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        Write(ERR_ARGS);
        return 1;
    }
    
    AssemblyState state;
    pthread_t station_threads[NUM_STATIONS];
    
    state.num_cars = atoi(argv[1]);
    if(state.num_cars < 1) {
        Write(ERR_ARGS);
        return 1;
    }
    
    state.car_station = malloc(state.num_cars * sizeof(int));
    memset(state.car_station, 0, state.num_cars * sizeof(int));
    state.car_complete = malloc(state.num_cars * sizeof(int));
    memset(state.car_complete, 0, state.num_cars * sizeof(int));
    state.total_complete = 0;
    
    srand(time(NULL));
    
    init_semaphores();
    
    for(int i = 0; i < NUM_STATIONS; i++) {
        ThreadArgs* args = malloc(sizeof(ThreadArgs));
        args->station_idx = i;
        args->state = &state;
        pthread_create(&station_threads[i], NULL, station_worker, args);
    }
    
    for(int car = 0; car < state.num_cars; car++) {
        SEM_wait(&sem_process_limit);
        state.car_station[car] = 0;
        SEM_signal(&sem_stations[0]);
    }
    
    for(int i = 0; i < NUM_STATIONS; i++) {
        pthread_join(station_threads[i], NULL);
    }
    
    cleanup_semaphores();
    free(state.car_station);
    free(state.car_complete);
    
    return 0;
}