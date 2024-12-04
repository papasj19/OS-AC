/*
    OPERATING SYSTEMS: SESSION 9
    Guillermo Nebra Aljama <guillermo.nebra>
    Spencer Johnson <spencerjames.johnson>
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include "semaphore_v2.h"
#include <signal.h>

#define printF(x) write(1, x, strlen(x))

typedef struct {
    char first_name[50];
    char last_name[50];
    int number;
    char team[50];
    double best_time;  
    int laps_completed;
} Rider;

int countDone = 0;

Rider* riders;
int total_riders;
semaphore track_sem; // Use the semaphore struct from semaphore_v2.h

int readUntil(int fd, char* buffer, char delimiter, int max_length) {
    int count = 0;
    char ch;
    while (count < max_length - 1) {
        int bytes_read = read(fd, &ch, 1);
        if (bytes_read <= 0 || ch == delimiter) {
            break;
        }
        buffer[count++] = ch;
    }
    buffer[count] = '\0'; 
    return count;
}

void load_riders(const char* filename) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    char buffer[256];
    int rider_count = 0;
    while (readUntil(fd, buffer, '\n', sizeof(buffer)) > 0) {
        rider_count++;
    }
    close(fd);
    riders = malloc(rider_count * sizeof(Rider));
    if (!riders) {
        perror("Error allocating memory for riders");
        exit(EXIT_FAILURE);
    }
    total_riders = rider_count;
    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Error reopening file");
        free(riders);
        exit(EXIT_FAILURE);
    }
    int idx = 0;
    while (readUntil(fd, buffer, '\n', sizeof(buffer)) > 0) {
        sscanf(buffer, "%49[^,],%49[^,],%d,%49[^\n]", riders[idx].first_name, riders[idx].last_name, &riders[idx].number, riders[idx].team);
        riders[idx].best_time = __DBL_MAX__; 
        riders[idx].laps_completed = 0;
        idx++;
    }
    close(fd);
}

double generate_lap_time() {
    double sector1 = 0.333 + ((rand() % 1334) / 1000.0);  
    double sector2 = 0.333 + ((rand() % 1334) / 1000.0);
    double sector3 = 0.333 + ((rand() % 1334) / 1000.0);
    return sector1 + sector2 + sector3; 
}

void* rider_on_track(void* arg) {
    Rider* rider = (Rider*)arg;
    SEM_wait(&track_sem); 
    
    char* track_msg = NULL;
    asprintf(&track_msg, "(%d) %s %s on track\n", rider->number, rider->first_name, rider->last_name);
    printF(track_msg);
    free(track_msg);

    for (int i = 0; i < 5; i++) {
        usleep((rand() % 1000000)); 
        double lap_time = generate_lap_time();  
        if (lap_time < rider->best_time) {
            rider->best_time = lap_time;  
        }
        rider->laps_completed++;

        char* lap_msg = NULL;
        asprintf(&lap_msg, "(%d) %s %s completed lap %d with time: %.3f seconds\n",
                 rider->number, rider->first_name, rider->last_name, rider->laps_completed, lap_time);
        printF(lap_msg);
        free(lap_msg);
    }

    char* leave_msg = NULL;
    asprintf(&leave_msg, "(%d) %s %s leaves the track\n", rider->number, rider->first_name, rider->last_name);
    printF(leave_msg);
    free(leave_msg);

    SEM_signal(&track_sem); 
    return NULL;
}

void display_standings() {
    printF("=== Current Classification ===\n");
    for (int i = 0; i < total_riders - 1; i++) {
        for (int j = i + 1; j < total_riders; j++) {
            if (riders[i].best_time > riders[j].best_time) {
                Rider temp = riders[i];
                riders[i] = riders[j];
                riders[j] = temp;
            }
        }
    }

    for (int i = 0; i < total_riders; i++) {
        char* standings_msg = NULL;
        if (riders[i].best_time != __DBL_MAX__) {
            int minutes = (int)(riders[i].best_time / 60);
            int seconds = (int)(riders[i].best_time) % 60;
            int milliseconds = (int)((riders[i].best_time - (int)riders[i].best_time) * 1000);
            asprintf(&standings_msg, "%d. (%d) %s %s: %02d:%02d:%03d\n",  i + 1, riders[i].number, riders[i].first_name, riders[i].last_name, minutes, seconds, milliseconds);
            printF(standings_msg);
            free(standings_msg);
        }
        
    }
    printF("\n");
}


void ignore_signal() {
    printF("SIGINT ignored. Use another way to terminate.\n");
}

void* countten() {
    sleep(10); 
    countDone = 1;
    return NULL;
}


int main(int argc, char* argv[]) {

    signal(SIGINT, ignore_signal);

    if (argc != 3) {
        char *usage_msg = NULL;
        asprintf(&usage_msg, "Usage: %s <riders_file> <max_riders_on_track>\n", argv[0]);
        printF(usage_msg);
        free(usage_msg);
        return EXIT_FAILURE;
    }

    load_riders(argv[1]);
    int max_riders = atoi(argv[2]);
    if (SEM_constructor(&track_sem) < 0) {
        perror("Error creating semaphore");
        return EXIT_FAILURE;
    }
    if (SEM_init(&track_sem, max_riders) < 0) {
        perror("Error initializing semaphore");
        return EXIT_FAILURE;
    }

    pthread_t threads[total_riders];
    for (int i = 0; i < total_riders; i++) {
        pthread_create(&threads[i], NULL, rider_on_track, (void*)&riders[i]);
    }

    pthread_t display_thread;
    pthread_create(&display_thread, NULL, countten, NULL);

    for (int i = 0; i < 10; i++) {
        
        if(countDone == 1){
            pthread_cancel(display_thread);
            break;
        }else{
            sleep(3);
            display_standings();
        }
    }

    for (int i = 0; i < total_riders; i++) {
        pthread_join(threads[i], NULL);
    }

    display_standings();
    printF("\nThe session has ended\n");
    free(riders);
    SEM_destructor(&track_sem); // Destroy the semaphore
    return EXIT_SUCCESS;
}
