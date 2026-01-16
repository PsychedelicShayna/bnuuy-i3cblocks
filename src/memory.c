#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "color/color.h"
#include "i3bar.h"

#ifndef USLEEPFOR
#define USLEEPFOR 1000000
#endif

// Prints memory usage statistics of the system for i3blocks, the % and used
// memory in GiB

typedef struct {
    double memtotal;
    double memfree;
    double memavail;
} memory_sample_t;

memory_sample_t sample_memory(void)
{
    memory_sample_t ret;

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

void output(void)
{
    setlocale(LC_ALL, "");

    GradientStep* gradient = Gradient(
      Threshold(32.0, GREEN), Threshold(64.0, ORANGE), Threshold(100.0, RED));

    i3bar_block_t block;
    i3bar_block_init(&block);
    block.full_text = malloc(sizeof(wchar_t) * 64);

    while(1) {
        memory_sample_t sample = sample_memory();
        double          used   = sample.memtotal - sample.memavail;
        double          perc   = (used / sample.memtotal) * 100;
        used                   = used / 1024 / 1024;

        swprintf(block.full_text, 64, L"%.0lf%% %.03lfG  ", perc, used);

        block.color = map_to_color_hex(used, gradient);
        i3bar_block_output(&block);

        usleep(USLEEPFOR);
    }
}

int main(void)
{
    output();
    return EXIT_SUCCESS;
}
