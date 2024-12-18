/*
    OPERATING SYSTEMS: SESSION 10
    Guillermo Nebra Aljama <guillermo.nebra>
    Spencer Johnson <spencerjames.johnson>

    DISCLAIMER: 
    As in the previous session, the error message shown when compiling the code is related to a conflict with the _GNU_SOURCE macro and the provided semaphore library.
    Aaron told us to write this disclaimer, as the error originates from "semaphore_v2.h" and not from the code itself :)
    Pease be kind and ignore the error message. 
    
    Thank you!
        - Guillermo and Spencer

*/


#define _GNU_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#define Write(x) write(1, x, strlen(x))
#define NUM_STATIONS 5
#define MAX_CARS_IN_PROCESS 2

#define ERR_ARGS "Usage: ./S10.exe <number of cars>\n"
#define STATION_START "\033[0;32m%s of car %d starting\n\033[0m"
#define STATION_COMPLETE "\033[0;34m%s of car %d assembled\n\033[0m"
#define QC_CHECK "\033[0;33mQuality control of car %d at %s \033[0m"
#define QC_STATUS "\033[0;%dm%s\033[0m\n"
#define QC_PASSED "\033[0;32mQuality control of car %d passed\n\033[0m"




int main(void){



    
    return 0;
}