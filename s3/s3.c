// Operating Systems Lab - Session 3. Forks
// Guillermo Nebra Aljama guillermo.nebra
// Spencer Johnson spencerjames.johnson

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

// In case we lazy
#define customWrite(x) write(1, x, strlen(x))


// used to read from file
char *read_until(int fd, char end) {
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

// ignore the sigint
/*
void sigint_handler(int signo) {
	return;
}
*/

int main(int argc, char *argv[]) {
    if (argc != 2) {
        customWrite("Usage: ./g <file>\n");
        return 1;
    }

    float *data = NULL;
    int size = 0;

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        customWrite("Error opening file\n");
        exit(1);
    }

    while (1) {
        char *line = read_until(fd, '\n');
        if (line[0] == '\0') {
            free(line);
            break;
        }

        data = (float *)realloc(data, sizeof(float) * (size + 1));
        data[size++] = atof(line);  // Store the float value
        free(line);
    }

    close(fd);  // Close the file descriptor











	// DO NOT REMOVE!! WE NEED TO FREEEEEEE
	free(data);

    return 0;
}
