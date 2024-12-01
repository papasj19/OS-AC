/*
*
* Solution S8 Operating Systems - Semaphores I
* 2023-24
*
* @Author: Alejandro Navarro-Soto
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include "semaphore_v2.h"

#define BOLDRED "\033[1m\033[31m"
#define RESET   "\033[0m"

#define FILE_NAME   "scientists.txt"
#define MAXTEAMS    5
#define MAXLEVELS   5

#define FILE_ERROR BOLDRED"Can't open scientists file" RESET
#define surface "Surface"
#define level1 "Level 1"
#define level2 "Level 2"
#define level3 "Level 3"
#define level4 "Level 4"
#define maxDepth "Max depth"

#define printF(x) write(1, x, strlen(x))

typedef struct {
    semaphore mtx;  // Use the semaphore type from semaphore_v2.h
  int visited;
} Level;

semaphore screen_mtx;  // Use a semaphore for screen_mtx
char* teamName[MAXTEAMS];
Level levels[MAXLEVELS];

char* readUntil(int fd, char endChar) {
    int i = 0;
    char c = '0';
    char* buffer = (char*)malloc(1); // Initialize with size 1

    while (c != endChar) {
        read(fd, &c, sizeof(char));
        if (c != endChar) {
            buffer[i] = c;
            buffer = (char*)realloc(buffer, (i + 2)); // Resize the buffer
        }
        i++;
    }
    buffer[i] = '\0'; // Terminate the string properly
    return buffer;
}


char* returnLevel(int x){
    switch (x) {
        case 0:
            return surface;
        case 1:
            return level1;
        case 2:
            return level2;
        case 3:
            return level3;
        case 4:
            return level4;
        default:
            return maxDepth;
    }
}

void readTeams(int fd) {
    int i;
    for(i = 0; i < MAXTEAMS; i++) {
        teamName[i] = readUntil(fd, '\n');
    }
    close(fd);
}

void* ExplorationSimulation(void *arg) {
    int t = *((int*) arg);
    int i;
    for (i = 0; i < MAXLEVELS; i++) {
        SEM_wait(&levels[i].mtx); 
        if (!levels[i].visited) {
            sleep(2);
            levels[i].visited = 1;

            char* str;
            asprintf(&str, "Team %s discovered %s!\n", teamName[t], returnLevel(i));
            SEM_wait(&screen_mtx);  
            printF(str);
            SEM_signal(&screen_mtx); 
            SEM_signal(&levels[i].mtx);  
            free(str);
        } else {
            char* str;
            asprintf(&str, "Team %s reached %s\n", teamName[t], returnLevel(i));
            sleep(1);
            SEM_wait(&screen_mtx); 
            printF(str);
            SEM_signal(&screen_mtx);  
            SEM_signal(&levels[i].mtx);  
            free(str);
        }
    }
    return NULL;
}

void SubmarineCompetition(pthread_t thread_ids[MAXTEAMS]) {
    int i;
    int thread_args[MAXTEAMS]; // An array to hold thread-specific arguments

    for (i = 0; i < MAXTEAMS; i++) {
        thread_args[i] = i; // Set a unique value for each thread
        pthread_create(&thread_ids[i], NULL, ExplorationSimulation, (void*) &thread_args[i]);
    }

    for (i = 0; i < MAXTEAMS; i++) {
        pthread_join(thread_ids[i], NULL);
    }
}


int main() {
    int fd;
    int i;
    pthread_t thread_ids[MAXTEAMS];

    fd = open(FILE_NAME, O_RDONLY);
    if (fd < 0) {
        perror(FILE_ERROR);
        return -1;
    }

    readTeams(fd);

    SEM_constructor_with_name(&screen_mtx, ftok("race.c", 1));  // Initialize the screen mutex as a semaphore
    SEM_signal(&screen_mtx); 

    for(i = 0; i < MAXLEVELS; i++){
        SEM_constructor_with_name(&levels[i].mtx, ftok("race.c", 2 + i));  // Initialize each level mutex as a semaphore
        levels[i].visited = 0;
        SEM_signal(&levels[i].mtx);  
    }


    SubmarineCompetition(thread_ids);

    for(i = 0; i < MAXTEAMS; i++){
        SEM_destructor(&levels[i].mtx);  // Use SEM_destructor to destroy each level semaphore
    }

    SEM_destructor(&screen_mtx);  // Destroy the screen mutex semaphore

    for (i = 0; i < MAXTEAMS; i++){
        free(teamName[i]);
    }

    return 0;
}
