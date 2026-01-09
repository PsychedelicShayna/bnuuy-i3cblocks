#ifndef _PANGO_H
#define _PANGO_H

#define PANGO_SZMAX 1300

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

static const char* TAGMAP_CSTR[257] = {
    NULL, "i",  "b",  NULL, "u",   NULL, NULL, NULL, "s",     NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, "tt",  NULL, NULL, NULL, NULL,    NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,  NULL, NULL, NULL, "sub",   NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL,    NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL,    NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, "sup", NULL, NULL, NULL, NULL,    NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL,    NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL,    NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL,    NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL,    NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,  NULL, NULL, NULL, "small", NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL,    NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL,    NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL,    NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL,    NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL,    NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL,    NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL,    NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL,    NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL,    NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL,    NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, "big"
};

static const wchar_t* TAGMAP[257] = {
    NULL, L"i", L"b", NULL,  L"u", NULL,  NULL, NULL,     L"s", NULL,   NULL,
    NULL, NULL, NULL, NULL,  NULL, L"tt", NULL, NULL,     NULL, NULL,   NULL,
    NULL, NULL, NULL, NULL,  NULL, NULL,  NULL, NULL,     NULL, NULL,   L"sub",
    NULL, NULL, NULL, NULL,  NULL, NULL,  NULL, NULL,     NULL, NULL,   NULL,
    NULL, NULL, NULL, NULL,  NULL, NULL,  NULL, NULL,     NULL, NULL,   NULL,
    NULL, NULL, NULL, NULL,  NULL, NULL,  NULL, NULL,     NULL, L"sup", NULL,
    NULL, NULL, NULL, NULL,  NULL, NULL,  NULL, NULL,     NULL, NULL,   NULL,
    NULL, NULL, NULL, NULL,  NULL, NULL,  NULL, NULL,     NULL, NULL,   NULL,
    NULL, NULL, NULL, NULL,  NULL, NULL,  NULL, NULL,     NULL, NULL,   NULL,
    NULL, NULL, NULL, NULL,  NULL, NULL,  NULL, NULL,     NULL, NULL,   NULL,
    NULL, NULL, NULL, NULL,  NULL, NULL,  NULL, NULL,     NULL, NULL,   NULL,
    NULL, NULL, NULL, NULL,  NULL, NULL,  NULL, L"small", NULL, NULL,   NULL,
    NULL, NULL, NULL, NULL,  NULL, NULL,  NULL, NULL,     NULL, NULL,   NULL,
    NULL, NULL, NULL, NULL,  NULL, NULL,  NULL, NULL,     NULL, NULL,   NULL,
    NULL, NULL, NULL, NULL,  NULL, NULL,  NULL, NULL,     NULL, NULL,   NULL,
    NULL, NULL, NULL, NULL,  NULL, NULL,  NULL, NULL,     NULL, NULL,   NULL,
    NULL, NULL, NULL, NULL,  NULL, NULL,  NULL, NULL,     NULL, NULL,   NULL,
    NULL, NULL, NULL, NULL,  NULL, NULL,  NULL, NULL,     NULL, NULL,   NULL,
    NULL, NULL, NULL, NULL,  NULL, NULL,  NULL, NULL,     NULL, NULL,   NULL,
    NULL, NULL, NULL, NULL,  NULL, NULL,  NULL, NULL,     NULL, NULL,   NULL,
    NULL, NULL, NULL, NULL,  NULL, NULL,  NULL, NULL,     NULL, NULL,   NULL,
    NULL, NULL, NULL, NULL,  NULL, NULL,  NULL, NULL,     NULL, NULL,   NULL,
    NULL, NULL, NULL, NULL,  NULL, NULL,  NULL, NULL,     NULL, NULL,   NULL,
    NULL, NULL, NULL, L"big"
};

#define PANGO_STYLE_NORMAL  "normal"
#define PANGO_STYLE_OBLIQUE "oblique"
#define PANGO_STYLE_ITALIC  "italic"

#define PANGO_WEIGHT_ULTRALIGHT "ultralight"
#define PANGO_WEIGHT_LIGHT      "light"
#define PANGO_WEIGHT_NORMAL     "normal"
#define PANGO_WEIGHT_BOLD       "bold"
#define PANGO_WEIGHT_ULTRABOLD  "ultrabold"
#define PANGO_WEIGHT_HEAVY      "heavy"

#define PANGO_ULINE_NONE   "none"
#define PANGO_ULINE_SINGLE "single"
#define PANGO_ULINE_DOUBLE "double"
#define PANGO_ULINE_LOW    "low"
#define PANGO_ULINE_ERROR  "error":

#define PANGO_DEFAULT_CSTR (NULL)
#define PANGO_DEFAULT_INT  (0)
#define PANGO_DEFAULT_FLT  ((float)0xFFFFFFFF)
#define PANGO_DEFAULT_BOOL (-1)

typedef enum { STYLE_REGULAR, STYLE_OBLIQUE, STYLE_ITALIC } panstyle;

typedef struct {
    // CSS color (`#rrggbb`, `rgb()`, named colors)
    char* foreground;

    // CSS color (`#rrggbb`, `rgb()`, named colors)
    char* background;

    /* Pango font description string (`family`, `style`, `weight`, `size`)
     *
     * Example: DejaVu Sans Mono 10 */
    char* font_desc;

    /* Integer in Pango units (`1pt` = `1024` units).
     * Use `size="large"/size="x-large"` shortcuts.
     *
     * Or calculate: `size="20480"` ≈ `20pt` */
    int size;

    /* Values: `light`, `normal`, `bold`, `ultrabold`, `heavy`
     *
     * Also See: `PANGO_WEIGHT_` macro for completion. */
    char* weight;

    /* Values: `normal`, `oblique`, `italic`
     *
     * Also See: `PANGO_STYLE_` macro for completion. */
    char* style;

    /* Values: `none`, `single`, `double`, `low`, `error`
     *
     * Also See: `PANGO_ULINE` macro for completion. */
    char* underline;

    // `1`=`true` / `0`=`false`
    int strikethrough;

    // Integer in Pango units (positive = `raise`, negative = `lower`)
    int rise;

    // Integer in Pango units (positive = extra space)
    int letter_spacing;

    // Floating-point multiplier (e.g., `0.8` = `80%` size)
    float scale;
} panspan;

typedef enum {
    PSBT_NULL = 0x0000,
    PSBT_ITAL = 0x0001,
    PSBT_BOLD = 0x0002,
    PSBT_ULIN = 0x0004,
    PSBT_STRK = 0x0008,
    PSBT_MONO = 0x0010,
    PSBT_SUB  = 0x0020,
    PSBT_SUP  = 0x0040,
    PSBT_SMAL = 0x0080,
    PSBT_BIG  = 0x0100
} pantag;

void panspan_init(panspan* pspan)
{
    pspan->foreground     = PANGO_DEFAULT_CSTR;
    pspan->background     = PANGO_DEFAULT_CSTR;
    pspan->font_desc      = PANGO_DEFAULT_CSTR;
    pspan->size           = PANGO_DEFAULT_INT;
    pspan->weight         = PANGO_DEFAULT_CSTR;
    pspan->style          = PANGO_DEFAULT_CSTR;
    pspan->underline      = PANGO_DEFAULT_CSTR;
    pspan->strikethrough  = PANGO_DEFAULT_INT;
    pspan->rise           = PANGO_DEFAULT_INT;
    pspan->letter_spacing = PANGO_DEFAULT_INT;
    pspan->scale          = PANGO_DEFAULT_FLT;
}

#define panf(span, tags, format, ...)                   \
    ({                                                  \
        panspan        s = span;                        \
        static wchar_t buffer[sizeof(wchar_t) * 128];   \
        static wchar_t buffer2[sizeof(wchar_t) * 128];  \
        memset(buffer, 0, sizeof(wchar_t) * 128);       \
        memset(buffer2, 0, sizeof(wchar_t) * 128);      \
        swprintf(buffer, 128, L"" format, __VA_ARGS__); \
        pango_format(buffer2, 128, buffer, tags, &s);   \
        buffer2;                                        \
    })

#define mkspan(...)                                                          \
    (panspan)                                                                \
    {                                                                        \
        .foreground = PANGO_DEFAULT_CSTR, .background = PANGO_DEFAULT_CSTR,  \
        .font_desc = PANGO_DEFAULT_CSTR, .size = PANGO_DEFAULT_INT,          \
        .weight = PANGO_DEFAULT_CSTR, .style = PANGO_DEFAULT_CSTR,           \
        .underline = PANGO_DEFAULT_CSTR, .strikethrough = PANGO_DEFAULT_INT, \
        .rise = PANGO_DEFAULT_INT, .letter_spacing = PANGO_DEFAULT_INT,      \
        .scale = PANGO_DEFAULT_FLT, __VA_ARGS__                              \
    }

void pango_format_cstr(
  char* dest, size_t ndest, char* string, pantag tags, panspan* span)
{
    for(pantag bit = 0x0001; bit <= 0x0100; bit <<= 1) {
        if(tags & bit) {
            const char* tag = TAGMAP_CSTR[bit];

            strlcat(dest, "<", ndest);
            strlcat(dest, tag, ndest);
            strlcat(dest, ">", ndest);
        }
    }

    if(span) {
        strlcat(dest, "<span ", ndest);

        char field_buf[256] = { 0 };
        memset(field_buf, 0, sizeof(field_buf));

#define CAT_SPAN_STRING(field)                                    \
    if(span->field != PANGO_DEFAULT_CSTR) {                       \
        snprintf(field_buf, 256, #field "=\"%s\" ", span->field); \
        strlcat(dest, field_buf, ndest);                          \
    }

#define CAT_SPAN_INT(field)                                   \
    if(span->field != PANGO_DEFAULT_INT) {                    \
        snprintf(field_buf, 256, #field "=%d ", span->field); \
        strlcat(dest, field_buf, ndest);                      \
    }

#define CAT_SPAN_FLOAT(field)                                     \
    {                                                             \
        float _flt_default = PANGO_DEFAULT_FLT;                   \
                                                                  \
        if(memcmp(&span->field, &_flt_default, sizeof(float))) {  \
            snprintf(field_buf, 256, #field "=%f ", span->field); \
            strlcat(dest, field_buf, ndest);                      \
        }                                                         \
    }

        CAT_SPAN_STRING(foreground);
        CAT_SPAN_STRING(background);
        CAT_SPAN_STRING(font_desc);
        CAT_SPAN_STRING(weight);
        CAT_SPAN_INT(size);
        CAT_SPAN_STRING(style);
        CAT_SPAN_STRING(underline);
        CAT_SPAN_INT(strikethrough);
        CAT_SPAN_INT(rise);
        CAT_SPAN_INT(letter_spacing);
        CAT_SPAN_FLOAT(scale);

        strlcat(dest, ">", ndest);
    }

    strlcat(dest, string, ndest);

    if(span) {
        strlcat(dest, "</span>", ndest);
    }

    for(pantag bit = 0x0100; bit >= 0x0001; bit >>= 1) {
        if(tags & bit) {
            const char* tag = TAGMAP_CSTR[bit];

            strlcat(dest, "</", ndest);
            strlcat(dest, tag, ndest);
            strlcat(dest, ">", ndest);
        }
    }
}

/**
 * @param dest Destination buffer for formatted Pango markup string
 * @param ndst Size of destination buffer in wchar_t units
 * @param string The text content to be wrapped with Pango markup
 * @param tags Bitwise combination of pantag enum values for text styles
 * @param span panspan ptr to specify a `<span with="attributes">` (can be NULL)
 */
void pango_format(
  wchar_t* dest, size_t ndest, wchar_t* string, pantag tags, panspan* span)
{

    for(pantag bit = 0x0001; bit <= 0x0100; bit <<= 1) {
        if(tags & bit) {
            wchar_t* tag = TAGMAP[bit];

            wcslcat(dest, L"<", ndest);
            wcslcat(dest, tag, ndest);
            wcslcat(dest, L">", ndest);
        }
    }

    if(span) {
        wcslcat(dest, L"<span ", ndest);

        wchar_t field_buf[256] = { 0 };
        memset(field_buf, 0, sizeof(field_buf));

#define CAT_SPAN_STRING(field)                                        \
    if(span->field != PANGO_DEFAULT_CSTR) {                           \
        swprintf(field_buf, 256, L"" #field "=\"%s\" ", span->field); \
        wcslcat(dest, field_buf, ndest);                              \
    }

#define CAT_SPAN_INT(field)                                       \
    if(span->field != PANGO_DEFAULT_INT) {                        \
        swprintf(field_buf, 256, L"" #field "=%d ", span->field); \
        wcslcat(dest, field_buf, ndest);                          \
    }

#define CAT_SPAN_FLOAT(field)                                         \
    {                                                                 \
        float _flt_default = PANGO_DEFAULT_FLT;                       \
                                                                      \
        if(memcmp(&span->field, &_flt_default, sizeof(float))) {      \
            swprintf(field_buf, 256, L"" #field "=%f ", span->field); \
            wcslcat(dest, field_buf, ndest);                          \
        }                                                             \
    }

        CAT_SPAN_STRING(foreground);
        CAT_SPAN_STRING(background);
        CAT_SPAN_STRING(font_desc);
        CAT_SPAN_STRING(weight);
        CAT_SPAN_INT(size);
        CAT_SPAN_STRING(style);
        CAT_SPAN_STRING(underline);
        CAT_SPAN_INT(strikethrough);
        CAT_SPAN_INT(rise);
        CAT_SPAN_INT(letter_spacing);
        CAT_SPAN_FLOAT(scale);

        wcslcat(dest, L">", ndest);
    }

    wcslcat(dest, string, ndest);

    if(span) {
        wcslcat(dest, L"</span>", ndest);
    }

    for(pantag bit = 0x0100; bit >= 0x0001; bit >>= 1) {
        if(tags & bit) {
            const wchar_t* tag = TAGMAP[bit];

            wcslcat(dest, L"</", ndest);
            wcslcat(dest, tag, ndest);
            wcslcat(dest, L">", ndest);
        }
    }
}

#endif // !_PANGO_H
