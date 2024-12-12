/***********************************************
* 
* @Purpose: Subway train management system using pipes and shared memory. S6
* @Author: Ferran Castañé
* @Creation Date: 2024-10-14
* 
************************************************/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#define NUM_STATIONS 3
#define NUM_TRAINS 10
#define PIPE_READ 0
#define PIPE_WRITE 1
#define MSG_ARRIVAL_REPORT "ARRIVAL_REPORT"
#define MSG_DEPARTURE_REPORT "DEPARTURE_REPORT"

#define printF(x) write(1, x, strlen(x))

typedef struct {
    int passengers[NUM_STATIONS]; // Shared memory to store passenger counts
} shared_data;

/**
 * Reads from the file descriptor until the specified character is found.
 *
 * @param fd File descriptor to read from.
 * @param cEnd Character indicating the end of reading.
 * @return A dynamically allocated string containing the read data.
 */
char *readUntil(int fd, char cEnd) {
    int i = 0;
    int charsRead = 0;
    char c = ' ';
    char *buffer = NULL;

    while (c != cEnd) {
        charsRead = (int) read(fd, &c, sizeof(char));
        if (charsRead <= 0) {
            break;
        }
        buffer = (char *) realloc(buffer, (sizeof(char)) * (i + 2));
        if (buffer == NULL) {
            exit(1); // Handle allocation failure
        }
        if (c != cEnd) {
            buffer[i] = c;
            i++;
        }
    }

    if (buffer != NULL) {
        buffer[i] = '\0';
    }
    return buffer;
}

void station_process(int station_id, int pipe_fd[], shared_data* data) {
    char *msg = NULL;
    close(pipe_fd[PIPE_READ]); // Close the read end for the station

    srand(getpid());  // Seed for random event generation
    for (int i = 0; i < NUM_TRAINS; i++) {
        int event = rand() % 2; // Randomly choose between arrival (0) and departure (1)
        int passenger_change = (rand() % 41) + 10; // Random change between 10 and 50 passengers
        int radomTime = (rand() % 4) + 1; // Random time between 1 and 5 seconds

        if (event == 0) {
            data->passengers[station_id] += passenger_change;
            asprintf(&msg, "%s\n", MSG_ARRIVAL_REPORT);
            write(pipe_fd[PIPE_WRITE], msg, strlen(msg));
        } else {
            data->passengers[station_id] -= passenger_change;
            if(data->passengers[station_id] < 0) {
                data->passengers[station_id] = 0;
            }
            asprintf(&msg, "%s\n", MSG_DEPARTURE_REPORT);  
            write(pipe_fd[PIPE_WRITE], msg, strlen(msg));
        }
        free(msg);
        sleep(radomTime); // Simulate time between trains
    }   
    close(pipe_fd[PIPE_WRITE]); // Close the write end for the station
    exit(0);
}


int main() {
    int pipes[NUM_STATIONS][2];
    pid_t station_pids[NUM_STATIONS];
    char *report   = NULL;
    char *msg = NULL;
    
    // Shared memory setup
    int shm_id = shmget(IPC_PRIVATE, sizeof(shared_data), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget");
        exit(1);
    }
    
    shared_data* data = (shared_data*)shmat(shm_id, NULL, 0);
    if (data == (void*)-1) {
        perror("shmat");
        exit(1);
    }

    // Initialize shared memory passenger counts to 0
    for (int i = 0; i < NUM_STATIONS; i++) {
        data->passengers[i] = 0;
    }

    // Create pipes and fork processes for each station
    for (int i = 0; i < NUM_STATIONS; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(1);
        }

        station_pids[i] = fork();
        if (station_pids[i] == -1) {
            perror("fork");
            exit(1);
        }

        if (station_pids[i] == 0) {
            // Child process (station)
            close(pipes[i][PIPE_READ]); // Close the write end for the station
            station_process(i, pipes[i], data);
        } else {
            // Parent process (Control Center)
            close(pipes[i][PIPE_WRITE]); // Close the write end for the Control Center
        }
    }

    // Control Center: Monitor reports from stations
    int stations_finished = 0;
    while (stations_finished < NUM_STATIONS*10) {
        for (int j = 0; j < NUM_STATIONS; j++) {
            report = readUntil(pipes[j][PIPE_READ], '\n');
            if (report != NULL) {
                if(strcmp(report, MSG_ARRIVAL_REPORT) == 0) {
                    asprintf(&msg, "[Control Center] Station %d - Train arrival. Station %d now has %d passengers.\n", j,j, data->passengers[j]);
                } else if(strcmp(report, MSG_DEPARTURE_REPORT) == 0) {
                    asprintf(&msg, "[Control Center] Station %d - Train departure. Station %d now has %d passengers.\n", j,j, data->passengers[j]);
                }else {
                    asprintf(&msg, "[Control Center] Station %d - Finished.\n", j);
                }
                
                printF(msg);
                free(msg);
                
                free(report);
                stations_finished++;
            }
        }
    }

    // Close the read ends of the pipes for the Control Center
    for (int i = 0; i < NUM_STATIONS; i++) {
        close(pipes[i][PIPE_READ]);
    }

    printF("Control Center: All stations have finished.\n");

    // Wait for all station processes to finish
    for (int i = 0; i < NUM_STATIONS; i++) {
        wait(NULL);
    }

    // Detach and destroy shared memory
    shmdt(data);
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}
