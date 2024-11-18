/* 
    Operating Systems: Lab 6: Pipes & Shared Memory
    
    Developed by:
        - Guillermo Nebra Aljama    <guillermo.nebra>
        - Spencer Johnson           <spencerjames.johnson>
    Developed on: November 13th, 2024
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




int main(void){


    printF("Hello world");


    return 0; 
}