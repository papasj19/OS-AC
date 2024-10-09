// Operating Systems Lab 2:
// Authors: Guillermo Nebra Aljama guillermo.nebra
// Spencer Johnson spencerjames.johnson

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>

int section_started = 0;
int current_section = 0;   

void signal_handler(int sig) {
    if (sig == SIGUSR1) {
        section_started = 1;
    }
}


void ignore_sigint() {
    // do nothing :)
    return;
}

void wait_for_section_to_complete() {
    char *completion_message;
    int status;
    
	while (wait(&status) > 0);
    asprintf(&completion_message, "Director: Section completed\n");
    write(STDOUT_FILENO, completion_message, strlen(completion_message));
    
	free(completion_message);
}

void start_instruments(int section) {
    char *section_message = NULL;

    // String Section
    if (section == 1) { 
        if (fork() == 0) {
            asprintf(&section_message, "Strings Viola is playing: Do\n");
            write(STDOUT_FILENO, section_message, strlen(section_message));
            free(section_message);
            
			sleep(2);
            
			exit(0);
        }
        if (fork() == 0) {
            asprintf(&section_message, "Strings Violin 1 is playing: Re\n");
            write(STDOUT_FILENO, section_message, strlen(section_message));
            free(section_message);
            
			sleep(2);
            
			exit(0);
        }
        if (fork() == 0) {
            asprintf(&section_message, "Strings Violin 2 is playing: Re\n");
            write(STDOUT_FILENO, section_message, strlen(section_message));
            free(section_message);
            
			sleep(2);
            
			exit(0);
        }
    } 
    else if (section == 2) { // Wind Section
        if (fork() == 0) {
            asprintf(&section_message, "Wind Flute is playing: Do Do\n");
            write(STDOUT_FILENO, section_message, strlen(section_message));
            free(section_message);
            
			sleep(2);
            
			exit(0);
        }
        if (fork() == 0) {
            asprintf(&section_message, "Wind Clarinet 1 is playing: Re Re\n");
            write(STDOUT_FILENO, section_message, strlen(section_message));
            free(section_message);
            
			sleep(2);
            
			exit(0);
        }
    } 
    else if (section == 3) { // Percussion Section
        if (fork() == 0) {
            asprintf(&section_message, "Percussion Triangle is playing: Do Re Mi\n");
            write(STDOUT_FILENO, section_message, strlen(section_message));
            free(section_message);
            
			sleep(2);
            
			exit(0);
        }
        if (fork() == 0) {
            asprintf(&section_message, "Percussion Vibraphone is playing: Do Re Re Mi\n");
            write(STDOUT_FILENO, section_message, strlen(section_message));
            free(section_message);
            
			sleep(2);
            exit(0);
        }
    }
    wait_for_section_to_complete();
}

void start_section(int section) {
    char *section_message = NULL;

    asprintf(&section_message, "Section is ready to start\n");
    write(STDOUT_FILENO, section_message, strlen(section_message));
    free(section_message);

    if (fork() == 0) {  
        start_instruments(section);
        exit(0);
        
    } 
    else {
        int status;
        wait(&status);
    }
}


int main() {
    char *output_message = NULL;

    signal(SIGUSR1, signal_handler);

    signal(SIGINT, ignore_sigint);

    asprintf(&output_message, "Director (PID %d) starting the concert. Use 'kill -SIGUSR1 %d' to start sections\n", getpid(), getpid());
    write(STDOUT_FILENO, output_message, strlen(output_message));
    free(output_message);
    output_message = NULL;

    for (int i = 1; i <= 3; i++) {
        current_section = i;
        
        if (i == 2) {
            asprintf(&output_message, "Waiting to start Wind section.\n");
        } 
		else if (i == 3) {
            asprintf(&output_message, "Waiting to start Percussion section.\n");
        }
        
        if (output_message != NULL) {
            write(STDOUT_FILENO, output_message, strlen(output_message));
            free(output_message);
            output_message = NULL;
        }

        while (!section_started) {
            pause();
        }

        start_section(i);

        section_started = 0;
    }

    asprintf(&output_message, "Director: Percussion section completed.\n\nConcert finished successfully\n");
    write(STDOUT_FILENO, output_message, strlen(output_message));
    free(output_message);
    
	output_message = NULL;

    return 0;
}
