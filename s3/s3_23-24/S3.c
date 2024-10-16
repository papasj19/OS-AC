/*
* Solution S3 Operating Systems - Threads
* Year 2023-24
*
* @author: Victor Xirau (⌐■_■)
*
*/

#define _GNU_SOURCE
#define _XOPEN_SOURCE 500
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

typedef struct{
    char*name;
    int age;
    float height;
    float weight;
    bool openWounds;
    bool conscious;
    bool respiratoryTrack;
    bool cronicCondition;
    bool thoraxBleeding;
    bool otherInjuries;
    int priority;
    float imc;
    pthread_t thread;
}Patient;

#define customWrite(x) write(1, x, strlen(x))

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

void* triage(void* p){
    Patient* patient = (Patient*) p;

    char*modifier = NULL;
    
    if(!patient->respiratoryTrack && !patient->conscious){
        patient->priority = 3;
        modifier = strdup("\033[0;31m");
    }else if( (patient->respiratoryTrack && !patient->conscious) || (patient->conscious && patient->thoraxBleeding)){
        patient->priority = 2;
        modifier = strdup("\x1B[33m");
    }else if((patient->conscious && patient->otherInjuries && patient->respiratoryTrack) || patient->cronicCondition){
        patient->priority = 1;
        modifier = strdup("\x1B[34m");
    }else{
        patient->priority = 0;
        modifier = strdup("\x1B[36m");
    }

    char*buff;
    asprintf(&buff, "%sPatient %s has been triaged with priority %d\033[0m\n", modifier, patient->name, patient->priority);
    customWrite(buff);
    free(buff);
    if(modifier!=NULL){
        free(modifier);
    }
    
    return NULL;
}

void* treat(void* p){
    Patient* patient = (Patient*) p;
    float *time = malloc(sizeof(float));
    switch(patient->priority){
        case 3:
            *time = 1.5 + (7/2000.0)*pow(patient->age-50, 2);
            break;
        case 2:;
            double imc = patient->weight / pow(patient->height, 2);
            *time = 2 + (imc>= 25 ? 1.3 : (imc < 18.5 ? 1.5 : 0));
            break;
        case 1:
            *time = 2 + (patient->cronicCondition ? 2 : 0);
            break;
        case 0:
            *time = 1;
            break;
    }
    char*buff;
    asprintf(&buff, "Patient %s is being treated for %.2f seconds\n", patient->name, *time);
    customWrite(buff);
    free(buff);
    sleep(*time);
    asprintf(&buff, "\x1B[32mPatient %s has been treated\033[0m\n", patient->name);    
    customWrite(buff);
    free(buff);
    return time;

}

void ignore(){
    signal(SIGINT, ignore);
}


int main( int argc, char* argv[] ){
    char*buff;

    if(argc != 2){
        asprintf(&buff, "Usage: %s <patients.txt>\n", argv[0]);
        customWrite(buff);
        free(buff);
        exit(EXIT_FAILURE);
    }

    int fd_patients = open(argv[1], O_RDONLY);
    if(fd_patients == -1){
        customWrite("Error opening patients file");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, ignore);

    Patient* patients = NULL;
    char*line = read_until(fd_patients, '\n');
    int totalPatients = atoi(line);
    free(line);
    patients = (Patient*) malloc(sizeof(Patient)*totalPatients);

    for(int i = 0; i < totalPatients; i++){
        line = read_until(fd_patients, '\n');
        char* token = strtok(line, "#");
        patients[i].name = strdup(token);
        token = strtok(NULL, "#");
        patients[i].age = atoi(token);
        token = strtok(NULL, "#");
        patients[i].height = atof(token);
        token = strtok(NULL, "#");
        patients[i].weight = atof(token);
        token = strtok(NULL, "#");
        patients[i].openWounds = (strcmp(token, "T") == 0);
        token = strtok(NULL, "#");
        patients[i].conscious = (strcmp(token, "T") == 0);
        token = strtok(NULL, "#");
        patients[i].respiratoryTrack = (strcmp(token, "OK") == 0);
        token = strtok(NULL, "#");
        patients[i].cronicCondition = (strcmp(token, "T") == 0);
        token = strtok(NULL, "#");
        patients[i].thoraxBleeding = (strcmp(token, "T") == 0);
        token = strtok(NULL, "#");
        patients[i].otherInjuries = (strcmp(token, "T") == 0);

        pthread_create(&patients[i].thread, NULL, triage, &patients[i]);
        free(line);
    }

    close(fd_patients);

    customWrite("\nWaiting for triage to finish...\n");

    for(int i = 0; i < totalPatients; i++){
        pthread_join(patients[i].thread, NULL);
    }

    for(int i = 0; i < totalPatients; i++){
        for(int j = i+1; j < totalPatients; j++){
            if(patients[i].priority < patients[j].priority){
                Patient aux = patients[i];
                patients[i] = patients[j];
                patients[j] = aux;
            }
        }
    }
    
    customWrite("Triage finished. Starting treatment...\n\n");

    pthread_t* doctors = (pthread_t*) malloc(sizeof(pthread_t)*5);
    float totalTime = 0;
    float* time = NULL;
    for(int i = 0; i < totalPatients; i+=5){
        for(int j = 0; j < 5 && (i+j)<totalPatients; j++){
            pthread_create(&doctors[j], NULL, treat, &patients[i+j]);
        }
        for(int j = 0; j < 5 && (i+j)<totalPatients; j++){
            pthread_join(doctors[j], (void **)&time);
            totalTime += *time;
            free(time);
        }
    }

    asprintf(&buff, "Crisis averted!! %d patients were cured in a total time of %.2fs\n", totalPatients, totalTime);
    customWrite(buff);
    free(buff);

    free(doctors);

    for(int i = 0; i < totalPatients; i++){
        free(patients[i].name);
    }
    free(patients);

    return 0;
}