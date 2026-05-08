# `<xer/image.h>`

## Purpose

`<xer/image.h>` provides lightweight image and framebuffer facilities.

The initial purpose of this header is not full photo editing or complete image-file handling. It is a small framebuffer-oriented layer for fixed-size images, VRAM-style emulation, simple drawing, and later integration with Tcl/Tk photo images.

Pure image processing and drawing belong in `<xer/image.h>`. Tcl/Tk photo integration belongs in `<xer/tk.h>`.

---

## Main Entities

At minimum, `<xer/image.h>` provides the following entities:

```cpp
namespace xer {

struct pixel;

struct argb32_policy;
struct rgba32_policy;
struct rgb24_policy;
struct bgr24_policy;

template <std::size_t Width,
          std::size_t Height,
          class Policy = argb32_policy>
class image;

template <class Policy = argb32_policy>
using dynamic_image = image<0, 0, Policy>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_hline(image<Width, Height, Policy>& img,
                int x,
                int y,
                int length,
                pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_vline(image<Width, Height, Policy>& img,
                int x,
                int y,
                int length,
                pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line(image<Width, Height, Policy>& img,
               int x0,
               int y0,
               int x1,
               int y1,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_rect(image<Width, Height, Policy>& img,
               int x,
               int y,
               int width,
               int height,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_rect(image<Width, Height, Policy>& img,
               int x,
               int y,
               int width,
               int height,
               pixel color) noexcept -> void;

}
```

---

## Logical Pixel

`xer::pixel` represents a logical color value.

It is not the same thing as the physical framebuffer storage element. The physical storage format is controlled by the image policy.

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

An image policy controls the physical framebuffer storage format.

A policy provides:

```cpp
using storage_type = /* physical storage element type */;

static constexpr auto get(const storage_type& value) noexcept -> pixel;
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

## `image`

The primary image type is:

```cpp
template <std::size_t Width,
          std::size_t Height,
          class Policy = argb32_policy>
class image;
```

Fixed-size images are the main model because the initial use case is framebuffer-style handling such as VRAM emulation.

Examples:

```cpp
using screen = xer::image<256, 192>;
using sprite = xer::image<16, 16>;
using rgba_screen = xer::image<256, 192, xer::rgba32_policy>;
```

A dynamic-size image is represented as:

```cpp
image<0, 0, Policy>
```

The convenience alias is:

```cpp
template <class Policy = argb32_policy>
using dynamic_image = image<0, 0, Policy>;
```

Only these dimension forms are valid:

```text
Width > 0 && Height > 0
Width == 0 && Height == 0
```

Partial dynamic dimensions such as `image<0, 192>` and `image<256, 0>` are invalid.

---

## Public Pixel Access

The public pixel API uses logical pixels:

```cpp
auto get_pixel(std::size_t x, std::size_t y) const noexcept -> pixel;
auto set_pixel(std::size_t x, std::size_t y, pixel value) noexcept -> void;
```

`image::at()` is intentionally not provided.

Returning a reference to the physical storage element would expose the framebuffer layout and would be incorrect when the storage policy is not ARGB. `pixel` is logical. `Policy::storage_type` is physical.

The coordinates passed to `get_pixel` and `set_pixel` are expected to be inside the image.

---

## Basic Member Functions

`image` provides basic size and utility operations:

```cpp
auto width() const noexcept -> std::size_t;
auto height() const noexcept -> std::size_t;
auto size() const noexcept -> std::size_t;
auto empty() const noexcept -> bool;
auto contains(int x, int y) const noexcept -> bool;
auto fill(pixel value) noexcept -> void;
auto clear() noexcept -> void;
```

`clear()` fills the image with opaque black.

---

## Drawing Functions

The initial drawing functions are simple framebuffer helpers:

```cpp
draw_hline
draw_vline
draw_line
draw_rect
fill_rect
```

Drawing coordinates use `int` rather than `std::size_t` because drawing often benefits from clipping negative coordinates.

Drawing operations clip to the image bounds. If the target area is fully outside the image, nothing is drawn.

`draw_line` uses a simple Bresenham-style integer line algorithm.

---

## Relationship to Tcl/Tk

`<xer/image.h>` does not depend on Tcl/Tk.

Tk photo bridge functions should live in `<xer/tk.h>`. They may convert between Tk photo image blocks and `xer::image` or `xer::dynamic_image` later, but pure image storage, drawing, and image processing remain in `<xer/image.h>`.

---

## Deferred Items

The following items are deferred from the first implementation:

- blur
- mosaic
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
    xer::image<4, 4> img;

    img.clear();
    xer::draw_hline(img, 1, 1, 2, xer::pixel(0xffu, 0x00u, 0x00u));

    const auto value = img.get_pixel(1, 1);
    return value.argb == 0xffff0000u ? 0 : 1;
}
```

---

## See Also

- `policy_image.md`
- `header_tk.md`
- `header_color.md`
