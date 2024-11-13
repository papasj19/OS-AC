
#define _GNU_SOURCE
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/shm.h>
#include <math.h>


#define printF(x) write(1, x, strlen(x))
// TEXT COLORS
#define C_RESET "\033[0m"
#define C_RED   "\033[31m"
#define C_GREEN "\033[32m"
#define C_YELLOW    "\033[33m"

#define STATIONS 3



void doPipe(int fileFd){

    
    int pipeFds[2];
    if (pipe(pipeFds) < 0)
    {
        printF("Error creating pipe\n");
        exit(-2);
    }



    int shmid;
    if ((shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) < 0)
    {
        printF("Error creating shared memory\n");
        exit(-3);
    }


    int pidPhase1;
       // Phase 1
    pidPhase1 = fork();
    switch (pidPhase1)
    {
    case -1:
        printF("Error creating fork\n");
        exit(-4);
    case 0:
        // Soon
        close(pipeFds[0]);
        phase1(pipeFds[1], fileFd);
        // Phase 1 done, clossing soon process
        close(pipeFds[1]);
        close(fileFd);
        exit(0);
    default:
        // Parent, wait for phase 1 to finish
        waitpid(pidPhase1, NULL, 0);
        break;
    }




}


void customWrite(const char *message) {
    write(STDOUT_FILENO, message, strlen(message));
}

char* read_until(int fd, char end) {
    char *string = NULL;
    char c;
    int i = 0, size;

    while (1) {
        size = read(fd, &c, sizeof(char));
        if (string == NULL) {
            string = (char *)malloc(sizeof(char));
        }
        if (c != end && size > 0) {
            string = (char *)realloc(string, sizeof(char) * (i + 2));
            string[i++] = c;
        } else {
            break;
        }
    }
    string[i] = '\0';
    return string;
}




int main(int argc, char *argv[]){

    //check Args
    if (argc != 2)
    {
        customWrite("Invalid number of arguments\n");
        return -1;
    }


    int shmid;
    if ((shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) < 0)
    {
        printF("Error creating shared memory\n");
        exit(-3);
    }


    int pipes[STATIONS][2];
    pid_t pids[STATIONS];

    for (int i = 0; i < STATIONS; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe creation failed");
            exit(1);
        }

        if ((pids[i] = fork()) == 0) {  // Child process (station)
            close(pipes[i][0]);  // Close read end in child
            // Station process code here (not shown in this section)
            exit(0);
        } else if (pids[i] < 0) {
            perror("fork failed");
            exit(1);
        }

        // Control Center keeps only the read end
        close(pipes[i][1]);
    }

      // 3. Control Center Waiting and Processing Reports
    int *passenger_counts;
    int active_stations = STATIONS;
    while (active_stations > 0) {
        for (int i = 0; i < STATIONS; i++) {
            char buffer[20];
            ssize_t bytes_read = read(pipes[i][0], buffer, sizeof(buffer) - 1);

            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';  // Null-terminate the string
                char *message = NULL;

                if (strcmp(buffer, "ARRIVAL") == 0) {
                    // Increase passengers by reading shared memory
                    passenger_counts[i] += 10 + (rand() % 41); // Increment by 10-50
                    asprintf(&message, "[Control Center] Station %d - Train arrival. Passengers now: %d\n", 
                             i + 1, passenger_counts[i]);
                    customWrite(message);
                } else if (strcmp(buffer, "DEPARTURE") == 0) {
                    // Decrease passengers while ensuring no negative count
                    int decrement = 10 + (rand() % 41);  // Decrement by 10-50
                    passenger_counts[i] = (passenger_counts[i] > decrement) ? 
                                          (passenger_counts[i] - decrement) : 0;
                    asprintf(&message, "[Control Center] Station %d - Train departure. Passengers now: %d\n", 
                             i + 1, passenger_counts[i]);
                    customWrite(message);
                } else if (strcmp(buffer, "FINISHED") == 0) {
                    active_stations--;  // Mark station as finished
                    asprintf(&message, "[Control Center] Station %d has completed all events.\n", i + 1);
                    customWrite(message);
                }

                free(message);  // Free dynamically allocated message
            }
        }
    }

    // Cleanup
    for (int i = 0; i < STATIONS; i++) {
        close(pipes[i][0]);
    }
    shmdt(passenger_counts);
    shmctl(shmid, IPC_RMID, NULL);

    char *final_message = NULL;
    asprintf(&final_message, "[Control Center] All stations have completed. System shutting down.\n");
    customWrite(final_message);
    free(final_message);  // Free final message

    return 0;

}