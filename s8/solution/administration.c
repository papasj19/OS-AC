/*
* Solution S8 Operating Systems - Queue
* Curs 2024-25
*
* @author: Ferran Castañé
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <fcntl.h>

/* Constants for print messages */
#define MSG_ADMIN_START "Administration process started.\n"
#define MSG_ADMIN_TERMINATED "\nAdministration process terminated.\n"
#define MSG_ERROR_MSGQ "Error creating message queue.\n"
#define MSG_REQUEST_TIMES "Requesting available times...\n"
#define MSG_TIME_RESERVED "Time reserved.\n"
#define MSG_TIME_NOT_AVAILABLE "Time not available.\n"
#define MSG_TIME_NOT_FOUND "Requested time not found.\n"
#define MSG_RESERVATION_RECEIVED "Reservation request received.\n"
#define MSG_UNKNOWN_COMMAND "Unknown command received.\n"

#define MAX_TIMES 8
#define MAX_APPOINTMENTS_PER_TIME 2
#define QUEUE_PATH "/tmp"
#define QUEUE_ID 'Q'

/* Macro for printing messages */
#define printF(x) write(1, x, strlen(x))

/* Structure for messages */
typedef struct message {
    long msgType;
    int personId;
    char time[6];      // For single time slots like "09:00"
    char command[20];
    char text[256];    // For variable-length messages
} message_t;

typedef struct appointment {
    char time[6];
    int reserved;
} appointment_t;

int running = 1;
appointment_t availableTimes[MAX_TIMES]; // Array of available times
const char *times[] = {"09:00", "10:00", "11:00", "12:00",
                 "13:00", "14:00", "15:00", "16:00"};

/***********************************************
* @Finalitat: Signal handler for graceful termination.
************************************************/
void handleSigint() {
    printF(MSG_ADMIN_TERMINATED);
    running = 0; 
}

/***********************************************
* @Finalitat: Load appointment times from file or initialize them.
************************************************/
void loadTimes() {
    int fd = open("appointments.dat", O_RDONLY);
    if (fd == -1) {
        // Initialize with 0 reservations
        for (int i = 0; i < MAX_TIMES; i++) {
            strcpy(availableTimes[i].time, times[i]);
            availableTimes[i].reserved = 0;
        }
    } else {
        // Read reservations from the file
        read(fd, availableTimes, sizeof(appointment_t) * MAX_TIMES); 
        close(fd);
    }
}

/***********************************************
* @Finalitat: Save appointment times to file.
************************************************/
void saveTimes() {
    int fd = open("appointments.dat", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    write(fd, availableTimes, sizeof(appointment_t) * MAX_TIMES);
    close(fd);
}

/***********************************************
* @Finalitat: Prepare a string of available times.
* @Parametres: buffer = pointer to hold formatted available times.
************************************************/
void showTimes(char **buffer) {
    *buffer = NULL;
    char *temp;
    for (int i = 0; i < MAX_TIMES; i++) {
        if (*buffer == NULL) {
            asprintf(buffer, "%d) %s\n", i + 1, availableTimes[i].time);
        } else {
            temp = *buffer;
            asprintf(buffer, "%s%d) %s\n", temp, i + 1, availableTimes[i].time);
            free(temp);
        }
    }
    if (*buffer == NULL) {
        asprintf(buffer, "No available times.\n");
    }
}

/***********************************************
* @Finalitat: Reserve a time slot.
* @Parametres: time = selected time to reserve.
* @Retorn: Returns 1 if reservation successful, 0 if not available.
************************************************/
int reserveTime(char *time) {
    for (int i = 0; i < MAX_TIMES; i++) {
        if (strcmp(availableTimes[i].time, time) == 0) {
            if (availableTimes[i].reserved < MAX_APPOINTMENTS_PER_TIME) {
                availableTimes[i].reserved++;
                saveTimes();
                printF(MSG_TIME_RESERVED);
                return 1;
            } else {
                printF(MSG_TIME_NOT_AVAILABLE);
                return 0;
            }
        }
    }
    printF(MSG_TIME_NOT_FOUND);
    return 0;
}

/***********************************************
* @Finalitat: Handle REQUEST_TIMES command.
* @Parametres: msg = message structure, msgid = message queue ID.
************************************************/
void handleRequestTimes(message_t *msg, int msgid) {
    printF(MSG_REQUEST_TIMES);
    char *buffer = NULL;

    // Send available times
    showTimes(&buffer);

    int personId = msg->personId;

    memset(msg, 0, sizeof(message_t));
    msg->msgType = personId; 
    strcpy(msg->command, "AVAILABLE_TIMES");
    strncpy(msg->text, buffer, sizeof(msg->text) - 1);
    msg->text[sizeof(msg->text) - 1] = '\0';

    msgsnd(msgid, msg, sizeof(message_t) - sizeof(long), 0);
    free(buffer);
}

/***********************************************
* @Finalitat: Handle RESERVE command.
* @Parametres: msg = message structure, msgid = message queue ID.
************************************************/
void handleReserve(message_t *msg, int msgid) {
    printF(MSG_RESERVATION_RECEIVED);
    char *buffer = NULL;
    asprintf(&buffer, "Person %d requested appointment at %s.\n", msg->personId, msg->time);
    printF(buffer);
    free(buffer);

    int personId = msg->personId; 

    if (reserveTime(msg->time)) {
        memset(msg, 0, sizeof(message_t));
        msg->msgType = personId;
        strcpy(msg->command, "CONFIRMED");
        msgsnd(msgid, msg, sizeof(message_t) - sizeof(long), 0);
    } else {
        memset(msg, 0, sizeof(message_t));
        msg->msgType = personId;
        strcpy(msg->command, "NOT_AVAILABLE");
        msgsnd(msgid, msg, sizeof(message_t) - sizeof(long), 0);
    }
}

/***********************************************
* @Finalitat: Process incoming messages.
* @Parametres: msg = message structure, msgid = message queue ID.
************************************************/
void processMessage(message_t *msg, int msgid) {
    if (strcmp(msg->command, "REQUEST_TIMES") == 0) {
        handleRequestTimes(msg, msgid);
    } else if (strcmp(msg->command, "RESERVE") == 0) {
        handleReserve(msg, msgid);
    } else {
        printF(MSG_UNKNOWN_COMMAND);
    }
}

/***********************************************
* @Finalitat: Main function to initialize message queue and process signals.
************************************************/
int main() {
    int msgid;
    signal(SIGINT, handleSigint);

    /* Using ftok to generate the message queue key */
    key_t queueKey = ftok(QUEUE_PATH, QUEUE_ID);
    if (queueKey == -1) {
        perror("Error generating IPC key with ftok");
        return 1;
    }

    msgid = msgget(queueKey, 0666 | IPC_CREAT);
    if (msgid == -1) {
        printF(MSG_ERROR_MSGQ);
        return 1;
    }

    loadTimes();
    printF(MSG_ADMIN_START);

    message_t msg;

    while (running) {
        memset(&msg, 0, sizeof(message_t));

        // Receive message from person
        if (msgrcv(msgid, &msg, sizeof(message_t) - sizeof(long), 1, IPC_NOWAIT) != -1) {
            processMessage(&msg, msgid);
        }
    }

    // Remove the message queue upon termination
    msgctl(msgid, IPC_RMID, NULL);

    return 0;
}
