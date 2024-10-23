/*
 * Solution S4 Operating Systems - Sockets 1
 * Year 2023-24
 *
 * @author: Marc Valsells Niubó
 *
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <signal.h>

#define printF(x) write(1, x, strlen(x))
#define UNEXPECTED_FRAME "Recived an unexpected frame from server, exiting\n"

#define BOARD_COLUMNS 3
#define BOARD_ROWS 3
#define BOARD_HORITZONTAL_LINE "· ·---·---·---·\n"
#define BOARD_TOP_LINE "  · A · B · C ·\n"

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

void handleUnexpectedFrame(char *frame)
{
    printF(UNEXPECTED_FRAME);
    free(frame);
    close(serverFd);
    exit(-3);
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

void newPlayer()
{
    char *buffer, *frame;
    // Request to play to the server

    printF("Enter your username: ");
    buffer = read_until(0, '\n');

    asprintf(&frame, "NP#%s#", buffer);
    free(buffer);
    write(serverFd, frame, strlen(frame));
    free(frame);

    // Check if a NP frame is recived
    frame = read_until(serverFd, '#');
    if (strcmp(frame, "NP") != 0)
    {
        handleUnexpectedFrame(frame);
    }
    free(frame);

    // Check if the username is accepted
    frame = read_until(serverFd, '#');
    if (strcmp(frame, "OK") != 0)
    {
       handleUnexpectedFrame(frame);
    }
    free(frame);
}

void printBoard(char board[BOARD_COLUMNS][BOARD_ROWS])
{
    int i;
    char *buffer;
    printF(BOARD_TOP_LINE);
    printF(BOARD_HORITZONTAL_LINE);
    for (i = 0; i < BOARD_ROWS; i++)
    {
        asprintf(&buffer, "%c | %c | %c | %c |\n",'1' + i, board[0][i], board[1][i], board[2][i]);
        printF(buffer);
        free(buffer);
        printF(BOARD_HORITZONTAL_LINE);
    }
}

void newPiece(char board[BOARD_COLUMNS][BOARD_ROWS], bool isMyPieceX)
{
    char *frame;
    int row, column;

    // Get column
    frame = read_until(serverFd, '#');
    if (frame[0] < 'A' || frame[0] > 'C')
    {
        handleUnexpectedFrame(frame);
    }
    column = frame[0] - 'A';
    free(frame);

    // Get row
    frame = read_until(serverFd, '#');
    if (frame[0] < '1' || frame[0] > '3')
    {
        handleUnexpectedFrame(frame);
    }
    row = atoi(frame)-1;
    free(frame);

    // Update board
    board[column][row] = (isMyPieceX) ? 'O' : 'X';
    printBoard(board);
}

void yourTurn(char board[BOARD_COLUMNS][BOARD_ROWS], bool isMyPieceX)
{
    char *frame, *buffer;
    int row, column;
    printF("It's your turn, enter the column[A-C]: ");
    buffer = read_until(0, '\n');
    column = toupper(buffer[0]) - 'A';
    free(buffer);

    printF("Enter the row[1-3]: ");
    buffer = read_until(0, '\n');
    row = atoi(buffer) - 1;
    free(buffer);

    board[column][row] = (isMyPieceX) ? 'X' : 'O';
    printBoard(board);

    asprintf(&frame, "MP#%c#%d#", column + 'A', row + 1);
    write(serverFd, frame, strlen(frame));
    free(frame);

    printF("Waiting for the other player...\n");
}

int main(int argc, char *argv[])
{

    int i, j;

    bool isGameRunning, isMyPieceX;
    char *buffer, *frame;
    char board[BOARD_COLUMNS][BOARD_ROWS];

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
    if (strcmp(frame, "SG") != 0)
    {
        handleUnexpectedFrame(frame);
    }
    free(frame);
    frame = read_until(serverFd, '#');

    if (strcmp(frame, "X") == 0)
    {
        isMyPieceX = true;
    }
    else if (strcmp(frame, "O") == 0)
    {
        isMyPieceX = false;
    }
    else
    {
        handleUnexpectedFrame(frame);
    }
    free(frame);

    // Init board
    for (i = 0; i < BOARD_ROWS; i++)
    {
        for (j = 0; j < BOARD_COLUMNS; j++)
        {
            board[j][i] = ' ';
        }
    }
    printBoard(board);
    isGameRunning = true;

    // Start parsing commands from the server
    do
    {
        frame = read_until(serverFd, '#');
        if (strcmp(frame, "YT") == 0)
        {
            yourTurn(board, isMyPieceX);
        }
        else if (strcmp(frame, "NP") == 0)
        {
            newPiece(board, isMyPieceX);
        }
        else if (strcmp(frame, "EG") == 0)
        {
            free(frame);
            frame = read_until(serverFd, '#');
            asprintf(&buffer, "The winner/s is/are %s!\n", frame);
            printF(buffer);
            free(buffer);
            isGameRunning = false;
        }
        else
        {
            handleUnexpectedFrame(frame);
        }
        free(frame);

    } while (isGameRunning);
    close(serverFd);
}