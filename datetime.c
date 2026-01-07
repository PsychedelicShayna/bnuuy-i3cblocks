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
    "", "", "󰲓", "󰲓", "󰲓", "",
    "", "", "󰉊", "󰉊", "󰉊", ""
};

void output(void)
{
    const char* TIME_FORMAT = " %%s  %a %d %b %H:%M:%S %%s  ";
    const char* COLOR       = "#b16286";
    char        bufferA[256], bufferB[64];
    time_t      t  = time(NULL);
    struct tm*  lt = NULL;

    while(1) {
        t  = time(NULL);
        lt = localtime(&t);

        strftime(bufferA, 256, TIME_FORMAT, lt);
        snprintf(
          bufferB, 64, bufferA, seasons[lt->tm_mon], clocks[lt->tm_hour]);
        snprintf(bufferA, 256, JSON_OUTPUT_TEMPLATE, bufferB, COLOR);
        fprintf(stdout, "%s", bufferA);
        fflush(stdout);
        usleep(USLEEPFOR);
    }
}

int main(void)
{
    output();
}
