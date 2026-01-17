#define DEBUG
#include "../src/include/i3bar.h"

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    char buf[1024];
    memset(buf, 0, 1024);

    FILE* fd = fopen("/tmp/i3b_click.json", "r");
    fread(buf, 1, 1024, fd);

    i3bar_click_t click = unmarshal_click_json(buf);

    print_i3bar_click(click);

    // wprintf(L"Click: %d, %d, %d, %d, %d %d\n",
    //         click.x,
    //         click.y,
    //         click.relative_x,
    //         click.relative_y,
    //         click.output_x,
    //         click.output_y);

    // for(int i = 0; i < 3; i++) {
    //     wprintf(L"i:%d of %s\n", i, click.modifiers[i]);
    // }

    fflush(stdout);

    fclose(fd);
}
