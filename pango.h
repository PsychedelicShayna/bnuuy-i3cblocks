
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

#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>

typedef struct {
    char*    buffer;
    wchar_t* wbuffer;
    size_t   size;
    size_t capacity;
} pango_t;

typedef enum {
    PANGO_ITALICS   = 0x0000000001,
    PANGO_BOLD      = 0x0000000002,
    PANGO_UNDERLINE = 0x0000000004,
    PANGO_STRIKE    = 0x0000000008,
    PANGO_MONO      = 0x0000000010,
    PANGO_SUB       = 0x0000000020,
    PANGO_SUP       = 0x0000000040,
    PANGO_SMALL     = 0x0000000080,
    PANGO_BIG       = 0x0000000100
} pango_tag_t;

pango_t pango_new() {
    pango_t p;
    p.buffer = (char*)malloc(4096);
    p.size = 0;
    p.capacity = 4096;


}


// <sub>   - Subscript (lowered baseline, smaller size)
// <sup>   - Superscript (raised baseline, smaller size)
void pango_feed(pango_t* p, char* text, pango_tag_t tags) {

}
