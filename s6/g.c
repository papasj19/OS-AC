/* 
    Operating Systems: Lab 6: Pipes & Shared Memory
    
    Developed by:
        - Guillermo Nebra Aljama    <guillermo.nebra>
        - Spencer Johnson           <spencerjames.johnson>
    Developed on: November 13th, 2024
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#define NUM_STATIONS 3
#define NUM_EVENTS 9

#define ARRIVAL_REPORT 1
#define DEPARTURE_REPORT 2
#define END_REPORT 3

typedef struct {
    int type;
    int station_id;
    int passenger_count;
} Report;

int shm_id;
int *passenger_counts;
int pipes[NUM_STATIONS][2];

void handle_error(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void close_all_pipes(int station_id) {
    for (int i = 0; i < NUM_STATIONS; i++) {
        if (i != station_id) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
    }
}

void station_process(int station_id) {
    close_all_pipes(station_id);
    srand(time(NULL) ^ (station_id << 8));
    
    for (int i = 0; i < NUM_EVENTS; i++) {
        Report report;
        report.station_id = station_id;
        report.type = (rand() % 2) ? ARRIVAL_REPORT : DEPARTURE_REPORT;
        
        int passenger_change = (rand() % 41) + 10;
        
        if (report.type == ARRIVAL_REPORT) {
            passenger_counts[station_id] += passenger_change;
        } else {
            passenger_counts[station_id] = (passenger_counts[station_id] < passenger_change) ?
                                           0 : passenger_counts[station_id] - passenger_change;
        }
        
        report.passenger_count = passenger_counts[station_id];
        
        write(pipes[station_id][1], &report, sizeof(Report));
        
        usleep((rand() % 1000) * 1000);
    }

    Report end_report = {END_REPORT, station_id, 0};
    write(pipes[station_id][1], &end_report, sizeof(Report));
    
    close(pipes[station_id][1]);
    exit(EXIT_SUCCESS);
}

void control_center() {
    fd_set read_fds;
    int stations_active = NUM_STATIONS;
    
    for (int i = 0; i < NUM_STATIONS; i++) {
        close(pipes[i][1]);
    }
    
    while (stations_active > 0) {
        FD_ZERO(&read_fds);
        int max_fd = 0;
        
        for (int i = 0; i < NUM_STATIONS; i++) {
            if (pipes[i][0] != -1) {
                FD_SET(pipes[i][0], &read_fds);
                if (pipes[i][0] > max_fd) max_fd = pipes[i][0];
            }
        }
        
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            handle_error("select");
        }
        
        for (int i = 0; i < NUM_STATIONS; i++) {
            if (pipes[i][0] != -1 && FD_ISSET(pipes[i][0], &read_fds)) {
                Report report;
                int bytes_read = read(pipes[i][0], &report, sizeof(Report));
                
                if (bytes_read == 0) {
                    close(pipes[i][0]);
                    pipes[i][0] = -1;
                    stations_active--;
                } else {
                    char buffer[128];
                    snprintf(buffer, sizeof(buffer),
                             "[Control Center] Station %d - Train %s. Passengers at station: %d\n",
                             report.station_id + 1,
                             report.type == ARRIVAL_REPORT ? "arrival" : "departure",
                             report.passenger_count);
                    write(STDOUT_FILENO, buffer, strlen(buffer));
                }
            }
        }
    }

    shmdt(passenger_counts);
    shmctl(shm_id, IPC_RMID, NULL);
}

int main() {
    shm_id = shmget(IPC_PRIVATE, NUM_STATIONS * sizeof(int), IPC_CREAT | 0666);
    if (shm_id == -1) handle_error("shmget");

    passenger_counts = (int *)shmat(shm_id, NULL, 0);
    if (passenger_counts == (void *)-1) handle_error("shmat");

    memset(passenger_counts, 0, NUM_STATIONS * sizeof(int));
    
    for (int i = 0; i < NUM_STATIONS; i++) {
        if (pipe(pipes[i]) == -1) handle_error("pipe");
    }
    
    for (int i = 0; i < NUM_STATIONS; i++) {
        pid_t pid = fork();
        if (pid == -1) handle_error("fork");
        if (pid == 0) {
            station_process(i);
        }
    }

    control_center();

    for (int i = 0; i < NUM_STATIONS; i++) {
        wait(NULL);
    }
    
    return EXIT_SUCCESS;
}
