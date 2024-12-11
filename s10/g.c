/*
    OPERATING SYSTEMS: SESSION 10
    Guillermo Nebra Aljama <guillermo.nebra>
    Spencer Johnson <spencerjames.johnson>



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

