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

#define NUM_STATIONS 3
#define NUM_EVENTS 9

#define ARRIVAL_REPORT 1
#define DEPARTURE_REPORT 2
#define END_REPORT 3

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


void doSomething(int messageNumber){

    switch(messageNumber){
        case 1:
            printF("Enter num1:");
            break;
        case 2:
            printF("Enter num2:");
            break;
        case 3:
            printF("Sending petition...\n");
            break;
        case 4:
            printF("Program finished\n\n");
            break;
        case 5:
            printF("Received: ");
            break;
        default:
            break;
    }
}



int main(){
    key_t key; int id_queue; Message1 M1; Message2 M2;


    // file and integer (must be the same for all )
    key = ftok("EnterNumbers.c", 12);
    if (key == (key_t)-1){ 
        printf("Error: Key\n\n"); exit(-1);
    }

    //create the queue
    id_queue = msgget(key, 0600|IPC_CREAT);
    if(id_queue == -1){
        printf("Error creating queue\n\n"); exit(-1);
    }

    M1.id_msg = 1;
    M1.n1 = 1;
    
    while(M1.n1 != 0){

        //rec info
        printf("Enter num1:");
        scanf("%d", &M1.n1);
        printf("Enter num2:");
        scanf("%d", &M1.n2);

        //send info in message struct 
        msgsnd(id_queue, (struct msgbuf *)&M1, sizeof(int)*2,IPC_NOWAIT);
        printf("Sending petition...\n"); 
        if (M1.n1 != 0){

            //rec message from other process 
            msgrcv(id_queue, (struct msgbuf *)&M2, sizeof(int), 2, 0);
            printf("%d+%d = %d\n\n", M1.n1, M1.n2, M2.result); 

            //do somethign with result 
            doSomething(M2.result);
        }
    }
    printf("Program finished\n\n");

    //close queue
    msgctl(id_queue, IPC_RMID, (struct msqid_ds *)NULL);

    return 0;
}