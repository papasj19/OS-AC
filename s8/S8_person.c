/**
 * S8_person.c
 * Authors: Guillermo and Spencer Johnson
 * Session: 8
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_DNI_LEN 10
#define MSG_SIZE 256

#define printF(x) write(STDOUT_FILENO, x, strlen(x));

const char *dni_letters = "TRWAGMYFPDXBNJZSQVHLCKE";

#define REQUEST_TIMES 1
#define RESERVE 2
#define CONFIRMED 3
#define UNAVAILABLE 4

typedef struct {
    long mtype;
    char text[MSG_SIZE];
    char hour[6];
} Message;

// Sends a message to the message queue
void sendMsg(int msgid, long mtype, const char *text, const char *hour) {
    Message msg;
    msg.mtype = mtype;
    strncpy(msg.text, text, MSG_SIZE);
    if (hour != NULL) strncpy(msg.hour, hour, 6);
    msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0);
}

// Receives a message from the message queue
void receiveMsg(int msgid, long mtype, Message *msg) {
    msgrcv(msgid, msg, sizeof(*msg) - sizeof(long), mtype, 0);
}

// Validates the format and checksum of a DNI
int validate_dni(const char *dni) {
    if (strlen(dni) != 9) return 0;

    for (int i = 0; i < 8; i++) {
        if (!isdigit(dni[i])) return 0;
    }

    int num = atoi(dni);
    char letter = toupper(dni[8]);
    return letter == dni_letters[num % 23];
}

int main() {
    char *buffer;
    char dni[MAX_DNI_LEN];
    
    // DNI validation loop
    while (1) {
        asprintf(&buffer, "Please enter your DNI: ");
        printF(buffer);
        free(buffer);

        read(STDIN_FILENO, dni, MAX_DNI_LEN);
        dni[strcspn(dni, "\n")] = 0; // Remove newline

        if (validate_dni(dni)) {
            break;
        } else {
            asprintf(&buffer, "Invalid DNI. Please try again.\n");
            printF(buffer);
            free(buffer);
        }
    }

    // Initialize message queue
    key_t key = ftok("S8_administration.c", 65);
    int msgid = msgget(key, 0666 | IPC_CREAT);

    Message response;

    while (1) {
        // Request available times
        sendMsg(msgid, REQUEST_TIMES, "REQUEST_TIMES", NULL);

        asprintf(&buffer, "Available times:\n");
        printF(buffer);
        free(buffer);

        while (1) {
            receiveMsg(msgid, REQUEST_TIMES, &response);
            if (strcmp(response.text, "END") == 0) break;
            printF(response.text);
            printF("\n");
        }

        char selected_hour[6];
        asprintf(&buffer, "Select a time slot: ");
        printF(buffer);
        free(buffer);

        read(STDIN_FILENO, selected_hour, 6);
        selected_hour[strcspn(selected_hour, "\n")] = 0;

        asprintf(&buffer, "Reserving appointment at %s...\n", selected_hour);
        printF(buffer);
        free(buffer);

        // Attempt to reserve appointment
        sendMsg(msgid, RESERVE, "RESERVE", selected_hour);
        receiveMsg(msgid, 0, &response);

        if (strcmp(response.text, "CONFIRMED") == 0) {
            asprintf(&buffer, "Appointment successfully reserved. Thank you!\n");
            printF(buffer);
            free(buffer);
            break;
        } else {
            asprintf(&buffer, "Could not reserve the appointment. Please try again.\n\n");
            printF(buffer);
            free(buffer);
        }
    }

    return 0;
}
