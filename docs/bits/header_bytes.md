# `<xer/bytes.h>`

## Purpose

`<xer/bytes.h>` provides byte-sequence conversion helpers.

The purpose of this header is to make it easy to pass ordinary byte-like or text storage to byte-oriented APIs such as Base64 encoding, binary streams, sockets, and process pipes.

---

## Main Entities

At minimum, `<xer/bytes.h>` provides the following functions:

```cpp
auto to_bytes_view(std::string_view value) noexcept
    -> std::span<const std::byte>;

auto to_bytes_view(std::u8string_view value) noexcept
    -> std::span<const std::byte>;

auto to_bytes_view(std::span<const char> value) noexcept
    -> std::span<const std::byte>;

auto to_bytes_view(std::span<const char8_t> value) noexcept
    -> std::span<const std::byte>;

auto to_bytes_view(std::span<const unsigned char> value) noexcept
    -> std::span<const std::byte>;

auto to_bytes_view(std::span<const std::byte> value) noexcept
    -> std::span<const std::byte>;

auto to_bytes(std::string_view value) -> std::vector<std::byte>;
auto to_bytes(std::u8string_view value) -> std::vector<std::byte>;
auto to_bytes(std::span<const char> value) -> std::vector<std::byte>;
auto to_bytes(std::span<const char8_t> value) -> std::vector<std::byte>;
auto to_bytes(std::span<const unsigned char> value) -> std::vector<std::byte>;
auto to_bytes(std::span<const std::byte> value) -> std::vector<std::byte>;
```

---

## `to_bytes_view`

`to_bytes_view` creates a non-owning `std::span<const std::byte>` view of the supplied storage.

No allocation or copying is performed. The returned span refers to the same memory as the input.

Because the returned value is a view, the caller must ensure that the input storage outlives the returned span.

---

## `to_bytes`

`to_bytes` creates an owning `std::vector<std::byte>` copy of the supplied storage.

This is useful when the caller needs an independent byte sequence whose lifetime is not tied to the original string or span.

---

## Design Notes

These helpers do not perform character encoding conversion.

For example, passing a `std::u8string_view` to `to_bytes_view` exposes the UTF-8 code units as bytes. It does not validate, normalize, or reinterpret the text as another encoding.

The distinction is simple:

- `to_bytes_view` is non-owning and does not copy
- `to_bytes` is owning and copies

---

## Relationship to Other Headers

`<xer/bytes.h>` is especially useful together with:

- `<xer/base64.h>`
- `<xer/stdio.h>`
- `<xer/socket.h>`
- `<xer/process.h>`

The rough boundary is:

- `<xer/bytes.h>` converts byte-like or text storage into explicit byte sequences
- `<xer/base64.h>` converts bytes to and from Base64 text
- `<xer/stdio.h>` handles binary and text streams

---

## Example

```cpp
#include <string_view>

#include <xer/base64.h>
#include <xer/bytes.h>

auto main() -> int
{
    constexpr std::u8string_view text = u8"hello";

    const auto bytes = xer::to_bytes_view(text);
    const auto encoded = xer::base64_encode(bytes);
    if (!encoded.has_value()) {
        return 1;
    }

    return 0;
}
```
