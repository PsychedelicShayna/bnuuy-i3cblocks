#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "i3bar.h"
#include "pango.h"

int main(void)
{
    pangosb    psb  = pangosb_create(4096);
    pango_span span = panspan_create();
    span.foreground = "#00FF00";


    // span.background = "#000000";
    // span.scale = 1.5;
    // span.size = 5;
    // span.strikethrough = 1;
    // span.rise = -1;

    pangosb_wpush(&psb, L"Hello World",  PSBT_ITAL, &span);
    // pangosb_wpush(&psb, L"Hello World2", PSBT_BIG | PSBT_SUP, NULL);
    // pangosb_wpush(
    //   &psb,
    //   L"Top of the morning to you laddies my name is the arena allocator and
    //   " L"welcome back to memory management fucking hell, with a twist!",
    //   PSBT_BIG | PSBT_SUP | PSBT_MONO,
    //   NULL);
    //

    // wchar_t fulltextbuf[4096];
    // memset(fulltextbuf, 0, sizeof(fulltextbuf));

    // swprintf(fulltextbuf, 4096, L"")

    wchar_t* ft = psb.strings[0];
    // wprintf(L"%ls\n", ft);


    i3bar_block_t block;
    i3bar_block_init(&block);
    block.markup = "pango";
    block.full_text = ft;
    i3bar_block_output(&block);

    pangosb_fini(&psb);
}
