#include <stdio.h>
#include <stdlib.h>

#include <locale.h>
#include <string.h>
#include <unistdio.h>
#include <wchar.h>

#include <inttypes.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

#include "braille.h"
#include "color/color.h"
#include "i3bar.h"
#include "pango.h"

#define DEBUG
#include "common.h"

typedef struct {
    uint64_t total;
    uint64_t free;
    uint64_t avail;
    uint64_t used_free;
    uint64_t used_avail;
    double   free_perc;
    double   avail_perc;
} disk_usage_t;

int get_disk_usage(const char* mount_point, disk_usage_t* out)
{
    struct statvfs vfs;

    if(statvfs(mount_point, &vfs) != 0) {
        return -1;
    }

    out->total      = vfs.f_blocks * vfs.f_frsize;
    out->free       = vfs.f_bfree * vfs.f_frsize;
    out->avail      = vfs.f_bavail * vfs.f_frsize;
    out->used_free  = out->total - out->free;
    out->used_avail = out->total - out->avail;

    out->free_perc =
      (double)out->used_free / (out->total ? (double)out->total : 1.0) * 100.0;

    out->avail_perc =
      (double)out->used_avail / (out->total ? (double)out->total : 1.0) * 100.0;

    return 0;
}

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

//    9B : 9B
//   99K : 99B
//  999B : 999B
// 9.99K : 9999B
// 99.9K : 99999B
// 0.99M : 999999B
// 9.99G : 9999999B
// 99.9G : 99999999B
// 1.99T : 999999999B

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

        human_size_t rate_r_hs = human_size(sample.rate.r * 1024);
        human_size_t rate_w_hs = human_size(sample.rate.w * 1024);

        disk_usage_t du1, du2;
        get_disk_usage("/", &du1);
        get_disk_usage("/mnt/alias/m2/", &du2);

        i3bcat(
          &i3b,
          wpomf(modspan(span, .foreground = rgbx(YELLOW)),
                PAT_NULL,
                "%4.*lf%c↑ %4.*lf%c↓|   %.lf%% %lldG   %.lf%% %lldG",
                rate_r_hs.precision,
                rate_r_hs.value,
                rate_r_hs.suffix,
                rate_w_hs.precision,
                rate_w_hs.value,
                rate_w_hs.suffix,
                du1.avail_perc,
                du1.avail / 1024 / 1024 / 1024,
                du2.avail_perc,
                du2.avail / 1024 / 1024 / 1024));

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
