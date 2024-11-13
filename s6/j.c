
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