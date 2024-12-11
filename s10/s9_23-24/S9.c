#define _GNU_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>

#define Write(x) write(1, x, strlen(x))

#define NUM_TEAMS       3

#define ERR_ARG         "Invalid number of arguments\n"
#define ERR_OPEN        "There was an error opening the file\n"

#define BOLTS_TEAM1 "\033[0;32m[Bolts TEAM]\033[0m\tBolts out!!!\t\t\033[1m[%d s]\033[0m\n"
#define BOLTS_TEAM2 "\033[0;32m[Bolts TEAM]\033[0m\tBolts on!!!\t\t\033[1m[%d s]\033[0m\n"
#define BOLTS_TEAM3 "\033[0;32m[Bolts TEAM]\033[0m\t* Removing bolts *\n"
#define BOLTS_TEAM4 "\033[0;32m[Bolts TEAM]\033[0m\t* Putting in bolts *\n"
#define TIRES_TEAM1   "\033[0;34m[Tires TEAM]\033[0m\t* Removing tires *\n"  
#define TIRES_TEAM2   "\033[0;34m[Tires TEAM]\033[0m\t* Putting on tires *\n"
#define TIRES_TEAM3   "\033[0;34m[Tires TEAM]\033[0m\tTires removed!!!\t\033[1m[%d s]\033[0m\n"  
#define TIRES_TEAM4   "\033[0;34m[Tires TEAM]\033[0m\tTires on!!!\t\t\033[1m[%d s]\033[0m\n"  
#define FUEL_TEAM1    "\033[0;31m[Fuel  TEAM]\033[0m\t* Opening tank *\t\033[1m[1 s]\033[0m\n"  
#define FUEL_TEAM2    "\033[0;31m[Fuel  TEAM]\033[0m\t* Refilling tank *\n"  
#define FUEL_TEAM3    "\033[0;31m[Fuel  TEAM]\033[0m\tTank refilled!!!\t\033[1m[%d s]\033[0m\n"  
#define TIEMPO_TOTAL    "\033[1m---------------------------------------------\033[0m\nBoxes time \033[1m[%s]\t\t[%d s]\033[0m\n\n\n"

pthread_mutex_t mtx_screen          = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx_fuel_team       = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx_bolts_team      = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx_tires_team      = PTHREAD_MUTEX_INITIALIZER;
pthread_t thread_ids[NUM_TEAMS];
int bolts_out       = 0;
int tires_on        = 0;
int refilled        = 0;
int t_bolts         = 0;
int t_tires         = 0;
int t_fuel          = 0;
char* aux;


void init_mutex(){
    pthread_mutex_init(&mtx_screen, NULL);
    pthread_mutex_init(&mtx_fuel_team, NULL);
    pthread_mutex_init(&mtx_bolts_team, NULL);
    pthread_mutex_init(&mtx_tires_team, NULL);
}

void destroy_mutex(){
    pthread_mutex_destroy(&mtx_screen);
    pthread_mutex_destroy(&mtx_fuel_team);
    pthread_mutex_destroy(&mtx_bolts_team);
    pthread_mutex_destroy(&mtx_tires_team);
}

void* bolts_team(void* arg) {

    int random_time;

    while(bolts_out < 2) {

        if(bolts_out == 0) {
            Write(BOLTS_TEAM3);
            srand(17);
            random_time = rand() % (2 - 1 + 1) + 1;
            sleep(random_time);
            bolts_out++;
            pthread_mutex_lock(&mtx_screen);
            asprintf(&aux, BOLTS_TEAM1, random_time);
            Write(aux);
            free(aux);
            pthread_mutex_unlock(&mtx_screen);
            pthread_mutex_lock(&mtx_bolts_team);
            t_bolts += random_time;
            pthread_mutex_unlock(&mtx_bolts_team);
        }

        if(tires_on == 1) {
            Write(BOLTS_TEAM4);
            srand(17);
            random_time = rand() % (2 - 1 + 1) + 1;
            sleep(random_time);
            bolts_out++;
            pthread_mutex_lock(&mtx_screen);
            asprintf(&aux, BOLTS_TEAM2, random_time);
            Write(aux);
            free(aux);
            pthread_mutex_unlock(&mtx_screen);
            pthread_mutex_lock(&mtx_bolts_team);
            t_bolts += random_time;
            pthread_mutex_unlock(&mtx_bolts_team);
        }
    }
    
    return NULL;
}

void* tires_team(void* arg) {

    int random_time;

    while(tires_on < 1) {

        if(bolts_out == 1) {

            Write(TIRES_TEAM1);
            srand(17);
            random_time = rand() % (2 - 1 + 1) + 1; 
            sleep(random_time);
            pthread_mutex_lock(&mtx_screen);
            asprintf(&aux, TIRES_TEAM3, random_time);
            Write(aux);
            free(aux);
            pthread_mutex_unlock(&mtx_screen);
            pthread_mutex_lock(&mtx_tires_team);
            t_tires += random_time;
            pthread_mutex_unlock(&mtx_tires_team);

            Write(TIRES_TEAM2);
            srand(17);
            random_time = rand() % (2 - 1 + 1) + 1; 
            sleep(random_time);
            pthread_mutex_lock(&mtx_screen);
            asprintf(&aux, TIRES_TEAM4, random_time);
            Write(aux);
            free(aux);
            pthread_mutex_unlock(&mtx_screen);
            pthread_mutex_lock(&mtx_tires_team);
            tires_on++;
            t_tires += random_time;
            pthread_mutex_unlock(&mtx_tires_team);
        }
    }
    
    return NULL;

}

void* fuel_team(void* arg) {

    int random_time;
        
    if(refilled == 0) {
        pthread_mutex_lock(&mtx_screen);
        Write(FUEL_TEAM1);
        pthread_mutex_unlock(&mtx_screen);
        sleep(1);
        Write(FUEL_TEAM2);
        srand(17);
        random_time = rand() % (7 - 4 + 1) + 4;
        t_fuel = random_time + 1;
        sleep(random_time);
        pthread_mutex_lock(&mtx_screen);
        asprintf(&aux, FUEL_TEAM3, random_time);
        Write(aux);
        free(aux);
        pthread_mutex_unlock(&mtx_screen);
        t_fuel = random_time + 1;
        refilled++;
    }
    

    return NULL;

}

char* readUntil(int fd, char del) {
    int i = 0;
    char c = '0';
    char* buffer = (char*)malloc(sizeof(char));

    while (c != del) {
        read(fd, &c, sizeof(char));
        if (c != del) {
            buffer[i] = c;
            buffer = (char*)realloc(buffer, sizeof(char) * (i + 2));
        }
        i++;
    }
    buffer[i - 1] = '\0';
    return buffer;
}

void createThreads() {

    for(int i = 0; i < NUM_TEAMS; i++) 
        pthread_create(&thread_ids[i], NULL, (i==0) ? bolts_team : (i==1) ? fuel_team : tires_team, (void*) NULL);
    
    for(int i = 0; i < NUM_TEAMS; i++)
        pthread_join(thread_ids[i], NULL);
  
}

void killThreads() {
    for (int i = 0; i < NUM_TEAMS; i++) {
        pthread_cancel(thread_ids[i]);
        pthread_detach(thread_ids[i]);
    }
}


int main(int argc, char*argv[]) {

    int fd;
    char* pilot;

    if(argc != 3){
        Write(ERR_ARG);
        return -1;
    }

    if((fd = open(argv[1], O_RDONLY)) < 0) {
        Write(ERR_OPEN);
        return -1;
    }

    if(atoi(argv[2]) == 1) {
        pilot = readUntil(fd, '\n');
        pthread_mutex_lock(&mtx_screen);
        asprintf(&aux, "\033[1m[%s]\n\n\033[0m", pilot);
        Write(aux);
        free(aux);
        pthread_mutex_unlock(&mtx_screen);
        createThreads();
        if(t_fuel >= t_bolts + t_tires) {
            asprintf(&aux, TIEMPO_TOTAL, pilot, t_fuel);
            Write(aux);
            free(aux);
        } else {
            asprintf(&aux, TIEMPO_TOTAL, pilot, (t_bolts + t_tires));
            Write(aux);
            free(aux);
        }
        killThreads();
    }

    if(atoi(argv[2]) == 2) {
        pilot = readUntil(fd, '\n');
        pthread_mutex_lock(&mtx_screen);
        asprintf(&aux, "\033[1m[%s]\033[0m\n\n", pilot);
        Write(aux);
        free(aux);
        pthread_mutex_unlock(&mtx_screen);
        createThreads();
        
        if(t_fuel >= t_bolts + t_tires) {
            asprintf(&aux, TIEMPO_TOTAL, pilot, t_fuel);
            Write(aux);
            free(aux);
        } else {
            asprintf(&aux, TIEMPO_TOTAL, pilot, t_bolts+t_tires);
            Write(aux);
            free(aux);
        }
        killThreads();
        t_bolts = t_tires = t_fuel = bolts_out = tires_on = refilled = 0;
        pilot = readUntil(fd, '\n');
        pthread_mutex_lock(&mtx_screen);
        asprintf(&aux, "\n[%s]\n\n", pilot);
        Write(aux);
        free(aux);
        pthread_mutex_unlock(&mtx_screen);

        createThreads();
        if(t_fuel >= t_bolts + t_tires) {
            asprintf(&aux, TIEMPO_TOTAL, pilot, t_fuel);
            Write(aux);
            free(aux);
        } else {
            asprintf(&aux, TIEMPO_TOTAL, pilot, t_bolts+t_tires);
            Write(aux);
            free(aux);
        }
        killThreads();

    }

}

