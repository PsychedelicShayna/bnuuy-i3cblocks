/* Core Idea
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * We only want to update i3blocks every second, but between each second, CPU
 * activity may have occurred that we don't want to lose either; CPU spikes.
 * We can retain that information by having sampling occur regardless of if
 * it's time to print or not. We could queue samples, but that would result
 * in the queue filling up, and then the information would be lost anyway
 * if information reliably enters faster than it exits. So perhaps, we can
 * combine samples until we hit the next second, each smapling for 50-100ms.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define u32 uint32_t
#define i32 int32_t
#define u64 uint64_t
#define i64 int64_t
#define u16 uint16_t
#define i16 int16_t
#define u8  uint8_t
#define i8  int8_t

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
} cputime_sample;

typedef struct {
    int64_t        t_idle;
    int64_t        t_active;
    int64_t        t_total;
    cputime_sample cpu_s;
} sample_t;

void print_proc_stat(cputime_sample* ps)
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

int64_t sum_cputime_sample(cputime_sample* ps)
{
    return ps->t_user_time + ps->t_user_nice + ps->t_system + ps->t_idle
           + ps->t_iowait + ps->t_interrupt + ps->t_soft_interrupt
           + ps->t_stolen + ps->t_guest + ps->t_guest_nice;
}

// echo cpu  25982793 198683 6559279 591520253 115176 1258526 611100 0 0 0 | wc
// -c Samples /proc/stat, then waits 1s, and samples again, to produce diff
sample_t sample_cputime()
{
    FILE* f = fopen("/proc/stat", "r");
    char  buf[256];
    int   nb = fread(buf, sizeof(char), sizeof(buf), f);
    fclose(f);
    buf[nb] = 0;

    sample_t st;

    sscanf(buf,
           "cpu %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
           &st.cpu_s.t_user_time,
           &st.cpu_s.t_user_nice,
           &st.cpu_s.t_system,
           &st.cpu_s.t_idle,
           &st.cpu_s.t_iowait,
           &st.cpu_s.t_interrupt,
           &st.cpu_s.t_soft_interrupt,
           &st.cpu_s.t_stolen,
           &st.cpu_s.t_guest,
           &st.cpu_s.t_guest_nice);

    st.t_idle   = st.cpu_s.t_idle;
    st.t_total  = sum_cputime_sample(&st.cpu_s);
    st.t_active = st.t_total - st.t_idle;

    return st;
}

// 100ms seems reasonable
// we could interlace 100ms with 50ms x2
// steps in between?                                        1s Mark
//
// both skip 1, left starts at 0, right starts at 1, converging
// to center. if next sN is already used in a result, skip until
// unused one is found, always by 2 though, so when hitting the left/right
// bounds, invert direction; this this will sync the step/start from 0, if
// the right hits the left wall, and the left will hit 0 at the right wall
// which may catch something the right missed, if we scale the number of
// samples
//          13
// 9     2      4    6   11  7    8   5   10   3   12  1    _______
// s100 s50 s50 s100 s50 s50 s100 s50 s50 s100 s50 s50 s100
//
// if there is one remaining, what if we diff the average of the total CPU
// time delta when summing the rest of the results together. The remainder
// should be in an meaningful spot.. so..intentionally introduce a
// remainder?
//
// and to combine the samples.. these  15 would produce an odd number 7.5
// of results if done in pairs.
//
// if we combine the  s50 s50s into a s100(s50-s50)

typedef enum { LEFT, RIGHT } Direction;

void hexdump(void* data, size_t size)
{
    for(int i = 0; i < size; ++i) {
        printf("%02lx", (unsigned long)(*(unsigned char*)(data + i)));
    }
    printf("\n");
}

float diffused_avg(sample_t* samples, u64 size)
{

    sample_t rsample;
    sample_t lsample;
    // printf("%02x\n", &lsample);
    // printf("RSample Hex:\n");
    // hexdump(&lsample, sizeof(sample_t) * 4);
    // printf("RSample Hex:\n");
    // hexdump(&rsample, sizeof(sample_t) * 4);


    // memset(&lsample,  0, sizeof(sample_t));
    // memset(&rsample,  0, sizeof(sample_t));

    for(int i = 0; i < size; ++i) {
        sample_t* l = &samples[i];
        sample_t* r = &samples[i + 1];

        lsample.t_active += l->t_active;
        lsample.t_total += l->t_total;
        lsample.t_idle += l->t_idle;

        rsample.t_active += r->t_active;
        rsample.t_total += r->t_total;
        rsample.t_idle += r->t_idle;

        i++;
    }

    int64_t *min = 0, *max = 0;

    // DIFF_IDLE=$((IDLE_NOW - IDLE_BEFORE))
    // DIFF_TOTAL=$((TOTAL_NOW - TOTAL_BEFORE))
    // DIFF_USAGE=$(( (1000 * (DIFF_TOTAL - DIFF_IDLE) / DIFF_TOTAL + 5) / 10 ))
    //
    // minmax(&lsample.t_idle, &rsample.t_idle, &min, &max);

    sample_t diff;
    diff.t_idle  = rsample.t_idle - lsample.t_idle;
    diff.t_total = rsample.t_total - lsample.t_total;

    float usage = (1000.0 * ((float)diff.t_total - (float)diff.t_idle)
                       / (float)diff.t_total
                   + 5.0)
                  / 10.0;

    return usage;
}

void handle_direction(Direction* d, i64* idx, size_t upper_bound)
{
    if(*d == LEFT) {
        *idx -= 2;
        if(*idx < 0) {
            *idx = 0;
            *d   = RIGHT;
        }
    } else if(*d == RIGHT) {
        *idx += 2;
        if(*idx >= upper_bound) {
            *idx = upper_bound - 1;
            *d   = LEFT;
        }
    }
}

float interlaced_avg(sample_t* samples, u64 size)
{
    Direction idx1_dir = LEFT, idx2_dir = RIGHT;

    i64 idx1 = 0, idx2 = size - 2;
    u32 processed = 0;

    float ratios[size];
    u32   ridx = 0;

    sample_t lsample, rsample;
    i64 *    min = 0, *max = 0;

    for(; processed < 12;) {
        handle_direction(&idx1_dir, &idx1, size);
        handle_direction(&idx2_dir, &idx2, size);
        // printf("IDXes: %li, %li\n", idx1, idx2);

        sample_t* s1 = &samples[idx1];
        sample_t* s2 = &samples[idx2];

        lsample.t_total += s1->t_total;
        rsample.t_total += s2->t_total;
        lsample.t_idle += s1->t_idle;
        rsample.t_idle += s2->t_idle;
        lsample.t_active += s1->t_active;
        rsample.t_active += s2->t_active;

        minmax(&(*s1).t_total, &(*s2).t_total, &min, &max);
        float delta_sum = *max - *min;
        minmax(&(*s1).t_idle, &(*s2).t_idle, &min, &max);
        float delta_idle = *max - *min;
        minmax(&(*s1).t_active, &(*s2).t_active, &min, &max);
        float delta_active = *max - *min;

        ratios[ridx] = (1000.0 * delta_active / delta_sum + 5.0) / 10.0;

        // printf("DBG: %.04f, %f, %f, (+/) %f\n",
        //        ratios[ridx],
        //        delta_active,
        //        delta_sum,
        //        (delta_active / delta_sum + 5.0) / 10.0);

        ridx++;

        processed += 2;
    }

    // printf("Ratios: ");
    float avg = 0.0;

    for(int i = 0; i < ridx; ++i) {
        // printf("%.08f, ", ratios[i]);
        avg += ratios[i];
    }

    avg /= ridx;
    // printf("\n\nAvg: %.08f\n", avg);

    return avg;
}

float get_diced_usage(int mode)
{
    sample_t* samples = alloca(sizeof(sample_t) * 10);
    memset(samples, 0, sizeof(sample_t) * 10);

    // u64 time = 4 * (MILLI * 50) + 8 * (MILLI * 100);
    for(int i = 0; i < 10; ++i) {
        // if(i % 2 == 0) {
        //     sample_t s50_1 = sample_cputime();
        //     usleep(MILLI * 50);
        //     sample_t s50_2 = sample_cputime();
        //     usleep(MILLI * 100);
        //     // 4x
        //     // usleep((i % 3 == 0) ? MILLI * 50 : MILLI * 100);
        //
        //     samples[i]             = s50_1;
        //     (*(samples + (i + 1))) = s50_2;
        //
        //     i += 1;
        //     continue;
        // } else {
        usleep(MILLI * 10);
        sample_t s100 = sample_cputime();
        samples[i]    = s100;
        // }
    }

    return mode == 1 ? interlaced_avg(samples, 10) : diffused_avg(samples, 10);
}

float usage_formula(float delta_idle, float delta_total)
{
    return (1000.0 * (delta_total - delta_idle) / delta_total + 5.0) / 10.0;
}

float get_climbing_usage(uint32_t mstimings[], size_t size)
{
    sample_t before = {
        .t_total  = 0,
        .t_idle   = 0,
        .t_active = 0,
    };

    memset(&before, 0, sizeof(sample_t));

    for(int i = 0; i < size; ++i) {
        uint32_t time = mstimings[i];
        sample_t s    = sample_cputime();
        before.t_total += s.t_total;
        before.t_idle += s.t_idle;
        before.t_active += s.t_active;
        before.cpu_s.t_user_time += s.cpu_s.t_user_time;
        before.cpu_s.t_user_nice += s.cpu_s.t_user_nice;
        before.cpu_s.t_system += s.cpu_s.t_system;
        before.cpu_s.t_idle += s.cpu_s.t_idle;
        before.cpu_s.t_iowait += s.cpu_s.t_iowait;
        before.cpu_s.t_interrupt += s.cpu_s.t_interrupt;
        before.cpu_s.t_soft_interrupt += s.cpu_s.t_soft_interrupt;
        before.cpu_s.t_stolen += s.cpu_s.t_stolen;
        before.cpu_s.t_guest += s.cpu_s.t_guest;
        before.cpu_s.t_guest_nice += s.cpu_s.t_guest_nice;
        usleep(MILLI * time);
    }

    sample_t after = sample_cputime();

    int64_t *min, *max, approx;

    approx = after.t_total * (size);
    minmax(&approx, &before.t_total, &min, &max);

    int64_t delta_total = *max - *min;

    approx = after.t_idle * (size);
    minmax(&approx, &before.t_idle, &min, &max);

    int64_t delta_idle = *max - *min;

    float usage = usage_formula(delta_idle, delta_total);

    return usage;
}

int main(int argc, char* argv[])
{

  uint32_t timings[18] = {50, 25, 225, 50, 25, 100, 25, 250, 50, 250, 25, 100, 25, 250, 50, 250, 25, 50};

    for(;;) {
        float m;
        if(!strcmp(argv[1], "1")) {
            m = get_diced_usage(1);
        } else if(!strcmp(argv[1], "2")) {
            m = get_diced_usage(0);
        } else if(!strcmp(argv[1], "3")) {
            m = get_climbing_usage(timings, 10);
        }

        fprintf(stdout, "%.08f\n", m);
        fflush(stdout);

        // for(int i = 0; i < 12; ++i) {
        //     // print_proc_stat(&samples[i]);
        //     // printf("%li\n", sum_cputime_sample(&samples[i]));
        // }

        // free(samples);
    }
}

void i3blocks_display(char* full_text, char* color)
{
    fprintf(stdout, TEMPLATE, full_text, color);
    fflush(stdout);
}
