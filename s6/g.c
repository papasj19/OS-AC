
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







    return 0; 

}


