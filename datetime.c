#include "color/color.h"

#ifndef USLEEPFOR
#define USLEEPFOR 1000000
#endif /* ifndef USLEEPFOR */

#include <time.h>
#include <unistd.h>

static const char* clocks[24]  = { "󱑖", "󱑋", "󱑌", "󱑍", "󱑎",
                                   "󱑏", "󱑐", "󱑑", "󱑒", "󱑓",
                                   "󱑔", "󱑕", "󱑊", "󱐿", "󱑀",
                                   "󱑁", "󱑂", "󱑃", "󱑄", "󱑅",
                                   "󱑆", "󱑇", "󱑈", "󱑉" };
static const char* seasons[12] = {
    "", "", "󰲓", "󰲓", "󰲓", "",
    "", "", "󰉊", "󰉊", "󰉊", ""
};

void output(void) {
    const char* TIME_FORMAT = "%%s %a %d %b %%s %H:%M:%S";
    const char* COLOR       = "#b16286";
    char        bufferA[256], bufferB[64];
    time_t      t  = time(NULL);
    struct tm*  lt = NULL;

    while(1) {
        t  = time(NULL);
        lt = localtime(&t);

        strftime(bufferA, 128, TIME_FORMAT, lt);
        sprintf(bufferB, bufferA, seasons[lt->tm_mon], clocks[lt->tm_hour]);
        sprintf(bufferA, JSON_OUTPUT_TEMPLATE, bufferB, COLOR);
        fprintf(stdout, "%s", bufferA);
        fflush(stdout);
        usleep(USLEEPFOR);
    }
}

int main(void) {
    output();
}
