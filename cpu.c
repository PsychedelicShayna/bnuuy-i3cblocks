#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "braille.h"
#include "color/color.h"
#include "i3bar.h"

#ifndef USLEEPFOR
#define USLEEPFOR 1000000
#endif

typedef struct {
    int64_t t_user_time;
    int64_t t_user_nice;
    int64_t t_system;
    int64_t t_idle;
    int64_t t_iowait;
    int64_t t_interrupt;
    int64_t t_soft_interrupt;
    int64_t t_stolen;

    int64_t t_guest; // VM Memory
    int64_t t_guest_nice;
} proc_stat_t;

void minmax(int64_t* a, int64_t* b, int64_t** out_min, int64_t** out_max) {
    if(*a >= *b) {
        *out_max = a;
        *out_min = b;
    } else {
        *out_max = b;
        *out_min = a;
    }
}

int64_t delta(int64_t a, int64_t b) {
    int64_t *min, *max;
    minmax(&a, &b, &min, &max);
    return *max - *min;
}

proc_stat_t sample_cpu(void) {
    FILE*         f = fopen("/proc/stat", "r");
    char          buf[256];
    unsigned long nb = fread(buf, sizeof(char), sizeof(buf), f);
    fclose(f);
    buf[nb - 1] = 0;

    proc_stat_t ps;

    sscanf(buf,
           "cpu %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
           &ps.t_user_time,
           &ps.t_user_nice,
           &ps.t_system,
           &ps.t_idle,
           &ps.t_iowait,
           &ps.t_interrupt,
           &ps.t_soft_interrupt,
           &ps.t_stolen,
           &ps.t_guest,
           &ps.t_guest_nice);

    return ps;
}

int64_t sum_proc_stat(proc_stat_t* ps) {
    return ps->t_user_time + ps->t_user_nice + ps->t_system + ps->t_idle +
           ps->t_iowait + ps->t_interrupt + ps->t_soft_interrupt +
           ps->t_stolen + ps->t_guest + ps->t_guest_nice;
}

double cpu_frequency(void) {
    char   buffer[256];
    double freq;
    memset(buffer, 0, 256);
    FILE* f =
      fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", "r");
    fscanf(f, "%lf", &freq);
    fclose(f);
    return freq / 1000000;
}

double cpu_usage(void) {
    static proc_stat_t before;
    before = sample_cpu();

    int64_t before_total  = sum_proc_stat(&before);
    int64_t before_idle   = before.t_idle;
    int64_t before_active = before_total - before_idle;

    usleep(USLEEPFOR);

    proc_stat_t now = sample_cpu();

    int64_t now_total  = sum_proc_stat(&now);
    int64_t now_idle   = now.t_idle;
    int64_t now_active = now_total - now_idle;

    int64_t diff_total  = delta(now_total, before_total);
    int64_t diff_active = delta(now_active, before_active);

    double usage =
      (1000.0 * (double)diff_active / (double)diff_total + 5) / 10.0;

    before = now;

    return usage;
}

void output(void) {
    setlocale(LC_ALL, "");
    GradientStep* color_gradient = Gradient(
      Threshold(10.0, GREEN), Threshold(50.0, ORANGE), Threshold(100.0, RED));

    const size_t history_size = 32;

    double history[history_size];

    Color color_history[history_size];
    for(size_t i = 0; i < history_size; i++) {
        color_history[i] = GREEN;
        history[i]       = 0.0;
    }

    wchar_t chart[17] = L"⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀\0";

    braille_inline_chart(
      chart, sizeof(chart) / sizeof(chart[0]), history, history_size, 0, 100);

    size_t full_text_size = 256;
    char   full_text[full_text_size * sizeof(char)];

    i3bar_block_t block;
    i3bar_block_init(&block);
    block.markup = "pango";

    while(1) {
        double usage     = cpu_usage();
        double frequency = cpu_frequency();
        Color  color     = map_to_color(usage, color_gradient);

        memcpy(history, &history[1], (history_size - 1) * sizeof(double));

        memcpy(
          color_history, &color_history[1], (history_size - 1) * sizeof(Color));

        history[31]       = usage;
        color_history[31] = color;

        braille_inline_chart(chart,
                             (sizeof(chart) - 1) / sizeof(chart[0]),
                             history,
                             sizeof(history) / sizeof(history[0]),
                             0,
                             100);

        const char* color_hex = rgbx(color);

        sprintf(full_text,
                "<span color=\"%s\"> "
                "%ls %.2lf%% "
                "%02.2lfGHz   </span>",
                color_hex,
                &chart[0],
                usage,
                frequency);

        wchar_t full_text_w[full_text_size];
        memset(full_text_w, 0, full_text_size * sizeof(wchar_t));

        mbstowcs(full_text_w, full_text, sizeof(full_text));
        block.full_text = full_text_w;
        i3bar_block_output(&block);

        // We don't sleep here, since the sampling algorithm includes usleep
        // in order to get two relative samples to calculate usgae.
    }
}

int main(void) {
    setlocale(LC_ALL, "");
    output();
    return EXIT_SUCCESS;
}
