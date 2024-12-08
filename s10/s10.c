/*
    OPERATING SYSTEMS: SESSION 9
    Guillermo Nebra Aljama <guillermo.nebra>
    Spencer Johnson <spencerjames.johnson>

    DISCLAIMER: 
    The error message shown when compiling the code is related to a conflict with the _GNU_SOURCE macro and the provided semaphore library.
    Aaron told us to write this disclaimer, as the error originates from "semaphore_v2.h" and not from the code itself :)
    Pease be kind and ignore the error message. 
    
    Thank you!
        - Guillermo and Spencer

*/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

#define printF(x) write(1, x, strlen(x))


