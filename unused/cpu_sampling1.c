#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MILLI  1000
#define SECOND 1000000

#define TEMPLATE       "{\"full_text\": \"%s\", \"color\":\"%s\"}\n"
#define GRUVBOX_RED    "#cc241d"
#define GRUVBOX_ORANGE "#d65d0e"

// Core neutrals
#define gb_bg     "#282828"
#define gb_fg     "#ebdbb2"
#define gb_fg_dim "#a89984"

// Accents (soft)
#define gb_blue   "#458588"
#define gb_aqua   "#689d6a"
#define gb_green  "#98971a"
#define gb_yellow "#d79921"
#define gb_orange "#d65d0e"
#define gb_red    "#cc241d"
#define gb_purple "#b16286"

// Find out color relationships later, i.e. how to brighten a color
// while maintaining gruvbox palette
//
// Also consider:
//
// - Using braille characters to draw a CPU history in a single line
// - Making the color switch not be instant, but based on the average % of
//   the last minute.
//
// - Updating text every 1s, but sampling many more times between that second
//   to not miss activity instead of sampling every 1 sescond.
//
//
//                        For LATER
////////////////////////////////////////////////////////////

/*
 * How this reaches a value is basically by checking how much of the total
 * CPU time did not consist of idle time, but it does so not on the current
 * values, but against the *delta* of the previous and current ones.
 *
 * It asks "how much of the difference in CPU time between two samples,
 * is a result of the CPU working, not idling?" this is the most important
 * part: (DIFF_TOTAL - DIFF_IDLE) / DIFF_TOTAL
 *
 * That's your percentage. After a fashion.
 *
 * Why diff and not operate directly? Because CPU time *always* increasees, so
 * you only want to know how much of the increase is not because of the time
 * itself but because of the relative activity betwen the two samples. In other
 * words, it filters out the increase due to total CPU time increasing, by
 * getting delta then once you have two values that are different, past and
 * present, of total and idle time, and then get the ratio, you're getting the
 * ratio of how much the increase was non-IDLE CPU time. If you don't get two
 * samples, the value you get as a result is interesting but not irrelevant
 * here; it always trends upwards, and the true percentage is lost in the noise
 * because two sources product a +delta: overall increase in CPU time, and lower
 * idle time resulting in higher total-idle value. When these two sources of
 * value mix, it becomes very difficult (although there probably is a
 * mathemtical way, maybe signal to noise isolation algorithms) to isolate the
 * difference that actually matters. The only way to know whta was the result of
 * CPU time increasing, is to know the invariants necessary for the word
 * "increasing/increase"'s definition to be coherent: a reference point.
 * Increase is a relative concept.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * DIFF_IDLE=$((IDLE_NOW - IDLE_BEFORE))                                     *
 * DIFF_TOTAL=$((TOTAL_NOW - TOTAL_BEFORE))                                  *
 * DIFF_USAGE=$(( (1000 * (DIFF_TOTAL - DIFF_IDLE) / DIFF_TOTAL + 5) / 10 )) *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// TODO TODO TODO TODO TODO
// Right Now ----------------------
// - See if you can find something in <sys/times.h> or another header
//   to avoid parsing the string in /proc/stat; directly getting ints.
//
// - Start with string parsing though to see the current performance.
//   and measure the benefit of doing the operation the same way, but
//   in C.

typedef struct {
    int64_t t_user_time;
    int64_t t_user_nice;
    int64_t t_system;
    int64_t t_idle;
    int64_t t_iowait;
    int64_t t_interrupt;
    int64_t t_soft_interrupt;
    int64_t t_stolen;
    int64_t t_guest; // VMs
    int64_t t_guest_nice;
} proc_stat_t;

void print_proc_stat(proc_stat_t* ps)
{
    printf("User Time: %lu\n", ps->t_user_time);
    printf("User Nice: %lu\n", ps->t_user_nice);
    printf("System: %lu\n", ps->t_system);
    printf("Idle: %lu\n", ps->t_idle);
    printf("I/O Wait: %lu\n", ps->t_iowait);
    printf("Interrupt: %lu\n", ps->t_interrupt);
    printf("Soft Interrupt: %lu\n", ps->t_soft_interrupt);
    printf("Stolen: %lu\n", ps->t_stolen);
    printf("Guest: %lu\n", ps->t_guest);
    printf("Guest Nice: %lu\n", ps->t_guest_nice);
}

// echo cpu  25982793 198683 6559279 591520253 115176 1258526 611100 0 0 0 | wc
// -c Samples /proc/stat, then waits 1s, and samples again, to produce diff
proc_stat_t sample_cpu()
{
    FILE* f = fopen("/proc/stat", "r");
    char  buf[256];
    int   nb = fread(buf, sizeof(char), sizeof(buf), f);
    fclose(f);
    buf[nb] = 0;

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

void display(char* full_text, char* color)
{
    fprintf(stdout, TEMPLATE, full_text, color);
    fflush(stdout);
}

typedef struct {
    int64_t lsum, rsum;
    int64_t lactive, ractive;
    int64_t dsum, dactive, didle;
    float   lratio, rratio;
    float   dratio;
    float   usage;
} dsd_t;

void print_dsd_t(dsd_t* p)
{
    fprintf(stdout,
            "00U->00U: %.08f, rsum:%ld,  lactive:%ld, ractive:%ld,  dsum:%ld, "
            "dactive:%ld, "
            "didle:%ld,  lratio:%f, rratio:%f,  dratio:%f,  lsum:%ld\n",
            p->usage,
            p->rsum,
            p->lactive,
            p->ractive,
            p->dsum,
            p->dactive,
            p->didle,
            p->lratio,
            p->rratio,
            p->dratio,
            p->lsum);
    fflush(stdout);
}

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

dsd_t dsd(proc_stat_t* l, proc_stat_t* r)
{
    dsd_t re;

    re.lsum = sum_proc_stat(l);
    re.rsum = sum_proc_stat(r);

    re.lactive = re.lsum - l->t_idle;
    re.ractive = re.rsum - r->t_idle;

    re.lratio = (float)re.lactive / (float)re.lsum;
    re.rratio = (float)re.ractive / (float)re.rsum;

    int64_t *min, *max;

    minmax(&re.lsum, &re.rsum, &min, &max);
    re.dsum = *max - *min;

    minmax(&re.lactive, &re.ractive, &min, &max);
    re.dactive = *max - *min;

    minmax(&l->t_idle, &r->t_idle, &min, &max);
    re.didle = *max - *min;

    re.dratio = (float)re.dactive / (float)re.dsum;
    re.usage  = (1000.0 * (float)re.dactive / (float)re.dsum + 5) / 10.0;

    return re;
}

float usage(int sleep_ms) {
    proc_stat_t sample1 = sample_cpu();
    usleep(sleep_ms * 1000);
    proc_stat_t sample2 = sample_cpu();
    dsd_t       re      = dsd(&sample1, &sample2);
    return re.usage;
}

void multisample(int samples, int sleep)
{
    proc_stat_t sret[samples];

    for(int i = 0; i < samples; ++i) {
        sret[i] = sample_cpu();
        usleep(sleep);
    }

    for(int i = 0; i < samples; ++i) {
    }
}

int main(int argc, char* argv[])
{

    // for(;;) {
    //   printf("00L->00R: %.08f\n", usage(500));
    // }

    for(;;) {
        proc_stat_t ls1 = sample_cpu();
        usleep(MILLI * 50);
        proc_stat_t ls2 = sample_cpu();
        usleep(1000000 / 27);
        proc_stat_t ls3 = sample_cpu();
        usleep(1000000 / 27);
        proc_stat_t rs1 = sample_cpu();
        usleep(1000000 / 27);
        proc_stat_t rs2 = sample_cpu();
        usleep(1000000 / 27);
        proc_stat_t rs3 = sample_cpu();
        dsd_t       re  = dsd(&ls1, &rs1);
        print_dsd_t(&re);
        re = dsd(&ls2, &rs2);
        print_dsd_t(&re);
        re = dsd(&ls3, &rs3);
        print_dsd_t(&re);
        re = dsd(&ls1, &rs3);
        printf("\nLS1->RS3: %.08f\n", re.usage);
        printf("00L->00R: %.08f\n\n", usage(37));
        continue;

        proc_stat_t ps1    = sample_cpu();
        int64_t     sum    = sum_proc_stat(&ps1);
        int64_t     active = sum - ps1.t_idle;
        float       ratio  = (float)active / (float)sum;
        float       hratio = (1000.0 * (float)active / (float)sum + 5) / 10.0;
        float       perc   = ratio * 100.0;
        printf("Sum: %ld, active: %ld, ratio: %.064f, hratio: %.064f, perc: "
               "%.064f\n",
               sum,
               active,
               ratio,
               hratio,
               perc);
        usleep(1000000 / 10);
    }

    // char buf[1024];
    //
    // char *x = gb_bg;
    //
    // memset(buf, 0, 1024);
    // sprintf(buf, "%s\n%s", "Hello", "World");
    //
    // fprintf(stdout, buf, "");

    // for (int i = 0; i < 10; ++i) {
    //   display(i, char *color)
    // }
    return 0;
}
