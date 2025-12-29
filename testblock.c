#include "i3bar.h"

int main(void) {
    i3bar_block_t block;
    i3bar_block_init(&block);

    block.full_text = "<span foreground=\"#00FF00\"><i>Hello World</i></span>";
    // block.short_text = "HW";
    block.color = "#FFFF00";
    // block.background = "#000000";
    block.border = "#FFF000";
    // block.align = I3BAR_ALIGN_RIGHT;
    // block.border_top = 2;
    // block.name = "testblock";
    // block.instance = "testblock1";
    // block.urgent = true;
    // block.markup = "pango";

    block.min_width.type = I3BAR_MIN_WIDTH_TEXT;
    block.min_width.text = "Hello World!";
    block.markup         = "pango";

    i3bar_block_output_json(&block);
}
