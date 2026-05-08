# Policy for Image and Framebuffer Handling

## Overview

XER provides image and framebuffer facilities in order to support lightweight image processing, drawing, and integration with Tcl/Tk photo images.

The initial motivation is not general-purpose photo editing, but practical framebuffer-style handling such as VRAM emulation, simple GUI image manipulation, and small image-processing utilities.

The image facility should therefore keep the following priorities:

- represent logical colors clearly
- allow framebuffer storage formats to vary
- keep image processing independent of Tcl/Tk
- make fixed-size images the primary model
- allow dynamic-size images only where needed
- avoid exposing physical framebuffer layout unnecessarily

---

## Header Placement

The image facility is provided through an independent public header:

```text
xer/image.h
```

The implementation may be placed in:

```text
xer/bits/image.h
```

Tcl/Tk photo integration remains in:

```text
xer/tk.h
```

Only the bridge between Tk photo images and XER images belongs in `xer/tk.h`.
Pure image processing, drawing, and framebuffer manipulation belong in `xer/image.h`.

---

## Separation Between Logical Pixel and Framebuffer Storage

### Logical Pixel

A `pixel` represents a logical color value.
It is not necessarily identical to the physical representation used in a framebuffer.

The logical representation uses ARGB order in a 32-bit integer:

```text
0xAARRGGBB
```

A conceptual shape is:

```cpp
struct pixel {
    std::uint32_t argb = 0xff000000u;

    constexpr pixel() noexcept = default;
    constexpr explicit pixel(std::uint32_t value) noexcept;
    constexpr pixel(std::uint8_t red, std::uint8_t green, std::uint8_t blue) noexcept;
    constexpr pixel(
        std::uint8_t alpha,
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
The four-argument constructor follows the logical ARGB representation and therefore takes arguments in this order:

```text
alpha, red, green, blue
```

This order may feel less natural than RGBA, but it is consistent with the internal `argb` representation.

### Physical Framebuffer Storage

The physical framebuffer representation is controlled by a policy type.

For example, different policies may represent storage as:

- ARGB32
- RGBA32
- RGB24
- BGR24
- grayscale
- indexed color
- platform-specific framebuffer formats

The public image operations should not assume that the framebuffer storage itself is ARGB.

---

## Pixel Storage Policy

The framebuffer storage format is expressed as a template policy.

A policy type defines at least:

```cpp
using storage_type = /* physical storage element type */;

static constexpr auto get(const storage_type& value) noexcept -> pixel;
static constexpr auto encode(pixel value) noexcept -> storage_type;
static constexpr auto set(storage_type& dst, pixel value) noexcept -> void;
```

Examples of policy names may include:

```cpp
struct argb32_policy;
struct rgba32_policy;
struct rgb24_policy;
struct bgr24_policy;
```

The policy converts between the logical `pixel` type and the physical storage element.

`encode` converts a logical pixel into one physical storage element without requiring an existing destination object. Drawing functions can use it once before entering an inner loop and then repeatedly write the encoded storage value.

This design avoids runtime `switch` dispatch during per-pixel processing and allows the compiler to optimize format-specific operations.

---

## Image Type

### Basic Shape

The primary image type is a fixed-size image:

```cpp
template <
    std::size_t Width,
    std::size_t Height,
    class Policy = argb32_policy>
class image;
```

The main intended use case is framebuffer-like storage, including VRAM emulation.
For that purpose, fixed dimensions are usually sufficient and often preferable.

Example:

```cpp
using screen = xer::image<256, 192>;
using sprite = xer::image<16, 16, xer::argb32_policy>;
```

### Dynamic-Size Image

A dynamic-size image is represented by the same template with both dimensions set to zero:

```cpp
using dynamic_image = image<0, 0, Policy>;
```

A convenience alias should be provided:

```cpp
template <class Policy = argb32_policy>
using dynamic_image = image<0, 0, Policy>;
```

This keeps the image API unified while still allowing runtime-sized images where needed, such as when reading a Tk photo image or loading an image file.

### Invalid Partial Dynamic Dimensions

Only the following two forms are valid:

```text
Width > 0 && Height > 0
Width == 0 && Height == 0
```

The following forms are invalid:

```text
Width == 0 && Height > 0
Width > 0 && Height == 0
```

A static assertion should reject partial dynamic dimensions.

---

## Storage Separation

The difference between fixed-size and dynamic-size images should be isolated in a storage class.

Conceptually:

```cpp
template <std::size_t Width, std::size_t Height, class Policy>
class image_storage;
```

For fixed-size images, `image_storage` owns:

```cpp
std::array<storage_type, Width * Height>
```

For dynamic-size images, `image_storage<0, 0, Policy>` owns:

```cpp
std::vector<storage_type>
```

and stores runtime width and height.

The main `image` class should share most member functions between fixed-size and dynamic-size images by delegating memory ownership details to `image_storage`.

---

## Public Pixel Access

The public image API should expose logical pixel access rather than raw framebuffer element access.

The ordinary accessors are:

```cpp
auto get_pixel(std::size_t x, std::size_t y) const noexcept -> pixel;
auto set_pixel(int x, int y, pixel value) noexcept -> void;
auto set_pixel_unchecked(
    std::size_t x,
    std::size_t y,
    pixel value) noexcept -> void;
```

`image::at()` returning a physical storage reference should not be provided as an ordinary public API.

The reason is that returning a reference to the physical framebuffer element would expose and depend on the storage format. If the framebuffer format is not ARGB, modifying such a reference as if it were a logical pixel would be incorrect.

`set_pixel` should be safe for ordinary use and may ignore coordinates outside the image boundary. `set_pixel_unchecked` is a low-level member for code that has already performed boundary checks or clipping. Its caller must guarantee that the coordinates are inside the framebuffer.

`pixel` is logical.
`Policy::storage_type` is physical.
The public API should preserve this distinction.

---

## Width, Height, and Stride

For the initial owning `image` type, the display width and the memory width are the same.

This is acceptable because ordinary users are not expected to manipulate framebuffer array elements directly.
All normal access goes through logical operations such as `get_pixel`, `set_pixel`, drawing functions, and image-processing functions.

If stride-aware external memory access becomes necessary, it should be introduced through a separate view type or low-level helper rather than complicating the primary owning `image` type.

Possible future types include:

```cpp
image_view
framebuffer_view
```

These may carry explicit stride information.

---

## Drawing Functions

Drawing functions belong to `xer/image.h`, not `xer/tk.h`.

At minimum, the following kinds of drawing functions are candidates:

```cpp
auto draw_line(image& img, point p0, point p1, pixel color) -> void;
auto draw_rect(image& img, rect area, pixel color) -> void;
auto fill_rect(image& img, rect area, pixel color) -> void;
auto draw_circle(image& img, point center, int radius, pixel color) -> void;
auto fill_circle(image& img, point center, int radius, pixel color) -> void;
```

Coordinate-oriented drawing functions should use signed integer coordinates.
This allows negative coordinates and makes clipping more natural.

Out-of-range drawing should be clipped rather than treated as an error.
If a shape lies completely outside the image, the function should do nothing.

Horizontal and vertical line drawing may use more direct internal storage access for efficiency, but such access should remain implementation detail and should still respect the framebuffer policy.

---

## Image Processing Functions

Image processing functions also belong to `xer/image.h` and should be independent of Tcl/Tk.

Candidate operations include:

- inversion
- grayscale conversion
- horizontal flip
- vertical flip
- raster scroll
- mosaic
- blur
- affine transformation

The image-processing functions should be templates over image dimensions and policy where practical:

```cpp
template <std::size_t W, std::size_t H, class Policy>
auto invert(image<W, H, Policy>& img) noexcept -> void;
```

When an operation needs to create a new image, the result should preserve the dimensions and policy unless there is a clear reason not to.

Operations that can fail because of invalid arguments, such as a zero mosaic block size, should return `xer::result`.
Operations with no ordinary failure condition may return directly or mutate in place.

---

## Tcl/Tk Photo Integration

Tk photo integration should be handled only as a boundary layer in `xer/tk.h`.

The Tcl/Tk side may use APIs such as:

```text
Tk_PhotoGetImage
Tk_PhotoPutBlock
```

The public XER API should avoid exposing Tcl/Tk photo block details unless necessary.

Conceptually, the bridge may provide functions such as:

```cpp
namespace xer::tk {

    auto photo_to_image(
        interpreter& interp,
        std::u8string_view photo_name)
        -> xer::result<xer::dynamic_image<>, error_detail>;

    template <std::size_t W, std::size_t H, class Policy>
    auto image_to_photo(
        interpreter& interp,
        std::u8string_view photo_name,
        const xer::image<W, H, Policy>& img)
        -> xer::result<void, error_detail>;

}
```

The exact API can be decided when implementing the Tk bridge.

The important rule is:

- Tk-specific pitch, offsets, and photo block details are handled in `xer/tk.h`
- pure image storage and processing are handled in `xer/image.h`

---

## Initial Implementation Scope

For the first implementation, the recommended scope is:

1. `pixel`
2. basic framebuffer policies such as `argb32_policy` and `rgba32_policy`
3. `image_storage`
4. `image<Width, Height, Policy>`
5. `dynamic_image<Policy>` alias
6. `get_pixel` / `set_pixel`
7. simple drawing functions such as horizontal line, vertical line, rectangle, and filled rectangle
8. simple image operations such as invert, grayscale, and flip

Tk photo integration, blur, mosaic, raster scroll, affine transformation, and more advanced drawing can be added after the core model is stable.

---

## Summary

- `pixel` is a logical ARGB color represented as `0xAARRGGBB`.
- The framebuffer storage format is controlled by a template policy.
- `image<Width, Height, Policy>` is the primary fixed-size image type.
- `image<0, 0, Policy>` is the dynamic-size specialization.
- `dynamic_image<Policy>` is an alias for `image<0, 0, Policy>`.
- Only both dimensions zero means dynamic size; partial dynamic dimensions are invalid.
- Fixed and dynamic memory ownership should be isolated in `image_storage`.
- Public pixel access uses `get_pixel` and `set_pixel`, not `at()` returning physical storage.
- Drawing and image processing are independent of Tcl/Tk.
- Tcl/Tk photo integration belongs only in `xer/tk.h` as a boundary layer.
