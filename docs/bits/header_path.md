# `<xer/path.h>`

## Purpose

`<xer/path.h>` provides XER's lexical path type and related path utilities.

Its role is not to wrap `std::filesystem::path`.
Instead, XER provides its own UTF-8-based path model designed around the following ideas:

- keep the internal representation simple and consistent
- treat path handling primarily as a lexical operation
- preserve important Windows-specific path distinctions
- separate lexical path handling from actual filesystem-dependent resolution

This header is therefore the public entry point for path representation and path-oriented utility functions in XER.

---

## Main Role

The main role of `<xer/path.h>` is to provide:

- a public UTF-8 path type
- lexical path composition and decomposition
- path classification helpers such as absolute/relative checks
- conversion to and from native platform path representations

This makes the header the foundation for path handling in the rest of the library, especially for file-entry and stream-related APIs.

---

## Main Entities

At minimum, `<xer/path.h>` provides the following entities:

```cpp
using native_path_char_t;
using native_path_string;
using native_path_view;

class path;

auto operator/(path lhs, const path& rhs) -> xer::result<path>;
auto basename(const path& value) noexcept -> std::u8string_view;
auto extension(const path& value) noexcept -> std::u8string_view;
auto stem(const path& value) noexcept -> std::u8string_view;
auto parent_path(const path& value) -> xer::result<path>;
auto is_absolute(const path& value) noexcept -> bool;
auto is_relative(const path& value) noexcept -> bool;

auto to_native_path(const path& value) -> std::expected<native_path_string, error<void>>;
auto from_native_path(native_path_view value) -> std::expected<path, error<void>>;
auto from_native_path(const native_path_char_t* value) -> std::expected<path, error<void>>;
```

The exact overload set may expand, but this is the core public shape.

---

## `path`

`path` is the central type of the header.

It represents a path in XER's own lexical UTF-8 model.

### Basic Shape

At minimum, the class has a shape like the following:

```cpp
class path {
public:
    using string_type = std::u8string;
    using view_type = std::u8string_view;

    path();
    explicit path(std::u8string_view value);
    explicit path(const char8_t* value);

    auto str() const noexcept -> view_type;
    auto operator/=(const path& rhs) -> xer::result<void>;
};
```

### Role

The role of `path` is intentionally narrow.

It exists primarily to:

* store the internal normalized path representation
* preserve path invariants
* support fundamental path composition

More elaborate path operations are generally provided as free functions rather than as member functions.

---

## Internal Representation

`path` uses a UTF-8 internal representation.

### Basic Rules

* internally, a path stores `std::u8string`
* the internal separator is always `/`
* input `\` is normalized to `/`
* the lexical meaning of leading components is preserved

### Why This Matters

This design keeps internal handling simple while still preserving important distinctions, especially on Windows.

For example, the following forms must remain distinguishable:

* `C:foo`
* `C:/foo`
* `/foo`
* `//server/share/foo`

This is one of the central design requirements of the path model.

---

## `str()`

```cpp
auto str() const noexcept -> std::u8string_view;
```

### Purpose

`str()` returns the internal normalized representation of the path.

### Notes

* the returned form is lexical and normalized
* separators are returned as `/`
* it is not a display-oriented native path string

This distinction is important.
`str()` returns the XER path representation, not a platform display form.

---

## Path Composition

Path composition is supported through both member and free-function forms.

### `operator/=`

```cpp
auto operator/=(const path& rhs) -> xer::result<void>;
```

This is the fundamental mutating composition operation.

### `operator/`

```cpp
auto operator/(path lhs, const path& rhs) -> xer::result<path>;
```

This is the non-mutating composition form.

### Design Direction

The general design is:

* `operator/=` is the fundamental mutating operation
* `operator/` may be implemented in terms of `operator/=`

This keeps the basic semantics centralized.

---

## Lexical Path Operations

Most path utilities in XER are free functions.

At minimum, these include:

* `basename`
* `extension`
* `stem`
* `parent_path`
* `is_absolute`
* `is_relative`

### Why Free Functions

XER prefers free functions for operations that do not need direct mutation or privileged internal control.

This keeps the responsibility of `path` itself small and focused.

---

## `basename`

```cpp
auto basename(const path& value) noexcept -> std::u8string_view;
```

### Purpose

`basename` returns the trailing lexical path component.

### Design Direction

Its behavior is intentionally close to PHP's `basename`, while still following XER's own path model.

### Important Notes

* it accepts `path`, not arbitrary raw strings
* it is locale-independent
* it treats only `/` as the separator
* any `\` has already been normalized by `path`

---

## `extension`

```cpp
auto extension(const path& value) noexcept -> std::u8string_view;
```

### Purpose

`extension` returns the extension-like suffix of the basename.

### XER Rule

In XER, `extension` returns the part of `basename(path)` beginning with the **first** `.`.

This means, for example:

* `c.txt` -> `.txt`
* `archive.tar.gz` -> `.tar.gz`
* `.foo` -> `.foo`

This is a deliberate rule and should not be assumed to match other libraries automatically.

### Optional Start Position

The design may also allow a start-position argument relative to the basename when needed.

---

## `stem`

```cpp
auto stem(const path& value) noexcept -> std::u8string_view;
```

### Purpose

`stem` returns the leading part of the basename after removing the extension as defined by XER.

### Examples

* `c.txt` -> `c`
* `archive.tar.gz` -> `archive`
* `.foo` -> empty string
* `foo.` -> `foo`

Its exact behavior should therefore always be interpreted together with XER's `extension` rule.

---

## `parent_path`

```cpp
auto parent_path(const path& value) -> xer::result<path>;
```

### Purpose

`parent_path` returns the lexical parent path.

### Important Property

This is a **purely lexical** operation.

It does not:

* consult the real filesystem
* resolve symbolic links
* normalize against actual filesystem state

### Behavior

The operation removes one trailing component while preserving the meaning of the leading part.

If no further lexical parent exists, it returns failure.

This behavior is important because it clearly separates lexical reasoning from filesystem-dependent reasoning.

---

## Absolute and Relative Classification

At minimum, `<xer/path.h>` provides:

```cpp
auto is_absolute(const path& value) noexcept -> bool;
auto is_relative(const path& value) noexcept -> bool;
```

### Purpose

These functions classify the path according to XER's path rules.

### Windows-Specific Importance

On Windows, the distinction is not reduced to a simple leading-separator check.

For example:

* `X:foo` is not absolute
* `X:/foo` is absolute
* `/foo` is absolute
* `//server/share/foo` is absolute

This is one of the reasons XER uses its own path model instead of simply delegating public semantics to another library.

---

## Native Path Conversion

`<xer/path.h>` also provides conversion between XER paths and native platform path representations.

At minimum, this includes:

```cpp
auto to_native_path(const path& value) -> std::expected<native_path_string, error<void>>;
auto from_native_path(native_path_view value) -> std::expected<path, error<void>>;
auto from_native_path(const native_path_char_t* value) -> std::expected<path, error<void>>;
```

### Purpose

These functions exist so that XER's internal UTF-8 lexical model can interoperate with platform-native path strings.

### Role of the Native Types

* `native_path_char_t` represents the platform-native path character type
* `native_path_string` represents the owned native path string type
* `native_path_view` represents a view form of the native path type

This keeps platform-specific details localized to the conversion boundary.

### Error Handling

Conversion failures are reported explicitly.

This is especially important when conversion involves:

* UTF-8 validation
* UTF-16 conversion
* platform-native character constraints

---

## Lexical Model vs Actual Filesystem

A central design principle of `<xer/path.h>` is that path handling is primarily lexical.

### Lexical Operations

The following belong naturally in the lexical path layer:

* joining paths
* extracting basename
* extracting extension
* extracting stem
* computing lexical parent paths
* classifying absolute vs relative form

### Filesystem-Dependent Operations

The following are conceptually separate and should not be confused with lexical path handling:

* resolving relative paths against current working context
* converting to actual absolute paths
* resolving symbolic links
* normalization based on real filesystem state

This separation is deliberate and important.

---

## Relationship to Other Headers

`<xer/path.h>` should be understood together with:

* `policy_project_outline.md`
* `policy_path.md`
* `header_stdio.md`

The rough boundary is:

* `<xer/path.h>` handles path representation and lexical path operations
* `<xer/stdio.h>` handles stream and file-entry operations that make use of paths

This makes `<xer/path.h>` a foundational header for path-aware APIs elsewhere in the library.

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that XER uses its own UTF-8 lexical path type
* that separators are normalized to `/`
* that Windows lexical distinctions are preserved
* that most path utilities are free functions
* that native path conversion is explicit and fallible

Detailed joining rules and edge cases belong in the detailed reference or generated API documentation.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* constructing a `path` from UTF-8 text
* joining two paths with `/`
* retrieving `basename`, `extension`, or `stem`
* retrieving `parent_path`
* converting to and from native paths

These are good candidates for executable examples in `examples/`.

---

## Example

```cpp
#include <xer/path.h>

auto main() -> int
{
    const xer::path base(u8"/usr");
    const xer::path child(u8"local");

    const auto joined = base / child;
    if (!joined.has_value()) {
        return 1;
    }

    if (joined->str() != u8"/usr/local") {
        return 1;
    }

    return 0;
}
```

This example shows the normal XER style:

* construct paths from UTF-8 text
* use lexical path composition
* check `xer::result` explicitly for fallible operations

---

## See Also

* `policy_project_outline.md`
* `policy_path.md`
* `header_stdio.md`
