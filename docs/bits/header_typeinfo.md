# `<xer/typeinfo.h>`

## Purpose

`<xer/typeinfo.h>` provides lightweight type information helpers for XER.

Its main purpose is to make type names usable in diagnostics, tracing, and other development-oriented output.
The header wraps standard C++ type information in a form that fits XER's UTF-8-oriented text model.

---

## Main Entities

At minimum, `<xer/typeinfo.h>` provides the following entities:

```cpp
class xer::type_info;

#define xer_typeid(...)
```

The exact implementation may use `std::type_index` internally so that `xer::type_info` can be compared and used in ordered containers.

---

## `xer::type_info`

`xer::type_info` is a lightweight wrapper around C++ runtime type information.

It provides at least the following operations:

```cpp
auto raw_name() const noexcept -> const char*;
auto name() const -> std::u8string;
auto index() const noexcept -> std::type_index;
auto operator==(const type_info& rhs) const noexcept -> bool;
auto operator!=(const type_info& rhs) const noexcept -> bool;
auto operator<(const type_info& rhs) const noexcept -> bool;
```

### `name()`

`name()` returns a UTF-8 type name intended for human-readable diagnostics.

On GCC, the implementation may demangle the implementation-provided type name.
If demangling fails, the raw implementation-provided name may be returned instead.

The returned name is intended for display and diagnostics, not for stable serialization or ABI-independent comparison.

---

## `xer_typeid`

`xer_typeid(...)` creates a `xer::type_info` object from the same kind of operand accepted by `typeid`.

It is a variadic macro so that template type operands containing commas can be passed naturally.

Examples:

```cpp
const auto a = xer_typeid(int);
const auto b = xer_typeid(std::pair<int, long>);
const auto c = xer_typeid(value);
```

---

## Design Role

This header is mainly a foundation for diagnostics and tracing.

In particular, it allows code to display:

- the type of a traced object
- an implementation-provided or demangled type name
- an ordered type key for lookup tables

---

## Relationship to Other Headers

`<xer/typeinfo.h>` is related to future diagnostic facilities such as tracing.

It is also useful together with formatted output facilities from `<xer/stdio.h>`, especially when type names are printed as UTF-8 text.

---

## Documentation Notes

Type names are inherently implementation-dependent.
Documentation should therefore describe `name()` as a diagnostic display facility rather than as a stable programmatic identifier.

---

## Example

```cpp
#include <xer/stdio.h>
#include <xer/typeinfo.h>

#include <utility>

auto main() -> int
{
    const auto info = xer_typeid(std::pair<int, long>);

    if (!xer::puts(info.name()).has_value()) {
        return 1;
    }

    return 0;
}
```

---

## See Also

* `header_stdio.md`
