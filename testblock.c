#include "braille.h"
#include "i3bar.h"
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

void urandom(uint8_t* out, size_t szout) {
    FILE* fd = fopen("/dev/urandom", "r");
    fread(out, 1, szout, fd);
    fclose(fd);
}

bool coinflip() {
    uint8_t buf[1];
    urandom(buf, 1);

    return buf[0] > 127;
}

int main(void) {

    setlocale(LC_ALL, "");
    int iters = 0;
    while(1) {
        iters++;
        uint8_t rbuf[128];
        urandom(rbuf, 128);

        wchar_t wbuf[128];
        size_t  w = braille_write(wbuf, 128, rbuf, 128);
        // wprintf(L"Wrote %zu\n", w);
        wbuf[64] = L'\0';
        // wprintf(L"%ls\n", wbuf);

        i3bar_block_t block;
        i3bar_block_init(&block);

        char full_text[4096];
        memset(full_text, 0, 4096);

        int wr = sprintf(full_text,
                         "<b><span color='#00FF00'>   %ls   </span></b>",
                         wbuf);
        // wprintf(L"%d\n%s\n", wr, full_text);
        wchar_t wfull[127];
        memset(wfull, 0, 127 * sizeof(wchar_t));

        // full_text[128] = '\0';
        mbstowcs(wfull, full_text, 128);

        // wprintf(L"FULL\n%ls\n", wfull);

        block.full_text = wfull;
        // block.full_text =
        //   L"<b><span foreground='#0FfF00'>H \"Hi\"  ello
        //   ⠅⠆⠇⡀⡁⡂ " L"  " "World World</span></b>";
        // block.short_text    = "HW";
        block.color         = "#FFFF00";
        block.background    = "#000000";
        block.border        = "#FFF000";
        block.align         = I3B_ALIGN_CENTER;
        block.border_top    = 0;
        block.border_bottom = 0;
        block.border_right  = 0;
        block.border_left   = 0;
        block.name          = "testblock";
        block.instance      = "testblock1";
        // block.urgent     = true;
        block.markup             = "pango";
        block.min_width.type     = I3B_MIN_WIDTH_PIXELS;
        block.min_width.u.pixels = 300;

        block.markup = "pango";
        // block.separator = false;

        i3bar_block_output(&block);
        fflush(stdout);
        usleep(100000);
    }
}
