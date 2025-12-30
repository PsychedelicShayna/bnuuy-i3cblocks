#ifndef _I3BAR_H
#define _I3BAR_H

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <wchar.h>

#define I3B_ALIGN_LEFT   "left"
#define I3B_ALIGN_RIGHT  "right"
#define I3B_ALIGN_CENTER "center"

#include <stdbool.h>

typedef enum { FALSE = 0, TRUE = 1, I3B_DEFAULT_BOOL = -1 } jboolean;

typedef enum {
    I3B_MIN_WIDTH_PIXELS,
    I3B_MIN_WIDTH_TEXT,
    I3B_MIN_WIDTH_NONE
} i3bar_proto_min_width_type;

typedef struct {
    i3bar_proto_min_width_type type;
    union {
        int   pixels;
        char* text;
    } u;
} i3bar_proto_min_width;

#define I3B_DEFAULT_MIN_WIDTH_PIXELS      300
#define I3B_DEFAULT_ALIGN                 I3B_ALIGN_LEFT
#define I3B_DEFAULT_BORDER                1
#define I3B_DEFAULT_SEPARATOR_BLOCK_WIDTH 9
#define I3B_DEFAULT_SEPARATOR             true
#define I3B_DEFAULT_MARKUP                "none"
#define I3B_DEFAULT_INT                   INT32_MAX

struct i3bar_proto_block {
    /* The full_text will be displayed by i3bar on the status line.
     * This is the only required key. If full_text is an empty string,
     * the block will be skipped. */
    wchar_t* full_text;

    /* Where appropriate, the short_text (string) entry should also be provided.
     * It will be used in case the status line needs to be shortened because it
     * uses more space than your screen provides. For example, when displaying
     * an IPv6 address, the prefix is usually (!) more relevant than the suffix,
     * because the latter stays constant when using autoconf, while the prefix
     * changes. When displaying the date, the time is more important than the
     * date (it is more likely that you know which day it is than what time it
     * is). */
    char* short_text;

    /* To make the current state of the information easy to spot, colors can be
     * used. For example, the wireless block could be displayed in red (using
     * the color (string) entry) if the card is not associated with any network
     * and in green or yellow (depending on the signal strength) when it is
     * associated. Colors are specified in hex (like in HTML), starting with a
     * leading hash sign. For example, #ff0000 means red.
     */
    char* color;

    /* Overrides the background color for this particular block.*/
    char* background;

    /* Overrides the border color for this particular block. */
    char* border;

    /* Defines the width (in pixels) of the top border of this block.
     * Defaults to 1. */
    int border_top;

    /* Defines the width (in pixels) of the right border of this block.
     * Defaults to 1. */
    int border_right;

    /* Defines the width (in pixels) of the bottom border of this block.
     * Defaults to 1. */
    int border_bottom;

    /* Defines the width (in pixels) of the left border of this block.
     * Defaults to 1. */
    int border_left;

    /* The minimum width (in pixels) of the block. If the content of the
     * full_text key take less space than the specified min_width, the block
     * will be padded to the left and/or the right side, according to the
     * align key. This is useful when you want to prevent the whole status
     * line to hift when value take more or less space between each iteration.
     * The value can also be a string. In this case, the width of the text given
     * by min_width determines the minimum width of the block. This is useful
     * when you want to set a sensible minimum width regardless of which font
     * you are using, and at what particular size. */
    i3bar_proto_min_width min_width;

    /* Align text on the center, right or left (default) of the block, when
     * the minimum width of the latter, specified by the min_width key, is not
     * reached. */
    char* align;

    /* A boolean which specifies whether the current value is urgent. Examples
     * are battery charge values below 1 percent or no more available disk space
     * (for non-root users). The presentation of urgency is up to i3bar.  */
    jboolean urgent;

    /* Every block should have a unique name (string) entry so that it can be
     * easily identified in scripts which process the output. i3bar completely
     * ignores the name and instance fields. */
    char* name;

    /* (See: char* name)
     * Make sure to also specify an instance (string) entry where appropriate.
     * For example, the user can have multiple disk space blocks for multiple
     * mount points. */
    char* instance;

    /* A boolean which specifies whether a separator line should be drawn after
     * this block. The default is true, meaning the separator line will be
     * drawn. Note that if you disable the separator line, there will still be a
     * gap after the block, unless you also use separator_block_width.  */
    jboolean separator;

    /* The amount of pixels to leave blank after the block. In the middle of
     * this gap, a separator line will be drawn unless separator is disabled.
     * Normally, you want to set this to an odd value (the default is 9 pixels),
     * since the separator line is drawn in the middle.  */
    int separator_block_width;

    /* A string that indicates how the text of the block should be parsed. Set
     * to "pango" to use Pango markup. Set to "none" to not use any markup
     * (default). Pango markup only works if you use a pango font. If you want
     * to put in your own entries into a block, prefix the key with an
     * underscore(_). i3bar will ignore all keys it doesn’t understand, and
     * prefixing them with an underscore makes it clear in every script that
     * they are not part of the i3bar protocol. */
    char* markup;
};

typedef struct i3bar_proto_block i3bar_block_t;

void i3bar_block_init(i3bar_block_t* block) {
    block->full_text     = NULL;
    block->short_text    = NULL;
    block->color         = NULL;
    block->background    = NULL;
    block->border        = NULL;
    block->border_top    = I3B_DEFAULT_INT;
    block->border_right  = I3B_DEFAULT_INT;
    block->border_bottom = I3B_DEFAULT_INT;
    block->border_left   = I3B_DEFAULT_INT;
    block->min_width = (i3bar_proto_min_width) { .type = I3B_MIN_WIDTH_NONE };
    block->align     = NULL;
    block->urgent    = I3B_DEFAULT_BOOL;
    block->name      = NULL;
    block->instance  = NULL;
    block->separator = I3B_DEFAULT_BOOL;
    block->separator_block_width = I3B_DEFAULT_INT;
    block->markup                = NULL;
}
/*

   JSON Output Example:
{
 "full_text": "E: 10.0.0.1 (1000 Mbit/s)",
 "short_text": "10.0.0.1",
 "color": "#00ff00",
 "background": "#1c1c1c",
 "border": "#ee0000",
 "border_top": 1,
 "border_right": 0,
 "border_bottom": 3,
 "border_left": 1,
 "min_width": 300,
 "align": "right",
 "urgent": false,
 "name": "ethernet",
 "instance": "eth0",
 "separator": true,
 "separator_block_width": 9,
 "markup": "'pango' makes the full_text capable of containing per character
markdown with the pango markup language"
}
   */

// Serialize any i3bar block to JSON format for i3bar consumption, including
// only the fields that are set, and escaping strings with quotes or other
// special characters as needed.
void i3bar_block_output(i3bar_block_t* block) {
    if(block->full_text == NULL || block->full_text[0] == '\0') {
        block->full_text = L"NULL";
    }

    wprintf(L"{");

    bool first = true;

#define HANDLE_SEPARATOR \
    if(!first) {         \
        wprintf(L", ");  \
    }

#define OUTPUT_FIELD_STRING(field, value)         \
    if(block->value && block->value[0] != '\0') { \
        HANDLE_SEPARATOR                          \
        wprintf(L"\"" #field "\": \"");           \
        for(char* p = block->value; *p; ++p) {    \
            if(*p == '\"' || *p == '\\') {        \
                wprintf(L"\\");                   \
            }                                     \
            wprintf(L"%c", *p);                   \
        }                                         \
        wprintf(L"\"");                           \
        first = false;                            \
    }

#define OUTPUT_FIELD_INT(field, value)                \
    if(block->value != I3B_DEFAULT_INT) {             \
        HANDLE_SEPARATOR                              \
        wprintf(L"\"" #field "\": %d", block->value); \
        first = false;                                \
    }

#define OUTPUT_FIELD_BOOL(field, value)                                       \
    if(block->value != I3B_DEFAULT_BOOL) {                                    \
        HANDLE_SEPARATOR                                                      \
        wprintf(L"\"" #field "\": %s", block->value >= 1 ? "true" : "false"); \
        first = false;                                                        \
    }

    if(block->full_text && block->full_text[0] != '\0') {
        if(!first) {
            wprintf(L", ");
        }
        wprintf(L"\""
                "full_text"
                "\": \"");
        for(wchar_t* p = block->full_text; *p; ++p) {
            if(*p == '\"' || *p == '\\') {
                wprintf(L"\\");
            }
            wprintf(L"%lc", *p);
        }
        wprintf(L"\"");
        first = 0;
    };

    OUTPUT_FIELD_STRING(short_text, short_text);
    OUTPUT_FIELD_STRING(color, color);
    OUTPUT_FIELD_STRING(background, background);
    OUTPUT_FIELD_STRING(border, border);
    OUTPUT_FIELD_INT(border_top, border_top);
    OUTPUT_FIELD_INT(border_right, border_right);
    OUTPUT_FIELD_INT(border_bottom, border_bottom);
    OUTPUT_FIELD_INT(border_left, border_left);

    switch(block->min_width.type) {
        case I3B_MIN_WIDTH_PIXELS:
            OUTPUT_FIELD_INT(min_width, min_width.u.pixels)
            break;
        case I3B_MIN_WIDTH_TEXT:
            OUTPUT_FIELD_STRING(min_width, min_width.u.text)
            break;
        default:
            break;
    }

    OUTPUT_FIELD_STRING(align, align);
    OUTPUT_FIELD_BOOL(urgent, urgent);
    OUTPUT_FIELD_STRING(name, name);
    OUTPUT_FIELD_STRING(instance, instance);
    OUTPUT_FIELD_BOOL(separator, separator);
    OUTPUT_FIELD_INT(separator_block_width, separator_block_width);
    OUTPUT_FIELD_STRING(markup, markup);

    wprintf(L"}\n");
    fflush(stdout);
}

#endif // !_I3BAR_H
