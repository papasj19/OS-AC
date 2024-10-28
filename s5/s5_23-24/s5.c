#define _GNU_SOURCE
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ctype.h>
#include <netdb.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/ioctl.h>
#include <math.h>

#define Write(x) write(STDOUT_FILENO, x, strlen(x))

#define ERR_ARG     "Error en el número de argumentos\n"
#define ERR_SOCK    "Error al crear el socket. Por favor, intentalo de nuevo\n"
#define ERR_BIND    "Error al hacer bind. Por favor, intentalo en unos segundos\n"
#define ERR_LST     "En nuestro sistema SOSmart ya estamos en contacto con este vehículo!\n"
#define ERR_OPEN    "Ha habido un error al abrir el archivo key.txt\n"

#define EXIT        "Exiting SOSmart\n\n"
#define WLCM        "Bienvenido al servidor SOSmart\n\n"
#define KEY         "KEY:kwmo62klmdz5sl90hbwvv1r4fi33pppx"
#define UBI         "UBI:kwmo62klmdz5sl90hbwvv1r4fi33pppx"
#define CALL        "CALL:kwmo62klmdz5sl90hbwvv1r4fi33pppx"
#define HOSP        "HOSP:kwmo62klmdz5sl90hbwvv1r4fi33pppx"
#define SALIR       "EXIT:kwmo62klmdz5sl90hbwvv1r4fi33pppx"
#define KEY_OK      "Key validation = True\n\n"
#define KEY_KO      "Key validation = False\n\n"
#define KEY_VAL     "KEY:Validated"
#define KEY_NVAL    "KEY:Not validated"
#define UBI_NOW     "UBI:41°23'28.1\"N 2°09'28.3\"E*C/ d'Enric Granados, 65, Barcelona"
#define UBI_NOW2    "UBI:41°07'23.8\"N 1°14'47.5\"E*Av. Catalunya, 47, Tarragona"
#define UBI_NOW3    "UBI:41°58'35.5\"N 2°49'09.9\"E*C. de Joan Maragall, 54, Girona"
#define UBI_NOW4    "UBI:41°37'02.9\"N 0°37'12.5\"E*Av. de Balmes, 11, Lleida"
#define LIST        "CALL:Mama*Papa*Policia"
#define LIST2       "CALL:Messi*Florentino*Ronaldinho*Cristiano"
#define LIST3       "CALL:Jordi ENP*Isabella Angela*Nacho Vidal*Helena Kramer"
#define LIST4       "CALL:Pique*Shakira"
#define EMERGEN     "Getting location and sending it to emergencies\n"

typedef struct {
    char* provincia;
    char* hospital;
}Prov_Hosp; 

void keyResponse(int userfd, int op) {

    char info[256];

    memset(info, '\0', 256);

    if(op) {
        strcpy(info, KEY_VAL);
    } else {
        strcpy(info, KEY_NVAL);
    }
    

    write(userfd, info, 256);
}

void ubiResponse(int userfd) {

    int ubi;
    char info[256];
    time_t t;

    srand((unsigned) time(&t));
    ubi = rand() % 4;

    memset(info, '\0', 256);

    if(ubi == 0) {
        strcpy(info, UBI_NOW);
    } else if(ubi == 1) {
        strcpy(info, UBI_NOW2);
    } else if(ubi == 2) {
        strcpy(info, UBI_NOW3);
    } else {
        strcpy(info, UBI_NOW4);
    }

    write(userfd, info, 256);
}

int op(char info[256]) {

    if(strcmp(info, UBI) == 0) {
        return 1;
    } else if(strcmp(info, CALL) == 0) {
        return 2;
    } else if(strstr(info, HOSP) != NULL) {
        return 3;
    } else if(strcmp(info, SALIR) == 0) {
        return 4;
    } else {
        return 5;
    }

}

char* readFile(int fd) {
    int i = 0;
    char c;
    
    int num = read(fd, &c, sizeof(char));
    char* fitxer = (char *) malloc(sizeof(char));

    while(num > 0) {
        fitxer[i] = c;
        i++;
        fitxer = (char *) realloc(fitxer, (i + 1));
        num = read(fd, &c, sizeof(char));
    }
    fitxer[i] = '\0';
    close(fd);

    return fitxer;
}

void hospResponse(int userfd, char* location, char* hospital) {

    char info[256];
    char* substring;

    memset(info, '\0', 256);
    asprintf(&substring, "HOSP:%s*%s", location, hospital);
    strcpy(info, substring);
    free(substring);
    write(userfd, info, 256);
}

void hospResponseNULL(int userfd, char* location) {

    char info[256];
    char* substring;

    memset(info, '\0', 256);
    asprintf(&substring, "HOSP:%s*NULL", location);
    strcpy(info, substring);
    free(substring);
    write(userfd, info, 256);
}

void callResponse(int userfd) {

    char info[256];
    int op;
    time_t t;

    srand((unsigned) time(&t));
    op = rand() % 4;

    memset(info, '\0', 256);

    if(op == 0) {
        strcpy(info, LIST);
    } else if(op == 1) {
        strcpy(info, LIST2);
    } else if(op == 2) {
        strcpy(info, LIST3);
    } else {
        strcpy(info, LIST4);
    }

    write(userfd, info, 256);
}


int main (int argc, char *argv[]) {

    int fd, listenfd, userfd, salir = 0, i = 0, ok;
    struct sockaddr_in server;
    char info[256];
    char* tok;
    char* file;
    char* string;
    char* location;
    Prov_Hosp* prov_hosps;
    
    if(argc != 3) {
        Write(ERR_ARG);
        return -1;
    }

    if((fd = open(argv[2], O_RDONLY)) < 0) { 
        Write(ERR_OPEN);        
        return -1;

    } else {
        file = readFile(fd);
        char** prov_hosp = (char **) malloc(sizeof(char *));

        tok = strtok(file, "\n");

        while(tok != NULL) {
            prov_hosp[i] = strdup(tok);
            tok = strtok(NULL, "\n");
            i++;
            prov_hosp = (char **) realloc(prov_hosp, sizeof(char *) * (i+1));
        }
        prov_hosp[i] = NULL;

        free(file);

        prov_hosps = (Prov_Hosp *) malloc(sizeof(Prov_Hosp) * 52);
        
        for(i = 0; i < 52; i++) {
            prov_hosps[i].provincia = strdup(strtok(prov_hosp[i], ":"));
            prov_hosps[i].hospital = strdup(strtok(NULL, ":"));
        }

        for(i = 0; prov_hosp[i] != NULL; i++) {
            free(prov_hosp[i]);
        }
        free(prov_hosp);
        
    }

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(atoi(argv[1]));

    if((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        Write(ERR_SOCK);

        for(i = 0; i < 52; i++) {
            free(prov_hosps[i].hospital);
            free(prov_hosps[i].provincia);
        }
        free(prov_hosps);
        return -1;
    }

    if(bind(listenfd, (struct  sockaddr*) &server, sizeof(server)) < 0) {
        Write(ERR_BIND);
        close(listenfd);
        for(i = 0; i < 52; i++) {
            free(prov_hosps[i].hospital);
            free(prov_hosps[i].provincia);
        }
        free(prov_hosps);
        return -1;
    }

    if(listen(listenfd, 3) < 0) {
        Write(ERR_LST);
    }

    Write(WLCM);
    userfd = accept(listenfd, (struct sockaddr *) NULL, NULL);  

    read(userfd, info, 256);

    if(strcmp(info, KEY) == 0) {
        Write(KEY_OK);
        keyResponse(userfd, 1);
    } else {
        Write(KEY_KO);
        keyResponse(userfd, 0);
        return -1;
    }

    while(!salir) {    
        
        read(userfd, info, 256);

        switch(op(info)) {

            case 1:

                Write(EMERGEN);
                ubiResponse(userfd);

                break;

            case 2:

                Write("Received call petition. Sending user's emergency profiles list\n");
                callResponse(userfd);
                read(userfd, info, 256);

                tok = strtok(info, ":");
                tok = strtok(NULL, ":");

                asprintf(&string, "Calling %s\n\n", tok);
                Write(string);
                free(string);

                break;

            case 3:
                
                ok = 0;
                tok = strtok(info, "*");
                location = strdup(strtok(NULL, "*"));
                asprintf(&string, "Location received: %s\nSearching nearest hospital available...\n\n", location);
                Write(string);
                free(string);
                
                for(i = 0; i < 52; i++) {

                    if(strcmp(location, prov_hosps[i].provincia) == 0) {
                        asprintf(&string, "\tHospital found: %s\n\n", prov_hosps[i].hospital);
                        Write(string);
                        free(string);
                        hospResponse(userfd, prov_hosps[i].provincia, prov_hosps[i].hospital);
                        ok = 1;
                    }

                }

                if(!ok) {
                    hospResponseNULL(userfd, location);
                }
                free(location);

                break;

            case 4:
            
                salir = 1;
                for(i = 0; i < 52; i++) {
                    free(prov_hosps[i].hospital);
                    free(prov_hosps[i].provincia);
                }
                free(prov_hosps);
                close(userfd);
                close(listenfd);
                Write(EXIT);
                break;
        
            default:
                break;
        
        }



    
    }
    
    return 0;

}