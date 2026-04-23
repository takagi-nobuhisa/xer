# `<xer/time.h>`

## Purpose

`<xer/time.h>` provides time-related facilities in XER.

Its purpose is not to reproduce the C standard library `time.h` as it is, nor to center the public API on `std::chrono`.
Instead, it provides a simpler time library that preserves the approachability of C-style time handling while aligning with XER's own design.

This header is intended for tasks such as:

- retrieving the current time
- converting between scalar time values and broken-down time
- formatting time values as text
- performing simple time-difference calculations

---

## Main Role

The main role of `<xer/time.h>` is to provide a practical and explicit time model based on the following ideas:

- `time_t` is simple and arithmetic-friendly
- ordinary failure is reported through `xer::result`
- broken-down time is represented by a `tm` structure
- sub-second precision is supported explicitly
- formatting is UTF-8-oriented

This makes the header suitable for code that wants simple time handling without the heavier expression style often associated with `std::chrono`.

---

## Main Entities

At minimum, `<xer/time.h>` provides the following entities:

```cpp
using time_t = double;
using clock_t = std::clock_t;

struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
    int tm_microsec;
};

auto time() -> xer::result<time_t>;
auto clock() -> xer::result<clock_t>;
auto difftime(time_t left, time_t right) noexcept -> double;

auto gmtime(time_t value) -> xer::result<tm>;
auto localtime(time_t value) -> xer::result<tm>;
auto mktime(const tm& value) -> xer::result<time_t>;

auto ctime(time_t value) -> std::u8string;
auto ctime(const tm& value) -> std::u8string;
auto strftime(std::u8string_view format, const tm& value) -> xer::result<std::u8string>;
```

The exact helper functions may expand in the future, but this is the current core public shape.

---

## `xer::time_t`

`xer::time_t` is the central scalar time type in XER.

### Basic Shape

```cpp
using time_t = double;
```

### Meaning

The unit of `xer::time_t` is seconds.

Its interpretation is:

* integer part: whole seconds
* fractional part: fractions of a second

### Why `double` Is Used

This design keeps time values:

* easy to manipulate arithmetically
* lightweight
* suitable for microsecond-level practical precision
* simpler to handle in ordinary code than heavier duration-style abstractions

### Precision Direction

The practical target is microsecond-level handling, but the public design does not promise a strict fixed-point representation or guaranteed nanosecond precision.

---

## Epoch

XER fixes the epoch of `xer::time_t` to the POSIX epoch.

### Meaning

```text
1970-01-01 00:00:00 UTC
```

corresponds to:

```text
0.0
```

as a `xer::time_t` value.

### Why This Matters

This avoids the implementation-defined ambiguity of traditional C `time_t` epoch interpretation and gives XER a stable project-wide rule.

---

## Supported Range

At least in the initial design, times before the epoch are unsupported.

### Meaning

* negative `time_t` values are unsupported
* pre-epoch broken-down times are unsupported
* such inputs result in failure

This is a deliberate simplification in the initial stage.

---

## `xer::tm`

`xer::tm` is XER's broken-down time structure.

### Basic Shape

```cpp
struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
    int tm_microsec;
};
```

### Relationship to C's `struct tm`

This structure is based on the C `struct tm`, but extends it by adding:

```cpp
tm_microsec
```

### `tm_microsec`

`tm_microsec` stores the fractional microseconds part of a second.

Its intended range is:

```text
0 .. 999999
```

### Why This Matters

This makes sub-second precision explicit without abandoning the familiar broken-down-time model.

---

## `time()`

```cpp
auto time() -> xer::result<time_t>;
```

### Purpose

`time()` returns the current calendar time as a `xer::time_t`.

### Design Direction

Although failure is uncommon in practice, XER still treats it as an ordinary fallible operation and reports failure through `xer::result`.

This keeps the header consistent with the rest of the library.

---

## `clock()`

```cpp
auto clock() -> xer::result<clock_t>;
```

### Purpose

`clock()` returns processor-time or clock-related information in the style of the underlying C facility.

### Design Direction

As with `time()`, XER treats this as an ordinary fallible operation and reports failure explicitly.

---

## `difftime`

```cpp
auto difftime(time_t left, time_t right) noexcept -> double;
```

### Purpose

`difftime` returns the difference between two time values.

### Meaning

The result is a scalar difference in seconds.

### Notes

This is one of the simpler parts of the header because its role is arithmetic rather than structural conversion.

---

## `gmtime`

```cpp
auto gmtime(time_t value) -> xer::result<tm>;
```

### Purpose

`gmtime` converts a scalar time value into broken-down UTC time.

### Behavior

* the fractional part is reflected in `tm_microsec`
* negative values are unsupported and result in failure

### Role

This is the main scalar-to-UTC broken-down conversion entry point.

---

## `localtime`

```cpp
auto localtime(time_t value) -> xer::result<tm>;
```

### Purpose

`localtime` converts a scalar time value into broken-down local time.

### Behavior

* the fractional part is reflected in `tm_microsec`
* negative values are unsupported and result in failure
* local conversion failure is reported explicitly

### Role

This is the local-time counterpart of `gmtime`.

---

## `mktime`

```cpp
auto mktime(const tm& value) -> xer::result<time_t>;
```

### Purpose

`mktime` converts a broken-down time value into scalar time.

### Behavior

* `tm_microsec` contributes to the fractional part
* out-of-range `tm_microsec` is an error
* pre-epoch results are errors

### Role

This is the reverse conversion entry point from `tm` back to `time_t`.

---

## `ctime`

XER provides `ctime` in two overloads:

```cpp
auto ctime(time_t value) -> std::u8string;
auto ctime(const tm& value) -> std::u8string;
```

### Purpose

`ctime` converts time values into human-readable UTF-8 text.

### Design Direction

In XER, the roles traditionally associated with C's `ctime` and `asctime` are unified under the `ctime` name.

That means:

* `ctime(time_t)` formats a scalar time value
* `ctime(const tm&)` formats a broken-down time value

### Important Notes

* the return type is `std::u8string`
* no static internal buffer is used
* the broken-down-time overload takes `const tm&`, not a raw pointer

This reflects XER's C++-style redesign while keeping a familiar function name.

---

## `strftime`

```cpp
auto strftime(std::u8string_view format, const tm& value) -> xer::result<std::u8string>;
```

### Purpose

`strftime` formats a broken-down time value according to a format string.

### Format String Model

The format string is UTF-8-oriented and uses:

```cpp
std::u8string_view
```

This means it may contain:

* ASCII format specifiers
* fixed UTF-8 text

### Example

A format such as:

```text
%Y年%m月%d日 %H時%M分%S秒
```

is intended to be acceptable.

### Return Model

The result is returned as:

```cpp
std::u8string
```

through `xer::result`.

### Initial Design Limits

At least in the initial stage:

* `%f` is not yet supported
* advanced locale behavior is not the priority
* advanced timezone extensions are deferred

---

## UTF-8-Oriented Formatting

One important characteristic of `<xer/time.h>` is that formatting is designed around UTF-8 text.

### Why This Matters

This keeps the header aligned with XER's broader public text model:

* public text APIs are UTF-8 oriented
* `std::u8string` is the standard owned text type
* `std::u8string_view` is the standard non-owning text input type

This distinguishes XER's design from more locale-centered or narrow-character-only interpretations.

---

## Error Handling

`<xer/time.h>` follows XER's ordinary failure model.

### Meaning

Operations that can fail report failure through `xer::result`.

Typical cases include:

* retrieving time information
* converting between scalar and broken-down time
* invalid broken-down-time input
* unsupported pre-epoch input
* invalid format handling where applicable

### Typical Error Categories

The detailed error taxonomy belongs to the implementation, but the intended design especially relates to categories such as:

* `runtime_error`
* `invalid_argument`

---

## Deferred Features

At least in the initial stage, the following are intentionally deferred or simplified:

* pre-epoch support
* `%f` formatting in `strftime`
* advanced locale control
* advanced timezone features
* C-style static-buffer behavior
* separate exposure of `asctime`

This is an intentional simplification, not an omission by accident.

---

## Relationship to Other Headers

`<xer/time.h>` should be understood together with:

* `policy_project_outline.md`
* `policy_time.md`
* `header_error.md`

The rough boundary is:

* `<xer/time.h>` handles time retrieval, time conversion, and time formatting
* `<xer/error.h>` provides the error/result model used by fallible time operations

This header is largely self-contained in its domain, but it depends directly on XER's ordinary error model.

---

## Relationship to XER's General Design

`<xer/time.h>` reflects several important project-wide design decisions:

* prefer explicit failure reporting
* keep APIs approachable for users familiar with C
* avoid unnecessary dependence on locale
* use UTF-8 for public text output
* avoid centering the public design on `std::chrono`

This makes the header one of the clearest examples of XER's general philosophy.

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that `xer::time_t` is `double`-based and measured in seconds
* that the epoch is fixed to the POSIX epoch
* that `xer::tm` extends the familiar broken-down time structure with `tm_microsec`
* that `ctime` returns `std::u8string`
* that `strftime` uses UTF-8 format strings and returns `xer::result<std::u8string>`

Detailed formatting rules and conversion edge cases belong in the detailed reference or generated API sections.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* retrieving the current time with `time()`
* converting a scalar time value with `gmtime` or `localtime`
* converting back with `mktime`
* formatting with `ctime`
* formatting with `strftime`

These are good candidates for executable examples in `examples/`.

---

## Example

```cpp
#include <xer/time.h>

auto main() -> int
{
    const auto now = xer::time();
    if (!now.has_value()) {
        return 1;
    }

    const auto utc = xer::gmtime(*now);
    if (!utc.has_value()) {
        return 1;
    }

    const auto text = xer::strftime(u8"%Y-%m-%d %H:%M:%S", *utc);
    if (!text.has_value()) {
        return 1;
    }

    return 0;
}
```

This example shows the normal XER style:

* retrieve time explicitly
* convert explicitly
* format explicitly
* check `xer::result` at each fallible step

---

## See Also

* `policy_project_outline.md`
* `policy_time.md`
* `header_error.md`
