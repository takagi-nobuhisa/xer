# `<xer/serialize.h>`

## Purpose

`<xer/serialize.h>` provides low-level binary transfer archives for fixed-schema data.

The design is intentionally simple. It does not store type names, field names, schema information, object identifiers, or version metadata. Callers and generated code are expected to know the exact field order and field types.

This header is intended to support generated `xfer` functions. A schema generator can emit one field-transfer function and use it for both output and input by passing either `binary_output_archive` or `binary_input_archive`.

---

## Binary Format Policy

The binary format is fixed as follows:

- scalar values are stored directly
- multi-byte scalar values are little-endian
- `float` is stored as IEEE 754 binary32 bytes
- `double` is stored as IEEE 754 binary64 bytes
- `bool` is stored as one byte, `0` or `1`
- `std::u8string` is stored as `uint64 byte_size` followed by UTF-8 bytes
- `std::vector<std::byte>` is stored as `uint64 byte_size` followed by raw bytes
- `std::array<T, N>` stores only its elements; the array size is not stored
- `std::vector<T>` stores `uint64 element_count` followed by elements
- `std::map<K, V>` stores `uint64 element_count` followed by key/value pairs in map iteration order

No byte-order marker is written. Little-endian is part of the format.

---

## Main Entities

```cpp
class binary_output_archive;
class binary_input_archive;
```

The archive objects expose `operator()` instead of separate `write` and `read` function names. This allows generated code to use a single transfer function for both directions.

```cpp
template<class Archive>
auto xfer(Archive& archive, sample& value) -> xer::result<void>
{
    if (auto r = archive(value.id); !r) {
        return std::unexpected(r.error());
    }
    if (auto r = archive(value.name); !r) {
        return std::unexpected(r.error());
    }
    return {};
}
```

---

## Supported Types

The initial low-level implementation directly supports the following types:

```cpp
bool

std::uint8_t
std::uint16_t
std::uint32_t
std::uint64_t

std::int8_t
std::int16_t
std::int32_t
std::int64_t

float
double

std::u8string
std::vector<std::byte>

std::array<T, N>
std::vector<T>
std::map<K, V>
```

The container element types must themselves be supported by the same archive.

Environment-dependent integer types such as `int`, `long`, and `std::size_t` are intentionally not provided as direct serialization types. Fixed-width integer types should be used in serialized structures.

---

## `binary_output_archive`

```cpp
class binary_output_archive;
```

`binary_output_archive` owns an internal byte buffer and appends serialized data to it.

```cpp
auto bytes() const noexcept -> std::span<const std::byte>;
auto release() noexcept -> std::vector<std::byte>;
```

`bytes` returns a non-owning view of the current buffer. The view is invalidated by later writes or by `release`.

`release` moves out the accumulated byte sequence and clears the archive.

The archive provides `operator()` overloads for supported output types.

```cpp
xer::binary_output_archive out;
std::uint32_t id = 42;
std::u8string name = u8"xer";

out(id);
out(name);
```

Output functions return `xer::result<void>` for symmetry and to report allocation or size-related failures.

---

## `binary_input_archive`

```cpp
class binary_input_archive;
```

`binary_input_archive` reads serialized data from a byte span.

```cpp
explicit binary_input_archive(std::span<const std::byte> data) noexcept;

auto remaining_size() const noexcept -> std::size_t;
auto empty() const noexcept -> bool;
```

The archive provides `operator()` overloads for supported input types.

```cpp
xer::binary_input_archive in(bytes);
std::uint32_t id{};
std::u8string name;

in(id);
in(name);
```

Input functions store the result into the supplied reference and return `xer::result<void>`.

When the input does not contain enough bytes for the requested value, the function returns:

```cpp
error_t::end_of_file
```

Invalid bool values and duplicate map keys in serialized input are reported as:

```cpp
error_t::invalid_argument
```

Excessively large length values are reported as:

```cpp
error_t::length_error
```

---

## Generated `xfer` Functions

A typical generated transfer function should call the archive for each field in fixed order.

```cpp
struct sample {
    std::uint32_t id;
    std::u8string name;
    std::vector<std::uint16_t> flags;
};

template<class Archive>
auto xfer(Archive& archive, sample& value) -> xer::result<void>
{
    if (auto r = archive(value.id); !r) {
        return std::unexpected(r.error());
    }
    if (auto r = archive(value.name); !r) {
        return std::unexpected(r.error());
    }
    if (auto r = archive(value.flags); !r) {
        return std::unexpected(r.error());
    }
    return {};
}
```

The same function is used for output and input:

```cpp
sample source{};
xer::binary_output_archive out;
xfer(out, source);

auto data = out.release();

sample restored{};
xer::binary_input_archive in(data);
xfer(in, restored);
```

---

## Versioning Model

This format is not self-describing. If a structure changes across versions, callers should handle compatibility outside the low-level archive layer.

Common strategies include:

- placing a version field at the start of the structure
- wrapping payloads in an outer versioned structure
- generating separate `xfer` functions for old and new layouts
- restricting additions to trailing fields when suitable

`<xer/serialize.h>` deliberately keeps this policy outside the low-level archive implementation.
