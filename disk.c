#include <math.h>
#include <stdio.h>
#include <stdlib.h>
void* hijack_malloc(unsigned long s)
{
    fprintf(stderr, "Malloc: %zu\n", s);
    fflush(stderr);
    return malloc(s);
}
void* hijack_calloc(unsigned long s, unsigned long c)
{
    fprintf(stderr, "Calloc: %zu*%zu=%zu\n", s, c, s * c);
    fflush(stderr);
    return calloc(s, c);
}
void hijack_free(void* __ptr)
{
    fprintf(stderr, "Free: %p\n", __ptr);
    fflush(stderr);
    free(__ptr);
}

#define malloc(s)    hijack_malloc(s)
#define calloc(s, c) hijack_calloc(s, c)
#define free(p)      hijack_free(p)

#include <locale.h>
#include <string.h>
#include <unistdio.h>
#include <wchar.h>

#include <sys/stat.h>

#include "color/color.h"
#include "i3bar.h"
#include "pango.h"

#define DEBUG
#include "common.h"

typedef struct {
    uint64_t r;
    uint64_t w;
} disk_rw_stats_t;

typedef struct {
    // Raw
    disk_rw_stats_t s1;
    disk_rw_stats_t s2;

    // KiB
    disk_rw_stats_t rate;
} disk_rw_rate_t;

disk_rw_rate_t sample_disk_rw_stats(const char* device)
{
    char sys_block_path[256];
    memset(sys_block_path, 0, 256);

    sprintf(sys_block_path, "/sys/block/%s/stat", device);
    FILE* fd = fopen(sys_block_path, "rb");

    // struct stat path_stat;

    // if(stat(sys_block_path, &path_stat) != 0) {
    //     perror("stat");
    //     exit(EXIT_FAILURE);
    // }

    disk_rw_rate_t rw_rate = { 0 };

    uint64_t null;

    fscanf(fd,
           "%ld %ld %ld %ld %ld %ld %ld",
           &null,
           &null,
           &rw_rate.s1.r,
           &null,
           &null,
           &null,
           &rw_rate.s1.w);

    fseek(fd, 0, 0);

    usleep(1000000);

    fscanf(fd,
           "%ld %ld %ld %ld %ld %ld %ld",
           &null,
           &null,
           &rw_rate.s2.r,
           &null,
           &null,
           &null,
           &rw_rate.s2.w);
    fclose(fd);

    rw_rate.rate.r = (rw_rate.s2.r - rw_rate.s1.r) * 512 / 1024;
    rw_rate.rate.w = (rw_rate.s2.w - rw_rate.s1.w) * 512 / 1024;

    return rw_rate;
}

void output()
{
    setlocale(LC_ALL, "");

    i3bar_block_t i3b;
    i3bar_block_init(&i3b);

    i3b.full_text = calloc(PANGO_SZMAX, sizeof(wchar_t));

    panspan span;
    panspan_init(&span);
    i3b.markup    = "pango";
    i3b.separator = false;

    span.foreground = rgbx(YELLOW);

    for(;;) {
        disk_rw_rate_t sample = sample_disk_rw_stats("nvme0n1");

        char r_suffix = 'K', w_suffix = 'K';

        double rate_r = (double)sample.rate.r;
        double rate_w = (double)sample.rate.w;

        rate_r = represent_size(rate_r, &r_suffix);
        rate_w = represent_size(rate_w, &w_suffix);

        bool r_float = (rate_r - floor(rate_r)) > 0;
        bool w_float = (rate_w - floor(rate_w)) > 0;

        const char* pre_fmt = " %c %s ↑↓ %s %c";

        static char b1[16 + 13];
        static char b2[16 + 13];
        memset(b1, 0, sizeof(b1));
        memset(b2, 0, sizeof(b2));

        sprintf(b1,
                pre_fmt,
                r_suffix,
                r_float ? "%6.02lf" : "%6ld",
                w_float ? "%-6.02lf" : "%-6ld",
                w_suffix);

        sprintf(b2, b1, r_float ? rate_r : (ulong)rate_r,
                        w_float ? rate_w : (ulong)rate_w);

        i3bcat(
          &i3b,
            wpomf(modspan(span, .foreground = rgbx(YELLOW)), PAT_NULL, "%s", b2)

        );

        i3bar_block_output(&i3b);
        memset(i3b.full_text, 0, PANGO_SZMAX * sizeof(wchar_t));
    }

    free(i3b.full_text);
}

int main(void)
{
    output();

    return EXIT_SUCCESS;
}

// printf("   %s %s", $used_perc, $avail_size);
// /dev/nvme1n1p1  1.8T  1.7T   44G  98% /mnt/alias/m2
//
// R=$(( (${NEW[0]} - ${OLD[0]}) * 512 / 1024 ))
// W=$(( (${NEW[1]} - ${OLD[1]}) * 512 / 1024 ))
// NEW=($(cat /sys/block/$DEVICE/stat | awk '{print $3, $7}'))
// echo "${R}K↓ ${W}K↑"
// 1097035   283840 63175118   220276  2327008  2721592 246934900  9284296 0
// 412086  9654669   159119        0 306546560    54765   270087    95330
