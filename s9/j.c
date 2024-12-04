/*
    OPERATING SYSTEMS: SESSION 9
    Guillermo Nebra Aljama <guillermo.nebra>
    Spencer Johnson <spencerjames.johnson>

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

typedef struct {
    char first_name[50];
    char last_name[50];
    int number;
    char team[50];
    double best_time;  
    int laps_completed;
} Rider;


Rider* riders;
int total_riders;
sem_t track_sem;  


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
    
    sem_wait(&track_sem); 


    dprintf(STDOUT_FILENO, "(%d) %s %s on track\n", rider->number, rider->first_name, rider->last_name);


    for (int i = 0; i < 5; i++) {
        usleep((rand() % 1000000)); 
        double lap_time = generate_lap_time();  
        if (lap_time < rider->best_time) {
            rider->best_time = lap_time;  
        }
        rider->laps_completed++;
        dprintf(STDOUT_FILENO, "(%d) %s %s completed lap %d with time: %.3f seconds\n", rider->number, rider->first_name, rider->last_name, rider->laps_completed, lap_time);
        if (rider->laps_completed == 5) {
            break; 
        }
    }
    dprintf(STDOUT_FILENO, "(%d) %s %s leaves the track\n", rider->number, rider->first_name, rider->last_name);

    sem_post(&track_sem);  
    return NULL;
}



void display_standings() {
    dprintf(STDOUT_FILENO, "=== Actual Classification ===\n");
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
        int minutes = (int)(riders[i].best_time / 60);
        int seconds = (int)(riders[i].best_time) % 60;
        int milliseconds = (int)((riders[i].best_time - (int)riders[i].best_time) * 1000);
        dprintf(STDOUT_FILENO, "%d. (%d) %s %s: %02d:%02d:%03d\n", i + 1, riders[i].number, riders[i].first_name, riders[i].last_name, minutes, seconds, milliseconds);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        dprintf(STDOUT_FILENO, "Usage: %s <riders_file> <max_riders_on_track>\n", argv[0]);
        return EXIT_FAILURE;
    }

    load_riders(argv[1]);

    int max_riders = atoi(argv[2]);
    sem_init(&track_sem, 0, max_riders);

    pthread_t threads[total_riders];

    for (int i = 0; i < total_riders; i++) {
        pthread_create(&threads[i], NULL, rider_on_track, (void*)&riders[i]);
    }

    for (int i = 0; i < 10; i++) {
        sleep(3);
        display_standings();
    }

    for (int i = 0; i < total_riders; i++) {
        pthread_join(threads[i], NULL);
    }

    display_standings();

    free(riders);
    sem_destroy(&track_sem);
    return EXIT_SUCCESS;
}
