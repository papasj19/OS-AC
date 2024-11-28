/*
 * Solution S7 Operating Systems - Select
 * Curs 2024-25
 *
 * @author: Ferran Castañé
 *
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <netdb.h>
#include <signal.h>

#define printF(x) write(1, x, strlen(x))

#define NUM_ARGS 4
#define MAX_CONN 20
#define CHECK_UP_TO MAX_CONN + 3
#define ERR_CODE_SOCKET -1
#define ERR_ARGS "ERROR: Number of args is not 2. Is necessary to provide the IP, the port and the filenime\n"
#define ERR_SOCKET "ERROR: could not create the socket\n"
#define ERR_CONV_IP "ERROR: could not convert the IP\n"
#define ERR_BIND "ERROR: could not bind to the port\n"
#define ERR_ACCEPT "ERROR accepting connection\n"
#define ERR_READ_DICT "ERROR reading the dictionary\n"
#define NEW_EXIT "New exit petition: %s has left the server\n"
#define WELCOME_MSG "Dictionary server started\nOpening connections...\n"
#define LISTENING "Waiting for connection...\n\n"
#define END_DELIMITER '\n'
#define PLUS_DELIMITER '*'
#define DEF_DELIMITER ':'
#define MSG_COMAND_RECEIVED "User %s has requested %s command\n"

#define ERR_WORD_NOT_FOUND "E*The word %s has not been found in the dictionary.\n"
#define ERR_ADD_WORD "E*The word %s is already in the dictionary.\n"
#define OK_ADD_WORD "OK*The word %s has been added to the dictionary.\n"
#define WORD_FOUND "D*%s*%s\n"
#define LIST_WORDS "L*%d*"
#define MSG_SAVE_DICT "Saving dictionary\n"

typedef struct {
    char *word;
    char *definition;
} Entry;

typedef struct {
    int fd;
    char *username;
} Client;

size_t num_clients = 0;
Client *clients = NULL;
Entry *dictionary = NULL;
int num_entries = 0;
char *filename = NULL;


/**
 * Reads from fd file descriptor until cEnd is found. This character is not included in the array, which is NULL
 * terminated. All the data is stored in dynamic memory.
 * @param fd file descriptor to read from
 * @param cEnd delimiting character
 * @return a pointer to an array of characters
 */
char *readUntil(int fd, char cEnd) {
    int i = 0;
    int chars_read = 0;
    char c = ' ';
    char *buffer = NULL;

    while (c != cEnd) {

        chars_read = (int)read(fd, &c, sizeof(char));
        if (chars_read > 0) {
            buffer = (char *)realloc(buffer, sizeof(char) * (i + 2));
            if (buffer == NULL) {
                // Handle allocation failure
                return NULL;
            }
            if (c != cEnd) {
                buffer[i++] = c;
            }
        } else {
            free(buffer);
            return NULL;
        }
    }

    if (buffer != NULL) {
        buffer[i] = '\0'; // Null-terminate the buffer
    }

    return buffer;
}

/**
 * Opens the server socket connection
 * @param ip IP address to listen from
 * @param port port to bind to
 * @return file descriptor on success, ERR_CODE_SOCKET otherwise
 */
int openListenConn(char *ip, int port) {
    struct sockaddr_in s_addr;
    int fd_socket = -1;

    fd_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (fd_socket < 0) {
        printF(ERR_SOCKET);
        return ERR_CODE_SOCKET;
    }

    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &s_addr.sin_addr) < 0) {
        printF(ERR_CONV_IP);
        close(fd_socket);
        return ERR_CODE_SOCKET;
    }

    if (bind(fd_socket, (void *) &s_addr, sizeof(s_addr)) < 0) {
        printF(ERR_BIND);
        close(fd_socket);
        return ERR_CODE_SOCKET;
    }

    listen(fd_socket, MAX_CONN);

    return fd_socket;
}


/**
 * Finds the definition of a word in the dictionary
 * @param word word to be searched
 * @return definition of the word
 */
char *findDefinition(char *word){
    printF("Searching for word -> ");
    printF(word);
    printF("\n");
    for(int i = 0; i < num_entries; i++){
        if(strcmp(dictionary[i].word, word) == 0){
            return dictionary[i].definition;
        }
    }
    return NULL;
}


/**
 * Adds a new entry to the dictionary
 * @param entry entry to be added
 * @return 0 on success, -1 otherwise
 */

int addEntry(Entry *entry){
    Entry new_entry;
    memset(&new_entry, 0, sizeof(Entry));
    // Check if the word is already in the dictionary
    for(int i = 0; i < num_entries; i++){
        if(strcmp(dictionary[i].word, entry->word) == 0){
            return -1;
        }
    }
    num_entries++;
    dictionary = (Entry *) realloc(dictionary, sizeof(Entry) * num_entries);
    new_entry.word = strdup(entry->word);
    new_entry.definition = strdup(entry->definition);
    dictionary[num_entries - 1] = new_entry;
    return 0;
}



/**
 * Lists all the words in the dictionary
 * @param fd file descriptor to write to
 */

void listWords(int fd){
    char *buffer = NULL;
    asprintf(&buffer,LIST_WORDS, num_entries);
    write(fd, buffer, strlen(buffer));
    free(buffer);
    for(int i = 0; i < num_entries; i++){
        asprintf(&buffer,"%s\n", dictionary[i].word);
        write(fd, buffer, strlen(buffer));
        free(buffer);
    }

}
/**
 * Deserializes a frame received from a client
 * @param fd file descriptor to read from
 * @param client_index index of the client in the array
 */
void deserializeFrame(int fd, int client_index) {
    Entry *entry = NULL;
    char *buffer = NULL;
    int status = 0;

    buffer = readUntil(fd, '*');
    if (buffer == NULL) {
        return;
    }

    if (strcmp(buffer, "C") == 0) {
        free(buffer); // Free buffer after use
        asprintf(&buffer, MSG_COMAND_RECEIVED, clients[client_index].username, "search word");
        printF(buffer);
        free(buffer);
        entry = (Entry *)malloc(sizeof(Entry));
        entry->word = readUntil(fd, END_DELIMITER);
        entry->definition = findDefinition(entry->word);
 
        if (entry->definition == NULL) {
            asprintf(&buffer, ERR_WORD_NOT_FOUND, entry->word);
            write(fd, buffer, strlen(buffer));
            free(buffer);
        } else {
            asprintf(&buffer, WORD_FOUND, entry->word, entry->definition);
            write(fd, buffer, strlen(buffer));
            free(buffer);
        }
        free(entry->word);
        free(entry); 
    } else if (strcmp(buffer, "A") == 0) {
        free(buffer); // Free buffer after use
        asprintf(&buffer, MSG_COMAND_RECEIVED, clients[client_index].username, "add word");
        printF(buffer);
        free(buffer);
        entry = (Entry *)malloc(sizeof(Entry));
        entry->word = readUntil(fd, PLUS_DELIMITER);
        entry->definition = readUntil(fd, END_DELIMITER);
        status = addEntry(entry);
        if (status < 0) {
            asprintf(&buffer, ERR_ADD_WORD, entry->word);
            write(fd, buffer, strlen(buffer));
            free(buffer);
        }else {
            asprintf(&buffer, OK_ADD_WORD, entry->word);
            write(fd, buffer, strlen(buffer));
            free(buffer);
        }
        free(entry->word);
        free(entry->definition);
        free(entry);
    } else if (strcmp(buffer, "L") == 0) {
        free(buffer);
        asprintf(&buffer, MSG_COMAND_RECEIVED, clients[client_index].username, "list words");
        printF(buffer);
        free(buffer);
        buffer = readUntil(fd, END_DELIMITER);
        free(buffer);
        listWords(fd);
    } else if (strcmp(buffer, "U") == 0) {
        free(buffer);
        clients[client_index].username = readUntil(fd, END_DELIMITER);
        asprintf(&buffer, "New user connected: %s\n", clients[client_index].username);
        printF(buffer);
        free(buffer);
    } else {
        free(buffer);
        printF("Invalid command\n");
        return;
    }
}

/**
 * Removes a client and resizes the array (all the proceeding elements are shifted left)
 * @param index index of the client to be removed
 */
void popClient(int index) {
    // Shift clients to fill the gap
    for (size_t i = index; i < num_clients - 1; i++) {
        clients[i] = clients[i + 1];
    }
    num_clients--;
    clients = (Client *)realloc(clients, sizeof(Client) * num_clients);
}

/**
 * Creates a fd_set with all the active file descriptors
 * @return set of active file descriptors
 */
fd_set buildSelectList() {
    fd_set fds_set;
    FD_ZERO(&fds_set);
    for (size_t i = 0; i < num_clients; i++) {
        FD_SET(clients[i].fd, &fds_set);
    }

    return fds_set;
}

/**
 * Accept an incoming connection and save it to the array of clients connected. On error, the connection is refused
 */
void handleNewConn() {
    clients = (Client *) realloc(clients, sizeof(Client) * (num_clients + 1));
    clients[num_clients].fd = accept(clients[0].fd, NULL, NULL);
    if (clients[num_clients].fd < 0) {
        printF(ERR_ACCEPT);
        return;
    }
    num_clients++;
}

/**
 * Removes a client that has terminated the connection from the array of clients, freeing all the memory that was
 * allocated to it
 * @param client client to be removed
 */
void removeClientData(int index) {
    char *builder = NULL;

    asprintf(&builder, NEW_EXIT, clients[index].username);
    printF(builder);
    free(builder);
    free(clients[index].username);
    close(clients[index].fd);
    popClient(index);
}


/**
 * Processes a socket file descriptor input buffer when it has received some data
 * @param index index of the client to be processed
 * @return NULL
 */
void *processFD(int index) {
    struct pollfd pfds[1];

    pfds[0].fd = clients[index].fd;
    pfds[0].events = POLLRDHUP;
    poll(pfds, (nfds_t)1, 0);

    if (pfds[0].revents & POLLRDHUP) {
        removeClientData(index); // Pass the index
    } else {
        deserializeFrame(clients[index].fd, index);
    }

    return NULL;
}

/**
 * Iterates all file descriptors, if their state has changed, the request received is processed. The server's file
 * descriptor is check as well in case a new connection should be processed
 * @param fds_set active file descriptors set
 */
void read_socks(fd_set *fds_set) {
    if (FD_ISSET(clients[0].fd, fds_set)) handleNewConn();

    for (size_t i = 1; i < num_clients; i++) {
        if (FD_ISSET(clients[i].fd, fds_set))  processFD(i);
    }
}

/**
 * Reads the dictionary from the file and stores it in memory
 */
void readDict(){
    int fd = open(filename, O_RDONLY);
    char *buffer = NULL;
    Entry entry;
    memset(&entry, 0, sizeof(Entry));
    if(fd < 0){
        printF(ERR_READ_DICT);
        exit(1);
        return;
    }
    buffer = readUntil(fd, END_DELIMITER);
    num_entries = atoi(buffer);
    free(buffer);
    dictionary = (Entry *) malloc(sizeof(Entry) * num_entries);

    for(int i = 0; i < num_entries; i++){
        entry.word = readUntil(fd, DEF_DELIMITER);
        entry.definition = readUntil(fd, END_DELIMITER);
        dictionary[i] = entry;
    }
    close(fd);
}

void saveDict(){
    printF(MSG_SAVE_DICT);
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if(fd<0){
        printF("Error saving the dictionary\n");
        return;
    }
    char *buffer = NULL;
    asprintf(&buffer, "%d\n", num_entries);
    write(fd, buffer, strlen(buffer));
    free(buffer);
    for(int i = 0; i < num_entries; i++){
        asprintf(&buffer, "%s:%s\n", dictionary[i].word, dictionary[i].definition);
        write(fd, buffer, strlen(buffer));
        free(buffer);
    }
    close(fd);
}

/**
 * Signal handler for SIGINT
 */
void signalHandler(){
    for(size_t i = 0; i < num_clients; i++){
        close(clients[i].fd);
    }
    free(clients);

    saveDict();

    for(int i = 0; i < num_entries; i++){
        free(dictionary[i].word);
        free(dictionary[i].definition);
    }
    if(num_entries > 0) free(dictionary);
    signal(SIGINT, SIG_DFL);
    raise(SIGINT);
}

/**
 * 
 */
int main(int argc, char **argv) {
    fd_set fds_set;
    int select_response;
    int fd_server = -1;

    if (argc < NUM_ARGS) {
        printF(ERR_ARGS);
        return 1;
    }
    signal(SIGINT, signalHandler);
    fd_server = openListenConn(argv[1], atoi(argv[2]));

    if (fd_server == ERR_CODE_SOCKET) {
        return 1;
    }
    filename = argv[3];
    readDict();
    printF(WELCOME_MSG);


    printF(LISTENING);

    clients = (Client *) malloc(sizeof(Client));
    clients[0].fd = fd_server;
    num_clients++;

    while(1){
        fds_set = buildSelectList();
        select_response = select(CHECK_UP_TO, &fds_set, NULL, NULL, NULL);
        if(select_response > 0){
            read_socks(&fds_set);
        }
    }
    return 0;
}
