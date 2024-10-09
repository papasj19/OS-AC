// Operating Systems Lab 1:
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

// Global variables for signal handling and section control
int section_started = 0;    // 0: section not started, 1: section started
int current_section = 0;    // 1: Strings, 2: Winds, 3: Percussion

// Signal handler for SIGUSR1
void signal_handler(int sig) {
    if (sig == SIGUSR1) {
        section_started = 1;
    }
}

// Signal handler for SIGINT (CTRL+C)
void ignore_sigint() {
    // Do nothing to ignore SIGINT
    return;
}

// Function to wait for all section child processes to complete
void wait_for_section_to_complete() {
    char *completion_message;
    int status;
    while (wait(&status) > 0);  // Wait for all child processes to finish
    asprintf(&completion_message, "Director: Section completed\n");
    write(STDOUT_FILENO, completion_message, strlen(completion_message));
    free(completion_message);
}

// Function to start each instrument process
void start_instruments(int section) {
    char *section_message = NULL;

    if (section == 1) { // String Section
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
    } else if (section == 2) { // Wind Section
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
    } else if (section == 3) { // Percussion Section
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
    wait_for_section_to_complete();  // Wait for instruments in the section to finish
}

// Function to start each section, which then starts the instruments
void start_section(int section) {
    char *section_message = NULL;

    asprintf(&section_message, "Section is ready to start\n");
    write(STDOUT_FILENO, section_message, strlen(section_message));
    free(section_message);

    if (fork() == 0) {  // Fork the section
        start_instruments(section);  // Each section forks its instruments
        exit(0);  // Section process exits after its instruments are done
    } else {
        // Wait only for the section process to finish, not for the instruments again
        int status;
        wait(&status);
    }
}


int main() {
    char *output_message = NULL;

    // Set up signal handler for SIGUSR1
    signal(SIGUSR1, signal_handler);

    // Set up signal handler for SIGINT to ignore CTRL+C
    signal(SIGINT, ignore_sigint);

    // Print initial message
    asprintf(&output_message, "Director (PID %d) starting the concert. Use 'kill -SIGUSR1 %d' to start sections\n", getpid(), getpid());
    write(STDOUT_FILENO, output_message, strlen(output_message));
    free(output_message);  // Free the memory and set to NULL to avoid double-free
    output_message = NULL;

    // Loop for the instrument sections 
    for (int i = 1; i <= 3; i++) {
        current_section = i;
        
        if (i == 2) {
            asprintf(&output_message, "Waiting to start Wind section.\n");
        } else if (i == 3) {
            asprintf(&output_message, "Waiting to start Percussion section.\n");
        }
        
        if (output_message != NULL) {
            write(STDOUT_FILENO, output_message, strlen(output_message));
            free(output_message);  // Free the memory after each message
            output_message = NULL;
        }

        // Wait for SIGUSR1 to start the section
        while (!section_started) {
            pause();  // Wait for signal
        }

        // Start the section processes
        start_section(i);

        // Reset section_started for the next round
        section_started = 0;
    }

    // Final message after all sections are completed
    asprintf(&output_message, "Director: Percussion section completed.\n\nConcert finished successfully\n");
    write(STDOUT_FILENO, output_message, strlen(output_message));
    free(output_message);  // Free the memory
    output_message = NULL;

    return 0;
}
