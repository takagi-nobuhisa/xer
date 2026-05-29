# `<xer/image.h>`


## Purpose

`<xer/image.h>` provides lightweight image and framebuffer facilities.

The initial purpose of this header is not full photo editing or complete image-file handling. It is a small framebuffer-oriented layer for fixed-size canvases, VRAM-style emulation, simple drawing, image processing, and later integration with Tcl/Tk photo images.

Pure image processing and drawing belong in `<xer/image.h>`. Tcl/Tk photo integration belongs in `<xer/tk.h>`.

---

## Namespace

Image-related types and functions are placed in the `xer::image` namespace.

The primary framebuffer owner type is `xer::image::canvas` so that `xer::image` can serve as the namespace for image storage, drawing, and image processing.

---

## Main Entities

At minimum, `<xer/image.h>` provides the following entities:

```cpp
namespace xer::image {

struct point;
struct pointf;
struct size;
struct sizef;
struct rect;
struct rectf;

struct filter_pixels_error_detail;

enum class bitmap_glyph_width : std::uint8_t;
struct bitmap_font_range;
struct bitmap_font;
struct text_draw_options;

struct pixel;

struct argb32_policy;
struct rgba32_policy;
struct rgb24_policy;
struct bgr24_policy;

template <std::size_t Width,
          std::size_t Height,
          class Policy = argb32_policy>
class canvas;

template <class Policy = argb32_policy>
using dynamic_canvas = canvas<0, 0, Policy>;

[[nodiscard]] auto bitmap_font_load(const xer::path& filename)
    -> xer::result<bitmap_font, xer::parse_error_detail>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_hline(canvas<Width, Height, Policy>& img,
                int x,
                int y,
                int length,
                pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_hline(canvas<Width, Height, Policy>& img,
                const point& p,
                int length,
                pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_vline(canvas<Width, Height, Policy>& img,
                int x,
                int y,
                int length,
                pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_vline(canvas<Width, Height, Policy>& img,
                const point& p,
                int length,
                pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line(canvas<Width, Height, Policy>& img,
               int x0,
               int y0,
               int x1,
               int y1,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line(canvas<Width, Height, Policy>& img,
               const point& p0,
               const point& p1,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line_aa(canvas<Width, Height, Policy>& img,
                  float x0,
                  float y0,
                  float x1,
                  float y1,
                  pixel color) noexcept -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line_aa(canvas<Width, Height, Policy>& img,
                  float x0,
                  float y0,
                  float x1,
                  float y1,
                  float width,
                  pixel color) noexcept -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line_aa(canvas<Width, Height, Policy>& img,
                  const pointf& p0,
                  const pointf& p1,
                  pixel color) noexcept -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line_aa(canvas<Width, Height, Policy>& img,
                  const pointf& p0,
                  const pointf& p1,
                  float width,
                  pixel color) noexcept -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_rect(canvas<Width, Height, Policy>& img,
               int x,
               int y,
               int width,
               int height,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_rect(canvas<Width, Height, Policy>& img,
               const point& origin,
               const size& extent,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_rect(canvas<Width, Height, Policy>& img,
               const rect& area,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_rect(canvas<Width, Height, Policy>& img,
               int x,
               int y,
               int width,
               int height,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_rect(canvas<Width, Height, Policy>& img,
               const point& origin,
               const size& extent,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_rect(canvas<Width, Height, Policy>& img,
               const rect& area,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_circle(canvas<Width, Height, Policy>& img,
                 int cx,
                 int cy,
                 int radius,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_circle(canvas<Width, Height, Policy>& img,
                 const point& center,
                 int radius,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_circle(canvas<Width, Height, Policy>& img,
                 int cx,
                 int cy,
                 int radius,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_circle(canvas<Width, Height, Policy>& img,
                 const point& center,
                 int radius,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_circle_aa(canvas<Width, Height, Policy>& img,
                    float cx,
                    float cy,
                    float radius,
                    pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_circle_aa(canvas<Width, Height, Policy>& img,
                    float cx,
                    float cy,
                    float radius,
                    float width,
                    pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_circle_aa(canvas<Width, Height, Policy>& img,
                    const pointf& center,
                    float radius,
                    pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_circle_aa(canvas<Width, Height, Policy>& img,
                    const pointf& center,
                    float radius,
                    float width,
                    pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_circle_aa(canvas<Width, Height, Policy>& img,
                    float cx,
                    float cy,
                    float radius,
                    pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_circle_aa(canvas<Width, Height, Policy>& img,
                    const pointf& center,
                    float radius,
                    pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse(canvas<Width, Height, Policy>& img,
                  int cx,
                  int cy,
                  int radius_x,
                  int radius_y,
                  pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse(canvas<Width, Height, Policy>& img,
                  const point& center,
                  int radius_x,
                  int radius_y,
                  pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_ellipse(canvas<Width, Height, Policy>& img,
                  int cx,
                  int cy,
                  int radius_x,
                  int radius_y,
                  pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_ellipse(canvas<Width, Height, Policy>& img,
                  const point& center,
                  int radius_x,
                  int radius_y,
                  pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_aa(canvas<Width, Height, Policy>& img,
                     float cx,
                     float cy,
                     float radius_x,
                     float radius_y,
                     pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_aa(canvas<Width, Height, Policy>& img,
                     float cx,
                     float cy,
                     float radius_x,
                     float radius_y,
                     float width,
                     pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_aa(canvas<Width, Height, Policy>& img,
                     const pointf& center,
                     float radius_x,
                     float radius_y,
                     pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_aa(canvas<Width, Height, Policy>& img,
                     const pointf& center,
                     float radius_x,
                     float radius_y,
                     float width,
                     pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_ellipse_aa(canvas<Width, Height, Policy>& img,
                     float cx,
                     float cy,
                     float radius_x,
                     float radius_y,
                     pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_ellipse_aa(canvas<Width, Height, Policy>& img,
                     const pointf& center,
                     float radius_x,
                     float radius_y,
                     pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_arc(canvas<Width, Height, Policy>& img,
              int cx,
              int cy,
              int radius,
              float start_angle,
              float sweep_angle,
              pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_arc(canvas<Width, Height, Policy>& img,
              const point& center,
              int radius,
              float start_angle,
              float sweep_angle,
              pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_arc_aa(canvas<Width, Height, Policy>& img,
                 float cx,
                 float cy,
                 float radius,
                 float start_angle,
                 float sweep_angle,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_arc_aa(canvas<Width, Height, Policy>& img,
                 float cx,
                 float cy,
                 float radius,
                 float start_angle,
                 float sweep_angle,
                 float width,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_arc_aa(canvas<Width, Height, Policy>& img,
                 const pointf& center,
                 float radius,
                 float start_angle,
                 float sweep_angle,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_arc_aa(canvas<Width, Height, Policy>& img,
                 const pointf& center,
                 float radius,
                 float start_angle,
                 float sweep_angle,
                 float width,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_arc(canvas<Width, Height, Policy>& img,
                      int cx,
                      int cy,
                      int radius_x,
                      int radius_y,
                      float start_angle,
                      float sweep_angle,
                      pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_arc(canvas<Width, Height, Policy>& img,
                      const point& center,
                      int radius_x,
                      int radius_y,
                      float start_angle,
                      float sweep_angle,
                      pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_arc_aa(canvas<Width, Height, Policy>& img,
                         float cx,
                         float cy,
                         float radius_x,
                         float radius_y,
                         float start_angle,
                         float sweep_angle,
                         pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_arc_aa(canvas<Width, Height, Policy>& img,
                         float cx,
                         float cy,
                         float radius_x,
                         float radius_y,
                         float start_angle,
                         float sweep_angle,
                         float width,
                         pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_arc_aa(canvas<Width, Height, Policy>& img,
                         const pointf& center,
                         float radius_x,
                         float radius_y,
                         float start_angle,
                         float sweep_angle,
                         pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_arc_aa(canvas<Width, Height, Policy>& img,
                         const pointf& center,
                         float radius_x,
                         float radius_y,
                         float start_angle,
                         float sweep_angle,
                         float width,
                         pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto flood_fill(canvas<Width, Height, Policy>& img,
                              int x,
                              int y,
                              pixel color)
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto flood_fill(canvas<Width, Height, Policy>& img,
                              const point& origin,
                              pixel color)
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto mosaic(canvas<Width, Height, Policy>& img,
                          const rect& area,
                          const size& block_size) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto box_blur(canvas<Width, Height, Policy>& img,
                            const rect& area,
                            const size& box_size)
    -> xer::result<void>;

struct filter_pixels_error_detail {
    point first_error_position;
    std::size_t error_count;
};

template <std::size_t Width, std::size_t Height, class Policy, class F>
[[nodiscard]] auto filter_pixels(canvas<Width, Height, Policy>& img,
                                 const rect& area,
                                 F&& filter)
    -> xer::result<void, filter_pixels_error_detail>;

}
```

---

## Geometry Types

The geometry helper types are simple value types used by drawing and image-processing APIs.

```cpp
struct point {
    int x;
    int y;

    constexpr point() noexcept = default;
    constexpr point(int x, int y) noexcept;
};

struct pointf {
    float x;
    float y;

    constexpr pointf() noexcept = default;
    constexpr pointf(float x, float y) noexcept;
};

struct size {
    int width;
    int height;

    constexpr size() noexcept = default;
    constexpr size(int width, int height) noexcept;
};

struct sizef {
    float width;
    float height;

    constexpr sizef() noexcept = default;
    constexpr sizef(float width, float height) noexcept;
};

struct rect {
    int x;
    int y;
    int width;
    int height;

    constexpr rect() noexcept = default;
    constexpr rect(int x, int y, int width, int height) noexcept;
    constexpr rect(const point& origin, const size& extent) noexcept;
};

struct rectf {
    float x;
    float y;
    float width;
    float height;

    constexpr rectf() noexcept = default;
    constexpr rectf(float x, float y, float width, float height) noexcept;
    constexpr rectf(const pointf& origin, const sizef& extent) noexcept;
};
```

Integer geometry types are intended for pixel-grid operations and clipping. Floating-point geometry types are intended for subpixel drawing, antialiasing, and future transformations.

`rect` and `rectf` can be constructed either from four scalar values or from an origin point plus an extent size:

```cpp
const auto area = xer::image::rect(
    xer::image::point(10, 20),
    xer::image::size(320, 240));
```

Geometry-type function parameters are passed by `const&` in public drawing and image-processing APIs. Scalar coordinates and `pixel` values remain ordinary value parameters.

When `<xer/iostream.h>` is included, the geometry types use compact diagnostic stream forms:

```text
point  -> (x, y)
size   -> {width, height}
rect   -> (x, y) {width, height}
```

The floating-point variants use the same spelling.

---

## Filter Error Detail

`filter_pixels_error_detail` reports partial failures from `filter_pixels`.

```cpp
struct filter_pixels_error_detail {
    point first_error_position{};
    std::size_t error_count = 0;
};
```

`first_error_position` is the first pixel where the user-supplied filter threw an exception, expressed in canvas coordinates. `error_count` is the total number of pixels whose filter call failed.

Only the first failing position is stored. This avoids allocating a potentially large list of failed pixels while still giving the caller a useful diagnostic location.

---

## Logical Pixel

`xer::image::pixel` represents a logical color value.

It is not the same thing as the physical framebuffer storage element. The physical storage format is controlled by the canvas policy.

The logical representation is ARGB in a 32-bit integer:

```text
0xAARRGGBB
```

The conceptual shape is:

```cpp
struct pixel {
    std::uint32_t argb = 0xff000000u;

    constexpr pixel() noexcept = default;
    constexpr explicit pixel(std::uint32_t value) noexcept;
    constexpr pixel(std::uint8_t red,
                    std::uint8_t green,
                    std::uint8_t blue) noexcept;
    constexpr pixel(std::uint8_t alpha,
                    std::uint8_t red,
                    std::uint8_t green,
                    std::uint8_t blue) noexcept;

    constexpr auto alpha() const noexcept -> std::uint8_t;
    constexpr auto red() const noexcept -> std::uint8_t;
    constexpr auto green() const noexcept -> std::uint8_t;
    constexpr auto blue() const noexcept -> std::uint8_t;

    constexpr auto alpha(std::uint8_t value) noexcept -> void;
    constexpr auto red(std::uint8_t value) noexcept -> void;
    constexpr auto green(std::uint8_t value) noexcept -> void;
    constexpr auto blue(std::uint8_t value) noexcept -> void;
};
```

The three-argument constructor represents RGB and sets alpha to `0xff`.
The four-argument constructor follows ARGB order:

```text
alpha, red, green, blue
```

---

## Framebuffer Storage Policies

A canvas policy controls the physical framebuffer storage format.

A policy provides:

```cpp
using storage_type = /* physical storage element type */;

static constexpr auto get(const storage_type& value) noexcept -> pixel;
static constexpr auto encode(pixel value) noexcept -> storage_type;
static constexpr auto set(storage_type& dst, pixel value) noexcept -> void;
```

The initial policies are:

```cpp
argb32_policy
rgba32_policy
rgb24_policy
bgr24_policy
```

`argb32_policy` stores `0xAARRGGBB` and therefore matches the logical `pixel` representation directly.

`rgba32_policy` stores `0xRRGGBBAA`.

`rgb24_policy` and `bgr24_policy` store three 8-bit components and do not preserve alpha. Reading through these policies returns a logical pixel with alpha set to `0xff`.

---

## `canvas`

The primary canvas type is:

```cpp
template <std::size_t Width,
          std::size_t Height,
          class Policy = argb32_policy>
class canvas;
```

Fixed-size canvases are the main model because the initial use case is framebuffer-style handling such as VRAM emulation.

Examples:

```cpp
using screen = xer::image::canvas<256, 192>;
using sprite = xer::image::canvas<16, 16>;
using rgba_screen = xer::image::canvas<256, 192, xer::image::rgba32_policy>;
```

A dynamic-size canvas is represented as:

```cpp
canvas<0, 0, Policy>
```

The convenience alias is:

```cpp
template <class Policy = argb32_policy>
using dynamic_canvas = canvas<0, 0, Policy>;
```

Only these dimension forms are valid:

```text
Width > 0 && Height > 0
Width == 0 && Height == 0
```

Partial dynamic dimensions such as `canvas<0, 192>` and `canvas<256, 0>` are invalid.

---

## Public Pixel Access

The public pixel API uses logical pixels:

```cpp
auto get_pixel(std::size_t x, std::size_t y) const noexcept -> pixel;
auto get_pixel(const point& p) const noexcept -> pixel;
auto set_pixel(int x, int y, pixel value) noexcept -> void;
auto set_pixel(const point& p, pixel value) noexcept -> void;
auto set_pixel(int x, int y, pixel value, float coverage) noexcept -> void;
auto set_pixel(const point& p, pixel value, float coverage) noexcept -> void;
auto set_pixel_unchecked(std::size_t x,
                         std::size_t y,
                         pixel value) noexcept -> void;
auto set_pixel_unchecked(std::size_t x,
                         std::size_t y,
                         pixel value,
                         float coverage) noexcept -> void;
```

`canvas::at()` is intentionally not provided.

Returning a reference to the physical storage element would expose the framebuffer layout and would be incorrect when the storage policy is not ARGB. `pixel` is logical. `Policy::storage_type` is physical.

`get_pixel` expects coordinates that are inside the canvas.

`set_pixel` accepts signed coordinates and does nothing when the coordinates are outside the canvas boundary.

The coverage overloads blend the source pixel over the destination. Coverage is clamped to `[0.0f, 1.0f]`. A coverage value of `0.0f` leaves the destination unchanged. A coverage value of `1.0f` applies the source pixel alpha normally.

`set_pixel_unchecked` does not perform boundary checks. The caller must guarantee that `x < width()` and `y < height()`. It is intended for code that has already performed clipping or bounds checks outside the inner drawing loop.

---

## Basic Member Functions

`canvas` provides basic size and utility operations:

```cpp
auto width() const noexcept -> std::size_t;
auto height() const noexcept -> std::size_t;
auto size() const noexcept -> std::size_t;
auto empty() const noexcept -> bool;
auto contains(int x, int y) const noexcept -> bool;
auto contains(const point& p) const noexcept -> bool;
auto fill(pixel value) noexcept -> void;
auto clear() noexcept -> void;
```

`clear()` fills the canvas with opaque black.

---

## Bitmap Font Types

`<xer/image.h>` defines a compact runtime representation for monospaced bitmap fonts loaded from XBF files.

```cpp
enum class bitmap_glyph_width : std::uint8_t {
    half,
    full,
};

struct bitmap_font_range {
    char32_t first_code_point {};
    char32_t last_code_point {};
    bitmap_glyph_width glyph_width = bitmap_glyph_width::half;
    std::uint64_t bitmap_offset = 0;
};

struct bitmap_font {
    int half_width = 0;
    int full_width = 0;
    int glyph_height = 0;
    std::vector<bitmap_font_range> ranges {};
    std::vector<std::uint8_t> bitmap {};
};

struct text_draw_options {
    int letter_spacing = 0;
    int line_spacing = 0;
};
```

`bitmap_font` stores:

- one half-width cell width
- one full-width cell width
- one glyph height shared by the whole font
- sorted non-overlapping Unicode code point ranges
- packed 1bpp glyph bitmap bytes

Each range selects either the half-width cell or the full-width cell. The width kind is stored in the font data rather than inferred from Unicode code points.

`text_draw_options` is a per-call layout control for `draw_text`.

- `letter_spacing` is added after each drawn glyph cell
- `line_spacing` is added to `glyph_height` when a line break is processed

Negative spacing values are permitted and may produce overlapping glyph cells.

---

## Bitmap Font Loading

```cpp
[[nodiscard]] auto bitmap_font_load(const xer::path& filename)
    -> xer::result<bitmap_font, xer::parse_error_detail>;
```

`bitmap_font_load` reads an XBF bitmap-font file and returns a validated `bitmap_font`.

XBF is xer's compact binary bitmap-font format. It stores:

- little-endian numeric fields
- monospaced half-width and full-width glyph cells
- one common glyph height
- Unicode code point ranges
- packed 1bpp bitmap data

The loader validates the XBF header, range table, bitmap spans, reserved fields, code point ranges, and related offsets before returning success.

### Errors

If file I/O fails before XBF parsing begins, `bitmap_font_load` preserves the underlying file-related error code and returns an otherwise empty `parse_error_detail` whose reason is `parse_error_reason::none`.

If XBF bytes are malformed, it returns `error_t::invalid_argument` together with `parse_error_detail`.

For XBF:

- `offset` is a byte offset from the beginning of the binary input
- `line` is `0`
- `column` is `0`

The XBF loader may report reasons such as:

- `parse_error_reason::invalid_magic`
- `parse_error_reason::unsupported_version`
- `parse_error_reason::invalid_header`
- `parse_error_reason::invalid_range`
- `parse_error_reason::invalid_offset`
- `parse_error_reason::truncated_input`

See `header_parse.md` and `policy_bitmap_font.md` for the shared parse-detail model and the XBF policy.

---

## Drawing Functions

The initial drawing functions are simple framebuffer helpers:

```cpp
draw_hline
draw_vline
draw_line
draw_line_aa
draw_rect
fill_rect
```

Integer drawing coordinates use `int` rather than `std::size_t` because drawing often benefits from clipping negative coordinates.

Drawing operations clip to the canvas bounds. If the target area is fully outside the canvas, nothing is drawn.

After clipping, `draw_hline`, `draw_vline`, and `fill_rect` write directly to framebuffer storage. They do not call `set_pixel` for every pixel. This keeps inner loops based on simple pointer or stride increments instead of repeated coordinate-to-offset calculation.

`draw_line` uses a simple Bresenham-style integer line algorithm. It still checks each generated point against the canvas boundary, but writes through `set_pixel_unchecked` after that check.

`draw_line_aa` uses floating-point pixel-center coordinates and draws an antialiased capsule-shaped stroke. The overload without a width argument draws a one-pixel-wide antialiased line. The width overload takes the width before the color argument. The `pointf` overloads are equivalent to the scalar-coordinate overloads.

`draw_line_aa` returns `xer::result<void>`. It returns `error_t::invalid_argument` when any coordinate is not finite, or when `width` is not finite or is less than or equal to zero. A line that lies completely outside the canvas is a successful no-op.

The `draw_rect` and `fill_rect` overloads accept either `point` plus `size`, or a single `rect`. The scalar-coordinate overloads remain available for callers that already have separate coordinate values.

---

## Bitmap Text Drawing

```cpp
template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto draw_text(canvas<Width, Height, Policy>& img,
                             int x,
                             int y,
                             std::u8string_view text,
                             const bitmap_font& font,
                             pixel color,
                             const text_draw_options& options = {}) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto draw_text(canvas<Width, Height, Policy>& img,
                             const point& origin,
                             std::u8string_view text,
                             const bitmap_font& font,
                             pixel color,
                             const text_draw_options& options = {}) noexcept
    -> xer::result<void>;
```

`draw_text` draws UTF-8 text onto a canvas using a loaded `bitmap_font`.

The origin is the top-left position of the first glyph cell. Baseline-oriented placement is intentionally not part of the initial bitmap-font API.

### Layout Rules

- ordinary glyphs are drawn from the loaded bitmap data
- after a drawn glyph, the pen advances by that glyph cell width plus `letter_spacing`
- `\n`, `\r`, and `\r\n` start a new line
- a line break resets the x position to the original line origin
- a line break advances the y position by `glyph_height + line_spacing`
- a code point missing from the font is skipped without drawing and without advancing the pen

The missing-glyph rule is deliberately minimal. The initial API does not infer fallback widths or substitute `?` automatically.

### Clipping

Drawing is clipped to the canvas boundary. Glyphs may start outside the canvas, and only visible set pixels are written.

### Errors

`draw_text` returns:

- `error_t::encoding_error` when `text` is not valid UTF-8
- `error_t::invalid_argument` when the supplied `bitmap_font` is structurally unusable for the requested glyph

An empty canvas or empty text is a successful no-op.

---

## Circle, Ellipse, and Arc Drawing

The curved-shape APIs are divided into integer one-pixel drawing and floating-point antialiased drawing.

- integer APIs use `point` or scalar `int` center coordinates
- antialiased APIs use `pointf` or scalar `float` center coordinates
- antialiased outline APIs accept an optional `width`
- every curved-shape function returns `xer::result<void>` without `[[nodiscard]]`

### Circle Drawing

```cpp
template <std::size_t Width, std::size_t Height, class Policy>
auto draw_circle(canvas<Width, Height, Policy>& img,
                 int cx,
                 int cy,
                 int radius,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_circle(canvas<Width, Height, Policy>& img,
                 const point& center,
                 int radius,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_circle(canvas<Width, Height, Policy>& img,
                 int cx,
                 int cy,
                 int radius,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_circle(canvas<Width, Height, Policy>& img,
                 const point& center,
                 int radius,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_circle_aa(canvas<Width, Height, Policy>& img,
                    float cx,
                    float cy,
                    float radius,
                    pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_circle_aa(canvas<Width, Height, Policy>& img,
                    float cx,
                    float cy,
                    float radius,
                    float width,
                    pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_circle_aa(canvas<Width, Height, Policy>& img,
                    const pointf& center,
                    float radius,
                    pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_circle_aa(canvas<Width, Height, Policy>& img,
                    const pointf& center,
                    float radius,
                    float width,
                    pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_circle_aa(canvas<Width, Height, Policy>& img,
                    float cx,
                    float cy,
                    float radius,
                    pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_circle_aa(canvas<Width, Height, Policy>& img,
                    const pointf& center,
                    float radius,
                    pixel color) noexcept
    -> xer::result<void>;
```

`draw_circle` draws a clipped one-pixel circle outline. `fill_circle` fills the clipped circle interior and includes the boundary.

`draw_circle_aa` draws an antialiased outline. The overload without a width argument uses `1.0f`; the width overload supports thick circular outlines. `fill_circle_aa` fills the circle while antialiasing the outer boundary.

Radius handling is:

- a negative radius returns `error_t::invalid_argument`
- a zero integer radius writes only the center pixel, if visible
- a zero antialiased outline radius draws a round point whose diameter follows `width`
- a zero antialiased filled radius draws a point at the center

For antialiased circle drawing, center coordinates, radius, and width must be finite. `width` must be greater than zero.

### Ellipse Drawing

```cpp
template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse(canvas<Width, Height, Policy>& img,
                  int cx,
                  int cy,
                  int radius_x,
                  int radius_y,
                  pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse(canvas<Width, Height, Policy>& img,
                  const point& center,
                  int radius_x,
                  int radius_y,
                  pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_ellipse(canvas<Width, Height, Policy>& img,
                  int cx,
                  int cy,
                  int radius_x,
                  int radius_y,
                  pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_ellipse(canvas<Width, Height, Policy>& img,
                  const point& center,
                  int radius_x,
                  int radius_y,
                  pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_aa(canvas<Width, Height, Policy>& img,
                     float cx,
                     float cy,
                     float radius_x,
                     float radius_y,
                     pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_aa(canvas<Width, Height, Policy>& img,
                     float cx,
                     float cy,
                     float radius_x,
                     float radius_y,
                     float width,
                     pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_aa(canvas<Width, Height, Policy>& img,
                     const pointf& center,
                     float radius_x,
                     float radius_y,
                     pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_aa(canvas<Width, Height, Policy>& img,
                     const pointf& center,
                     float radius_x,
                     float radius_y,
                     float width,
                     pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_ellipse_aa(canvas<Width, Height, Policy>& img,
                     float cx,
                     float cy,
                     float radius_x,
                     float radius_y,
                     pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_ellipse_aa(canvas<Width, Height, Policy>& img,
                     const pointf& center,
                     float radius_x,
                     float radius_y,
                     pixel color) noexcept
    -> xer::result<void>;
```

Ellipse APIs use independent x and y radii. `draw_ellipse` draws a one-pixel outline, `fill_ellipse` fills the interior, `draw_ellipse_aa` supports antialiased and thick outlines, and `fill_ellipse_aa` antialiases the outer boundary of the filled shape.

Degenerate ellipses are defined rather than rejected:

- both radii zero: a point
- `radius_x == 0`: a vertical line segment
- `radius_y == 0`: a horizontal line segment

The same policy applies to antialiased ellipses. Antialiased outline degeneration keeps the requested width; antialiased filled degeneration draws the corresponding one-pixel-equivalent antialiased point or line.

A negative radius returns `error_t::invalid_argument`. Antialiased ellipse drawing also rejects non-finite center coordinates, radii, and width, and rejects `width <= 0.0f`.

### Circular Arc Drawing

```cpp
template <std::size_t Width, std::size_t Height, class Policy>
auto draw_arc(canvas<Width, Height, Policy>& img,
              int cx,
              int cy,
              int radius,
              float start_angle,
              float sweep_angle,
              pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_arc(canvas<Width, Height, Policy>& img,
              const point& center,
              int radius,
              float start_angle,
              float sweep_angle,
              pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_arc_aa(canvas<Width, Height, Policy>& img,
                 float cx,
                 float cy,
                 float radius,
                 float start_angle,
                 float sweep_angle,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_arc_aa(canvas<Width, Height, Policy>& img,
                 float cx,
                 float cy,
                 float radius,
                 float start_angle,
                 float sweep_angle,
                 float width,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_arc_aa(canvas<Width, Height, Policy>& img,
                 const pointf& center,
                 float radius,
                 float start_angle,
                 float sweep_angle,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_arc_aa(canvas<Width, Height, Policy>& img,
                 const pointf& center,
                 float radius,
                 float start_angle,
                 float sweep_angle,
                 float width,
                 pixel color) noexcept
    -> xer::result<void>;
```

Arc angles are expressed in τrad units. `0` points right. Positive sweep angles move counterclockwise in the mathematical sense; because image y coordinates grow downward, the point formula is:

```text
x = cx + radius * cos(angle * τ)
y = cy - radius * sin(angle * τ)
```

A negative `sweep_angle` draws clockwise. When `abs(sweep_angle)` is at least one full turn, arc drawing is treated as full circle drawing. Multiple turns are not accumulated.

Circular arc degeneration is:

- `radius == 0`: the center point
- `sweep_angle == 0`: the start point on the circle
- `abs(sweep_angle) >= 1`: a full circle

Antialiased arc endpoints use round caps. A zero-radius or zero-sweep antialiased arc therefore appears as a round point with the requested outline width.

Arc drawing rejects negative radii and non-finite angles. Antialiased arc drawing also rejects non-finite center coordinates, radius, and width, and rejects `width <= 0.0f`.

### Elliptical Arc Drawing

```cpp
template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_arc(canvas<Width, Height, Policy>& img,
                      int cx,
                      int cy,
                      int radius_x,
                      int radius_y,
                      float start_angle,
                      float sweep_angle,
                      pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_arc(canvas<Width, Height, Policy>& img,
                      const point& center,
                      int radius_x,
                      int radius_y,
                      float start_angle,
                      float sweep_angle,
                      pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_arc_aa(canvas<Width, Height, Policy>& img,
                         float cx,
                         float cy,
                         float radius_x,
                         float radius_y,
                         float start_angle,
                         float sweep_angle,
                         pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_arc_aa(canvas<Width, Height, Policy>& img,
                         float cx,
                         float cy,
                         float radius_x,
                         float radius_y,
                         float start_angle,
                         float sweep_angle,
                         float width,
                         pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_arc_aa(canvas<Width, Height, Policy>& img,
                         const pointf& center,
                         float radius_x,
                         float radius_y,
                         float start_angle,
                         float sweep_angle,
                         pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_arc_aa(canvas<Width, Height, Policy>& img,
                         const pointf& center,
                         float radius_x,
                         float radius_y,
                         float start_angle,
                         float sweep_angle,
                         float width,
                         pixel color) noexcept
    -> xer::result<void>;
```

Elliptical arc angles follow the same convention as circular arcs:

```text
x = cx + radius_x * cos(angle * τ)
y = cy - radius_y * sin(angle * τ)
```

A sweep of at least one full turn is treated as a full ellipse. A zero sweep draws the start point. If both radii are zero, the result is the center point. If exactly one radius is zero, the arc degenerates onto the corresponding vertical or horizontal line while preserving the angle-based parameterization.

Antialiased elliptical arcs use round caps and support thick strokes through `width`.

Elliptical arc drawing rejects negative radii and non-finite angles. Antialiased elliptical arc drawing also rejects non-finite center coordinates, radii, and width, and rejects `width <= 0.0f`.

### Clipping, Pixels, and Return Values

All curved-shape drawing clips to the canvas boundary. A shape that lies completely outside the canvas is a successful no-op.

Integer circle and ellipse drawing write the supplied logical `pixel` directly. Antialiased drawing uses coverage blending through the canvas pixel API.

These drawing functions return `xer::result<void>`, but the return values are intentionally not marked `[[nodiscard]]`. This keeps drawing calls lightweight in rendering code while still allowing invalid arguments to be handled where needed.

---

## Flood Fill

```cpp
template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto flood_fill(canvas<Width, Height, Policy>& img,
                              int x,
                              int y,
                              pixel color)
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto flood_fill(canvas<Width, Height, Policy>& img,
                              const point& origin,
                              pixel color)
    -> xer::result<void>;
```

`flood_fill` replaces the four-connected region that contains the start position.

The original logical `pixel` value at the start position is used as the target color. Every reachable pixel whose logical ARGB value exactly matches that original color is replaced with `color`.

### Connectivity

The initial implementation uses four-connected adjacency only:

- left
- right
- up
- down

Diagonal contact alone does not connect two regions.

### No-Op Cases

`flood_fill` is a successful no-op when:

- the start position is outside the canvas
- the replacement color is equal to the original color at the start position

### Result

`flood_fill` returns `xer::result<void>`.

The operation uses an internal pending-position buffer rather than recursive traversal, so it does not rely on call-stack depth for large filled regions.

---

## Image Processing Functions

`mosaic`, `box_blur`, and `filter_pixels` are in-place image-processing operations.

```cpp
template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto mosaic(canvas<Width, Height, Policy>& img,
                          const rect& area,
                          const size& block_size) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto box_blur(canvas<Width, Height, Policy>& img,
                            const rect& area,
                            const size& box_size)
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy, class F>
[[nodiscard]] auto filter_pixels(canvas<Width, Height, Policy>& img,
                                 const rect& area,
                                 F&& filter)
    -> xer::result<void, filter_pixels_error_detail>;
```

All three functions clip `area` to the canvas boundary. Empty areas and fully clipped areas are successful no-ops.

`mosaic` divides the clipped area into blocks of `block_size`. Each block is replaced with the average logical ARGB color of the pixels in that block. Blocks at the right and bottom edges use their actual clipped size.

`box_blur` treats `box_size` as the averaging kernel size. For example, `size(3, 3)` applies a 3x3 average around each destination pixel. Source samples are taken from a copy of the original pixels in the clipped target area, so pixels outside the requested area do not affect the result. Kernel portions outside the clipped area are ignored.

Even kernel dimensions are supported. In that case, the extra sample is placed on the left or top side of the current pixel.

`mosaic` and `box_blur` return `error_t::invalid_argument` when either size dimension is not positive.

`filter_pixels` applies a user-supplied per-pixel filter to the clipped area. For each pixel, the filter receives the current logical `pixel` value and returns the replacement logical `pixel` value. This supports grayscale conversion, thresholding, channel adjustment, inversion, and similar operations without adding a dedicated function for each effect.

The operation is in-place and does not allocate a full temporary image. If the filter throws an exception for a pixel, that pixel is left unchanged and processing continues with the next pixel. If one or more pixels fail, the function returns `error_t::user_error` with `filter_pixels_error_detail`. Successfully filtered pixels remain updated.

Example grayscale-style use:

```cpp
auto result = xer::image::filter_pixels(
    img,
    xer::image::rect(xer::image::point(0, 0), xer::image::size(16, 16)),
    [](xer::image::pixel p) -> xer::image::pixel {
        const auto gray = static_cast<std::uint8_t>(
            (static_cast<unsigned>(p.red()) +
             static_cast<unsigned>(p.green()) +
             static_cast<unsigned>(p.blue())) / 3u);

        return xer::image::pixel(p.alpha(), gray, gray, gray);
    });
```

---

## Relationship to Tcl/Tk

`<xer/image.h>` does not depend on Tcl/Tk.

Tk photo bridge functions should live in `<xer/tk.h>`. They may convert between Tk photo image blocks and `xer::image::canvas` or `xer::image::dynamic_canvas` later, but pure image storage, drawing, and image processing remain in `<xer/image.h>`.

---

## Deferred Items

The following items are deferred from the current implementation:

- affine transformation
- raster scroll
- grayscale conversion
- image flipping
- file format loading and saving
- direct Tk photo conversion helpers

These can be added once the basic framebuffer type is stable.

---

## Example

```cpp
#include <xer/image.h>
#include <xer/stdio.h>

auto main() -> int
{
    xer::image::canvas<4, 4> img;

    img.clear();

    // This line intentionally starts outside the canvas.
    // xer clips it to the framebuffer boundary.
    xer::image::draw_hline(
        img,
        -2,
        1,
        4,
        xer::image::pixel(0xffu, 0x00u, 0x00u));

    const auto value = img.get_pixel(0, 1);
    return value.argb == 0xffff0000u ? 0 : 1;
}
```

Additional examples:

- `examples/example_image_basic.cpp`
- `examples/example_image_geometry_io.cpp`
- `examples/example_image_effects.cpp`
- `examples/example_image_filter_pixels.cpp`
- `examples/example_image_bitmap_text.cpp`
- `examples/example_image_flood_fill.cpp`
- `examples/example_image_circle.cpp`
- `examples/example_image_curves.cpp`

---

## See Also

- `header_iostream.md`
- `header_parse.md`
- `policy_image.md`
- `policy_bitmap_font.md`
