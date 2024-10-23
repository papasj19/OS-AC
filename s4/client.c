// Operating Systems Lab - Session 4
// Guillermo Nebra Aljama <guillermo.nebra>
// Spencer Johnson <spencerjames.johnson>

//9655-9659 my ports

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_IP2 "192.168.1.4" 
#define SERVER_IP "172.16.205.4" 
#define PORT 9655

#define UNEXPECTED_FRAME "Recived an unexpected frame from server, exiting\n"


int serverFd;

char * read_until(int fd, char end) {
	char *string = NULL;
	char c;
	int i = 0, size;

	while (1) {
		size = read(fd, &c, sizeof(char));
		if(string == NULL){
			string = (char *) malloc(sizeof(char));
		}
		if(c != end && size > 0){
			string = (char *) realloc(string, sizeof(char)*(i + 2));
			string[i++] = c;
		}else{
			break;
		}
	}
	string[i] = '\0';
	return string;
}

void sigintHandler()
{
    // Do nothing
    signal(SIGINT, sigintHandler);
}





int connectToServer(char *address, int port)
{
    struct sockaddr_in server;
    // Create socket
    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd == -1)
    {
        printF("Error creating the socket!\n");
        exit(-2);
    }

    server.sin_addr.s_addr = inet_addr(address);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    // Connect to remote server
    if (connect(serverFd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printF("Error connecting to the server!\n");
        close(serverFd);
        exit(-2);
    }

    return serverFd;
}






int main(int argc, char *argv[])
{

    int i, j;

   
    char *buffer, *frame;
    

    if (argc != 3)
    {
        printF("Number of parameters incorrect!\n");
        return -1;
    }

    signal(SIGINT, sigintHandler);

    serverFd = connectToServer(argv[1], atoi(argv[2]));

    newPlayer(serverFd);

    // Wait for StartGame
    printF("Waiting to start the game...\n");
    frame = read_until(serverFd, '#');
    if (strcmp(frame, "F") != 0)
    {
        handleUnexpectedFrame(frame);
    }
    free(frame);







    // Start parsing commands from the server
    int flag = 0; 
    do
    {
        frame = read_until(serverFd, '#');
        if (strcmp(frame, "YT") == 0)
        {
            //yourTurn(board, isMyPieceX);
            printf("Hi\n");
        }
        else if (strcmp(frame, "NP") == 0)
        {
            //newPiece(board, isMyPieceX);
            printf("hello there");
        }
        else if (strcmp(frame, "X") == 0)
        {
            free(frame);
            frame = read_until(serverFd, '#');
            asprintf(&buffer, "The winner/s is/are %s!\n", frame);
            printF(buffer);
            free(buffer);
            flag = 1;
        }
        else
        {
            handleUnexpectedFrame(frame);
        }
        free(frame);

    } while (!flag);
    close(serverFd);
}
