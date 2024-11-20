
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define printF(x) write(1, x, strlen(x))

typedef struct {
    char *word;
    char *definition;
} Entry;

Entry *dictionary = NULL;
int num_words = 0;

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

void load_dictionary(const char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Error opening dictionary file");
        exit(EXIT_FAILURE);
    }

    // Read the number of words
    char *line = read_until(fd, '\n');
    num_words = atoi(line);
    free(line);

    dictionary = (Entry *)malloc(sizeof(Entry) * num_words);

    for (int i = 0; i < num_words; i++) {
        line = read_until(fd, '\n');
        char *colon = strchr(line, ':');
        if (colon) {
            *colon = '\0';
            dictionary[i].word = strdup(line);
            dictionary[i].definition = strdup(colon + 1);
        }
        free(line);
    }

    close(fd);
}

void handle_client_request(int client_fd) {
    char buffer[1024];
    int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) {
        return;
    }
    buffer[bytes_read] = '\0';

    if (buffer[0] == 'C') { // Query word definition
        char *word = buffer + 2; // Skip "C*"
        word[strcspn(word, "\n")] = '\0';

        for (int i = 0; i < num_words; i++) {
            if (strcmp(dictionary[i].word, word) == 0) {
                char *response;
                asprintf(&response, "D*%s*%s\n", dictionary[i].word, dictionary[i].definition);
                write(client_fd, response, strlen(response));
                free(response);
                return;
            }
        }

        char *response;
        asprintf(&response, "E* The word %s has not been found in the dictionary.\n", word);
        write(client_fd, response, strlen(response));
        free(response);

    } else if (buffer[0] == 'A') { // Add new word
        char *word = strtok(buffer + 2, "*");
        char *definition = strtok(NULL, "\n");

        for (int i = 0; i < num_words; i++) {
            if (strcmp(dictionary[i].word, word) == 0) {
                char *response;
                asprintf(&response, "E* The word %s is already in the dictionary.\n", word);
                write(client_fd, response, strlen(response));
                free(response);
                return;
            }
        }

        dictionary = realloc(dictionary, sizeof(Entry) * (num_words + 1));
        dictionary[num_words].word = strdup(word);
        dictionary[num_words].definition = strdup(definition);
        num_words++;

        char *response;
        asprintf(&response, "OK* The word %s has been added to the dictionary.\n", word);
        write(client_fd, response, strlen(response));
        free(response);

    } else if (buffer[0] == 'L') { // List all words
        char *response;
        asprintf(&response, "L*%d*", num_words);
        for (int i = 0; i < num_words; i++) {
            response = realloc(response, strlen(response) + strlen(dictionary[i].word) + 2);
            strcat(response, dictionary[i].word);
            strcat(response, "\n");
        }
        write(client_fd, response, strlen(response));
        free(response);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        char *msg;
        asprintf(&msg, "Usage: %s <IP> <Port> <Dictionary File>\n", argv[0]);
        printF(msg);
        free(msg);
        return EXIT_FAILURE;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);
    const char *filename = argv[3];

    load_dictionary(filename);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = inet_addr(ip),
    };

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return EXIT_FAILURE;
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        return EXIT_FAILURE;
    }

    fd_set active_fds, read_fds;
    FD_ZERO(&active_fds);
    FD_SET(server_fd, &active_fds);
    int max_fd = server_fd;

    while (1) {
        read_fds = active_fds;
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("Select failed");
            return EXIT_FAILURE;
        }

        for (int fd = 0; fd <= max_fd; fd++) {
            if (FD_ISSET(fd, &read_fds)) {
                if (fd == server_fd) {
                    int client_fd = accept(server_fd, NULL, NULL);
                    if (client_fd < 0) {
                        perror("Accept failed");
                        continue;
                    }
                    FD_SET(client_fd, &active_fds);
                    if (client_fd > max_fd) {
                        max_fd = client_fd;
                    }
                } else {
                    handle_client_request(fd);
                    close(fd);
                    FD_CLR(fd, &active_fds);
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
