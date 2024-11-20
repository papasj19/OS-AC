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


typedef struct {
    char *key;
    char *value;
} Entry;

Entry * dictionary; 
int dictionary_size = 0;

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


void parse_dictionary(const char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    // Read the first line to get the number of entries
    char *line = read_until(fd, '\n');
    int num_entries = atoi(line);
    free(line);

    dictionary_size = num_entries;

    // Allocate memory for storing the entries
    dictionary = malloc(num_entries * sizeof(Entry));
    if (!dictionary) {
        perror("Memory allocation failed");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Parse each line into key-value pairs
    for (int i = 0; i < num_entries; i++) {
        line = read_until(fd, '\n');

        // Locate the delimiter ':'
        char *delimiter_pos = strchr(line, ':');
        if (delimiter_pos == NULL) {
            fprintf(stderr, "Invalid line format: %s\n", line);
            free(line);
            continue;
        }

        // Compute lengths of key and value
        size_t key_length = delimiter_pos - line;
        size_t value_length = strlen(line) - key_length - 1;

        // Allocate memory for key and value
        dictionary[i].key = malloc(key_length + 1);
        dictionary[i].value = malloc(value_length + 1);

        if (!dictionary[i].key || !dictionary[i].value) {
            perror("Memory allocation failed");
            free(line);
            continue;
        }

        // Copy key and value into allocated memory using strcpy
        strncpy(dictionary[i].key, line, key_length);
        dictionary[i].key[key_length] = '\0';  // Null-terminate the key
        strcpy(dictionary[i].value, delimiter_pos + 1);

        // Print the result
        printf("Key: %s\nValue: %s\n\n", dictionary[i].key, dictionary[i].value);

        free(line);
    }

    // Free allocated memory
    for (int i = 0; i < num_entries; i++) {
        free(dictionary[i].key);
        free(dictionary[i].value);
    }
    free(dictionary);

    close(fd);
}

int search_word(const char *word) {
    for (int i = 0; i < dictionary_size; i++) {
        if (strcmp(dictionary[i].word, word) == 0) {
            return i;  // Return the index of the word
        }
    }
    return -1;  // Word not found
}

// Function to handle a query
void handle_query(const char *word) {
    int index = search_word(word);
    if (index == -1) {
        printf("E* The word %s has not been found in the dictionary.\n", word);
    } else {
        printf("D*%s*%s\n", dictionary[index].word, dictionary[index].definition);
    }
}

// Function to handle adding a new word
void handle_add(const char *word, const char *definition) {
    if (search_word(word) != -1) {
        printf("E* The word %s is already in the dictionary.\n", word);
        return;
    }
    // Add the word and definition
    dictionary[dictionary_size].word = strdup(word);
    dictionary[dictionary_size].definition = strdup(definition);
    dictionary_size++;
    printf("OK* The word %s has been added to the dictionary.\n", word);
}

// Function to handle listing all words
void handle_list() {
    printf("L*%d\n", dictionary_size);
    for (int i = 0; i < dictionary_size; i++) {
        printf("%s\n", dictionary[i].word);
    }
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
    int operation = 0;

    while(1){

        char *choice = read_until(connectFD, '\n');

        if (choice[1] != '*') {
            printf("E* Invalid frame format.\n");
        return;
    }
    operation = choice[0];

        switch (atoi(choice))
        {
            case 'C':
                doSearchWord(connectFD);

            break;

            case 'A':
                printF("\nUser has requested add word command\n");

            break;

            case 'L':
                printF("\nUser has requested list words command\n");
            break;

            case '':
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

    parse_dictionary(argv[3]);

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