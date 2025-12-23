// language:c

#include <stdint.h>
#include <stdio.h>

#define i3bjt "{\"full_text\": \"%s\", \"color\":\"%s\"}\n"

// Gruvbox (GB) Color Definitions/Helpers

#define GB_COLOR char*

#define GB_BG     "#282828" // [40, 40, 40]
#define GB_FG     "#ebdbb2" // [235, 219, 178]
#define GB_FG_DIM "#a89984" // [168, 153, 132]
#define GB_BLUE   "#458588" // [69, 133, 136]
#define GB_AQUA   "#689d6a" // [104, 157, 106]
#define GB_GREEN  "#98971a" // [152, 151, 26]
#define GB_YELLOW "#d79921" // [215, 153, 33]
#define GB_ORANGE "#d65d0e" // [214, 93, 14]
#define GB_RED    "#cc241d" // [204, 36, 29]
#define GB_PURPLE "#b16286" // [177, 98, 134]

#define GB_RGB_BG     {.R = 40, .G = 40, .B = 40}
#define GB_RGB_FG     {.R = 235, .G = 219, .B = 178}
#define GB_RGB_FG_DIM {.R = 168, .G = 153, .B = 132}
#define GB_RGB_BLUE   {.R = 69, .G = 133, .B = 136}
#define GB_RGB_AQUA   {.R = 104, .G = 157, .B = 106}
#define GB_RGB_GREEN  {.R = 152, .G = 151, .B = 26}
#define GB_RGB_YELLOW {.R = 215, .G = 153, .B = 33}
#define GB_RGB_ORANGE {.R = 214, .G = 93, .B = 14}
#define GB_RGB_RED    {.R = 204, .G = 36, .B = 29}
#define GB_RGB_PURPLE {.R = 177, .G = 98, .B = 134}

void hexdump(void* data, size_t size)
{
    for(size_t i = 0; i < size; ++i) {
        printf("%02lx", (uint64_t)(*((uint8_t*)data + i)));
    }
    printf("\n");
}

typedef struct {
    unsigned char R, G, B;
} RGB_Color;

typedef struct {
    double    threshold;
    RGB_Color transition;
} GB_Color_Transition_Step;

// typedef struct {
// size_t                    n_transitions;
// GB_COLOR                  default_color;
//     GB_Color_Transition_Step* steps;
// } GB_Color_Transitions;

char* RGB_to_HEX(RGB_Color color)
{
    static char hex_color[8];
    sprintf(hex_color, "#%02x%02x%02x", color.R, color.G, color.B);
    return hex_color;
}

// Maps usage to a color using an analog multiplication factor that brings it
// closer to orange, or from orange to red, depending on the thresholds, with
// certain offsets/ranges so that the color keeps looking like Gruvbox as it
// transitions from Green -> Orange -> Red.
static inline GB_COLOR gb_map_percent(double                    value,
                                      GB_Color_Transition_Step* steps,
                                      size_t                    n_steps,
                                      GB_COLOR                  default_color)
{
    for(size_t i = 0; i < n_steps; i++) {
        // The first one is a special case
        if(i == 0) {
            if(value < steps[i].threshold) {
                return RGB_to_HEX(steps[i].transition);
            }

            continue;
        }

        if(value < steps[i].threshold) {
            double    lower_threshold = steps[i - 1].threshold;
            RGB_Color lower_color     = steps[i - 1].transition;
            RGB_Color upper_color     = steps[i].transition;
            double    factor          = (value - lower_threshold)
                            / (steps[i].threshold - lower_threshold);

            int r =
                (int)(lower_color.R + factor * (upper_color.R - lower_color.R));
            int g =
                (int)(lower_color.G + factor * (upper_color.G - lower_color.G));
            int b =
                (int)(lower_color.B + factor * (upper_color.B - lower_color.B));

            RGB_Color result;
            result.R = (unsigned char)r;
            result.G = (unsigned char)g;
            result.B = (unsigned char)b;

            return RGB_to_HEX(result);
        }
    }

    return default_color;
}

// if(value < 10.0) {
//     return GB_GREEN;
// } else if(value < 50.0) { // Green -> Orange
//     double       factor = (value - 10.0) / 50.0;
//     int         r = (int)(152 + factor * (214 - 152)); // R: 152 ->
//     214 int         g = (int)(151 - factor * (151 - 93));  // G: 151
//     -> 93 int         b = (int)(26 - factor * (26 - 14));    // B: 26
//     -> 14 static char color[8]; sprintf(color, "#%02x%02x%02x", r, g,
//     b); return color;
// } else { // Orange -> Red
//     double       factor = (value - 50.0) / 50.0;
//     int         r = (int)(214 + factor * (204 - 214)); // R: 214 ->
//     204 int         g = (int)(93 - factor * (93 - 36));    // G: 93
//     -> 36 int         b = (int)(14 - factor * (14 - 29));    // B: 14
//     -> 29 static char color[8]; sprintf(color, "#%02x%02x%02x", r, g,
//     b); return color;
// }
