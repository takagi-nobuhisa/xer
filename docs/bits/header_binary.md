# `<xer/binary.h>`

## Purpose

`<xer/binary.h>` provides small binary-data utility functions.

The current scope is intentionally narrow. This header covers fixed-width unsigned integer splitting and composition, bit-order reversal, byte-order reversal support for XER's 128-bit unsigned integer type, simple checksum calculation, CRC calculation for byte sequences and files, and hexadecimal conversion helpers.

These functions treat input values as fixed-width binary values. They do not depend on the CPU's native endian setting.

---

## Main Entities

At minimum, `<xer/binary.h>` provides the following facilities:

```cpp
enum class byte_order {
    little_endian,
    big_endian,
};

using std::byteswap;

auto high_u8(std::uint16_t value) noexcept -> std::uint8_t;
auto low_u8(std::uint16_t value) noexcept -> std::uint8_t;
auto make_u16(std::uint8_t high, std::uint8_t low) noexcept -> std::uint16_t;

auto high_u16(std::uint32_t value) noexcept -> std::uint16_t;
auto low_u16(std::uint32_t value) noexcept -> std::uint16_t;
auto make_u32(std::uint16_t high, std::uint16_t low) noexcept -> std::uint32_t;

auto high_u32(std::uint64_t value) noexcept -> std::uint32_t;
auto low_u32(std::uint64_t value) noexcept -> std::uint32_t;
auto make_u64(std::uint32_t high, std::uint32_t low) noexcept -> std::uint64_t;
```

When `xer::uint128_t` is available, the header also provides:

```cpp
auto high_u64(xer::uint128_t value) noexcept -> std::uint64_t;
auto low_u64(xer::uint128_t value) noexcept -> std::uint64_t;
auto make_u128(std::uint64_t high, std::uint64_t low) noexcept -> xer::uint128_t;

auto byteswap(xer::uint128_t value) noexcept -> xer::uint128_t;
```

The header also provides bit-order reversal:

```cpp
auto reverse_bits(std::uint8_t value) noexcept -> std::uint8_t;
auto reverse_bits(std::uint16_t value) noexcept -> std::uint16_t;
auto reverse_bits(std::uint32_t value) noexcept -> std::uint32_t;
auto reverse_bits(std::uint64_t value) noexcept -> std::uint64_t;
```

When `xer::uint128_t` is available:

```cpp
auto reverse_bits(xer::uint128_t value) noexcept -> xer::uint128_t;
```

For simple checksums, the header provides additive checksums, XOR checksums, and convenience aliases in 8-bit, 16-bit, and 32-bit forms. It also provides CRC16 and CRC32 calculation helpers.

The header also provides PHP-style hexadecimal conversion helpers:

```cpp
auto bin2hex(std::span<const std::byte> bytes) -> std::u8string;

auto bin2hex(const void* data, std::size_t size)
    -> xer::result<std::u8string>;

template<std::input_iterator InputIt>
auto bin2hex(InputIt first, InputIt last) -> std::u8string;

auto hex2bin(std::u8string_view hex) -> xer::result<std::vector<std::byte>>;
```

---

## Integer Splitting and Composition

The `high_uN` and `low_uN` functions extract the upper or lower half of an unsigned integer value.

```cpp
auto high = xer::high_u8(std::uint16_t{0x1234}); // 0x12
auto low = xer::low_u8(std::uint16_t{0x1234});  // 0x34
```

The `make_uN` functions compose a larger unsigned integer value from its upper and lower parts.

```cpp
auto value = xer::make_u16(0x12, 0x34); // 0x1234
```

The naming convention indicates the size of the extracted part:

- `high_u8` and `low_u8` extract 8-bit parts from a 16-bit value
- `high_u16` and `low_u16` extract 16-bit parts from a 32-bit value
- `high_u32` and `low_u32` extract 32-bit parts from a 64-bit value
- `high_u64` and `low_u64` extract 64-bit parts from a 128-bit value

The composition functions indicate the size of the composed result:

- `make_u16` composes a 16-bit value from two 8-bit values
- `make_u32` composes a 32-bit value from two 16-bit values
- `make_u64` composes a 64-bit value from two 32-bit values
- `make_u128` composes a 128-bit value from two 64-bit values

These functions operate on integer values, not on object representation in memory. Their behavior is independent of native endian.

---

## `byteswap`

`<xer/binary.h>` imports `std::byteswap` into the `xer` namespace.

```cpp
using std::byteswap;
```

This makes standard C++23 byte swapping available as `xer::byteswap` for the standard unsigned integer types supported by `std::byteswap`.

When `xer::uint128_t` is available, XER also provides a 128-bit overload:

```cpp
auto byteswap(xer::uint128_t value) noexcept -> xer::uint128_t;
```

For example, a 128-bit value whose high and low halves are:

```text
0x0011223344556677_8899aabbccddeeff
```

is byte-swapped to:

```text
0xffeeddccbbaa9988_7766554433221100
```

The 128-bit overload is provided because C++23 `std::byteswap` does not cover XER's extended `uint128_t` type.

---

## `reverse_bits`

`reverse_bits` reverses the order of all bits in a fixed-width unsigned integer value.

```cpp
auto value = xer::reverse_bits(std::uint8_t{0b0001'0010}); // 0b0100'1000
```

This is different from byte-order reversal.

```cpp
std::uint16_t value = 0x1234;

xer::byteswap(value);     // 0x3412
xer::reverse_bits(value); // 0x2c48
```

`reverse_bits` treats the input as a fixed-width value. For example, the 8-bit overload reverses exactly 8 bits, and the 32-bit overload reverses exactly 32 bits.

---

## `byte_order`

`byte_order` specifies how consecutive bytes are grouped into 16-bit or 32-bit words for checksum calculation.

```cpp
enum class byte_order {
    little_endian,
    big_endian,
};
```

This enum is used only when byte sequences are interpreted as 16-bit or 32-bit words.

For 8-bit checksums, no byte-order setting is needed because each input byte is already one checksum unit.

---

## 8-bit Checksums

The 8-bit checksum functions calculate a simple checksum over individual bytes.

```cpp
auto checksum8(std::span<const std::byte> bytes) noexcept -> std::uint8_t;
auto checksum_add8(std::span<const std::byte> bytes) noexcept -> std::uint8_t;
auto checksum_xor8(std::span<const std::byte> bytes) noexcept -> std::uint8_t;
```

`checksum8` is the usual additive 8-bit checksum. It is equivalent to `checksum_add8`.

`checksum_add8` adds all bytes modulo 256.

`checksum_xor8` XORs all bytes.

For example:

```cpp
const std::array<std::byte, 4> bytes {
    std::byte{0x01},
    std::byte{0x02},
    std::byte{0x03},
    std::byte{0x04},
};

const auto sum = xer::checksum8(std::span<const std::byte>(bytes));      // 0x0a
const auto x = xer::checksum_xor8(std::span<const std::byte>(bytes));   // 0x04
```

---

## 16-bit Checksums

The 16-bit checksum functions group the input bytes into 16-bit words and then calculate either an additive checksum or an XOR checksum.

```cpp
auto checksum16(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint16_t;

auto checksum_add16(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint16_t;

auto checksum_xor16(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint16_t;
```

`checksum16` is the usual additive 16-bit checksum. It is equivalent to `checksum_add16`.

The byte order controls how each pair of bytes is converted into a word.

For bytes `{0x01, 0x02}`:

- `byte_order::big_endian` reads the word as `0x0102`
- `byte_order::little_endian` reads the word as `0x0201`

If the input has an odd number of bytes, the final missing byte is treated as `0x00`.

For example, with big-endian grouping, `{0x01, 0x02, 0x03}` is treated as:

```text
0x0102, 0x0300
```

With little-endian grouping, the same input is treated as:

```text
0x0201, 0x0003
```

---

## 32-bit Checksums

The 32-bit checksum functions group the input bytes into 32-bit words and then calculate either an additive checksum or an XOR checksum.

```cpp
auto checksum32(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint32_t;

auto checksum_add32(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint32_t;

auto checksum_xor32(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint32_t;
```

`checksum32` is the usual additive 32-bit checksum. It is equivalent to `checksum_add32`.

For bytes `{0x01, 0x02, 0x03, 0x04}`:

- `byte_order::big_endian` reads the word as `0x01020304`
- `byte_order::little_endian` reads the word as `0x04030201`

If the input byte count is not a multiple of four, the final incomplete word is padded with zero bytes.

For example, with big-endian grouping, `{0x01, 0x02, 0x03}` is treated as:

```text
0x01020300
```

With little-endian grouping, the same input is treated as:

```text
0x00030201
```

---

## Checksum Input Forms

The checksum functions are provided for four input forms.

### `std::span<const std::byte>`

The span overloads are the primary in-memory byte-sequence overloads.

```cpp
auto checksum8(std::span<const std::byte> bytes) noexcept -> std::uint8_t;
auto checksum_add8(std::span<const std::byte> bytes) noexcept -> std::uint8_t;
auto checksum_xor8(std::span<const std::byte> bytes) noexcept -> std::uint8_t;

auto checksum16(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint16_t;

auto checksum_add16(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint16_t;

auto checksum_xor16(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint16_t;

auto checksum32(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint32_t;

auto checksum_add32(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint32_t;

auto checksum_xor32(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint32_t;
```

These overloads do not allocate and do not fail.

### Pointer and Size

Pointer-and-size overloads are provided for C-style byte buffers.

```cpp
auto checksum8(const void* data, std::size_t size) noexcept
    -> xer::result<std::uint8_t>;

auto checksum_add8(const void* data, std::size_t size) noexcept
    -> xer::result<std::uint8_t>;

auto checksum_xor8(const void* data, std::size_t size) noexcept
    -> xer::result<std::uint8_t>;

auto checksum16(const void* data, std::size_t size, byte_order order) noexcept
    -> xer::result<std::uint16_t>;

auto checksum_add16(const void* data, std::size_t size, byte_order order) noexcept
    -> xer::result<std::uint16_t>;

auto checksum_xor16(const void* data, std::size_t size, byte_order order) noexcept
    -> xer::result<std::uint16_t>;

auto checksum32(const void* data, std::size_t size, byte_order order) noexcept
    -> xer::result<std::uint32_t>;

auto checksum_add32(const void* data, std::size_t size, byte_order order) noexcept
    -> xer::result<std::uint32_t>;

auto checksum_xor32(const void* data, std::size_t size, byte_order order) noexcept
    -> xer::result<std::uint32_t>;
```

If `data` is `nullptr` and `size` is not zero, these overloads fail with `error_t::invalid_argument`.

`data == nullptr` with `size == 0` is accepted and represents an empty byte sequence.

### Iterator Range

Iterator-range overloads are provided for byte-like ranges.

```cpp
template<std::input_iterator InputIt>
auto checksum8(InputIt first, InputIt last) -> std::uint8_t;

template<std::input_iterator InputIt>
auto checksum_add8(InputIt first, InputIt last) -> std::uint8_t;

template<std::input_iterator InputIt>
auto checksum_xor8(InputIt first, InputIt last) -> std::uint8_t;

template<std::input_iterator InputIt>
auto checksum16(InputIt first, InputIt last, byte_order order) -> std::uint16_t;

template<std::input_iterator InputIt>
auto checksum_add16(InputIt first, InputIt last, byte_order order) -> std::uint16_t;

template<std::input_iterator InputIt>
auto checksum_xor16(InputIt first, InputIt last, byte_order order) -> std::uint16_t;

template<std::input_iterator InputIt>
auto checksum32(InputIt first, InputIt last, byte_order order) -> std::uint32_t;

template<std::input_iterator InputIt>
auto checksum_add32(InputIt first, InputIt last, byte_order order) -> std::uint32_t;

template<std::input_iterator InputIt>
auto checksum_xor32(InputIt first, InputIt last, byte_order order) -> std::uint32_t;
```

The iterator value type must be `std::byte` or a byte-like integer value convertible to `std::uint8_t`.

These overloads are useful with containers such as `std::vector<std::byte>`, `std::array<std::uint8_t, N>`, and similar byte-oriented storage.

### File Path

File overloads calculate the checksum of a whole file.

```cpp
auto checksum8(const path& filename) -> xer::result<std::uint8_t>;
auto checksum_add8(const path& filename) -> xer::result<std::uint8_t>;
auto checksum_xor8(const path& filename) -> xer::result<std::uint8_t>;

auto checksum16(const path& filename, byte_order order) -> xer::result<std::uint16_t>;
auto checksum_add16(const path& filename, byte_order order) -> xer::result<std::uint16_t>;
auto checksum_xor16(const path& filename, byte_order order) -> xer::result<std::uint16_t>;

auto checksum32(const path& filename, byte_order order) -> xer::result<std::uint32_t>;
auto checksum_add32(const path& filename, byte_order order) -> xer::result<std::uint32_t>;
auto checksum_xor32(const path& filename, byte_order order) -> xer::result<std::uint32_t>;
```

These overloads read the whole file content and then calculate the checksum. File I/O failures are reported through `xer::result`.

---

## Additive and XOR Checksum Meaning

The additive checksum functions add each checksum unit and keep only the low bits of the result.

- `checksum8` and `checksum_add8` keep the low 8 bits
- `checksum16` and `checksum_add16` keep the low 16 bits
- `checksum32` and `checksum_add32` keep the low 32 bits

`checksum8`, `checksum16`, and `checksum32` are convenience aliases for the additive checksum functions. Use `checksum_add8`, `checksum_add16`, or `checksum_add32` when the additive method should be explicit at the call site.

The XOR checksum functions XOR each checksum unit and return the result in the corresponding width.

These are intentionally simple checksums. They are useful for small binary formats, simple diagnostics, and compatibility with simple protocols.

They are not cryptographic hashes. Use `crc16` or `crc32` when compatibility with common CRC algorithms is needed.


---

## CRC16 and CRC32

`crc16` and `crc32` calculate standard CRC values over a byte sequence.

```cpp
auto crc16(std::span<const std::byte> bytes) noexcept -> std::uint16_t;
auto crc32(std::span<const std::byte> bytes) noexcept -> std::uint32_t;
```

`crc16` uses CRC-16/ARC parameters:

- polynomial: `0xa001`
- initial value: `0x0000`
- final XOR: none
- check value for `"123456789"`: `0xbb3d`

`crc32` uses CRC-32/ISO-HDLC parameters:

- polynomial: `0xedb88320`
- initial value: `0xffffffff`
- final XOR: `0xffffffff`
- check value for `"123456789"`: `0xcbf43926`

These functions operate on bytes. Unlike 16-bit and 32-bit simple checksums, CRC calculation does not use `byte_order`.

---

## CRC Input Forms

The CRC functions follow the same input-form policy as the checksum functions.

### `std::span<const std::byte>`

The span overloads are the primary in-memory byte-sequence overloads.

```cpp
auto crc16(std::span<const std::byte> bytes) noexcept -> std::uint16_t;
auto crc32(std::span<const std::byte> bytes) noexcept -> std::uint32_t;
```

These overloads do not allocate and do not fail.

### Pointer and Size

Pointer-and-size overloads are provided for C-style byte buffers.

```cpp
auto crc16(const void* data, std::size_t size) noexcept -> xer::result<std::uint16_t>;
auto crc32(const void* data, std::size_t size) noexcept -> xer::result<std::uint32_t>;
```

If `data` is `nullptr` and `size` is not zero, these overloads fail with `error_t::invalid_argument`.

`data == nullptr` with `size == 0` is accepted and represents an empty byte sequence.

### Iterator Range

Iterator-range overloads are provided for byte-like ranges.

```cpp
template<std::input_iterator InputIt>
auto crc16(InputIt first, InputIt last) -> std::uint16_t;

template<std::input_iterator InputIt>
auto crc32(InputIt first, InputIt last) -> std::uint32_t;
```

The iterator value type must be `std::byte` or a byte-like integer value convertible to `std::uint8_t`.

### File Path

File overloads calculate the CRC of a whole file.

```cpp
auto crc16(const path& filename) -> xer::result<std::uint16_t>;
auto crc32(const path& filename) -> xer::result<std::uint32_t>;
```

These overloads read the whole file content and then calculate the CRC. File I/O failures are reported through `xer::result`.

---

## Empty Input for CRC

Empty input is valid.

For empty input:

- `crc16` returns `0x0000`
- `crc32` returns `0x00000000`

---

## Empty Input

Empty input is valid.

For empty input, all checksum functions return zero.

---


## `bin2hex` and `hex2bin`

`bin2hex` converts binary data to a lowercase hexadecimal string.

```cpp
auto bin2hex(std::span<const std::byte> bytes) -> std::u8string;

auto bin2hex(const void* data, std::size_t size)
    -> xer::result<std::u8string>;

template<std::input_iterator InputIt>
auto bin2hex(InputIt first, InputIt last) -> std::u8string;
```

Each input byte is represented by exactly two hexadecimal digits.

```cpp
const std::array<std::byte, 4> bytes {
    std::byte{0x12},
    std::byte{0x34},
    std::byte{0xab},
    std::byte{0xcd},
};

const auto text = xer::bin2hex(std::span<const std::byte>(bytes));
// text == u8"1234abcd"
```

The output uses lowercase letters `a` through `f`.

The iterator overload accepts byte-like values. The iterator value type must be `std::byte` or a byte-like integer value convertible to `std::uint8_t`.

The pointer-and-size overload is provided for C-style byte buffers. If `data` is `nullptr` and `size` is not zero, it fails with `error_t::invalid_argument`. `data == nullptr` with `size == 0` is accepted and represents an empty byte sequence.

`hex2bin` converts a hexadecimal string back to binary data.

```cpp
auto hex2bin(std::u8string_view hex) -> xer::result<std::vector<std::byte>>;
```

`hex2bin` accepts both lowercase and uppercase hexadecimal digits.

```cpp
const auto bytes = xer::hex2bin(u8"1234ABCD");
if (!bytes.has_value()) {
    return 1;
}

// *bytes contains { 0x12, 0x34, 0xab, 0xcd }
```

The input string must contain an even number of characters. If the input length is odd, `hex2bin` fails with `error_t::invalid_argument`.

If the input contains a character other than `0` through `9`, `a` through `f`, or `A` through `F`, `hex2bin` fails with `error_t::invalid_argument`.

Empty input is valid:

- `bin2hex` returns an empty string for an empty byte sequence.
- `hex2bin` returns an empty vector for an empty string.

These functions are modeled after PHP's `bin2hex` and `hex2bin`, but use XER's usual C++ types and `xer::result` for fallible conversion.

---

## Relationship to Other Headers

`<xer/binary.h>` is useful together with:

- `<xer/bytes.h>`
- `<xer/stdio.h>`
- `<xer/stdint.h>`

The rough boundary is:

- `<xer/bytes.h>` converts text or byte-like storage into explicit byte views or byte vectors
- `<xer/binary.h>` performs small binary value manipulation, simple checksum calculation, and CRC calculation
- `<xer/stdio.h>` handles stream-based binary I/O and whole-file operations

---

## Example

```cpp
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include <xer/binary.h>

auto main() -> int
{
    const auto value = xer::make_u16(0x12, 0x34);
    const auto high = xer::high_u8(value);
    const auto low = xer::low_u8(value);

    if (value != 0x1234 || high != 0x12 || low != 0x34) {
        return 1;
    }

    const std::array<std::byte, 4> bytes {
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
    };

    const auto checksum = xer::checksum16(
        std::span<const std::byte>(bytes),
        xer::byte_order::big_endian);

    if (checksum != 0x0406) {
        return 1;
    }

    const auto crc = xer::crc32(std::span<const std::byte>(bytes));
    if (crc != 0xb63cfbcd) {
        return 1;
    }

    return 0;
}
```
