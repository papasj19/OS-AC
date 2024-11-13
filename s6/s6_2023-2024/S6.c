/*
 * Solution S6 Operating Systems - Pipes and Shared Memory
 * Year 2023-24
 *
 * @author: Marc Valsells Niubó
 *
 * NOTE: Must be compiled with -lm flag to link math library
 */

#define _GNU_SOURCE
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/shm.h>
#include <math.h>


#define printF(x) write(1, x, strlen(x))
// TEXT COLORS
#define C_RESET "\033[0m"
#define C_RED   "\033[31m"
#define C_GREEN "\033[32m"
#define C_YELLOW    "\033[33m"

typedef struct
{
    float bigParticles;
    float smallParticles;
    float alkalinity;
    float ph;
    float chlorine;
} Analysis;

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
void phase1(int pipeInFd, int fileFd)
{
    char *buffer;
    Analysis analysis;

    printF("\nStarting phase 1...\n");
    // Read current cycle analysis data
    buffer = read_until(fileFd, '_');
    analysis.bigParticles = atof(buffer);
    free(buffer);
    buffer = read_until(fileFd, '_');
    analysis.smallParticles = atof(buffer);
    free(buffer);
    buffer = read_until(fileFd, '_');
    analysis.alkalinity = atof(buffer);
    free(buffer);
    buffer = read_until(fileFd, '_');
    analysis.ph = atof(buffer);
    free(buffer);
    buffer = read_until(fileFd, '\n');
    analysis.chlorine = atof(buffer);
    free(buffer);

    // Send data to pipe
    write(pipeInFd, &analysis, sizeof(Analysis));
}

void phase2(int pipeOutFd, int shmid)
{
    Analysis *analysisPtr;
    char *buffer;
    int seconds;

    printF("\nStarting phase 2...\n");

    // Attach to shared memory
    analysisPtr = shmat(shmid, NULL, 0);
    if (analysisPtr == (Analysis *)-1)
    {
        printF("Error attaching to shared memory\n");
        exit(-5);
    }

    // Read phase 1 data from pipe
    read(pipeOutFd, analysisPtr, sizeof(Analysis));

    // Process big particles
    seconds = ceil(analysisPtr->bigParticles / 10.0);
    asprintf(&buffer, "Removing %f big particles in %d seconds...\n", analysisPtr->bigParticles, seconds);
    printF(buffer);
    free(buffer);
    sleep(seconds);

    // Process small particles
    seconds = ceil(analysisPtr->smallParticles / 140.0);
    asprintf(&buffer, "Removing %f small particles in %d seconds...\n", analysisPtr->smallParticles, seconds);
    printF(buffer);
    free(buffer);
    sleep(seconds);
}

void phase3(int shmid)
{
    char *buffer;

     printF("\nStarting phase 3...\n");

    Analysis *analysisPtr = shmat(shmid, NULL, 0);
    if (analysisPtr == (Analysis *)-1)
    {
        printF("Error attaching to shared memory\n");
        exit(-5);
    }

    // Check alkalinity
    if (analysisPtr->alkalinity < 80)
    {
        asprintf(&buffer, "Current cycle alkalinity is %f. Adding %f alkalinity...\n",analysisPtr->alkalinity, 100 - analysisPtr->alkalinity);
        printF(buffer);
    } else if (analysisPtr->alkalinity > 120)
    {
        asprintf(&buffer, "Current cycle alkalinity is %f. Removing %f alkalinity...\n",analysisPtr->alkalinity, analysisPtr->alkalinity - 100);
        printF(buffer);
    } else {
        asprintf(&buffer, "Current cycle alkalinity is %f. Value OK, doing nothing.\n", analysisPtr->alkalinity);
        printF(buffer);
    }
    free(buffer);

    // Check ph
    if (analysisPtr->ph < 7.2)
    {
        asprintf(&buffer, "Current cycle ph is %f. Adding %f ph...\n",analysisPtr->ph, 7.4 - analysisPtr->ph);
        printF(buffer);
    } else if (analysisPtr->ph > 7.6)
    {
        asprintf(&buffer, "Current cycle ph is %f. Removing %f ph...\n",analysisPtr->ph, analysisPtr->ph - 7.4);
        printF(buffer);
    } else {
        asprintf(&buffer, "Current cycle ph is %f. Value OK, doing nothing.\n", analysisPtr->ph);
        printF(buffer);
    }
    free(buffer);

    // Check chlorine
    if (analysisPtr->chlorine < 1)
    {
        asprintf(&buffer, "Current cycle chlorine is %f. Adding %f chlorine...\n",analysisPtr->chlorine, 2 - analysisPtr->chlorine);
        printF(buffer);
    } else if (analysisPtr->chlorine > 3)
    {
        asprintf(&buffer, "Current cycle chlorine is %f. Removing %f chlorine...\n",analysisPtr->chlorine, analysisPtr->chlorine - 2);
        printF(buffer);
    } else {
        asprintf(&buffer, "Current cycle chlorine is %f. Value OK, doing nothing.\n", analysisPtr->chlorine);
        printF(buffer);
    }
    free(buffer);
    shmdt(analysisPtr);
    shmctl(shmid, IPC_RMID, NULL);
}

void processCylce(int fileFd)
{
    printF("\n----- Starting to process a cycle -----\n");

    // Phase 1 to phase 2 Pipe
    int pipeFds[2];
    if (pipe(pipeFds) < 0)
    {
        printF("Error creating pipe\n");
        exit(-2);
    }

    // Phase 2 to phase 3 Shared Memory
    int shmid;
    if ((shmid = shmget(IPC_PRIVATE, sizeof(Analysis), IPC_CREAT | 0666)) < 0)
    {
        printF("Error creating shared memory\n");
        exit(-3);
    }

    // Processes PIDs
    int pidPhase1, pidPhase2, pidPhase3;

    // Phase 1
    pidPhase1 = fork();
    switch (pidPhase1)
    {
    case -1:
        printF("Error creating fork\n");
        exit(-4);
    case 0:
        // Soon
        close(pipeFds[0]);
        phase1(pipeFds[1], fileFd);
        // Phase 1 done, clossing soon process
        close(pipeFds[1]);
        close(fileFd);
        exit(0);
    default:
        // Parent, wait for phase 1 to finish
        waitpid(pidPhase1, NULL, 0);
        break;
    }

    // Phase 2
    pidPhase2 = fork();
    switch (pidPhase2)
    {
    case -1:
        printF("Error creating fork\n");
        exit(-4);
    case 0:
        // Soon
        close(pipeFds[1]);
        close(fileFd);
        phase2(pipeFds[0], shmid);
        // Phase 2 done, clossing soon process
        close(pipeFds[0]);
        exit(0);
    default:
        // Parent, wait for phase 2 to finish
        waitpid(pidPhase2, NULL, 0);
        break;
    }

    close(pipeFds[0]);
    close(pipeFds[1]);

    // Phase 3
    pidPhase3 = fork();
    switch (pidPhase3)
    {
    case -1:
        printF("Error creating fork\n");
        exit(-4);
    case 0:
        // Soon
        close(pipeFds[0]);
        close(pipeFds[1]);
        close(fileFd);
        phase3(shmid);
        // Phase 3 done, clossing soon process
        exit(0);
    default:
        // Parent, wait for phase 3 to finish
        waitpid(pidPhase3, NULL, 0);
        break;
    }
}

int main(int argc, char *argv[])
{

    int litersLeft, cycles, fileFd, i;
    char *buffer;

    if (argc != 2)
    {
        printF("Invalid number of arguments\n");
        return -1;
    }

    // Read number of liters
    fileFd = open(argv[1], O_RDONLY);
    if (fileFd < 0)
    {
        printF("Error opening file\n");
        return -2;
    }
    buffer = read_until(fileFd, '\n');
    litersLeft = atoi(buffer);
    free(buffer);

    // Calculate numbers of cycles rounded up
    cycles = ceil(litersLeft / 100.0);

    for (i = 0; i < cycles; i++)
    {
        processCylce(fileFd);
    }
    close(fileFd);
}