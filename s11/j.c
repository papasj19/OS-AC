/*
    OPERATING SYSTEMS: SESSION 11
    Guillermo Nebra Aljama <guillermo.nebra>
    Spencer Johnson <spencerjames.johnson>

*/


#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Error: Invalid number of arguments.\nUsage: ./s11 <MESSAGE_TYPE>\n");
        return 1;
    }

    if (strcmp(argv[1], "EMERGENCY") == 0) {
        printf(
            "****************************************\n"
            "*                                      *\n"
            "*       !!!! EMERGENCY !!!!            *\n"
            "*       TAKE IMMEDIATE ACTION         *\n"
            "*                                      *\n"
            "****************************************\n"
            "*                                      *\n"
            "*       !!!! EMERGENCY !!!!            *\n"
            "*       TAKE IMMEDIATE ACTION         *\n"
            "*                                      *\n"
            "*            ______                    *\n"
            "*         .-\"      \"-.                 *\n"
            "*        /            \\                *\n"
            "*       |              |               *\n"
            "*       |,  .-.  .-.  ,|               *\n"
            "*       | )(__/  \\__)(|               *\n"
            "*       |/     /\\    \|               *\n"
            "*       (_     ^^     _)               *\n"
            "*        \\__|IIIIII|__/                *\n"
            "*         | \\IIIIII/ |                 *\n"
            "*         \\          /                 *\n"
            "*          `--------`                  *\n"
            "*                                      *\n"
            "****************************************\n"
        );
    } else if (strcmp(argv[1], "REPORT") == 0) {
        printf(
            "****************************************\n"
            "*                                      *\n"
            "*         MISSION REPORT               *\n"
            "*       ALL SYSTEMS NOMINAL           *\n"
            "*      NO ANOMALIES DETECTED          *\n"
            "*                                      *\n"
            "****************************************\n"
        );
    } else if (strcmp(argv[1], "WELCOME") == 0) {
        printf(
            "****************************************\n"
            "*                                      *\n"
            "*       WELCOME, ASTRONAUTS!           *\n"
            "*     TO THE JOURNEY OF A LIFETIME     *\n"
            "*                                      *\n"
            "*              __                      *\n"
            "*           __(o )                     *\n"
            "*           ===  |                     *\n"
            "*             | \\___/|                 *\n"
            "*             \\ \\=== |                 *\n"
            "*              \\_\\==/                  *\n"
            "*                ||                    *\n"
            "*               ===                    *\n"
            "*                                      *\n"
            "****************************************\n"
        );
    } else {
        printf("Error: Invalid message type. Valid types: EMERGENCY, REPORT, WELCOME.\n");
        return 1;
    }

    return 0;
}