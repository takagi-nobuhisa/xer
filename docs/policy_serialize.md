# Policy for Binary Serialization

## Overview

XER provides a compact binary serialization layer through `<xer/serialize.h>`.

The serialization design is based on fixed schemas and generated field-transfer functions. It is intentionally not a reflection-based or self-describing serialization framework.

The primary goals are:

- compact binary output
- predictable cross-platform layout
- simple generated C++ code
- easy interoperation with other languages through the same schema source
- no dependence on C++ reflection, runtime field registration, or intrusive base classes

---

## Responsibility Split

### Low-Level XER Layer

`<xer/serialize.h>` is responsible for transferring supported scalar and standard container types.

It defines:

- `xer::binary_output_archive`
- `xer::binary_input_archive`
- archive `operator()` overloads for supported types

It does not know about field names, type names, structure names, or schema versions.

### Generated Code Layer

User-defined structures are handled by generated C++ code.

The generator emits:

- C++ structure definitions
- one `xfer` function per structure
- a generated-at schema timestamp constant

A schema file may define one structure or multiple structures. For generated examples and reproducible tests, the generation timestamp may be supplied explicitly by the generator command.

The generated `xfer` function calls the archive once for each field in fixed order.

---

## Binary Format

The format stores data only.

It does not store:

- field names
- type names
- structure names
- schema descriptions
- object identifiers
- byte-order markers
- version records

Multi-byte scalar values are always little-endian.

Variable-length data stores a `std::uint64_t` length before its contents.

---

## Supported Field Types

The initial supported field types are:

- `bool`
- fixed-width signed and unsigned integers
- `float`
- `double`
- `std::u8string`
- `std::vector<std::byte>`
- `std::array<T, N>`
- `std::vector<T>`
- `std::map<K, V>`

Environment-dependent integer types such as `int`, `long`, and `std::size_t` are not serialized directly.

---

## PHP Schema DSL

The PHP generator uses short type tokens to keep schemas concise.

Examples:

```php
'id' => u32,
'name' => s,
'payload' => bin,
'values' => [f64, v],
'fixed' => [u32, [a, 16]],
'scores' => [[s, f64], m],
```

The important container forms are:

```php
[T, v]          // std::vector<T>
[T, [a, N]]     // std::array<T, N>
[[K, V], m]     // std::map<K, V>
```

This DSL is intentionally small. More containers should be added only when there is a clear need.

---

## Versioning Policy

The low-level format is fixed-schema and not self-describing.

When a structure changes, compatibility must be handled above the low-level archive layer. Recommended techniques include:

- a version field at the beginning of a structure
- an outer envelope structure containing version and payload
- separate generated structures for old and new layouts
- carefully restricted trailing-field additions
- out-of-band schema timestamp checks for diagnostics

The generated-at timestamp identifies the schema source used to produce a C++ header. It is not automatically written to serialized payloads.

---

## Examples and Documentation

Serialization examples are mandatory because the intended usage involves both generated and hand-written pieces.

At minimum, examples should cover:

- hand-written `xfer` use
- PHP schema use
- generated header use
- binary output and binary input through the same `xfer` function

The reference manual for `<xer/serialize.h>` should describe this workflow directly rather than only listing low-level functions.

When the generator is changed, at least one generated-code example should be regenerated and compiled so that the documented schema DSL remains executable.
