// Operating Systems Lab - Session 4
// Guillermo Nebra Aljama <guillermo.nebra>
// Spencer Johnson <spencerjames.johnson>

//9655-9659 my ports
#define _GNU_SOURCE
#define _XOPEN_SOURCE 500
#define _POSIX_C_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>



int main(int argc, char *argv[]) {
    if (argc != 2) {
        customWrite("Usage: ./g <file>\n");
        return 1;
    }


    


    return 0; 
}