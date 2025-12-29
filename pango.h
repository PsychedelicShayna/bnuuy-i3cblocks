
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















