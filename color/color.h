/* Color Palette Selection
 * -----------------------
 * If a color palette is not defined at compile time, the color palette here
 * will default to Gruvbox, unless another palette is specified by defining
 * COLOR_PALETTE to point to another palette header file. You can modify this
 * line to point to your preferred palette header file, or define COLOR_PALETTE
 * at compile time using -DCOLOR_PALETTE="path/to/palette.h"
 * */

#ifndef COLOR_PALETTE
#define COLOR_PALETTE "./palettes/gruvbox.h"
#endif

// ````````````````````````````````````````````````````````````````````````````

#include <stdint.h>
#include <string.h>

typedef struct {
    uint8_t r, g, b;
} Color;

typedef struct {
    Color  color;
    double thresh;
} GradientStep;

// Acts as a nullbyte in a GradientStep[]
static const GradientStep _null_GradientStep = { 0 };
#define GSTEP_ISNULL(p) (memcmp((p), &_null_GradientStep, sizeof *(p)) == 0)

// clang-format off
#define Threshold(T, C) (GradientStep) {  .color = C, .thresh = T }
#define Gradient(...) (GradientStep[]) { __VA_ARGS__, _null_GradientStep }
// clang-format on

#define _PALETTE_COLOR_TYPE Color
#include COLOR_PALETTE

/* Fallback Color Palette ~ used when no Palette header is included */
#ifndef PALETTE
#define PALETTE FALLBACK
// clang-format off //                     Fallback Color Palette
#define BG     (_PALETTE_COLOR_TYPE) { .R = 000, .G = 000, .B = 000 }
#define FG     (_PALETTE_COLOR_TYPE) { .R = 255, .G = 255, .B = 255 }
#define FG_DIM (_PALETTE_COLOR_TYPE) { .R = 127, .G = 127, .B = 127 }
#define BLUE   (_PALETTE_COLOR_TYPE) { .R = 000, .G = 000, .B = 255 }
#define AQUA   (_PALETTE_COLOR_TYPE) { .R = 000, .G = 000, .B = 255 }
#define GREEN  (_PALETTE_COLOR_TYPE) { .R = 000, .G = 255, .B = 000 }
#define YELLOW (_PALETTE_COLOR_TYPE) { .R = 255, .G = 255, .B = 000 }
#define ORANGE (_PALETTE_COLOR_TYPE) { .R = 255, .G = 255, .B = 000 }
#define RED    (_PALETTE_COLOR_TYPE) { .R = 255, .G = 000, .B = 000 }
#define PURPLE (_PALETTE_COLOR_TYPE) { .R = 255, .G = 000, .B = 255 }
// clang-format on //
#endif

// ----------------------------------------------------------------------------

#include <stdint.h>
#include <stdio.h>

#define JSON_OUTPUT_TEMPLATE "{\"full_text\": \"%s\", \"color\":\"%s\"}\n"

void hexdump(void* data, size_t size) {
    for(size_t i = 0; i < size; ++i) {
        printf("%02lx", (uint64_t)(*((uint8_t*)data + i)));
    }
    printf("\n");
}

char* rgbx(Color color) {
    static char hex_color[8];
    sprintf(hex_color, "#%02x%02x%02x", color.r, color.g, color.b);
    return hex_color;
}

// const RGB_Color BG     = {.R = 40, .G = 40, .B = 40};
// const RGB_Color FG     = {.R = 235, .G = 219, .B = 178};
// const RGB_Color FG_DIM = {.R = 168, .G = 153, .B = 132};
// const RGB_Color BLUE   = {.R = 69, .G = 133, .B = 136};
// const RGB_Color AQUA   = {.R = 104, .G = 157, .B = 106};
// const RGB_Color GREEN  = {.R = 152, .G = 151, .B = 26};
// const RGB_Color YELLOW = {.R = 215, .G = 153, .B = 33};
// const RGB_Color ORANGE = {.R = 214, .G = 93, .B = 14};
// const RGB_Color RED    = {.R = 204, .G = 36, .B = 29};
// const RGB_Color PURPLE = {.R = 177, .G = 98, .B = 134};

// typedef struct {
// size_t                    n_transitions;
// GB_COLOR                  default_color;
//     Transition* steps;
// } GB_Color_Transitions;

// Maps usage to a color using an analog multiplication factor that brings it
// closer to orange, or from orange to red, depending on the threshs, with
// certain offsets/ranges so that the color keeps looking like Gruvbox as it
// transitions from Green -> Orange -> Red.
// static inline GB_COLOR gb_map_percent(double                    value,
//                                       Transition* steps,
//                                       size_t                    n_steps,
//                                       GB_COLOR default_color)
// {
//     for(size_t i = 0; i < n_steps; i++) {
//         // The first one is a special case
//         if(i == 0) {
//             if(value < steps[i].thresh) {
//                 return RGB_to_HEX(steps[i].color);
//             }
//
//             continue;
//         }
//
//         if(value < steps[i].thresh) {
//             double    lower_thresh = steps[i - 1].thresh;
//             RGB_Color lower_color     = steps[i - 1].color;
//             RGB_Color upper_color     = steps[i].color;
//             double    factor          = (value - lower_thresh)
//                             / (steps[i].thresh - lower_thresh);
//
//             int r =
//                 (int)(lower_color.R + factor * (upper_color.R -
//                 lower_color.R));
//             int g =
//                 (int)(lower_color.G + factor * (upper_color.G -
//                 lower_color.G));
//             int b =
//                 (int)(lower_color.B + factor * (upper_color.B -
//                 lower_color.B));
//
//             RGB_Color result;
//             result.R = (unsigned char)r;
//             result.G = (unsigned char)g;
//             result.B = (unsigned char)b;
//
//             return RGB_to_HEX(result);
//         }
//     }
//
//     return default_color;
// }

static inline char* map_to_color(double value, GradientStep steps[]) {
    size_t length = 0;

    for(size_t i = 0; i < INT16_MAX; i++) {
        GradientStep s = steps[i];

        // Checks for the null terminator GradientStep _null_GradientStep
        if(GSTEP_ISNULL(&s)) {
            break;
        }

        // On the first step, don't perform interpolation, we want a flat
        // baseline color until we exceed the first threshold.
        if(i == 0) {
            if(value < steps[i].thresh) {
                return rgbx(steps[i].color);
            }

            continue;
        }

        // NOTE: Maybe try experimenting with lower being i, and upper being i+1
        // which would require deleting the first iteration special case above.

        if(value < steps[i].thresh) {
            double thresh = steps[i - 1].thresh;
            Color  start  = steps[i - 1].color;
            Color  end    = steps[i].color;

            // This factor determines how far along we are between the two
            // colors being interpolated, in order to get a smooth transition
            // between them.
            double factor = (value - thresh) / (steps[i].thresh - thresh);

            // Here we perform linear interpolation between the start and end
            // colors based on the calculated factor. The result is clamped to
            // the range of 0-255 for each color component by casting to int32_t
            // before assigning back to uint8_t.
            int32_t r = (int32_t)(start.r + factor * (end.r - start.r));
            int32_t g = (int32_t)(start.g + factor * (end.g - start.g));
            int32_t b = (int32_t)(start.b + factor * (end.b - start.b));

            Color result;
            result.r = (uint8_t)r;
            result.g = (uint8_t)g;
            result.b = (uint8_t)b;

            return rgbx(result);
        }

        printf("%f, %d %d %d",
               steps[0].thresh,
               steps[0].color.r,
               steps[0].color.g,
               steps[0].color.b);
    }

    return length > 0 ? rgbx(steps[length - 1].color) : "#FFFFFF";
}

void x(void) {
    map_to_color(50.0,
                 Gradient(Threshold(20.0, GREEN),
                          Threshold(50.0, ORANGE),
                          Threshold(10.0, RED)));
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
