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


/*
void sigintHandler()
{
    // Do nothing
    signal(SIGINT, sigintHandler);
}
*/





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

void send_frame(int sockfd, const char *message) {
    if (write(sockfd, message, strlen(message)) < 0) {
        perror("Error writing to server");
    }
}


// Function to receive data from the server
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


// Function to read input from the user using dynamic memory
char* readInput() {
    char *input = NULL;
    size_t size = 0;
    char c;
    int i = 0;

    // Dynamically read each character until newline
    while (read(STDIN_FILENO, &c, 1) > 0 && c != '\n') {
        input = realloc(input, size + 2);
        input[i++] = c;
        size++;
    }
    if (input != NULL) {
        input[i] = '\0';  // Null-terminate the string
    }
    return input;
}





int main(int argc, char *argv[])
{


   

    

    if (argc != 3)
    {
        printF("Number of parameters incorrect!\n");
        return -1;
    }
    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int sockfd = connectToServer(server_ip, server_port);


    printF("Welcome to RiddleQuest!\nEnter your name: ");
    char *username = readInput();  // Dynamic memory for user input

    char *frame = malloc(strlen(username) + 2);  // Allocate memory for frame
    snprintf(frame, strlen(username) + 2, "%s\n", username);  // Prepare frame
    send_frame(sockfd, frame);

    //free(frame);    // Free dynamically allocated memory
    free(username); // Free dynamically allocated memory

    


      while (1) {
        int choice = atoi(print_menu());
        switch (choice) {
            case 1:  // Request Current Challenge
                send_frame(sockfd, "1\n");
                receive_response(sockfd);
                break;
            case 2:  // Send Response to Challenge
                printF("Enter your response: ");
                
                // Dynamically allocate memory for the response
                char *response = NULL;
                size_t response_size = 0;
                char c;
                int i = 0;

                // Use read() to capture user input, allowing for spaces
                while (read(STDIN_FILENO, &c, 1) > 0 && c != '\n') {
                    response = realloc(response, response_size + 2);
                    response[i++] = c;
                    response_size++;
                }
                if (response != NULL) {
                    response[i] = '\0';  // Null-terminate the string
                }

                
                // Calculate the required size for frame based on the response length
                size_t frame_size = strlen(response) + 4;  // 4 bytes for "2\n" and "\n\0"
                frame = realloc(frame, frame_size);  // Reallocate memory for frame

                if (frame == NULL) {
                    printF("Memory allocation failed!\n");
                    exit(EXIT_FAILURE);
                }

                snprintf(frame, frame_size, "2\n%s\n", response);  // Create the frame
                send_frame(sockfd, frame);  // Send the frame
                receive_response(sockfd);

                // Free dynamically allocated memory for the response
                free(response);
                break;

            case 3:  // Request Hint
                send_frame(sockfd, "3\n");
                receive_response(sockfd);
                break;
            case 4:  // View Current Mission Status
                send_frame(sockfd, "4\n");
                receive_response(sockfd);
                break;
            case 5:  // Terminate Connection and Exit
                send_frame(sockfd, "5\n");
                printF("Terminating connection and exiting...\n");
                close(sockfd);
                exit(EXIT_SUCCESS);
            default:
                printF("Invalid choice. Please try again.\n");
        }
    }

    close(serverFd);
}
