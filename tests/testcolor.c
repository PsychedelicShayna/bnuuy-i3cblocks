#include "color/color.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {

    Color a = (Color) { 255, 0, 0 };
    Color b = (Color) { 0, 255, 0 };
    Color c = (Color) { 0, 0, 255 };

    char* hexes[3];
    hexes[0] = rgbx(a);
    hexes[1] = rgbx(b);
    rgbx(c);

    hexes[1][0] = 1;

    char buff[3][9];
    memset(buff, 0, 3 * 4);

    // sprintfhex(buff[0], a);
    // sprintfhex(buff[1], b);

    printf("%s, %s", buff[0], buff[1]);

    return 0;


    char buf[32];
    memset(buf, 0, 32);
    memcpy(buf, hexes[0], 8);

    write(1, rgbx(a), 8);
    write(1, "\n", 1);
    write(1, rgbx(b), 8);
    write(1, "\n", 1);

    write(1, hexes[0], 8);
    write(1, "\n", 1);
    fflush(stdout);
    // write(1, "\n", 1);
    // fflush(stdout);
    // write(1, hexes[1], 8);
    // write(1, "\n", 1);
    // fflush(stdout);
    //
    return EXIT_SUCCESS;
}
