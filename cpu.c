#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "i3blocks_common.h"

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
    int64_t t_guest;      // VM Memory
    int64_t t_guest_nice; //
} proc_stat_t;

void minmax(int64_t* a, int64_t* b, int64_t** out_min, int64_t** out_max)
{
    if(*a >= *b) {
        *out_max = a;
        *out_min = b;
    } else {
        *out_max = b;
        *out_min = a;
    }
}

int64_t delta(int64_t a, int64_t b)
{
    int64_t *min, *max;
    minmax(&a, &b, &min, &max);
    return *max - *min;
}

proc_stat_t sample_cpu(void)
{
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

int64_t sum_proc_stat(proc_stat_t* ps)
{
    return ps->t_user_time + ps->t_user_nice + ps->t_system + ps->t_idle
           + ps->t_iowait + ps->t_interrupt + ps->t_soft_interrupt
           + ps->t_stolen + ps->t_guest + ps->t_guest_nice;
}

double cpu_frequency(void)
{
    char   buffer[256];
    double freq;
    memset(buffer, 0, 256);
    FILE* f =
        fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", "r");
    fscanf(f, "%lf", &freq);
    fclose(f);
    return freq / 1000000;
}

double cpu_usage(void)
{
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

static inline void output(GB_COLOR color, double usage, double frequency)
{
    char full_text[64], out[256];
    sprintf(full_text, "%2.lf%% %2.2lfGHz", usage, frequency);
    sprintf(out, i3bjt, full_text, color);
    fprintf(stdout, "%s", out);
    fflush(stdout);
}

void output_loop(atomic_bool* alive)
{
    GB_Color_Transition_Step baseline_green = {
        .threshold  = 10.0,
        .transition = GB_RGB_GREEN,
    };
    GB_Color_Transition_Step green_to_orange = {
        .threshold  = 50.0,
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

    while(*alive) {
        double   usage     = cpu_usage();
        double   frequency = cpu_frequency();
        GB_COLOR color     = gb_map_percent(usage, steps, 3, GB_GREEN);
        output(color, usage, frequency);
    }
}

#ifndef SINGLE_SHOT

int main(void)
{

    atomic_bool alive;
    atomic_store(&alive, 1);
    output_loop(&alive);
    return 0;
}

#else

int main()
{
    double usage = cpu_usage(), frequency = cpu_frequency();
    output(usage, frequency);
    return 0;
}

#endif
