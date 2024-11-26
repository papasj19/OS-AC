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

bool sendMsg(int msqid, long type, char *header, char *pizzaName, int pizzaQuantity){
    Msg message;

    if (msqid < 0) {
        return false;
    }

    //Save type
    if (type < 0) {
        return false;
    } else {
        message.mtype = type+TYPE_OFFSET;
    }
    
    //Save header
    if (header == NULL || strlen(header) > HEADER_SIZE) {
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
            strncpy(message.pizzaName, pizzaName, PIZZA_SIZE);
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

Msg reciveMsg(int msqid, int type){
    Msg message;
    if (msqid < 0) {
        message.mtype = -1;
        return message;
    }
    msgrcv(msqid, &message, sizeof(Msg)-sizeof(long), type, 0);
    return message;
}


int main(int argc, char *argv[])
{
    int msqid, numTables, tablesLeft, proccessTime;
    key_t key;
    char *buffer;
    bool open;
    // Check arguments
    if (argc != 2)
    {
        printF("Please introduce the number of tables\n");
        exit(-1);
    }
    numTables = atoi(argv[1]);
    tablesLeft = numTables;

    // Create the message queue
    key = ftok("bar.c", 1234);
    msqid = msgget(key, 0666 | IPC_CREAT);
    if (msqid == -1)
    {
        printF("Error when creating the message queue\n");
        exit(-1);
    }

    // Wait for tables to be ready
    printF("Press enter when the tables are ready to make orders\n");
    free(read_until(0, '\n'));

    // Tell tables to make orders
    for(int i = 1; i <= numTables; i++){
        if (!sendMsg(msqid, i, "ORDER", NULL, 0)) {
            asprintf(&buffer, "Error when sending message to table %d\n", i);
            printF(buffer);
            free(buffer);
        }
    }

    // Read orders from tables
    open = true;
    while (open)
    {
        Msg message;
        message = reciveMsg(msqid, -numTables);
        if (message.mtype == -1) {
            printF("Error when reciving a message\n");
            exit(-1);
        }

        if (strcmp(message.header, "PIZZA") == 0) {
            proccessTime = 3*message.pizzaQuantity;
            asprintf(&buffer, "Table %ld ordered %d %s pizzas\nTaking %d seconds to process them...\n", message.mtype, message.pizzaQuantity, message.pizzaName, proccessTime);
            printF(buffer);
            free(buffer);
            sleep(proccessTime);

            printF("Order processed\n");
            if(!sendMsg(msqid, message.mtype, "DELIVERY", NULL, 0)){
                asprintf(&buffer, "Error when sending message to table %ld\n", message.mtype);
                printF(buffer);
                free(buffer);
            }
        } else if (strcmp(message.header, "FINISH") == 0) {
            tablesLeft--;
            asprintf(&buffer, "Table %ld has finished\n", message.mtype);
            printF(buffer);
            free(buffer);
            if (tablesLeft == 0) {
                printF("All tables have finished, closing the bar...\n");
                open = false;
            }
        } else {
            printF("Error: Wrong header\n");
            exit(-1);
        }
    }
    
    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        printF("Error when deleting the message queue\n");
        exit(-1);
    }
    return 0;
}