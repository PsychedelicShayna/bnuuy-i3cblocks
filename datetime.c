#include <locale.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <wchar.h>

#include "color/color.h"
#include "i3bar.h"
#include "pango2.h"

#include "meteo.h"
#include "private.h"

#include "common.h"

#include "braille.h"

#ifndef USLEEPFOR
#define USLEEPFOR 1000000
#endif /* ifndef USLEEPFOR */

#include <time.h>
#include <unistd.h>

#include <curl/curl.h>
#include <curl/easy.h>

#define printf(format, ...)        wprintf(L##format, ##__VA_ARGS__)
#define fprintf(file, format, ...) fwprintf(file, L##format, ##__VA_ARGS__)

#define DEBUG

static const char* clocks[24]  = { "󱑖", "󱑋", "󱑌", "󱑍", "󱑎",
                                   "󱑏", "󱑐", "󱑑", "󱑒", "󱑓",
                                   "󱑔", "󱑕", "󱑊", "󱐿", "󱑀",
                                   "󱑁", "󱑂", "󱑃", "󱑄", "󱑅",
                                   "󱑆", "󱑇", "󱑈", "󱑉" };
static const char* seasons[12] = {
    "", "", "󰲓", "󰲓", "󰲓", "",
    "", "", "󰉊", "󰉊", "󰉊", ""
};

static const Color season_colors[12] = { YELLOW, YELLOW, ORANGE, ORANGE,
                                         ORANGE, BLUE,   BLUE,   BLUE,
                                         GREEN,  GREEN,  GREEN,  YELLOW };

void output(void)
{

    setlocale(LC_ALL, "");

    i3bar_block_t i3b;
    i3bar_block_init(&i3b);

    i3b.full_text = calloc(PANGO_SZMAX * 8, sizeof(wchar_t));
    i3b.markup    = "pango";

    panspan psa;
    panspan_init(&psa);
    psa.foreground = rgbx(GREEN);

    time_t     t  = time(NULL);
    struct tm* lt = NULL;

    double temperature = -1;

    temperature = get_current_temperature_2m(LOCATION1_LAT, LOCATION1_LON);

    for(uint64_t i = 1; i < UINT64_MAX; ++i) {
        usleep(USLEEPFOR);

        setlocale(LC_ALL, "");
        fflush(stdout);

        t  = time(NULL);
        lt = localtime(&t);

        // Update weater every 2 minutes if usleep is 1 second.
        if(i == (60 * 2)) {
            i = 1;

            temperature =
              get_current_temperature_2m(LOCATION1_LAT, LOCATION1_LON);
        }

        char tf[24] = { 0 };
        strftime(tf, sizeof(tf) / sizeof(tf[0]), "%a%d%b%H%M%S", lt);

        // clang-format off
        i3bcat(
          &i3b,

          // Weather
          wpomf(modspan(psa, .foreground = rgbx(YELLOW)),
                PAT_ITAL | PAT_SMAL, "  %.02lf°C ", temperature ),

          wpomf(modspan(psa, .foreground = rgbx(FG_DIM)),
                PAT_ITAL,
                // F r i  0 9  J a n 
                " %c%c%c %c%c %c%c%c",
                // J       a       n
                tf[5],  tf[6],  tf[7],  
                // 0       9
                tf[3],  tf[4],  
                // F       r       i
                tf[0],  tf[1],  tf[2]), 

          wpomf(modspan(psa, .foreground = rgbx(season_colors[lt->tm_mon])),
                PAT_NULL,
                " %s  ",
                seasons[lt->tm_mon]),

          wpomf(modspan(psa, .foreground = rgbx(FG_DIM)),
                PAT_ITAL,
               // 2 3: 5 1: 4 2
                "%c%c:%c%c:%c%c ",
                // H       H
                tf[8],  tf[9],
                // M       M
                tf[10], tf[11],
                // S       S
                tf[12], tf[13]),

          wpomf(modspan(psa, .foreground = rgbx(FG_DIM)),
              PAT_ITAL,
              "%lc%lc%lc ",
              BRAILLE_TABLE[lt->tm_hour],
              BRAILLE_TABLE[lt->tm_min],
              BRAILLE_TABLE[lt->tm_sec]),

          wpomf(modspan(psa, .foreground = rgbx(AQUA)),
                PAT_NULL,
                "%s  ",
                clocks[lt->tm_hour]));
        // clang-format on

        i3bar_block_output(&i3b);
        memset(i3b.full_text, 0, PANGO_SZMAX * 8);
    }

    free(i3b.full_text);
}

int main(void)
{
    output();
}
