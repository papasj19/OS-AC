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

// Struct to hold rider data
typedef struct {
    char first_name[50];
    char last_name[50];
    int number;
    char team[50];
    double best_time;  // Best lap time in seconds
    int laps_completed;
} Rider;

// Global variables
Rider* riders;
int total_riders;
sem_t track_sem;  // Semaphore for limiting track entry

// Function to read until a delimiter
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
    buffer[count] = '\0';  // Null-terminate the string
    return count;
}

// Function to load riders from a file
void load_riders(const char* filename) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Count the number of riders
    char buffer[256];
    int rider_count = 0;
    while (readUntil(fd, buffer, '\n', sizeof(buffer)) > 0) {
        rider_count++;
    }
    close(fd);

    // Allocate memory for riders
    riders = malloc(rider_count * sizeof(Rider));
    if (!riders) {
        perror("Error allocating memory for riders");
        exit(EXIT_FAILURE);
    }
    total_riders = rider_count;

    // Re-open file and parse lines
    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Error reopening file");
        free(riders);
        exit(EXIT_FAILURE);
    }

    int idx = 0;
    while (readUntil(fd, buffer, '\n', sizeof(buffer)) > 0) {
        sscanf(buffer, "%49[^,],%49[^,],%d,%49[^\n]",
               riders[idx].first_name, riders[idx].last_name,
               &riders[idx].number, riders[idx].team);
        riders[idx].best_time = __DBL_MAX__;  // Set best time as a high value initially
        riders[idx].laps_completed = 0;
        idx++;
    }
    close(fd);
}

// Function to generate a random lap time (simulating sector times)
double generate_lap_time() {
    // Generate random values for each sector in the range [0.333, 1.667] seconds
    double sector1 = 0.333 + ((rand() % 1334) / 1000.0);  // Random value between 0.333 and 1.667
    double sector2 = 0.333 + ((rand() % 1334) / 1000.0);
    double sector3 = 0.333 + ((rand() % 1334) / 1000.0);

    return sector1 + sector2 + sector3;  // Total lap time will be between 1 and 5 seconds
}



/*
// Function to simulate a rider on track
void* rider_on_track(void* arg) {
    Rider* rider = (Rider*)arg;
    
    sem_wait(&track_sem);  // Ensure limited number of riders on track

    // Display when the rider enters the track
    dprintf(STDOUT_FILENO, "(%d) %s %s on track\n", rider->number, rider->first_name, rider->last_name);

    // Simulate the rider completing 5 laps
    for (int i = 0; i < 5; i++) {
        usleep((rand() % 1000000));  // Simulate the time taken for the lap
        double lap_time = generate_lap_time();  // Generate random lap time
        
        if (lap_time < rider->best_time) {
            rider->best_time = lap_time;  // Update best time
        }
        rider->laps_completed++;
    }

    // Display when the rider leaves the track
    dprintf(STDOUT_FILENO, "(%d) %s %s leaves the track\n", rider->number, rider->first_name, rider->last_name);

    sem_post(&track_sem);  // Allow another rider to enter the track
    return NULL;
}
*/

void* rider_on_track(void* arg) {
    Rider* rider = (Rider*)arg;
    
    sem_wait(&track_sem);  // Ensure limited number of riders on track

    // Display when the rider enters the track
    dprintf(STDOUT_FILENO, "(%d) %s %s on track\n", rider->number, rider->first_name, rider->last_name);

    // Simulate the rider completing 5 laps
    for (int i = 0; i < 5; i++) {
        usleep((rand() % 1000000));  // Simulate the time taken for the lap
        double lap_time = generate_lap_time();  // Generate random lap time
        
        if (lap_time < rider->best_time) {
            rider->best_time = lap_time;  // Update best time
        }
        rider->laps_completed++;

        // Print the lap time after each lap (optional for debugging)
        dprintf(STDOUT_FILENO, "(%d) %s %s completed lap %d with time: %.3f seconds\n", 
                rider->number, rider->first_name, rider->last_name, rider->laps_completed, lap_time);

        if (rider->laps_completed == 5) {
            break;  // Exit the loop after 5 laps
        }
    }

    // Display when the rider leaves the track (after completing 5 laps)
    dprintf(STDOUT_FILENO, "(%d) %s %s leaves the track\n", rider->number, rider->first_name, rider->last_name);

    sem_post(&track_sem);  // Allow another rider to enter the track
    return NULL;
}




// Function to display standings
void display_standings() {
    dprintf(STDOUT_FILENO, "=== Actual Classification ===\n");
    // Sort riders by best lap time (ascending)
    for (int i = 0; i < total_riders - 1; i++) {
        for (int j = i + 1; j < total_riders; j++) {
            if (riders[i].best_time > riders[j].best_time) {
                Rider temp = riders[i];
                riders[i] = riders[j];
                riders[j] = temp;
            }
        }
    }

    // Display the standings
    for (int i = 0; i < total_riders; i++) {
        int minutes = (int)(riders[i].best_time / 60);
        int seconds = (int)(riders[i].best_time) % 60;
        int milliseconds = (int)((riders[i].best_time - (int)riders[i].best_time) * 1000);
        dprintf(STDOUT_FILENO, "%d. (%d) %s %s: %02d:%02d:%03d\n", i + 1, riders[i].number,
                riders[i].first_name, riders[i].last_name, minutes, seconds, milliseconds);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        dprintf(STDOUT_FILENO, "Usage: %s <riders_file> <max_riders_on_track>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Load rider data from file
    load_riders(argv[1]);

    // Initialize semaphore for limiting track entry
    int max_riders = atoi(argv[2]);
    sem_init(&track_sem, 0, max_riders);

    pthread_t threads[total_riders];

    // Create a thread for each rider
    for (int i = 0; i < total_riders; i++) {
        pthread_create(&threads[i], NULL, rider_on_track, (void*)&riders[i]);
    }

    // Periodically display standings every 3 seconds
    for (int i = 0; i < 10; i++) {
        sleep(3);
        display_standings();
    }

    // Wait for all rider threads to finish
    for (int i = 0; i < total_riders; i++) {
        pthread_join(threads[i], NULL);
    }

    // Final standings display
    display_standings();

    // Clean up
    free(riders);
    sem_destroy(&track_sem);
    return EXIT_SUCCESS;
}
