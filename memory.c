#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "i3blocks_common.h"

#ifndef USLEEPFOR
    #define USLEEPFOR 1000000
#endif

// Prints memory usage statistics of the system for i3blocks, the % and used
// memory in GiB
//
//

typedef struct {
    double memtotal;
    double memfree;
    double memavail;
} memory_sample_t;

memory_sample_t sample_memory(void)
{
    // the return value.
    memory_sample_t ret;
    // hexdump((void*)&ret.memtotal, sizeof(double));

    FILE* f = fopen("/proc/meminfo", "r");

    fscanf(f, /* these are in KiB */
           "MemTotal: %lf kB\n"
           "MemFree: %lf kB\n"
           "MemAvailable: %lf kB\n",
           &ret.memtotal,
           &ret.memfree,
           &ret.memavail);

    fclose(f);

    return ret;
}

static inline void output(GB_COLOR color, double perc, double used)
{
    char full_text[64], out[256];
    sprintf(full_text, "%.0lf%% %.03lfG", perc, used);
    sprintf(out, i3bjt, full_text, color);
    fprintf(stdout, "%s", out);
    fflush(stdout);
}

void output_loop(atomic_bool alive)
{
    GB_Color_Transition_Step baseline_green = {
        .threshold  = 32.0,
        .transition = GB_RGB_GREEN,
    };
    GB_Color_Transition_Step green_to_orange = {
        .threshold  = 64.0,
        .transition = GB_RGB_ORANGE,
    };
    GB_Color_Transition_Step orange_to_red = {
        .threshold  = 100.0,
        .transition = GB_RGB_RED,
    };

    GB_Color_Transition_Step steps[3];

    steps[0] = baseline_green;
    steps[1] = green_to_orange;
    steps[2] = orange_to_red;

    while(alive) {
        memory_sample_t sample = sample_memory();
        double          used   = sample.memtotal - sample.memavail;
        double          perc   = (used / sample.memtotal) * 100;
        used                   = used / 1024 / 1024;

        GB_COLOR color = gb_map_percent(perc, steps, 3, GB_GREEN);
        output(color, perc, used);

        usleep(USLEEPFOR);
    }
}

int main(void)
{
    atomic_bool alive;
    atomic_store(&alive, 1);
    output_loop(alive);

    return EXIT_SUCCESS;
}
