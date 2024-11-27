/* 
    Operating Systems: Lab 7: Select
    
    Developed by:
        - Guillermo Nebra Aljama    <guillermo.nebra>
        - Spencer Johnson           <spencerjames.johnson>
    Developed on: November 20th, 2024
    :)
*/

#define printF(x) write(1, x, strlen(x))


#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#define REQUEST_TIMES 1
#define RESERVE 2
#define CONFIRMED 3
#define NOT_AVAILABLE 4

#define TOT 8

const char *time_slots[TOT] = {"09:00", "10:00", "11:00", "12:00", "13:00", "14:00", "15:00", "16:00"};
int bookings[TOT] = {0};


#define RESET "\x1b[0m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define GREEN "\x1b[32m"

#define printF(x) write(1, x, strlen(x))

typedef struct{
    long id_msg;
    int n1,n2; 
}Message1;


typedef struct{
    long id_msg;
    int result; 
} Message2;

struct msg {
    long mtype;
    char header[20];
    char data[256];
};


char *read_until(int fd, char end)
{
    char *string = NULL;
    char c;
    int i = 0, size;

    while (1)
    {
        size = read(fd, &c, sizeof(char));
        if (string == NULL)
        {
            string = (char *)malloc(sizeof(char));
        }
        if (c != end && size > 0)
        {
            string = (char *)realloc(string, sizeof(char) * (i + 2));
            string[i++] = c;
        }
        else
        {
            break;
        }
    }
    string[i] = '\0';
    return string;
}


void load_appointments(const char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return; 
    }
    if (read(fd, bookings, sizeof(bookings)) != sizeof(bookings)) {
        perror("Error reading file");
    }
    close(fd);
}


void save_appointments(const char *filename) {
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        perror("Error opening file");
        return;
    }
    if (write(fd, bookings, sizeof(bookings)) != sizeof(bookings)) {
        perror("Error writing file");
    }
    close(fd);
}

void reserveTime(struct msg *mess, int queue_num){
    printF("Reservation request received.\n");
    int slot_found = -1;

    for (int i = 0; i < TOT; i++) {
        if (strcmp(mess->data, time_slots[i]) == 0) {
            slot_found = i;
            break;
        }
    }

    if (slot_found != -1 && bookings[slot_found] < 2) {
        bookings[slot_found]++;


           char *reservation_info = NULL;
        asprintf(&reservation_info, "Person %s requested appointment at %s.\n", 
                 mess->header, time_slots[slot_found]);
        char *reserved_msg = NULL;
        asprintf(&reserved_msg, "Time reserved.\n");

        // Output the dynamically created strings
        if (reservation_info) {
            printF(reservation_info);
            free(reservation_info);
        }
        if (reserved_msg) {
            printF(reserved_msg);
            free(reserved_msg);
        }



        mess->mtype = CONFIRMED;
        strcpy(mess->data, "CONFIRMED");
    } else {
        mess->mtype = NOT_AVAILABLE;
        strcpy(mess->data, "NOT_AVAILABLE");
    }

    if (msgsnd(queue_num, mess, sizeof(*mess) - sizeof(long), 0) == -1) {
        perror("Error sending response");
    }
}

void requestTimes(struct msg *message, int queueNum) {
    printF("Requesting available times...\n");
    char response[256] = "";

    for (int i = 0; i < TOT; i++) {
        if (bookings[i] < 2) { 
            strcat(response, time_slots[i]);
            strcat(response, " ");
        }
    }
    message->mtype = REQUEST_TIMES;
    strcpy(message->data, response);

    if (msgsnd(queueNum, message, sizeof(*message) - sizeof(long), 0) == -1) {
        perror("Error sending response");
    }
}


void doThings(int queue_num){
    struct msg message;

    while (1) {
        if (msgrcv(queue_num, &message, sizeof(message) - sizeof(long), 0, 0) == -1) {
            perror("Error receiving message");
            continue;
        }

        if (strcmp(message.header, "REQUEST_TIMES") == 0) {
            requestTimes(&message, queue_num);
        } else if (strcmp(message.header, "RESERVE") == 0) {
            reserveTime(&message, queue_num);
        }
    }
}




int main(){
    key_t key; int id_queue; Message1 M1; Message2 M2;

    printF("Adminstration process started\n");

    printF("Requesting available times...\n");

    load_appointments("appointments.dat");

    key = ftok("S8_administration.c", 12);
    if (key == (key_t)-1){ 
        printf("Error: Key\n\n"); exit(-1);
    }

    //create the queue
    id_queue = msgget(key, 0600|IPC_CREAT);
    if(id_queue == -1){
        printf("Error creating queue\n\n"); exit(-1);
    }


    doThings(id_queue);




    //close queue
    msgctl(id_queue, IPC_RMID, (struct msqid_ds *)NULL);

    return 0;
}