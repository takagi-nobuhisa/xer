# Policy for Bitmap Fonts

## Overview

XER provides bitmap-font facilities for drawing text on `xer::image::canvas`.

The purpose of this facility is not to implement a general-purpose font engine.
Instead, XER focuses on a small and predictable model that is sufficient for framebuffer-style text rendering, simple GUI labels, and generated bitmap fonts.

The bitmap-font facility should therefore keep the following priorities:

- store font data outside C++ source files
- load font data at runtime from a file path
- use a compact binary format
- support monospaced bitmap fonts only
- distinguish only half-width and full-width glyph cells
- keep glyph height uniform within one font
- keep text layout policy separate from font data
- make generated data from BDF files or font images practical

---

## Scope

The initial bitmap-font facility covers:

- a compact external binary font format named XBF
- runtime loading of XBF files
- monospaced bitmap fonts with half-width and full-width glyph cells
- one-bit-per-pixel glyph bitmaps
- Unicode code point ranges
- text drawing on `xer::image::canvas`

The following are outside the initial scope:

- proportional fonts
- vector fonts
- antialiased glyph data
- kerning
- ligatures
- OpenType, TrueType, and other complex font formats
- baseline-based text layout
- general-purpose text shaping
- BDF parsing in the public XER API

BDF files and rasterized font images may be used as input to converter tools, but the runtime image API operates on XBF data.

---

## Public Placement

Bitmap-font support belongs to the image facility.

The public API should be provided through:

```text
xer/image.h
```

The implementation may be placed in:

```text
xer/bits/image.h
```

Font conversion tools should be placed together with other project PHP utilities:

```text
php/
```

For example:

```text
php/convert_bdf_font.php
php/convert_bitmap_font_image.php
```

---

## Font Model

### Monospaced Bitmap Fonts

A bitmap font is monospaced.

Each glyph belongs to one of two fixed cell widths:

- half-width
- full-width

The font stores one common glyph height shared by all glyphs.

The full-width cell is not required to be exactly twice the half-width cell. Both widths are stored explicitly.

Example:

```text
half-width cell:  8 pixels
full-width cell: 16 pixels
glyph height:    16 pixels
```

---

## Text Layout Policy

The font data stores glyph-cell geometry only.

The following layout choices are controlled by the text drawing API rather than by the font file:

- letter spacing
- line spacing
- wrapping policy, if such a feature is added later

The font format does not store:

- x advance
- line height
- baseline

This keeps the font format focused on reusable bitmap data, while allowing the caller to choose spacing per drawing operation.

---

## In-Memory Representation

The runtime representation should follow this general shape:

```cpp
namespace xer::image
{
    enum class bitmap_glyph_width : std::uint8_t
    {
        half,
        full,
    };

    struct bitmap_font_range
    {
        char32_t first_code_point {};
        char32_t last_code_point {};
        bitmap_glyph_width glyph_width {};
        std::uint64_t bitmap_offset {};
    };

    struct bitmap_font
    {
        int half_width {};
        int full_width {};
        int glyph_height {};

        std::vector<bitmap_font_range> ranges {};
        std::vector<std::uint8_t> bitmap {};
    };
}
```

### `bitmap_glyph_width`

`bitmap_glyph_width` identifies whether a range uses the font's half-width cell or full-width cell.

```cpp
enum class bitmap_glyph_width : std::uint8_t
{
    half,
    full,
};
```

The width kind is attached to each range rather than inferred from Unicode code points.
This avoids hard-coding assumptions about East Asian width classification, punctuation, symbol ranges, or converter policy.

### `bitmap_font_range`

A `bitmap_font_range` represents a continuous Unicode code point range stored in the font.

```cpp
struct bitmap_font_range
{
    char32_t first_code_point {};
    char32_t last_code_point {};
    bitmap_glyph_width glyph_width {};
    std::uint64_t bitmap_offset {};
};
```

- `first_code_point` is the first code point in the range.
- `last_code_point` is the last code point in the range.
- `glyph_width` selects the half-width or full-width cell size.
- `bitmap_offset` is the byte offset from the beginning of the loaded bitmap data to the first glyph in this range.

Ranges are stored in ascending code point order and must not overlap.

### `bitmap_font`

A `bitmap_font` stores the font-wide geometry, range table, and packed bitmap bytes.

```cpp
struct bitmap_font
{
    int half_width {};
    int full_width {};
    int glyph_height {};

    std::vector<bitmap_font_range> ranges {};
    std::vector<std::uint8_t> bitmap {};
};
```

- `half_width` is the width of half-width glyph cells.
- `full_width` is the width of full-width glyph cells.
- `glyph_height` is the common height of all glyph cells.
- `ranges` describes which Unicode code points are present and where their data begins.
- `bitmap` stores packed 1bpp glyph data.

The in-memory representation intentionally follows the XBF file structure closely, making runtime loading straightforward and glyph lookup predictable.

---

## XBF Binary Font Format

### Purpose

XBF is the binary external font format used by XER's bitmap-font facility.

It is designed for:

- fast and simple loading
- compact storage
- deterministic glyph lookup
- easy generation from converter tools

### Endianness

All multi-byte integer fields in XBF are encoded in:

```text
little-endian byte order
```

A loader must decode XBF fields as little-endian values regardless of the native byte order of the host environment.

---

## XBF File Structure

An XBF file consists of three logical regions:

```text
header
range table
bitmap data
```

The file usually stores these regions in that order, but their actual locations are determined by offsets stored in the header.

---

## XBF Header

### Header Size

The version 1 header size is:

```text
36 bytes
```

### Header Fields

| Offset | Size | Type | Description |
|---:|---:|---|---|
| 0  | 4 | `char[4]` | Magic value `"XBF0"` |
| 4  | 2 | `uint16_le` | Format version |
| 6  | 2 | `uint16_le` | Header size |
| 8  | 2 | `uint16_le` | Half-width glyph cell width |
| 10 | 2 | `uint16_le` | Full-width glyph cell width |
| 12 | 2 | `uint16_le` | Glyph cell height |
| 14 | 2 | `uint16_le` | Reserved, must be `0` |
| 16 | 4 | `uint32_le` | Number of range-table entries |
| 20 | 8 | `uint64_le` | Absolute file offset of the range table |
| 28 | 8 | `uint64_le` | Absolute file offset of the bitmap data |

### Header Rules

- The magic value is exactly `"XBF0"`.
- The initial format version is `1`.
- The version 1 header size is `36`.
- `half_width`, `full_width`, and `glyph_height` must be greater than zero.
- Reserved fields must be zero.

---

## XBF Range Table

### Entry Size

Each range-table entry uses:

```text
24 bytes
```

### Range Entry Fields

| Offset | Size | Type | Description |
|---:|---:|---|---|
| 0  | 4 | `uint32_le` | First Unicode code point |
| 4  | 4 | `uint32_le` | Last Unicode code point |
| 8  | 1 | `uint8` | Glyph width kind |
| 9  | 7 | `uint8[7]` | Reserved, must be `0` |
| 16 | 8 | `uint64_le` | Offset from the beginning of bitmap data |

### Width Kind

The width-kind field uses the following values:

| Value | Meaning |
|---:|---|
| 0 | half-width |
| 1 | full-width |

Other values are invalid.

### Range Rules

Each range entry must satisfy:

- `first_code_point <= last_code_point`
- the width kind is valid
- reserved bytes are all zero
- entries are sorted by `first_code_point`
- ranges do not overlap
- the corresponding bitmap bytes fit within the bitmap-data region

The width kind applies to every glyph in the range.
A converter may split adjacent Unicode ranges when different width kinds are required.

---

## XBF Bitmap Data

### Glyph Bitmap Model

Each glyph is stored as a 1bpp bitmap inside its fixed cell.

A glyph uses:

```text
bytes_per_row = (glyph_width + 7) / 8
bytes_per_glyph = bytes_per_row * glyph_height
```

Here, `glyph_width` is either the font's `half_width` or `full_width`, depending on the range width kind.

### Glyph Order

Within a range, glyphs are stored in ascending Unicode code point order.

For example, a range from `U+0020` through `U+007E` stores glyph bitmaps in this order:

```text
U+0020
U+0021
U+0022
...
U+007E
```

### Row Order

Rows are stored from top to bottom.

```text
row 0
row 1
row 2
...
row glyph_height - 1
```

### Bit Order

Within each byte:

```text
the most significant bit represents the leftmost pixel
```

For an 8-pixel row:

```text
10000000
```

means that only the leftmost pixel is set.

```text
00000001
```

means that only the rightmost pixel is set.

### Unused Bits

If a row width is not divisible by 8, unused low-order bits at the end of the final byte must be zero.

---

## Glyph Offset Calculation

Let:

- `cp` be the target Unicode code point
- `range` be the range that contains `cp`
- `bitmap_data_offset` be the absolute bitmap-data offset from the XBF header

The glyph index inside the range is:

```text
glyph_index = cp - range.first_code_point
```

The row size is:

```text
bytes_per_row = (glyph_width + 7) / 8
```

The glyph size is:

```text
bytes_per_glyph = bytes_per_row * glyph_height
```

The absolute file offset of the glyph bitmap is:

```text
glyph_file_offset =
    bitmap_data_offset
    + range.bitmap_offset
    + glyph_index * bytes_per_glyph
```

After loading into `bitmap_font::bitmap`, the same position is obtained by removing `bitmap_data_offset` from the calculation.

---

## Validation Requirements

An XBF loader should reject malformed files that violate the format.

At minimum, it should verify:

- the magic value is `"XBF0"`
- the format version is supported
- the header size is valid
- all reserved fields are zero
- half-width, full-width, and glyph height are non-zero
- the range-table offset is within the file
- the bitmap-data offset is within the file
- the range table itself fits within the file
- all range entries are structurally valid
- range entries are sorted and non-overlapping
- width-kind values are valid
- bitmap offsets are valid
- every range's bitmap span fits within the bitmap-data region
- all size and offset calculations avoid integer overflow

The loader should not accept files whose internal offsets or sizes are inconsistent even if a partial subset of data could be read.

---

## Conversion Policy

XBF is intended to be generated rather than authored by hand.

The project may provide converter tools under `php/` for sources such as:

- BDF bitmap fonts
- images containing arranged glyph cells
- rasterized monospaced PC fonts exported to bitmap images

Converter tools are responsible for:

- selecting Unicode ranges
- assigning half-width or full-width range kinds
- packing glyph rows into XBF bitmap data
- emitting valid little-endian XBF files
- zeroing reserved bytes and unused row bits

The public runtime API should not depend on a specific converter implementation.

---

## Drawing Policy

Bitmap-font drawing should use the loaded `bitmap_font` representation.

The initial text drawing API should place glyph cells from a top-left position.
Baseline-oriented drawing is intentionally omitted.

Spacing decisions should be provided at draw time, not stored in the XBF file.

Conceptually, a text-drawing option type may include:

```cpp
struct text_draw_options
{
    int letter_spacing {};
    int line_spacing {};
};
```

The exact public API may be finalized when `draw_text` is designed.

---

## Summary

XER bitmap fonts use a deliberately small and predictable model:

- runtime-loaded external binary data
- XBF format with little-endian fields
- monospaced glyphs with half-width and full-width cells
- one common glyph height
- Unicode range tables
- packed 1bpp bitmap data
- draw-time control of spacing
- no baseline, proportional layout, or shaping support in the initial design

This is sufficient for practical bitmap text rendering in `xer::image` while keeping the implementation compact and deterministic.
