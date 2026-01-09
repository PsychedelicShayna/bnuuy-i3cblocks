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

#include "arena.h"

/* clang-format off

                        Pango Reference Sheet
                          (thank you Lumo)


Pango Tags - All tags must be properly closed (</tag>). Nesting works normally.
_______________________________________________________________________________

<b>     - Bold weight
<i>     - Italic style
<u>     - Underline
<s>     - Strikethrough
<span>  - Generic container for any combination of styling
<small> - Reduces font size (≈80% of surrounding text)
<big>   - Increases font size (≈120% of surrounding text)
<tt>    - Monospace (uses the “monospace” family)
<sub>   - Subscript (lowered baseline, smaller size)
<sup>   - Superscript (raised baseline, smaller size)

<span> Attribute Details
_______________________________________________________________________________

- foreground
    - CSS color (#rrggbb, rgb(), named colors)
    - <span foreground="#00ff00">green</span>
- background
    - Same as foreground
    - <span background="black">on black</span>
- font_desc
    - Pango font description string (family, style, weight, size)
    - <span font_desc="DejaVu Sans Mono 10">text</span>
- size
    - Integer in Pango units (1pt =1024 units). Use size="large"/size="x-large"
      shortcuts or calculate: size="20480" ≈ 20pt.
    - <span size="20480">20pt</span>
- weight
    - ultralight, light, normal, bold, ultrabold, heavy
    - <span weight="bold">Bold</span>
- style
    - normal, oblique, italic
    - <span style="italic">Italic</span>
- underline
    - none, single, double, low, error
    - <span underline="double">Double‑underlined</span>
- strikethrough
    - true / false
    - <span strikethrough="true">Crossed</span>
- rise
    - Integer in Pango units (positive = raise, negative = lower)
    - <span rise="5120"> ↑ Raised </span>
- letter_spacing
    - Integer in Pango units (positive = extra space)
    - <span letter_spacing="256">Spaced</span>
- scale
    - Floating‑point multiplier (e.g., 0.8 = 80% size)
    - <span scale="0.8">Smaller</span>

Practical Examples for i3blocks
_______________________________________________________________________________

- Simple colored, bold text
    <span foreground="#ff6600" weight="bold">Alert</span>

- Italic, green, slightly larger
    <i><span foreground="#00ff00" scale="1.1">Running</span></i>

- Mixed styles: red background, white foreground, underlined
    <span background="#ff0000" foreground="#ffffff" underline="single">
        Critical
    </span>

- Monospace timestamp with subscript version number
    <tt>2025‑12‑29 14:03<sub>v2</sub></tt>

Tips & Gotchas
_______________________________________________________________________________

    Escape ampersands – In i3blocks the whole string is passed through the
    shell, so replace & with &amp; if you need a literal ampersand.

    No CSS classes – Pango markup does not support external CSS; everything
    must be inline via the attributes above.

    Limited font families – The bar will fall back to the default UI font if
    the requested family isn’t installed on the system.

    Performance – Excessive nesting or many <span> elements can slightly
    increase rendering time; keep it reasonable for rapid‑refresh blocks.

    Testing – Use pango-view (part of pango-utils) to preview markup offline:
    echo '<span foreground="#00ff00">test</span>' | pango-view --no-display

clang-format on */

/* ~~~~~~ Implementation Idea ~~~~~~
 *
 * we have a struct that holds its own buffer char* and a method where we
 * supply it some text, along with a pango tag represented as a struct for
 * tags that are complex like <span>, or just constants for italics, bold,
 * underline, etc. The function should somehow give back the input string
 * wrapped in the desired tag.
 *
 * Naively, we could just malloc and let the caller manage it but a better
 * idea would be to have a large buffer in the struct, and that buffer will
 * store multiple strings, and the struct will take care of resizing it when
 * new tags are requested. Maybe like a string builder pattern, but with tags?
 *
 *
 * We make a new pango feeder/string builder type thing.
 *  - pango_t pango_new()
 *
 * We read the string we want to operate on into the buffer.
 *  - pango_read(pango_t*, char* string)
 *
 * Operate on it with pango functions
 *  - pango_span(pango_t*, pango_span_t*)
 *  - pango_italics(pango_t*)
 *
 * Ask it how large of a buffer we need to store the result
 *  - size_t pango_size(pango_t*)
 *
 * Then ask it to write the result out
 *  - pango_write(pango_t*, char* out, char* szout)
 *
 *
 *  Or maybe
 *
 *  We append new data to it with bitmask for simple tags, or just raw text
 *  with regulra
 *
 * */

#define SWAP_PTRS(a, b)       \
    {                         \
        wchar_t* temp = a;    \
        a             = b;    \
        b             = temp; \
    }

typedef enum {
    STYLE_REGULAR,
    STYLE_OBLIQUE,
    STYLE_ITALIC
} pango_style,
  panstyle_t;

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
} pango_span, panspan_t;

void panspan_init(pango_span* pspan)
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

pango_span panspan_create(void)
{
    pango_span pspan;
    panspan_init(&pspan);
    return pspan;
}

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
} pango_tag,
  pantag_t;

typedef struct {
    marena*   arena;
    wchar_t** strings;
    size_t    count;
    size_t    capacity;

    wchar_t*  buffer;
    wchar_t** tag_map;
} pango_string_builder, pangosb;

wchar_t* TAG_MAP[256] = {
    L"", L"i", L"b", L"",    L"u",   L"", L"", L"", L"s",     L"", L"", L"",
    L"", L"",  L"",  L"",    L"tt",  L"", L"", L"", L"",      L"", L"", L"",
    L"", L"",  L"",  L"",    L"",    L"", L"", L"", L"sub",   L"", L"", L"",
    L"", L"",  L"",  L"",    L"",    L"", L"", L"", L"",      L"", L"", L"",
    L"", L"",  L"",  L"",    L"",    L"", L"", L"", L"",      L"", L"", L"",
    L"", L"",  L"",  L"",    L"sup", L"", L"", L"", L"",      L"", L"", L"",
    L"", L"",  L"",  L"",    L"",    L"", L"", L"", L"",      L"", L"", L"",
    L"", L"",  L"",  L"",    L"",    L"", L"", L"", L"",      L"", L"", L"",
    L"", L"",  L"",  L"",    L"",    L"", L"", L"", L"",      L"", L"", L"",
    L"", L"",  L"",  L"",    L"",    L"", L"", L"", L"",      L"", L"", L"",
    L"", L"",  L"",  L"",    L"",    L"", L"", L"", L"small", L"", L"", L"",
    L"", L"",  L"",  L"",    L"",    L"", L"", L"", L"",      L"", L"", L"",
    L"", L"",  L"",  L"",    L"",    L"", L"", L"", L"",      L"", L"", L"",
    L"", L"",  L"",  L"",    L"",    L"", L"", L"", L"",      L"", L"", L"",
    L"", L"",  L"",  L"",    L"",    L"", L"", L"", L"",      L"", L"", L"",
    L"", L"",  L"",  L"",    L"",    L"", L"", L"", L"",      L"", L"", L"",
    L"", L"",  L"",  L"",    L"",    L"", L"", L"", L"",      L"", L"", L"",
    L"", L"",  L"",  L"",    L"",    L"", L"", L"", L"",      L"", L"", L"",
    L"", L"",  L"",  L"",    L"",    L"", L"", L"", L"",      L"", L"", L"",
    L"", L"",  L"",  L"",    L"",    L"", L"", L"", L"",      L"", L"", L"",
    L"", L"",  L"",  L"",    L"",    L"", L"", L"", L"",      L"", L"", L"",
    L"", L"",  L"",  L"big",
};
// psb->tag_map[0x0001] = L"i";     // psb->_ITAL
// psb->tag_map[0x0002] = L"b";     // psb->_BOLD
// psb->tag_map[0x0004] = L"u";     // psb->_ULIN
// psb->tag_map[0x0008] = L"s";     // psb->_STRK
// psb->tag_map[0x0010] = L"tt";    // psb->_MONO
// psb->tag_map[0x0020] = L"sub";   // psb->_SUB
// psb->tag_map[0x0040] = L"sup";   // psb->_SUP
// psb->tag_map[0x0080] = L"small"; // psb->_SMAL
// psb->tag_map[0x0100] = L"big";   // psb->_BIG

/* Returns the size in `wchar_t` of the last string pushed to the
 * pango string builder `psb`. If no strings have been pushed yet,
 * returns `0`. */
size_t pangosb_size_last(pango_string_builder* psb)
{
    if(psb->count == 0) {
        return 0;
    }

    if(psb->strings[psb->count - 1] == NULL) {
        return 0;
    }

    return wcslen(psb->strings[psb->count - 1]);
}

size_t pangosb_get_last(pango_string_builder* psb, wchar_t* out, size_t szout)
{

    if(psb->count == 0) {
        return 0;
    }

    if(psb->strings[psb->count - 1] == NULL) {
        return 0;
    }

    size_t szwc = pangosb_size_last(psb) + 1;
    if(szout < szwc * sizeof(wchar_t)) {
        size_t needed = szwc * sizeof(wchar_t);
        return needed;
    }

    memcpy(out, psb->strings[psb->count - 1], szwc * sizeof(wchar_t));

    return szwc * sizeof(wchar_t);
}
//
// size_t   szclone = (wcslen(psb->strings[0]) + 1) * sizeof(wchar_t);
// wchar_t* clone   = malloc(szclone);
// memcpy(clone, psb->strings[0], szclone);

pango_string_builder*
  pangosb_init(pango_string_builder* psb, marena* arena, size_t string_capacity)
{
    psb->capacity = string_capacity;
    psb->count    = 0;

    psb->arena   = arena; // marena_create(KiB(65536));
    psb->tag_map = marena_alloc(psb->arena, 0x1000, ARENA_ALIGN, false);

    memset(psb->tag_map, 0, (0x0100 * sizeof(wchar_t*)));

    // psb->tag_map = marena_alloc_array(psb->arena, wchar_t*, 0x1000);

    psb->tag_map[0x0001] = L"i";     // psb->_ITAL
    psb->tag_map[0x0002] = L"b";     // psb->_BOLD
    psb->tag_map[0x0004] = L"u";     // psb->_ULIN
    psb->tag_map[0x0008] = L"s";     // psb->_STRK
    psb->tag_map[0x0010] = L"tt";    // psb->_MONO
    psb->tag_map[0x0020] = L"sub";   // psb->_SUB
    psb->tag_map[0x0040] = L"sup";   // psb->_SUP
    psb->tag_map[0x0080] = L"small"; // psb->_SMAL
    psb->tag_map[0x0100] = L"big";   // psb->_BIG

    // psb->strings = PUSH_ARRAY(psb->arena, wchar_t*, psb->capacity);
    psb->strings = marena_alloc(
      psb->arena, sizeof(wchar_t*) * psb->capacity, ARENA_ALIGN, false);

    memset(psb->strings, 0, sizeof(wchar_t*) * psb->capacity);

    return psb;
}

void pangosb_fini(pango_string_builder* psb)
{
    // marena_free(psb->arena);
    psb->buffer  = NULL;
    psb->tag_map = NULL;
    psb->arena   = NULL;
}

/* Pushes a wide string (`wchar_t* wstr`) of size `wstr_sz` to the pango
 * string builder `psb` wrapped in the tags specified by the `pango_tag_t`
 * bitmask, which can be `PSBT_NULL (0)` for no tags, or can house various
 * tags which will be nested, e.g. `PSBT_ITAL | PSBT_BOLD`
 *
 * `-`
 *
 * For spans, provide a `pango_span_t*`, or `NULL` for no span. To make a basic
 * span for foreground color: `&(pango_tag_t) { .foreground="#FF00FF" }`.
 *
 * `-`
 *
 * This function returns the same `pangosb_t*` that was passed in as `psb`
 * so that calls can be chained.
 * */
pango_string_builder* pangosb_wpush(pango_string_builder* psb,
                                    wchar_t*              wbuffer,
                                    size_t                szwbuffer,
                                    wchar_t*              wstr,
                                    pango_tag             ptag,
                                    pango_span*           pspan)
{
    size_t wstr_length = wcslen(wstr);
    size_t wstr_size   = wstr_length * sizeof(wchar_t);

    // Set up buffers (this costs no mallocs since we're using the arena)
    size_t szbuf = PANGO_SZMAX + wstr_size + 256;

    static wchar_t *buf = NULL, *alt = NULL;

    if(buf == NULL || alt == NULL) {
        buf = PUSH_ARRAY(psb->arena, wchar_t, szbuf);
        alt = PUSH_ARRAY(psb->arena, wchar_t, szbuf);

        memset(alt, 0, szbuf);
    }

    wcslcpy(buf, wstr, szbuf);

    for(pango_tag bit = 0x0001; bit <= 0x0100; bit <<= 1) {
        if(ptag & bit) {
            wchar_t* tag = psb->tag_map[bit];
            swprintf(alt, szbuf, L"<%ls>%ls</%ls>", tag, buf, tag);
            SWAP_PTRS(buf, alt);
        }
    }

    if(pspan == NULL) {
        size_t szclone = (wcslen(buf) + 1) * sizeof(wchar_t);
        // wchar_t* clone   = arena_push(psb->arena, szclone, false);
        // wchar_t* clone = marena_alloc(psb->arena, szclone, ARENA_ALIGN,
        // false);

        wchar_t* clone = PUSH_ARRAY(psb->arena, wchar_t, szclone);
        memcpy(clone, buf, szclone);
        psb->strings[psb->count] = clone;
        psb->count++;
        return psb;
    }

    // wchar_t* span_buf = PUSH_ARRAY(psb->arena, wchar_t, PANGO_SZMAX);

    wchar_t* span_buf =
      marena_alloc(psb->arena, PANGO_SZMAX, ARENA_ALIGN, false);

    wchar_t field_buf[256];
    memset(field_buf, 0, sizeof(field_buf));

    // wcsncat(wchar_t *restrict dest, const wchar_t *restrict src, size_t n)
    // wcscat(wchar_t *restrict dest, const wchar_t *restrict src)

    // Safest:
    // wcslcat(wchar_t *restrict dest, const wchar_t *restrict src, size_t n)

#define CAT_SPAN_STRING(field)                                         \
    if(pspan->field != PANGO_DEFAULT_CSTR) {                           \
        swprintf(field_buf, 256, L"" #field "=\"%s\" ", pspan->field); \
        wcslcat(span_buf, field_buf, PANGO_SZMAX);                     \
    }

#define CAT_SPAN_INT(field)                                        \
    if(pspan->field != PANGO_DEFAULT_INT) {                        \
        swprintf(field_buf, 256, L"" #field "=%d ", pspan->field); \
        wcslcat(span_buf, field_buf, PANGO_SZMAX);                 \
    }

#define CAT_SPAN_FLOAT(field)                                          \
    {                                                                  \
        float _flt_default = PANGO_DEFAULT_FLT;                        \
                                                                       \
        if(memcmp(&pspan->field, &_flt_default, sizeof(float))) {      \
            swprintf(field_buf, 256, L"" #field "=%f ", pspan->field); \
            wcslcat(span_buf, field_buf, PANGO_SZMAX);                 \
        }                                                              \
    }

    CAT_SPAN_STRING(foreground)
    CAT_SPAN_STRING(background)
    CAT_SPAN_STRING(font_desc)
    CAT_SPAN_STRING(weight)
    CAT_SPAN_INT(size)
    CAT_SPAN_STRING(style)
    CAT_SPAN_STRING(underline)
    CAT_SPAN_INT(strikethrough)
    CAT_SPAN_INT(rise)
    CAT_SPAN_INT(letter_spacing)
    CAT_SPAN_FLOAT(scale)

    swprintf(alt, szbuf, L"<span %ls>%ls</span>", span_buf, buf);

    size_t   szclone = (wcslen(alt) + 1) * sizeof(wchar_t);
    wchar_t* clone   = marena_alloc(psb->arena, szclone, ARENA_ALIGN, false);
    memcpy(clone, alt, szclone);
    psb->strings[psb->count] = clone;
    psb->count++;
}

#endif
