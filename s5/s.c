// S5.c
// Guillermo Nebra Aljama <guillermo.nebra>
// Spencer Johnson <spencerjames.johnson>

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define RESET "\x1b[0m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define CYAN "\x1b[36m"
#define GREEN "\x1b[32m"

#define CHALLENGE_FILE "challenges.txt"
#define BUFFER_SIZE 1024
#define MAX_QUESTIONS 50

typedef struct {
    char *question;
    char *answer;
    char *hint;
} Challenge;

Challenge challenges[10];
int total_challenges = 0;

void customWrite(const char *message) {
    write(STDOUT_FILENO, message, strlen(message));
}

void print_ascii_art() {
    char *line;

    asprintf(&line, CYAN "  _______\n" RESET);
    customWrite(line);
    free(line);

    asprintf(&line, CYAN " \\/_____;__.—\n" RESET);
    customWrite(line);
    free(line);

    asprintf(&line, CYAN "  |         |\n" RESET);
    customWrite(line);
    free(line);

    asprintf(&line, CYAN "   . ())oo() .\n" RESET);
    customWrite(line);
    free(line);

    asprintf(&line, CYAN "   \\(%%()*^^()^/ \n" RESET);
    customWrite(line);
    free(line);

    asprintf(&line, CYAN "    | -_%%------|\n" RESET);
    customWrite(line);
    free(line);

    asprintf(&line, CYAN "    | %%      ))\n" RESET);
    customWrite(line);
    free(line);

    asprintf(&line, CYAN "    \\ %%______|\n" RESET);
    customWrite(line);
    free(line);
}

void welcomeMessage() {
    print_ascii_art();
    char *message;
    asprintf(&message, CYAN "Welcome to the Guardian of Enigmas Server. Prepare to embark on a journey of puzzles and mysteries!\n" RESET);
    customWrite(message);
    free(message);
}

void reqChall(const char *username) {
    char *message;
    asprintf(&message, BLUE "%s — request challenge...\n" RESET, username);
    customWrite(message);
    free(message);
}

void sendAns(const char *username) {
    char *message;
    asprintf(&message, BLUE "%s — sending answer...\n" RESET, username);
    customWrite(message);
    free(message);
}


void checkAns() {
    char *message;
    asprintf(&message, YELLOW "Checking answer...\n" RESET);
    customWrite(message);
    free(message);
}

void giveHint(const char *username) {
    char *message;
    asprintf(&message, BLUE "%s — request hint...\n" RESET, username);
    customWrite(message);
    free(message);
}

void hinTGiven() {
    char *message;
    asprintf(&message, GREEN "Hint sent!\n" RESET);
    customWrite(message);
    free(message);
}

void viewStatus(const char *username) {
    char *message;
    asprintf(&message, YELLOW "%s — request to view current mission status...\n" RESET, username);
    customWrite(message);
    free(message);
}

void terminateConnMessage(const char *username) {
    char *message;
    asprintf(&message, CYAN "%s - decide to terminate the connection\n" RESET, username);
    customWrite(message);
    free(message);
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

void parse_challenges(const char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        customWrite("Error opening challenge file\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < MAX_QUESTIONS; i++) {
        char *line = read_until(fd, '\n');
        if (strlen(line) == 0) break;

        char *question = strtok(line, "|");
        char *answer = strtok(NULL, "&");
        char *hint = strtok(NULL, "\n");

        if (question && answer && hint) {
            challenges[total_challenges].question = strdup(question);
            challenges[total_challenges].answer = strdup(answer);
            challenges[total_challenges].hint = strdup(hint);
            total_challenges++;
        }
        free(line);
    }
    close(fd);
}


void send_frame(int sockfd, const char *message) {
    if (message == NULL) return;  
    size_t length = strlen(message);
    write(sockfd, message, length);
}

void welcomeUser(const char *username) {
    char *message;
    asprintf(&message, "Welcome, %s!\n", username);
    customWrite(message);
    free(message);
}

void process_client(int client_fd) {

    char *username = read_until(client_fd, '\n');
    welcomeUser(username);

    int current_challenge = 0;

    while (1) {
        char *choice = read_until(client_fd, '\n');
        switch (atoi(choice)) {
            case 1:
                reqChall(username);
                send_frame(client_fd, challenges[current_challenge].question);
                send_frame(client_fd, "\n");
                break;
            case 2: {
                sendAns(username);
                char *response = read_until(client_fd, '\n');
                checkAns();
                int myFlag = 0; 
                if(current_challenge >= total_challenges){
                    myFlag = 1;
                }
                if (strcmp(response, challenges[current_challenge].answer) == 0 && myFlag == 0) {
                    send_frame(client_fd, "Correct answer! Proceeding to the next challenge\n");
                    current_challenge++;
                    if (current_challenge >= total_challenges) {
                        send_frame(client_fd, "Congratulations! You've completed all challenges. Press 4 to get the treasure coordinates.\n");
                        close(client_fd);
                        free(response);
                        return;
                    }
                } else {
                    send_frame(client_fd, "Incorrect answer, try again.\n");
                }
                free(response);
                break;
            }
            case 3:
                giveHint(username);
                send_frame(client_fd, challenges[current_challenge].hint);
                send_frame(client_fd, "\n");
                hinTGiven();
                break;
            case 4: {
                viewStatus(username);
                char *status_message;
                asprintf(&status_message, "Current Challenge: %d\n", current_challenge + 1);
                
                if (current_challenge >= total_challenges) {
                    asprintf(&status_message, "Congratulations! You've found the treasure at coordinates: X:100, Y:200. Disconnecting.\n");
                }
                
                send_frame(client_fd, status_message);
                free(status_message); // Free only this temporary message
                break;
            }


            case 5:
                terminateConnMessage(username);
                send_frame(client_fd, "Goodbye!\n");
                close(client_fd);
                free(username);
                return;
            default:
                send_frame(client_fd, "Opción no valida. Por favor intente de nuevo.\n");

        }
        }
        free(choice);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        customWrite("Usage: ./server <server_ip> <port>\n");
        return EXIT_FAILURE;
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);

    parse_challenges(CHALLENGE_FILE);

    int server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(server_ip);

    if (bind(server_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind");
        return EXIT_FAILURE;
    }

    listen(server_fd, 5);

    welcomeMessage("Guardian of Enigmas");

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }
        process_client(client_fd);
    }

    close(server_fd);
    return 0;
}
