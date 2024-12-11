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

// Messages
#define ERR_ARGS "Usage: ./S10.exe <number of cars>\n"
#define STATION_START "\033[0;32m[%s] of car [%d] starting\033[0m\n"
#define STATION_COMPLETE "\033[0;34m[%s] of car [%d] assembled\033[0m\n"
#define QC_CHECK "\033[0;33mQuality control of car [%d] at [%s]\033[0m\n"
#define QC_STATUS "\033[0;%dm[%s]\033[0m\n"
#define QC_PASSED "\033[0;32mQuality control of car [%d] passed\033[0m\n"

// Station names
const char* STATION_NAMES[] = {"Body Station", "Engine Station", "Wheel Station", "Paint Station", "Quality Control"};

// Semaphores for synchronization
semaphore sem_screen;          // For screen output
semaphore sem_process_limit;   // Limit concurrent cars
semaphore sem_stations[NUM_STATIONS];  // For each station
pthread_t* station_threads;

// Shared state
typedef struct {
    int num_cars;
    int* car_station;     // Current station for each car
    int* car_complete;    // Track completed cars
    int total_complete;   // Total completed cars
} AssemblyState;

AssemblyState state;
char* aux;

// Initialize semaphores
void init_semaphores() {
    // Create semaphores with unique keys
    SEM_constructor_with_name(&sem_screen, 1000);
    SEM_constructor_with_name(&sem_process_limit, 1001);
    
    for(int i = 0; i < NUM_STATIONS; i++) {
        SEM_constructor_with_name(&sem_stations[i], 1002 + i);
    }
    
    // Initialize semaphores
    SEM_init(&sem_screen, 1);
    SEM_init(&sem_process_limit, MAX_CARS_IN_PROCESS);
    
    for(int i = 0; i < NUM_STATIONS; i++) {
        SEM_init(&sem_stations[i], 1);
    }
}

// Cleanup semaphores
void cleanup_semaphores() {
    SEM_destructor(&sem_screen);
    SEM_destructor(&sem_process_limit);
    for(int i = 0; i < NUM_STATIONS; i++) {
        SEM_destructor(&sem_stations[i]);
    }
}

// Random time between 1-5 seconds
int get_random_time() {
    return (rand() % 5) + 1;
}

// Simulates work at a station
void do_station_work(int car_num, int station_idx) {
    int work_time = get_random_time();
    
    SEM_wait(&sem_screen);
    asprintf(&aux, STATION_START, STATION_NAMES[station_idx], car_num);
    Write(aux);
    free(aux);
    SEM_signal(&sem_screen);
    
    sleep(work_time);
    
    SEM_wait(&sem_screen);
    asprintf(&aux, STATION_COMPLETE, STATION_NAMES[station_idx], car_num);
    Write(aux);
    free(aux);
    SEM_signal(&sem_screen);
}

// Quality control check for a specific aspect
int check_quality() {
    return (rand() % 10) > 5;
}

// Handle quality control station
void do_quality_control(int car_num) {
    const char* aspects[] = {"Body", "Engine", "Wheels", "Paint"};
    int all_passed;
    
    do {
        all_passed = 1;
        for(int i = 0; i < 4; i++) {
            SEM_wait(&sem_screen);
            asprintf(&aux, QC_CHECK, car_num, aspects[i]);
            Write(aux);
            free(aux);
            
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
    asprintf(&aux, QC_PASSED, car_num);
    Write(aux);
    free(aux);
    SEM_signal(&sem_screen);
}

// Station thread function
void* station_worker(void* arg) {
    int station_idx = *(int*)arg;
    free(arg);
    
    while(state.total_complete < state.num_cars) {
        SEM_wait(&sem_stations[station_idx]);
        
        // Process cars at this station
        for(int car = 0; car < state.num_cars; car++) {
            if(state.car_complete[car]) continue;
            
            if(state.car_station[car] == station_idx) {
                // Do station work
                if(station_idx == NUM_STATIONS - 1) {
                    do_quality_control(car);
                } else {
                    do_station_work(car, station_idx);
                }
                
                // Update car state
                state.car_station[car]++;
                if(state.car_station[car] == NUM_STATIONS) {
                    state.car_complete[car] = 1;
                    SEM_signal(&sem_process_limit);  // Allow new car to enter
                    state.total_complete++;
                }
                
                // Signal next station
                if(state.car_station[car] < NUM_STATIONS) {
                    SEM_signal(&sem_stations[state.car_station[car]]);
                }
            }
        }
        
        SEM_signal(&sem_stations[station_idx]);
        usleep(100000); // Small delay to prevent busy waiting
    }
    
    return NULL;
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        Write(ERR_ARGS);
        return 1;
    }
    
    state.num_cars = atoi(argv[1]);
    if(state.num_cars < 1) {
        Write(ERR_ARGS);
        return 1;
    }
    
    // Initialize state
    state.car_station = calloc(state.num_cars, sizeof(int));
    state.car_complete = calloc(state.num_cars, sizeof(int));
    state.total_complete = 0;
    
    // Initialize random seed
    srand(time(NULL));
    
    // Initialize semaphores
    init_semaphores();
    
    // Create station threads
    station_threads = malloc(NUM_STATIONS * sizeof(pthread_t));
    for(int i = 0; i < NUM_STATIONS; i++) {
        int* station_idx = malloc(sizeof(int));
        *station_idx = i;
        pthread_create(&station_threads[i], NULL, station_worker, station_idx);
    }
    
    // Start cars (max 2 at a time)
    for(int car = 0; car < state.num_cars; car++) {
        SEM_wait(&sem_process_limit);
        state.car_station[car] = 0;
        SEM_signal(&sem_stations[0]);  // Signal first station
    }
    
    // Wait for all stations to complete
    for(int i = 0; i < NUM_STATIONS; i++) {
        pthread_join(station_threads[i], NULL);
    }
    
    // Cleanup
    cleanup_semaphores();
    free(station_threads);
    free(state.car_station);
    free(state.car_complete);
    
    return 0;
}