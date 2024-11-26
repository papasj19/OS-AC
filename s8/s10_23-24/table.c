/*
* S10 Operating Systems - Message Queues
* Author: Marc Valsells Niubó
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>

#define printF(x) write(1, x, strlen(x));

#define HEADER_SIZE 9
#define PIZZA_SIZE 21
#define TYPE_OFFSET 500

typedef struct
{
    long mtype;
    char header[HEADER_SIZE];
    char pizzaName[PIZZA_SIZE];
    int pizzaQuantity;
} Msg;

char *read_until(int fd, char end)
{
    int i = 0, size;
    char c = '\0';
    char *string = (char *)malloc(sizeof(char));

    while (1)
    {
        size = read(fd, &c, sizeof(char));

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

Msg reciveMsg(int msqid, int type){
    Msg message;
    if (msqid < 0) {
        message.mtype = -1;
        return message;
    }

    ssize_t bytesRead = msgrcv(msqid, &message, sizeof(Msg)-sizeof(long), type+TYPE_OFFSET, 0);
    if (bytesRead == -1) {
        message.mtype = -1;
    }
    return message;
}

bool sendMsg(int msqid, long type, char *header, char *pizzaName, int pizzaQuantity){
    Msg message;

    if (msqid < 0) {
        return false;
    }

    //Save type
    if (type < 0) {
        return false;
    } else {
        message.mtype = type;
    }
    
    //Save header
    if (header == NULL || strlen(header) > HEADER_SIZE-1) {
        return false;
    } else {
        memset(message.header, ' ', HEADER_SIZE);
        strcpy(message.header, header);
    }

    //Save pizza
    if (pizzaName != NULL){
        if (strlen(pizzaName) > PIZZA_SIZE || pizzaQuantity <= 0) {
            return false;
        } else {
            memset(message.pizzaName, ' ', PIZZA_SIZE);
            strcpy(message.pizzaName, pizzaName);
            message.pizzaQuantity = pizzaQuantity;
        }
    } else {
        message.pizzaQuantity = 0;
        memset(message.pizzaName, ' ', PIZZA_SIZE);
        message.pizzaName[0] = '\0';
    }

    //Send message
    if (msgsnd(msqid, &message, sizeof(Msg)-sizeof(long), 0) == -1) {
        return false;
    }

    return true;
}

int main(int argc, char*argv[]){
    int msqid, tableNum, menuOption;
    key_t key;
    char *buffer, *pizzaName;
    Msg message;

    // Check arguments
    if (argc != 2)
    {
        printF("Please introduce the table number\n");
        exit(-1);
    }
    tableNum = atoi(argv[1]);

    // Create the message queue
    key = ftok("bar.c", 1234);
    msqid = msgget(key, 0666 | IPC_CREAT);
    if (msqid == -1)
    {
        printF("Error when creating the message queue\n");
        exit(-1);
    }

    printF("Wating to be able to make orders...\n");

    message = reciveMsg(msqid, tableNum);

    if (strcmp(message.header, "ORDER") == 0) {
        printF("You can make orders now\n");
    } else {
        printF("Error when processing a reciving message. Was expecting ORDER in header.\n");
        exit(-1);
    }

    do {
        asprintf(&buffer, "\n--- Menu ---\n1. Order a pizza\n2. Exit\nOption: ");
        printF(buffer);
        free(buffer);
        buffer = read_until(0, '\n');
        menuOption = atoi(buffer);
        free(buffer);
        switch (menuOption)
        {
        case 1:
            // Order Pizza
            asprintf(&buffer, "Introduce the pizza name (max chars: %d): ", PIZZA_SIZE-1);
            printF(buffer);
            free(buffer);
            pizzaName = read_until(0, '\n');
            asprintf(&buffer, "Introduce the pizza quantity: ");
            printF(buffer);
            free(buffer);
            buffer = read_until(0, '\n');
            if (!sendMsg(msqid, tableNum, "PIZZA", pizzaName, atoi(buffer))){
                printF("Error when sending message\n");
                exit(-1);
            };
            free(buffer);
            free(pizzaName);

            // Wait for pizza
            printF("Waiting for your pizzas...\n");
            message = reciveMsg(msqid, tableNum);
            if (message.mtype == -1) {
                printF("Error when reciving a message\n");
                exit(-1);
            }
            if (strcmp(message.header, "DELIVERY") != 0){
                printF("Error: Wrong header");
                exit(-1);
            } else {
                printF("Your pizzas are ready! Nyaaami\n");
            }

            break;
        case 2:
            if(!sendMsg(msqid, tableNum, "FINISH", NULL, 0)){
                printF("Error when sending message\n");
                exit(-1);
            }
            printF("Goodbye!\n");
            break;
        default:
            printF("Invalid option\n");
            break;
        }
    } while (menuOption != 2);

}