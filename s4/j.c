// Operating Systems Lab - Session 4
// Guillermo Nebra Aljama <guillermo.nebra>
// Spencer Johnson <spencerjames.johnson>

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#define printF(x) write(1, x, strlen(x))

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


char* print_menu() {
    char *choice;

    printF("\n=== RiddleQuest Menu ===\n");
    printF("1- Request Current Challenge\n");
    printF("2- Send Response to Challenge\n");
    printF("3- Request Hint\n");
    printF("4- View Current Mission Status\n");
    printF("5- Terminate Connection and Exit\n");
    printF("Choose an option: ");
    choice = read_until(0, '\n');
    return choice;
}


int connectToServer(char *address, int port)
{
    struct sockaddr_in server;
    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (serverFd == -1) {
        printF("Error creating the socket!\n");
        exit(-2);
    }

    server.sin_addr.s_addr = inet_addr(address);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    // Connect to remote server
    if (connect(serverFd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printF("Error connecting to the server!\n");
        close(serverFd);
        exit(-2);
    }

    return serverFd;
}

void send_frame(int sockfd, const char *message) {
    if (write(sockfd, message, strlen(message)) < 0) {
        perror("Error writing to server");
    }
}


void receive_response(int sockfd) {
    char buffer[120];
    ssize_t bytes_received = read(sockfd, buffer, sizeof(buffer) - 1);

    if (bytes_received < 0) {
        printF("Error reading from server\n");
    } else if (bytes_received == 0) {
        printF("Server closed the connection\n");
        exit(EXIT_FAILURE);
    } else {
        buffer[bytes_received] = '\0';  // Null-terminate the received string
        printF("Server: ");
        printF(buffer);
        printF("\n");
    }
}



char* readInput() {
    char *input = NULL;
    size_t size = 0;
    char c;
    int i = 0;

    while (read(STDIN_FILENO, &c, 1) > 0 && c != '\n') {
        input = realloc(input, size + 2);
        input[i++] = c;
        size++;
    }
    if (input != NULL) {
        input[i] = '\0';  
    }

    return input;
}


int main(int argc, char *argv[]) {

    if (argc != 3)
    {
        printF("Number of parameters incorrect!\n");
        return -1;
    }

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int sockfd = connectToServer(server_ip, server_port);

    printF("Welcome to RiddleQuest!\nEnter your name: ");
    char *username = readInput();

    char *frame = malloc(strlen(username) + 2);
    snprintf(frame, strlen(username) + 2, "%s\n", username);
    send_frame(sockfd, frame);

    free(username);

      while (1) {
        int choice = atoi(print_menu());

        switch (choice) {
            case 1:  // challenge
                send_frame(sockfd, "1\n");
                receive_response(sockfd);
                break;

            case 2:  // Response
                printF("Enter your response: ");

                char *response = NULL;
                size_t response_size = 0;
                char c;
                int i = 0;

                while (read(STDIN_FILENO, &c, 1) > 0 && c != '\n') {
                    response = realloc(response, response_size + 2);
                    response[i++] = c;
                    response_size++;
                }
                if (response != NULL) {
                    response[i] = '\0';
                }
                
                size_t frame_size = strlen(response) + 4;  
                frame = realloc(frame, frame_size);

                if (frame == NULL) {
                    printF("Memory allocation failed!\n");
                    exit(EXIT_FAILURE);
                }

                snprintf(frame, frame_size, "2\n%s\n", response);
                send_frame(sockfd, frame);
                receive_response(sockfd);

                free(response);
                break;

            case 3:  // Hint
                send_frame(sockfd, "3\n");
                receive_response(sockfd);
                break;

            case 4:  // Mission status
                send_frame(sockfd, "4\n");
                receive_response(sockfd);
                break;

            case 5:  // Disconnect
                send_frame(sockfd, "5\n");
                printF("Terminating connection and exiting...\n");
                close(sockfd);
                exit(EXIT_SUCCESS);
            default:
                printF("Invalid choice. Please try again.\n");
        }
    }

    close(serverFd);
    free(frame);
}
