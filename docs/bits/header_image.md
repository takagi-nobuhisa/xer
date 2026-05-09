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
                  pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line_aa(canvas<Width, Height, Policy>& img,
                  float x0,
                  float y0,
                  float x1,
                  float y1,
                  float width,
                  pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line_aa(canvas<Width, Height, Policy>& img,
                  const pointf& p0,
                  const pointf& p1,
                  pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line_aa(canvas<Width, Height, Policy>& img,
                  const pointf& p0,
                  const pointf& p1,
                  float width,
                  pixel color) noexcept -> void;

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
[[nodiscard]] auto mosaic(canvas<Width, Height, Policy>& img,
                          const rect& area,
                          const size& block_size) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto box_blur(canvas<Width, Height, Policy>& img,
                            const rect& area,
                            const size& box_size)
    -> xer::result<void>;

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

The `draw_rect` and `fill_rect` overloads accept either `point` plus `size`, or a single `rect`. The scalar-coordinate overloads remain available for callers that already have separate coordinate values.

---

## Image Processing Functions

`mosaic` and `box_blur` are in-place image-processing operations.

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
```

Both functions clip `area` to the canvas boundary. Empty areas and fully clipped areas are successful no-ops.

`mosaic` divides the clipped area into blocks of `block_size`. Each block is replaced with the average logical ARGB color of the pixels in that block. Blocks at the right and bottom edges use their actual clipped size.

`box_blur` treats `box_size` as the averaging kernel size. For example, `size(3, 3)` applies a 3x3 average around each destination pixel. Source samples are taken from a copy of the original pixels in the clipped target area, so pixels outside the requested area do not affect the result. Kernel portions outside the clipped area are ignored.

Even kernel dimensions are supported. In that case, the extra sample is placed on the left or top side of the current pixel.

Both functions return `error_t::invalid_argument` when either size dimension is not positive.

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
- circle drawing
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
    // XER clips it to the framebuffer boundary.
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

---

## See Also

- `header_iostream.md`
- `policy_image.md`
