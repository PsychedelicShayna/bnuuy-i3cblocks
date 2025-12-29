#include "braille.h"
#include "color/color.h"
#include "i3bar.h"
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

int main(void) {

    setlocale(LC_ALL, "");
    int iters = 0;
    while(1) {
        iters++;

        size_t random_count = 150;

        uint8_t random_bytes[random_count];
        urandom(random_bytes, random_count);

        wchar_t random_braille[random_count];
        braille_write(random_braille, random_count, random_bytes, random_count);

        // NOTE:
        // This was the part that broke it. The random braille needs a null
        // in order for sprintf to be able to know how much of it to write
        // to the block_text_mb buffer.
        random_braille[random_count] = L'\0';

        i3bar_block_t block;
        i3bar_block_init(&block);

        size_t block_size_mb =
          (sizeof(wchar_t) * random_count) + sizeof(wchar_t) * 256;

        char block_text_mb[block_size_mb];
        memset(block_text_mb, 0, block_size_mb);

        // Write the formatted Pango string containing the random braille to
        // the block_text_mb multibyte string buffer. This has to be a char
        // and not wchar_t buffer because I have not found a "wsprintf"
        sprintf(block_text_mb,
                "<b><span color='%s'>%ls</span></b>",
                rgbx(GREEN),
                random_braille);

        // Using the same size as that of the multibyte is sure to give us
        // more space than the multibyte buffer because these will be treated
        // as wchar_t's, so it's sizeof(wchar_t) * block_size_mb due to the
        // type of this buffer being a wchar_t[].
        size_t  block_size_w = block_size_mb;
        wchar_t block_text_w[block_size_w];
        memset(block_text_w, 0, block_size_w);

        // Convert: block_text_mb -> block_text_w
        mbstowcs(block_text_w, block_text_mb, block_size_w);

        // Put a nullbyte at a reasonable location.
        // block_text_w[512] = L'\0';

        // printf("%s\n", block_text_mb);

        block.full_text = block_text_w;
        // block.full_text =
        //   L"<b><span foreground='#0FfF00'>H \"Hi\"  ello
        //   ⠅⠆⠇⡀⡁⡂ " L"  " "World World</span></b>";
        // block.short_text    = "HW";
        // block.color         = "#FFFF00";
        // block.background    = rgbx(BG_DIM);
        // block.border        = "#FFF000";
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

        block.markup                = "pango";
        block.separator             = false;
        block.separator_block_width = 0;

        i3bar_block_output(&block);
        fflush(stdout);
        usleep(500000);
    }
}
