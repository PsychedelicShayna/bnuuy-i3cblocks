#include <assert.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

#ifndef CHARTSIZE
#define CHARTSIZE 32

#endif

#include "braille.h"
#include "color/color.h"
#include "i3bar.h"
#include "pango2.h"

// #include "pango.h"

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
    return ps->t_user_time + ps->t_user_nice + ps->t_system + ps->t_idle +
           ps->t_iowait + ps->t_interrupt + ps->t_soft_interrupt +
           ps->t_stolen + ps->t_guest + ps->t_guest_nice;
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

double cpu_usage(__useconds_t micro)
{
    static proc_stat_t before;
    before = sample_cpu();

    int64_t before_total  = sum_proc_stat(&before);
    int64_t before_idle   = before.t_idle;
    int64_t before_active = before_total - before_idle;

    usleep(micro != 0 ? micro : USLEEPFOR);

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

void output(void)
{
    setlocale(LC_ALL, "");

    GradientStep* color_gradient = Gradient(
      Threshold(10.0, GREEN), Threshold(50.0, ORANGE), Threshold(100.0, RED));

    // history size must be double the chart size because the chart represents
    // two datapoints in history for every one braille chracter in the chart.
    size_t HISTORYSIZE = CHARTSIZE * 2;

    double     history2[HISTORYSIZE];
    cpu_sample history[HISTORYSIZE];
    Color      chistory[HISTORYSIZE];

    for(size_t i = 0; i < sizeof(history) / sizeof(history[0]); i++) {
        chistory[i] = GREEN;
        // history[i]  = 0.0;
        cpu_sample_init(&history[i]);
    }

    i3bar_block_t block;
    i3bar_block_init(&block);
    block.markup = "pango";
    block.separator = false;

#ifdef TODO_REPLACE_THIS
    // String builder for pango markup. We will use this to write to the
    // block.full_text after programatically decorating the output with
    // Pango markup. Namely, coloring each individual chart character
    // differently according to the color in chistory.

    pango_string_builder psb = pangosb_create(8);
#endif

    wchar_t chart[CHARTSIZE];
    memset(chart, L'⣀', sizeof(chart) / sizeof(chart[0]));

    size_t   largebuf_size  = 16383;
    wchar_t* largebuf_pango = malloc(largebuf_size);
    memset((void*)largebuf_pango, 0, largebuf_size);

    panspan braille_span;
    panspan_init(&braille_span);
    wchar_t pre_outbuf[256];

    size_t test = sizeof(chistory);

    int pair_offset = 0;

    cpu_sample resample[CHARTSIZE];
    memset(resample, 0, (sizeof(resample) / sizeof(resample[0])));

    while(1) {
        double usage     = cpu_usage(USLEEPFOR);
        double frequency = cpu_frequency();
        Color  color     = map_to_color(usage, color_gradient);

        memmove(history, &history[1], sizeof(history) - sizeof(cpu_sample));
        memmove(chistory, &chistory[1], sizeof(chistory) - sizeof(Color));
        memmove(resample, &resample[1], sizeof(resample) - sizeof(cpu_sample));
        history[sizeof(history) / sizeof(history[0]) - 1].scalar = usage;
        chistory[sizeof(chistory) / sizeof(chistory[0]) - 1]     = color;

        write_braille_chart_samples(&resample[0],
                                    CHARTSIZE,
                                    history,
                                    (sizeof(history) / sizeof(history[0])),
                                    -1,
                                    -1);

        // wprintf(L"Lastonne: %d\n", resample[CHARTSIZE-1].scalar);
        // wprintf(L"SemiLastonne: %d\n", resample[CHARTSIZE-2].scalar);
        // 6 good, 6 offby1, 7, good 7 offby1, it's the uneven index, i*2
        // is not a good strategy.

        for(size_t i = 0; i < CHARTSIZE; i++) {
            cpu_sample s1      = resample[i];
            cpu_sample s2      = resample[i +  1 < CHARTSIZE ? i + 1 : i];
            double     scalar1 = s1.scalar;
            double     scalar2 = s2.scalar;

            // size_t resample_idx = i / 2;
            // wprintf(L"Loop idx: %zu\n", i);
            // wprintf(L"Chart idx: %zu\n", resample_idx);

            // size_t s0 = pair_offset + 2 * i;
            // size_t s1 = s0 + 1;

            // if(s0 >= HISTORYSIZE)
            //     break;
            //
            // cpu_sample s0 =
            //   resample[i >= (sizeof(resample) / sizeof(resample[0])) ? i - 1
            //                                                          : i];
            //
            // // wprintf(L"S0: %lf\n", s0.scalar);
            // // cpu_sample s1 = resample[(2 * i) + 1];
            //
            // cpu_sample s1 =
            //   resample[i + 1 >= (sizeof(resample) / sizeof(resample[0]))
            //              ? i
            //              : i + 1];
            // // wprintf(L"S1: %lf\n", s1.scalar);

            //
            // double v0 = history[s0];
            // double v1 = (s1 < HISTORYSIZE) ? history[s1] : v0;

            // Color c0 = chistory[s0];
            // Color c1 = (s1 < HISTORYSIZE) ? chistory[s1] : c0;

            // Color chosen = map_to_color(
            //   (s0.scalar >= s1.scalar) ? s0.scalar : s1.scalar,
            //   color_gradient); // history[s0] >= history[s1] ? c1 : c2;
            // // wprintf(L"Color: %d %d %d\n", color.r, color.g, color.b);

            // if(i >= CHARTSIZE - 1) {
            //     chosen = color;
            // }

            // if(i >= CHARTSIZE - 1) {
            //     chosen = color;
            // }

            // Color *min = &color_a, *max = &color_b;
            //
            // if((min->r + min->g + min->b) > (max->r + max->g + max->b)) {
            //     max = &color_a;
            //     min = &color_b;
            // }

            // wchar_t braille_chars[2] = { sample.wchr ? sample.wchr :
            // sample.wchr, L'\0' };
            wchar_t braille_chars[2] = { s1.wchr, L'\0' };
            //
            // wprintf(L"braillechars: '%ls'\n", braille_chars);
            braille_span.foreground =
              rgbx(map_to_color(scalar1, color_gradient));

            memset(pre_outbuf, 0, sizeof(pre_outbuf));

            pango_format(pre_outbuf,
                         (sizeof(pre_outbuf) / sizeof(pre_outbuf[0])),
                         braille_chars,
                         PAT_NULL,
                         &braille_span);

            // wprintf(L"preoutbuf: %ls\n", pre_outbuf);
            wcslcat(largebuf_pango,
                    pre_outbuf,
                    largebuf_size / sizeof(largebuf_pango[0]));

            // wprintf(L"Color A: %s, Color B: %s for BC: '%lc'\n",
            //         rgbx(color_a),
            //         rgbx(color_b),
            //         braille_char);
        }

        memset(pre_outbuf, 0, sizeof(pre_outbuf));
        const char* color_hex = rgbx(color);

        swprintf(pre_outbuf,
                 sizeof(pre_outbuf) - 1,
                 L" %05.2lf%% %02.2lfGHz  ",
                 // &chart[0],
                 usage,
                 frequency);

        wchar_t outbuf[1024];
        memset(outbuf, 0, 1024 * sizeof(wchar_t));

        panspan ps;
        panspan_init(&ps);
        ps.foreground = (char*)color_hex;

        pango_format(outbuf, 1024, pre_outbuf, PAT_NULL, &ps);

        // wprintf(L"outbuf: %ls\n", outbuf);
        wcscat(largebuf_pango, outbuf);
        wcslcat(largebuf_pango, outbuf, wcslen(largebuf_pango));

#ifdef TODO_REPLACE_THIS
        // Write the full_text string to the pango string builder, to make
        // it a colored span using the foreground color determined earlier.
        pangosb_wpush(&psb,
                      full_text,
                      PSBT_NULL,
                      &((pango_span) { .foreground = (char*)color_hex }));

        // Then we extract the last string from the pango string builder.

        wchar_t final_buf[pangosb_size_last(&psb) + 1];
        memset(final_buf, 0, sizeof(final_buf));
        pangosb_get_last(&psb, final_buf, sizeof(final_buf));
#endif

        // Then set it as the value for the JSON full_text ky and output it.
        block.full_text = largebuf_pango;
        i3bar_block_output(&block);

        memset(largebuf_pango, 0, largebuf_size);

        // We don't sleep here, since the sampling algorithm includes usleep
        // in order to get two relative samples to calculate usgae.
    }

    free(largebuf_pango);
}

int main(void)
{

    setlocale(LC_ALL, "");
    output();
    return EXIT_SUCCESS;
}
