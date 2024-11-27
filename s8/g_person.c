/**
 * S8_person.c
 * Authors: Guillermo and Spencer Johnson
 * Session: 8
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_DNI_LEN 10
#define MSG_SIZE 256

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
    char dni[MAX_DNI_LEN];
    while (1) {
        write(STDOUT_FILENO, "Enter DNI (8 digits + letter): ", 31);
        read(STDIN_FILENO, dni, MAX_DNI_LEN);
        dni[strcspn(dni, "\n")] = 0; // Remove newline
        if (validate_dni(dni)) break;
        write(STDOUT_FILENO, "Invalid DNI. Try again.\n", 24);
    }

    key_t key = ftok("S8_administration.c", 65);
    int msgid = msgget(key, 0666 | IPC_CREAT);

    sendMsg(msgid, REQUEST_TIMES, "REQUEST_TIMES", NULL);

    Message response;
    write(STDOUT_FILENO, "Available hours:\n", 18);
    while (1) {
        receiveMsg(msgid, REQUEST_TIMES, &response);
        if (strcmp(response.text, "END") == 0) break;
        write(STDOUT_FILENO, response.text, strlen(response.text));
        write(STDOUT_FILENO, "\n", 1);
    }

    char selected_hour[6];
    write(STDOUT_FILENO, "Select an hour: ", 16);
    read(STDIN_FILENO, selected_hour, 6);
    selected_hour[strcspn(selected_hour, "\n")] = 0;

    sendMsg(msgid, RESERVE, "RESERVE", selected_hour);

    receiveMsg(msgid, 0, &response);
    if (strcmp(response.text, "CONFIRMED") == 0) {
        write(STDOUT_FILENO, "Appointment confirmed.\n", 23);
    } else {
        write(STDOUT_FILENO, "Appointment unavailable.\n", 25);
    }

    return 0;
}
