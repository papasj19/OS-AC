/* 
    Operating Systems: Lab 7: Select
    
    Developed by:
        - Guillermo Nebra Aljama    <guillermo.nebra>
        - Spencer Johnson           <spencerjames.johnson>
    Developed on: November 20th, 2024
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
//include for sockets

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define NUM_STATIONS 3
#define NUM_EVENTS 9

#define ARRIVAL_REPORT 1
#define DEPARTURE_REPORT 2
#define END_REPORT 3

#define RESET "\x1b[0m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define GREEN "\x1b[32m"

#define printF(x) write(1, x, strlen(x))


char *read_until(int fd, char end)
{
    char *string = NULL;
    char c;
    int i = 0, size;

    while (1)
    {
        size = read(fd, &c, sizeof(char));
        if (string == NULL)
        {
            string = (char *)malloc(sizeof(char));
        }
        if (c != end && size > 0)
        {
            string = (char *)realloc(string, sizeof(char) * (i + 2));
            string[i++] = c;
        }
        else
        {
            break;
        }
    }
    string[i] = '\0';
    return string;
}


void newUser(char* username){
    char * data = NULL; 
    //New user connected: %s
    asprintf(&data,"New user connected: %s",username);
    printF(data);
    free(data); 
}

int setup_server(const char *ip, int port) {
    printF("Opening connections...\n");
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0) {
        perror("Invalid IP address format");
        exit(EXIT_FAILURE);
    }

    if (bind(server_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

void doSearchWord(int connectFD){
    printF("User has requested search word command\n");
    write(connectFD, "Search word command\n", 20);
    char *choice = read_until(connectFD, '\n');
    printF(choice);
}



//useee 
int do_connection(int connectFD){

    char *username = read_until(connectFD, '\n');
    newUser(username);


    while(1){

        char *choice = read_until(connectFD, '\n');

        switch (atoi(choice))
        {
            case 1:
                doSearchWord(connectFD);

            break;

            case 2:
                printF("\nUser has requested add word command\n");

            break;

            case 3:
                printF("\nUser has requested list words command\n");
            break;

            case 4:
                return 100; 
            break;
        
            default:
                printF("\nprovide proper input please\n");
            break;
        }
    }

}





int main(int argc, char *argv[]) {
    if (argc != 4) {
        printF("\nError: Number of args is not 2. It is neccessary to provide an IP, port and filename\n");
        return 1;
    }

    printf("Dictionary Server Started\n");

    int server_fd = setup_server(argv[1], atoi(argv[2]));

    while (1) {
        printF("Waiting on new connections...\n");
        int newConnect = accept(server_fd, NULL, NULL);
        if (newConnect < 0) {
            perror("Accept failed");
            continue;
        }
        do_connection(newConnect);
        break;
    }


    printF("Hello world");


    return 0; 
}