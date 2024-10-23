// Operating Systems Lab - Session 4
// Guillermo Nebra Aljama <guillermo.nebra>
// Spencer Johnson <spencerjames.johnson>


// Spencer's ports:     9655-9659
// Guillermo's ports:   9680-9684

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

// Shortcut for printing to stdout
#define printF(x) write(1, x, strlen(x))

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printF("Usage: ./g <server_ip> <port>\n");
        return 1;
    }


    


    return 0; 
}