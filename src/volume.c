#include <errno.h>
#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>

#define JSON_MACROS
#include "common.h"

#include "color/color.h"
#include "i3bar.h"
#include "pango.h"
#include "private.h"

static void trim_nl(char* s)
{
    size_t n = strlen(s);
    if(n && s[n - 1] == '\n')
        s[n - 1] = '\0';
}

int get_volume(int* out, bool* muted)
{
    // Volume: 0.64 [MUTED]

    double vol = 0;

    FILE* fd = popen("wpctl get-volume @DEFAULT_AUDIO_SINK@", "r");

    if(fd == NULL) {
        return -1;
    }

    char mute_char = '\0';
    int  n         = fscanf(fd, "Volume: %lf [MUTE%c]", &vol, &mute_char);

    // fscanf(fd, "[%c", &mute_char);

    pclose(fd);

    if(out != NULL) {
        *out = (int)round(vol * 100);
    }

    if(muted != NULL) {
        *muted = mute_char != '\0';
    }

    return n;
}

/*
 * If offset is 0 then volume is set directly to `vol`, otherwise if it is
 * positive e.g. `1` then the volume is increased relative to `vol` and if
 * offset is negative e.g. `-1` the volume is decreased relative to `vol`
 * */
int set_volume(int vol, int offset)
{
    char buf[256];
    memset(buf, 0, 256);

    const char* suffix = "%";

    if(offset == 0) {
        suffix = "%";
    } else if(offset < 0) {
        suffix = "%-";
    } else if(offset > 0) {
        suffix = "%+";
    }

    sprintf(buf, "wpctl set-volume @DEFAULT_AUDIO_SINK@ %d%s", vol, suffix);
    return system(buf);
}

int set_mute(bool mute)
{
    char buf[256];
    memset(buf, 0, 256);

    sprintf(buf, "wpctl set-mute @DEFAULT_AUDIO_SINK@ %d", mute ? 1 : 0);
    return system(buf);
}

int toggle_mute()
{
    return system("wpctl set-mute @DEFAULT_AUDIO_SINK@ toggle");
}

wchar_t get_volume_icon()
{
    wchar_t normal = L'';
    wchar_t low    = L'';
    wchar_t muted  = L'';

    int  volume   = -1;
    bool is_muted = false;

    get_volume(&volume, &is_muted);

    if(volume == -1) {
        return L'?';
    }

    if(is_muted) {
        return muted;
    }

    if(volume < 50) {
        return low;
    } else {
        return normal;
    }
}

int paint_slider(wchar_t* dst, size_t width, int volume)
{
    // examples
    // ⠶⣿⠶⠶⠶⠶⠶⠶⠶⠶⠶⠶⠶ ⣀⣀⣀⣀⣀⣀⣿⣀⣀ ⠤⠤⠤⠤⠤⠤⠤⠤⠤⠤⠤⠤⠤⡇ ⠤⠤⠤⠤⠤⠤⣿⠤⠤

    wchar_t slider[width];
    wchar_t notch = L'⣿';
    wchar_t slide = L'⠶';

    wmemset(slider, slide, width);

    double idxf = ceil(((double)volume * (double)width) / 100.0);
    int    idx  = (int)fmin(fmax(idxf, 0), (double)width - 1); // clamp

    slider[idx] = notch;
    wmemcpy(dst, slider, width);

    return idx;
}

void output(void)
{
    setlocale(LC_ALL, "");

    i3bar_block_t block;
    i3bar_block_init(&block);
    block.markup    = "pango";
    block.full_text = calloc(PANGO_SZMAX, sizeof(wchar_t));
    block.separator = false;
    // memset(block.full_text, 0, PANGO_SZMAX * sizeof(wchar_t));

    panspan span;
    panspan_init(&span);
    span.foreground = rgbx(AQUA);

    // setvbuf(stdout, NULL, _IONBF, 0); // unbuffered stdout

    char*   line    = NULL;
    size_t  linecap = 0;
    ssize_t linelen;

    // wprintf("VOL\n");
    // fflush(stdout);  

    // i3bcat(&block, wpomf(span, PAT_NULL, "%lc  ", get_volume_icon()));
    // i3bar_block_output(&block);
    //
    // memset(block.full_text, 0, PANGO_SZMAX * sizeof(wchar_t));

    bool expanded = false;

    const int TIMEOUT_SEC = 2;

    while(1) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);

        struct timeval tv;

        if(expanded) {
            tv.tv_sec  = TIMEOUT_SEC;
            tv.tv_usec = 0;
        } else {
            tv.tv_sec  = 0;
            tv.tv_usec = 150000;
        }

        int rv = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);

        if(rv < 0) {
            if(errno == EINTR)
                continue;
            perror("select");
            break;
        } else if(rv == 0) {
            // timeout, collapse to icon
            expanded = false;

            int  vol   = -1;
            bool muted = false;
            get_volume(&vol, &muted);

            Color fg = muted ? FG_DIM : (vol > 100 ? RED : AQUA);

            i3bcat(&block,
                   wpomf(modspan(span, .foreground = rgbx(fg)),
                         PAT_NULL,
                         "%lc",
                         get_volume_icon()));

            i3bar_block_output(&block);

            memset(block.full_text, 0, PANGO_SZMAX * sizeof(wchar_t));
            usleep(1000);
        }

        if(FD_ISSET(STDIN_FILENO, &rfds)) {
            linelen = getline(&line, &linecap, stdin);

            if(linelen < 0) {
                if(feof(stdin)) {
                    // pipe closed exit cleanly
                    break;
                } else {
                    perror("getline");
                    break;
                }
            }

            trim_nl(line);

            //  󰕿 󰕾            󰸈  󰝟  󰝞  󰝝  󰖀
            // 󰕿 󰖁  󰕾  󱄠 system("wpctl set-volume
            // @DEFAULT_AUDIO_SINK@ 5%-"); system("wpctl set-volume
            // @DEFAULT_AUDIO_SINK@ 5%+");

            // FILE* fd = fopen("/tmp/i3b_click.json", "w");
            // fwrite(line, sizeof(char), 252, fd);
            // fclose(fd);

            i3bar_click_t click = unmarshal_click_json(line);

            // i3bar_click_t click = unmarshal_click_json(line);
            // wprintf(L"Click: %d, %d, %d, %d, %d %d\n",
            //         click.x,
            //         click.y,
            //         click.relative_x,
            //         click.relative_y,
            //         click.output_x,
            //         click.output_y);
            //
            // fflush(stdout);

            int vol = -1;

            // 9ish difference on x coordinate per character?

            if(click.button == I3B_WHEELDOWN) {
                set_volume(5, -1);
            } else if(click.button == I3B_WHEELUP) {
                set_volume(5, 1);
            } else if(click.button == I3B_RCLICK) {
                toggle_mute();

                if(!expanded) {
                    continue;
                }
            }

            expanded = true;

            bool muted = false;

            get_volume(&vol, &muted);

            wchar_t slider[17];
            paint_slider(slider, 16, vol);
            slider[16] = L'\0';

            Color fg = muted ? FG_DIM : (vol > 100 ? RED : AQUA);

            i3bcat(&block,
                   wpomf(modspan(span, .foreground = rgbx(fg)),
                         PAT_NULL,
                         "%ls %3d%%",
                         slider,
                         vol));

            i3bar_block_output(&block);
            memset(block.full_text, 0, PANGO_SZMAX * sizeof(wchar_t));
            usleep(1);
        }
    }

    free(block.full_text);
}

int main(void)
{
    setlocale(LC_ALL, "");
    output();
    return 0;

    wchar_t slider[17];
    int     vol = -1;
    get_volume(&vol, NULL);
    paint_slider(slider, 16, vol);
    slider[33] = L'\0';

    wprintf(L"Slider: %ls\n", slider);
    fflush(stdout);

    // int vol = -1;
    // int x   = get_volume(&vol);
    // wprintf(L"x:%d, v:%d%%\n", x, vol);
    // fflush(stdout);
    // set_volume(5, 1);
    return EXIT_SUCCESS;
}
