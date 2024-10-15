#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

void printReady(){
    char buffer[100];
    sprintf(buffer, "Section is ready to start\n");
    write(STDOUT_FILENO, buffer, strlen(buffer));
}

void printStrings(){
    char buffer[100];
    sprintf(buffer, "Strings Viola is playing: Do");
    write(STDOUT_FILENO, buffer, strlen(buffer));
    sprintf(buffer, "Strings Violin 2 is playing: Re");
    write(STDOUT_FILENO, buffer, strlen(buffer));
    sprintf(buffer, "Strings Violin 1 is playing: re");
    write(STDOUT_FILENO, buffer, strlen(buffer));
}

void printWind(){

}

void printPercussion(){

}


void signalHandler(int sig) {
        switch (sig) {
        case SIGUSR1:
            // Action 1
            secondSignalHandler(sig);
            break;
        case SIGALRM:
            solarInterference();
            break;
        }
}




int main(void){
    char buffer[100];
    int pid = getpid(); 

     for (int i = 0; i < 32; i++) {
        if (i != SIGKILL && i != SIGSTOP) {
            signal(i, signalHandler); // SIGKILL and SIGSTOP can't be reassigned
        }
    }

    sprintf(buffer, "Director (PID %d) starting the concert. Use 'kill -SIGUSR PID' to start sections", pid);
    write(STDOUT_FILENO, buffer, strlen(buffer));
    printReady();






    return 0; 
}