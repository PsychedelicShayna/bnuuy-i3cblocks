#include <curl/curl.h>
#include <locale.h>
#include <stdlib.h>

#include "color/color.h"
#include "i3bar.h"
#include "meteo.h"
#include "pango.h"
#include "private.h"

void output(void)
{
    setlocale(LC_ALL, "");

    i3bar_block_t block;
    i3bar_block_init(&block);

    panspan span;
    panspan_init(&span);
    span.foreground = rgbx(YELLOW);
    block.separator = false;

    block.markup    = "pango";
    block.full_text = calloc(PANGO_SZMAX, sizeof(wchar_t));

    while(1) {
        double temperature = 0.0;

        get_current_temperature_2m(&temperature, LOCATION1_LAT, LOCATION1_LON);

        i3bcat(&block,
               wpomf(span, PAT_ITAL | PAT_SMAL, "  %.02lf°C", temperature));

        i3bar_block_output(&block);
        memset(block.full_text, 0, PANGO_SZMAX * sizeof(wchar_t));

        usleep((1000000 * 60) * 2);
    }

    free(block.full_text);
}

int main(void)
{
    output();
    return EXIT_SUCCESS;
}
