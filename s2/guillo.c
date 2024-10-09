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

// Function to wait for all section child processes to complete
void wait_for_section_to_complete() {
    char *completion_message;
    int status;
    while (wait(&status) > 0);  // Wait for all child processes to finish
    asprintf(&completion_message, "Director: Section completed\n");
    write(STDOUT_FILENO, completion_message, strlen(completion_message));
    free(completion_message);
}

// Function to start each section and spawn its subprocesses
void start_section(int section) {
    char *section_message;
    asprintf(&section_message, "Section is ready to start\n");
    write(STDOUT_FILENO, section_message, strlen(section_message));
    free(section_message);

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
        wait_for_section_to_complete();
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
        wait_for_section_to_complete();
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
        wait_for_section_to_complete();
    }
}

int main() {
    char *output_message;

    // Set up signal handler for SIGUSR1
    signal(SIGUSR1, signal_handler);

    // Print initial message
    asprintf(&output_message, "Director (PID %d) starting the concert. Use 'kill -SIGUSR1 %d' to start sections\n", getpid(), getpid());
    write(STDOUT_FILENO, output_message, strlen(output_message));
    free(output_message);

    // Loop for three sections: Strings, Winds, Percussion
    for (int i = 1; i <= 3; i++) {
        current_section = i;

        // Waiting message for the next section
        if (i == 1) {
            asprintf(&output_message, "Waiting to start String section. Send SIGUSR1 to continue.\n");
        } else if (i == 2) {
            asprintf(&output_message, "Waiting to start Wind section. Send SIGUSR1 to continue.\n");
        } else if (i == 3) {
            asprintf(&output_message, "Waiting to start Percussion section. Send SIGUSR1 to continue.\n");
        }
        write(STDOUT_FILENO, output_message, strlen(output_message));
        free(output_message);

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
    asprintf(&output_message, "Director: Percussion section completed and Concert finished successfully\n");
    write(STDOUT_FILENO, output_message, strlen(output_message));
    free(output_message);

    return 0;
}
