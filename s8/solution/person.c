/*
* Solution S8 Operating Systems - Queue
* Curs 2024-25
*
* @author: Ferran Castañé
*/

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>

/* Constants for print messages */
#define MSG_PERSON_TERMINATED "\nPerson process terminated.\n"
#define MSG_ENTER_DNI "Please enter your DNI: "
#define MSG_DNI_VALID "DNI is valid.\n"
#define MSG_DNI_INVALID "Invalid DNI. Please try again.\n"
#define MSG_ERROR_READING_DNI "Error reading DNI.\n"
#define MSG_REQUESTING_TIMES "Requesting available times...\n"
#define MSG_ERROR_SENDING_MSG "Error sending message.\n"
#define MSG_ERROR_RECEIVING_MSG "Error receiving message.\n"
#define MSG_ERROR_PARSING_TIMES "Error parsing available times.\n"
#define MSG_AVAILABLE_TIMES "Available times:\n"
#define MSG_SELECT_TIME "Select a time slot: "
#define MSG_ERROR_READING_CHOICE "Error reading your choice.\n"
#define MSG_INVALID_SELECTION "Invalid selection.\n"
#define MSG_RESERVING_APPOINTMENT "\nReserving appointment at "
#define MSG_APPOINTMENT_CONFIRMED "Appointment successfully reserved.\nThank you!\n"
#define MSG_APPOINTMENT_NOT_CONFIRMED "Could not reserve the appointment. Please try again.\n"
#define MSG_ERROR_OBTAINING_MSGQ "Error obtaining message queue.\n"

/* Function-like macro for printing */
#define printF(x) write(1, x, strlen(x))

/* Define dynamic message queue key */
#define QUEUE_PATH "/tmp"
#define QUEUE_ID 'Q'

/* Structure for messages */
typedef struct message {
    long msg_type;
    int person_id;
    char time[6];      // For single time slots like "09:00"
    char command[20];
    char text[256];    // For variable-length messages
} message_t;

/* Global variables */
int running = 1;

/***********************************************
* @Finalitat: Read input from user until a specific character is encountered.
* @Parametres: fd = file descriptor, cEnd = termination character.
* @Retorn: Returns the user input string.
************************************************/
char *readUntil(int fd, char cEnd) {
    int i = 0;
    char c;
    char *buffer = NULL;

    while (read(fd, &c, sizeof(char)) > 0 && c != cEnd) {
        char *new_buffer = realloc(buffer, i + 2);
        if (new_buffer == NULL) {
            free(buffer);
            return NULL;
        }
        buffer = new_buffer;
        buffer[i++] = c;
    }

    if (buffer != NULL) {
        buffer[i] = '\0'; // Null-terminate the buffer
    }

    return buffer;
}

/***********************************************
* @Finalitat: Split a string into lines.
* @Parametres: text = string to split, num_lines = pointer to store the number of lines.
* @Retorn: Returns a list of lines.
************************************************/
char **splitLines(const char *text, int *num_lines) {
    char **lines = NULL;
    int count = 0;
    char *copy = strdup(text);
    char *line = strtok(copy, "\n");

    while (line != NULL) {
        lines = realloc(lines, sizeof(char *) * (count + 1));
        lines[count++] = strdup(line);
        line = strtok(NULL, "\n");
    }
    free(copy);
    *num_lines = count;
    return lines;
}

/***********************************************
* @Finalitat: Signal handler for graceful termination.
************************************************/
void handleSigint() {
    printF(MSG_PERSON_TERMINATED);
    running = 0;
}

/***********************************************
* @Finalitat: Validate the DNI entered by the user.
* @Parametres: dni = the DNI to validate.
* @Retorn: Returns 1 if valid, 0 if invalid.
************************************************/
int validateDni(char *dni) {
    const char *dni_letters = "TRWAGMYFPDXBNJZSQVHLCKE";
    if (strlen(dni) != 9) {
        return 0;
    }
    for (int i = 0; i < 8; i++) {
        if (!isdigit(dni[i])) {
            return 0;
        }
    }
    char letter = toupper(dni[8]);
    if (letter < 'A' || letter > 'Z') {
        return 0;
    }
    char dni_numbers[9] = {0};
    strncpy(dni_numbers, dni, 8);
    int number = atoi(dni_numbers);
    char expected_letter = dni_letters[number % 23];
    return letter == expected_letter;
}

/***********************************************
* @Finalitat: Extract person_id from DNI.
* @Parametres: dni = the DNI from which to extract the ID.
* @Retorn: Returns the person_id extracted from DNI.
************************************************/
int getPersonId(char *dni) {
    char dni_numbers[9];
    strncpy(dni_numbers, dni, 8);
    dni_numbers[8] = '\0';
    return atoi(dni_numbers);
}

/***********************************************
* @Finalitat: Request available times from administration.
* @Parametres: msg_id = message queue ID, person_id = ID of the person.
************************************************/
void requestAvailableTimes(int msg_id, int person_id) {
    printF(MSG_REQUESTING_TIMES);

    message_t msg;
    memset(&msg, 0, sizeof(message_t));
    msg.msg_type = 1;
    msg.person_id = person_id;
    strcpy(msg.command, "REQUEST_TIMES");
    if (msgsnd(msg_id, &msg, sizeof(message_t) - sizeof(long), 0) == -1) {
        perror("Error sending message");
        printF(MSG_ERROR_SENDING_MSG);
    }
}
/***********************************************
* @Finalitat: Display available times to the user.
* @Parametres: times_text = available times, num_lines = number of lines, times_list = list of times.
************************************************/
void displayAvailableTimes(char *times_text, int *num_lines, char ***times_list) {
    *times_list = splitLines(times_text, num_lines);
    if (*times_list == NULL) {
        printF(MSG_ERROR_PARSING_TIMES);
        return;
    }

    printF(MSG_AVAILABLE_TIMES);
    for (int i = 0; i < *num_lines; i++) {
        printF((*times_list)[i]);
        printF("\n");
    }
}

/***********************************************
* @Finalitat: Allow user to select a time slot.
* @Parametres: times_list = list of available times, num_lines = number of times, selected_time = selected time.
* @Retorn: Returns 0 on success, -1 on failure.
************************************************/
int selectTimeSlot(char **times_list, int num_lines, char **selected_time) {
    printF(MSG_SELECT_TIME);
    char *choice = readUntil(0, '\n');
    if (choice == NULL) {
        printF(MSG_ERROR_READING_CHOICE);
        return -1;
    }

    int index = atoi(choice) - 1; // Convert choice to 0-based index
    free(choice);

    if (index < 0 || index >= num_lines) {
        printF(MSG_INVALID_SELECTION);
        return -1;
    }

    // Extract the time from the selected line. We assume the time starts after the first ") ".
    char *time_str = strchr(times_list[index], ')');
    if (time_str != NULL) {
        time_str += 2; // Move past ") " to the actual time part
    } else {
        time_str = times_list[index]; // If no ") ", use the entire string
    }

    *selected_time = strdup(time_str);

    printF(MSG_RESERVING_APPOINTMENT);
    printF(*selected_time);
    printF("\n");
    return 0;
}

/***********************************************
* @Finalitat: Reserve the selected appointment.
* @Parametres: msg_id = message queue ID, person_id = ID of the person, time_str = selected time.
************************************************/
void reserveAppointment(int msg_id, int person_id, char *time_str) {
    message_t msg;
    memset(&msg, 0, sizeof(message_t));
    msg.msg_type = 1;
    msg.person_id = person_id;
    strcpy(msg.command, "RESERVE");
    strncpy(msg.time, time_str, sizeof(msg.time) - 1);
    msg.time[sizeof(msg.time) - 1] = '\0';

    if (msgsnd(msg_id, &msg, sizeof(message_t) - sizeof(long), 0) == -1) {
        perror("Error sending message");
        printF(MSG_ERROR_SENDING_MSG);
    }
}

/***********************************************
* @Finalitat: Receive confirmation from administration.
* @Parametres: msg_id = message queue ID, person_id = ID of the person.
************************************************/
void receiveConfirmation(int msg_id, int person_id) {
    message_t msg;
    memset(&msg, 0, sizeof(message_t));

    if (msgrcv(msg_id, &msg, sizeof(message_t) - sizeof(long), person_id, 0) == -1) {
        printF(MSG_ERROR_RECEIVING_MSG);
        return;
    }

    if (strcmp(msg.command, "CONFIRMED") == 0) {
        printF(MSG_APPOINTMENT_CONFIRMED);
        running = 0;
    } else {
        printF(MSG_APPOINTMENT_NOT_CONFIRMED);
    }
}

/***********************************************
 * @Finalitat: Main function for the person process.
************************************************/
int main() {
    signal(SIGINT, handleSigint);

    key_t queue_key = ftok(QUEUE_PATH, QUEUE_ID);
    if (queue_key == -1) {
        printF(MSG_ERROR_OBTAINING_MSGQ);
        return 1;
    }

    int msg_id = msgget(queue_key, 0666);
    if (msg_id == -1) {
        printF(MSG_ERROR_OBTAINING_MSGQ);
        return 1;
    }

    char *dni = NULL;
    int dni_valid = 0;
    while (!dni_valid) {
        printF(MSG_ENTER_DNI);
        dni = readUntil(0, '\n');
        if (dni == NULL) {
            printF(MSG_ERROR_READING_DNI);
            continue;
        }
        if (validateDni(dni)) {
            printF(MSG_DNI_VALID);
            dni_valid = 1;
        } else {
            printF(MSG_DNI_INVALID);
            free(dni);
            dni = NULL;
        }
    }

    int person_id = getPersonId(dni);
    free(dni);

    while (running) {
        requestAvailableTimes(msg_id, person_id);

        message_t msg;
        memset(&msg, 0, sizeof(message_t));

        // Receive available times
        if (msgrcv(msg_id, &msg, sizeof(message_t) - sizeof(long), person_id, 0) == -1) {
            printF(MSG_ERROR_RECEIVING_MSG);
            continue;
        }

        char **times_list = NULL;
        int num_lines = 0;
        displayAvailableTimes(msg.text, &num_lines, &times_list);

        char *selected_time = NULL;
        if (selectTimeSlot(times_list, num_lines, &selected_time) == -1) {
            // Free times_list
            for (int i = 0; i < num_lines; i++) {
                free(times_list[i]);
            }
            free(times_list);
            continue;
        }

        reserveAppointment(msg_id, person_id, selected_time);

        // Free times_list and selected_time after use
        for (int i = 0; i < num_lines; i++) {
            free(times_list[i]);
        }
        free(times_list);
        free(selected_time);

        receiveConfirmation(msg_id, person_id);
    }

    return 0;
}
