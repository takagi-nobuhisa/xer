# `<xer/error.h>`

## Purpose

`<xer/error.h>` provides the core error and result facilities used throughout XER.

In XER, ordinary failure is represented explicitly rather than by exceptions or special sentinel values wherever practical.
This header defines the common vocabulary used for that purpose.

---

## Main Entities

At minimum, `<xer/error.h>` provides the following entities:

```cpp
enum class xer::error_t : std::int32_t;

template <class Detail = void>
struct xer::error;

template <class T, class Detail = void>
using xer::result = std::expected<T, error<Detail>>;

constexpr auto xer::make_error(
    error_t code,
    std::source_location location = std::source_location::current()) noexcept
    -> error<void>;

template <class Detail, class T>
constexpr auto xer::make_error(
    error_t code,
    T&& value,
    std::source_location location = std::source_location::current()) noexcept
    -> error<Detail>;
```

The exact set of enumerators in `error_t` may grow as the library evolves, but the overall design is stable.

---

## Design Role

This header defines the basic error-reporting model for XER.

Its role is to make the following possible:

* report normal failure explicitly
* keep ordinary public APIs easy to understand
* attach optional structured detail when needed
* preserve source-location information for diagnostics

In other words, `<xer/error.h>` is the foundation for XER-style failure handling.

---

## `xer::result`

`xer::result<T, Detail>` is the standard result type in XER.

For the common case:

```cpp
xer::result<T>
```

means:

```cpp
std::expected<T, xer::error<void>>
```

When extra detail is needed, a second template argument may be supplied:

```cpp
xer::result<T, Detail>
```

which means:

```cpp
std::expected<T, xer::error<Detail>>
```

### Basic Policy

As a rule, fallible public APIs in XER return `xer::result`.

This keeps normal success and normal failure explicit in the type system.

### Typical Pattern

```cpp
const auto result = some_operation();
if (!result.has_value()) {
    return result.error();
}

const auto& value = *result;
```

The exact style may vary, but explicit checking is the normal expectation.

---

## `xer::error<void>`

`xer::error<void>` is the common error type used when no additional payload is needed.

It stores at least the following information:

* the error code
* the source location at which the error was created

This makes it lightweight while still preserving useful diagnostic context.

### Purpose

`error<void>` is suitable when:

* only the error category matters
* no extra structured data is necessary
* the caller mainly needs to distinguish success from failure

---

## `xer::error<Detail>`

`xer::error<Detail>` is used when an error needs additional structured information.

The exact representation depends on the design of `Detail`, but the intent is:

* preserve the common error code and source location
* carry extra information specific to that failure

This is useful for cases such as:

* attaching a position
* attaching a problematic value
* attaching structured context for later reporting

### Design Direction

If `Detail` is a class type, `error<Detail>` may expose that detail naturally through inheritance or an equivalent mechanism.
If `Detail` is not a class type, it may be stored as a member.

The important point is not the internal representation itself, but that the extra detail remains explicit and type-safe.

---

## `xer::error_t`

`xer::error_t` is the common error-code enumeration used across XER.

### Basic Direction

Its design is guided primarily by the following ideas:

* preserve practical compatibility with `errno`-style categories where useful
* allow XER-specific error categories where necessary
* avoid using a success enumerator inside the error type itself

### General Interpretation

* positive values correspond, where practical, to target-environment `errno`-style meanings
* negative values are reserved for XER-specific categories

Examples of XER-specific categories may include things such as:

* `logic_error`
* `invalid_argument`
* `io_error`
* `encoding_error`
* `not_found`
* `divide_by_zero`

The exact enumerator set is defined by the implementation.

---

## `xer::make_error`

`make_error` is the standard helper for constructing XER error objects.

### Why `make_error` Exists

The helper exists so that:

* callers do not need to repeat `std::source_location::current()`
* error construction stays uniform across the library
* code remains concise and readable

### Basic Forms

Without extra detail:

```cpp
const auto error = xer::make_error(xer::error_t::invalid_argument);
```

With extra detail:

```cpp
const auto error = xer::make_error<my_detail>(
    xer::error_t::invalid_argument,
    my_detail{/* ... */}
);
```

### Source Location

The source location is captured at the call site through the default argument of `make_error`.

This is important because it records where the error was created, not merely where the type was defined.

---

## Relationship to Other Policies

`<xer/error.h>` is closely related to the following project-wide policies:

* normal failure is represented by `xer::result`
* ordinary public APIs generally take ordinary values rather than `xer::result` arguments
* exceptions are reserved for cases where they are appropriate by design, such as assertion failures or fundamentally exceptional situations

Accordingly, this header should be understood together with:

* `policy_project_outline.md`
* `policy_result_arguments.md`

---

## Documentation Notes

When documenting a fallible API that uses `xer::result`, it is usually enough to describe:

* the success value
* the general failure condition
* important error categories when they matter

It is not always necessary to enumerate every possible `error_t` value unless that distinction is important to users.

---

## Example

```cpp
#include <xer/error.h>

auto parse_positive(int value) -> xer::result<int>
{
    if (value <= 0) {
        return std::unexpected(
            xer::make_error(xer::error_t::invalid_argument));
    }

    return value;
}
```

This example shows the normal XER pattern:

* return a success value directly on success
* return `std::unexpected(xer::make_error(...))` on failure

---

## See Also

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `header_assert.md`
