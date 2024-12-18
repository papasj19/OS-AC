/*
    OPERATING SYSTEMS: LAB 11
    Guillermo Nebra Aljama <guillermo.nebra>
    Spencer Johnson <spencerjames.johnson>
*/

#include <stdio.h>
#include <string.h>

void print_emergency(void) {
    printf("⚠️  ⚠️  ⚠️  ⚠️  ⚠️  ⚠️  ⚠️  ⚠️  ⚠️  ⚠️  ⚠️  ⚠️  ⚠️  ⚠️  ⚠️\n");
    printf("*******************************************\n");
    printf("*           🚨 EMERGENCY ALERT 🚨         *\n");
    printf("*         ✨ Houston, we have ✨          *\n");
    printf("*     🛸    a MAJOR problem!    🛸        *\n");
    printf("*                                         *\n");
    printf("*     🔴 STATUS: CRITICAL SITUATION 🔴    *\n");
    printf("*                                         *\n");
    printf("*  📡 Requesting immediate assistance 📡  *\n");
    printf("*******************************************\n");
    printf("⚠️  ⚠️  ⚠️  ⚠️  ⚠️  ⚠️  ⚠️  ⚠️  ⚠️  ⚠️  ⚠️  ⚠️  ⚠️  ⚠️  ⚠️\n");
}

void print_doge() {
    printf("░░░░░░░░░▄░░░░░░░░░░░░░░▄░░░░\n");
    printf("░░░░░░░░▌▒█░░░░░░░░░░░▄▀▒▌░░░\n");
    printf("░░░░░░░░▌▒█░░░░░░░░░░░▄▀▒▌░░░\n");
    printf("░░░░░░░░▌▒▒█░░░░░░░░▄▀▒▒▒▐░░░\n");
    printf("░░░░░░░▐▄▀▒▒▀▀▀▀▄▄▄▀▒▒▒▒▒▐░░░\n");
    printf("░░░░░▄▄▀▒░▒▒▒▒▒▒▒▒▒█▒▒▄█▒▐░░░\n");
    printf("░░░▄▀▒▒▒░░░▒▒▒░░░▒▒▒▀██▀▒▌░░░\n");
    printf("░░▐▒▒▒▄▄▒▒▒▒░░░▒▒▒▒▒▒▒▀▄▒▒▌░░\n");
    printf("░░▌░░▌█▀▒▒▒▒▒▄▀█▄▒▒▒▒▒▒▒█▒▐░░\n");
    printf("░▐░░░▒▒▒▒▒▒▒▒▌██▀▒▒░░░▒▒▒▀▄▌░\n");
    printf("░▌░▒▄██▄▒▒▒▒▒▒▒▒▒░░░░░░▒▒▒▒▌░\n");
    printf("▐▒▀▐▄█▄█▌▄░▀▒▒░░░░░░░░░░▒▒▒▐░\n");
    printf("▐▒▒▐▀▐▀▒░▄▄▒▄▒▒▒▒▒▒░▒░▒░▒▒▒▒▌\n");
    printf("▐▒▒▒▀▀▄▄▒▒▒▄▒▒▒▒▒▒▒▒░▒░▒░▒▒▐░\n");
    printf("░▌▒▒▒▒▒▒▀▀▀▒▒▒▒▒▒░▒░▒░▒░▒▒▒▌░\n");
    printf("░▐▒▒▒▒▒▒▒▒▒▒▒▒▒▒░▒░▒░▒▒▄▒▒▐░░\n");
    printf("░░▀▄▒▒▒▒▒▒▒▒▒▒▒░▒░▒░▒▄▒▒▒▒▌░░\n");
    printf("░░░░▀▄▒▒▒▒▒▒▒▒▒▒▄▄▄▀▒▒▒▒▄▀░░░\n");
    printf("░░░░░░▀▄▄▄▄▄▄▀▀▀▒▒▒▒▒▄▄▀░░░░░\n");
    printf("░░░░░░░░░▒▒▒▒▒▒▒▒▒▒▀▀░░░░░░░░\n");
    printf("******************************\n");
    printf("*       👍  all good  👍     *\n");
    printf("*       ✨  many star ✨     *\n");
    printf("*       🚀 much space 🚀     *\n");
    printf("*          such wow...       *\n");
    printf("******************************\n");
}

void print_report(void) {
    printf("🌟 ✨ 🌟 ✨ 🌟 ✨ 🌟 ✨ 🌟 ✨ 🌟 ✨ 🌟\n");
    printf("============================================\n");
    printf("*         🚀 MISSION PROGRESS REPORT 🚀     *\n");
    printf("--------------------------------------------\n");
    printf("*    🛸 Systems Check:                      *\n");
    printf("*    ⚡ Power Systems: OPTIMAL              *\n");
    printf("*    🔋 Battery Levels: 98%%                *\n");
    printf("*    🌡️  Temperature: NORMAL                *\n");
    printf("*    📡 Communication: STABLE               *\n");
    printf("*                                          *\n");
    printf("* 🌍 Mission Status: PROCEEDING AS PLANNED 🌍*\n");
    printf("============================================\n");
    printf("🌟 ✨ 🌟 ✨ 🌟 ✨ 🌟 ✨ 🌟 ✨ 🌟 ✨ 🌟\n");
}

void print_welcome(void) {
    printf("🌠 🚀 🌠 🚀 🌠 🚀 🌠 🚀 🌠 🚀 🌠 🚀 🌠\n");
    printf("*******************************************\n");
    printf("*     🎉 Welcome aboard, Astronaut! 🎉    *\n");
    printf("*                                         *\n");
    printf("*      🌎  Destination: Deep Space  🌌     *\n");
    printf("*                                         *\n");
    printf("*    👨‍🚀 Your cosmic adventure begins! 👩‍🚀   *\n");
    printf("*                                         *\n");
    printf("*     🌟 Reach for the stars and         *\n");
    printf("*        beyond infinity! ✨              *\n");
    printf("*******************************************\n");
    printf("🌠 🚀 🌠 🚀 🌠 🚀 🌠 🚀 🌠 🚀 🌠 🚀 🌠\n");
}

void print_error(void) {
    printf("❌ ❌ ❌ ❌ ❌ ❌ ❌ ❌ ❌ ❌ ❌ ❌ ❌\n");
    printf("🚫 ERROR: Invalid message type detected! 🚫\n");
    printf("✅ Allowed message types are:\n");
    printf("   → EMERGENCY 🚨\n");
    printf("   → REPORT 📊\n");
    printf("   → WELCOME 👋\n");
    printf("❌ ❌ ❌ ❌ ❌ ❌ ❌ ❌ ❌ ❌ ❌ ❌ ❌\n");
}

int main(int argc, char *argv[]) {
    // Check for correct number of arguments
    if (argc != 2) {
        printf("Usage: %s <EMERGENCY|REPORT|WELCOME>\n", argv[0]);
        return 1;
    }

    // Use if-else chain instead of switch since we can't use variables
    if (!strcmp(argv[1], "EMERGENCY")) {
        print_emergency();
        print_doge();
        return 0;
    }
    
    if (!strcmp(argv[1], "REPORT")) {
        print_report();
        return 0;
    }
    
    if (!strcmp(argv[1], "WELCOME")) {
        print_welcome();
        return 0;
    }

    // Invalid message type
    print_error();
    return 1;
}