# XER Reference Manual

Target version: **v0.2.0a2**

---

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

---

# `<xer/assert.h>`

## Purpose

`<xer/assert.h>` provides XER's assertion facilities.

These facilities are used primarily by XER's own test programs, but they may also be exposed to library users for lightweight checks.

Unlike the standard C `assert`, XER assertions do not terminate the process immediately.
Instead, they report failure by throwing an exception.

---

## Main Entities

At minimum, `<xer/assert.h>` provides the following entities:

```cpp
xer_assert(expr)
xer_assert_not(expr)
xer_assert_eq(lhs, rhs)
xer_assert_ne(lhs, rhs)
xer_assert_lt(lhs, rhs)
xer_assert_throw(expr, exception_type)
xer_assert_nothrow(expr)

class xer::assertion_error;
```

The exact implementation details may vary, but these names form the core assertion interface.

---

## Design Role

This header exists to support explicit and readable checks in execution tests.

Its role is to make the following possible:

* verify expected conditions in test code
* stop the current test flow immediately on failure
* preserve diagnostic information such as source location
* avoid process termination as the default assertion behavior

This makes the assertion facilities suitable for automated test execution and for debugging library behavior.

---

## Difference from the Standard `assert`

The standard C and C++ `assert` facility typically aborts the process when a condition fails.

XER deliberately uses a different design.

### Standard `assert`

* reports failure by terminating the process
* is mainly intended for debugging internal assumptions
* is not well suited to a test framework that wants structured failure reporting

### XER Assertions

* report failure by throwing `xer::assertion_error`
* are suitable for execution tests
* preserve source-level diagnostic context
* can also be used in lightweight user-side checks when appropriate

This difference is intentional.

---

## `xer::assertion_error`

`xer::assertion_error` is the exception type thrown when an XER assertion fails.

### Purpose

Its purpose is to carry assertion-failure information in a form that can be caught and reported by test runners or surrounding code.

### Expected Contents

At minimum, diagnostics for an assertion failure should make it possible to identify:

* which assertion failed
* where it failed
* what expression or comparison was involved

Depending on the assertion macro, additional value-oriented information may also be included.

---

## Assertion Macros

## `xer_assert`

```cpp
xer_assert(expr)
```

This macro checks that `expr` is true.

If `expr` is false, it throws `xer::assertion_error`.

### Typical Use

```cpp
xer_assert(result.has_value());
```

---

## `xer_assert_not`

```cpp
xer_assert_not(expr)
```

This macro checks that `expr` is false.

If `expr` is true, it throws `xer::assertion_error`.

### Typical Use

```cpp
xer_assert_not(buffer.empty());
```

---

## `xer_assert_eq`

```cpp
xer_assert_eq(lhs, rhs)
```

This macro checks that `lhs == rhs`.

If the comparison is false, it throws `xer::assertion_error`.

### Purpose

This macro is used when equality of two values is the important condition being tested.

### Typical Use

```cpp
xer_assert_eq(value, 42);
```

---

## `xer_assert_ne`

```cpp
xer_assert_ne(lhs, rhs)
```

This macro checks that `lhs != rhs`.

If the comparison is false, it throws `xer::assertion_error`.

### Typical Use

```cpp
xer_assert_ne(ptr, nullptr);
```

---

## `xer_assert_lt`

```cpp
xer_assert_lt(lhs, rhs)
```

This macro checks that `lhs < rhs`.

If the comparison is false, it throws `xer::assertion_error`.

### Purpose

This is the currently defined ordering-oriented assertion in the basic policy.

### Typical Use

```cpp
xer_assert_lt(index, size);
```

---

## `xer_assert_throw`

```cpp
xer_assert_throw(expr, exception_type)
```

This macro checks that evaluating `expr` throws the specified exception type.

If `expr` does not throw that exception type, it throws `xer::assertion_error`.

### Argument Order

The argument order is intentional:

* first: the expression to evaluate
* second: the exception type expected

This order follows the project testing policy.

### Typical Use

```cpp
xer_assert_throw(f(), std::runtime_error);
```

---

## `xer_assert_nothrow`

```cpp
xer_assert_nothrow(expr)
```

This macro checks that evaluating `expr` does not throw.

If `expr` throws, it throws `xer::assertion_error`.

### Typical Use

```cpp
xer_assert_nothrow(run_test_case());
```

---

## Diagnostic Policy

Assertion failures should provide diagnostics that are useful during development.

At minimum, they should make it possible to identify:

* the source file
* the line
* the assertion form
* the compared or checked expression text

For value-comparison assertions such as `xer_assert_eq`, it is also desirable to include the observed left-hand and right-hand values when practical.

However, the assertion facilities are not intended to guarantee perfect formatting for every possible type.

This point is important:

* they are primarily for XER development and lightweight testing
* they should remain practical and readable
* they should not accumulate excessive special handling for every conceivable output case

---

## Intended Scope

These assertion macros are primarily intended for:

* XER execution tests
* small utility checks during development
* lightweight user-side verification when convenient

They are not intended to replace a full-featured external test framework in every scenario.

---

## Relationship to Other Policies

`<xer/assert.h>` should be understood together with the following documents:

* `policy_project_outline.md`
* `policy_testing_and_php.md`

The project outline explains why assertion failure is treated separately from ordinary runtime failure.
The testing policy explains the role of XER assertions in execution tests.

---

## Documentation Notes

When this header is referenced from a generated manual, it is usually enough to explain:

* that XER assertions throw instead of aborting
* the available macro names
* the intended role of these macros in tests and lightweight verification

The full operational philosophy belongs in the policy documents rather than in per-header API summaries.

---

## Example

```cpp
#include <xer/assert.h>

auto main() -> int
{
    xer_assert_eq(1 + 1, 2);
    xer_assert_not(false);
    xer_assert_nothrow(static_cast<void>(0));
    return 0;
}
```

This example illustrates the general style:

* use explicit assertion macros
* let failures throw `xer::assertion_error`
* keep checks readable and localized

---

## See Also

* `policy_project_outline.md`
* `policy_testing_and_php.md`
* `header_error.md`

---

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

---

# Purpose

`<xer/diag.h>` provides lightweight diagnostic facilities for XER.

It groups tracing and logging support under one public diagnostic header while keeping the shared category and level vocabulary common to both facilities.

---

# Main Entities

At minimum, `<xer/diag.h>` provides the following entities:

```cpp
using xer::diag_level_t = int;

inline constexpr xer::diag_level_t xer::diag_error;
inline constexpr xer::diag_level_t xer::diag_warning;
inline constexpr xer::diag_level_t xer::diag_info;
inline constexpr xer::diag_level_t xer::diag_debug;
inline constexpr xer::diag_level_t xer::diag_verbose;

enum class xer::diag_category : std::uint32_t;

xer_trace(category, level, object)
xer_log(category, level, message)
xer_log(category, level, format, ...)
```

It also provides functions for setting trace and log output streams and levels.

---

# Design Role

This header is intended for diagnostics, development-time tracing, and simple runtime logging.

Tracing and logging share:

* `diag_category`
* `diag_level_t`
* the named level constants

Their output destinations and current levels are configured independently.

---

# Trace

`xer_trace(category, level, object)` prints one diagnostic line containing:

* the diagnostic category
* the diagnostic level
* the source expression text
* the statically derived type name
* the value formatted through XER's `%@` printf conversion

The output form is conceptually:

```text
[category][level] expression (type) = value
```

When `NDEBUG` is defined, `xer_trace` expands to a no-op expression and does not evaluate its arguments.

---

# Log

`xer_log(category, level, message)` writes one simple log record.

`xer_log(category, level, format, ...)` writes one formatted log record. The message body uses XER printf formatting rules, including `%@`.

Unlike `xer_trace`, logging is not disabled merely because `NDEBUG` is defined. It can be disabled at compile time by defining `XER_ENABLE_LOG` to `0` before including `<xer/diag.h>`.

## Log Record Format

Each log call writes one CSV data record.

The columns are fixed as follows:

```text
timestamp,category,level,message
```

No header row is written automatically.

The timestamp uses local time and has millisecond precision:

```text
YYYY-MM-DD HH:MM:SS.mmm
```

A typical log record is:

```csv
2026-04-26 18:42:15.123,io,30,"opened sample.txt"
```

The first three fields are generated by XER and are written without CSV quoting.
The message field is always written as a quoted CSV field. Double quote characters in the message are escaped by doubling them.

## Log Cost Policy

Logging is designed to remain reasonably lightweight.

* disabled log messages are filtered before formatting arguments are evaluated
* simple message logging does not use `sprintf`
* formatted message logging uses `sprintf` only after the level check succeeds
* the timestamp prefix up to whole seconds is cached per thread
* the level field is formatted without `printf`
* the message field is quoted directly to the stream without constructing a separate quoted string

---

# Output Streams

Trace and log output default to the standard error text stream.
They can be changed independently through the corresponding stream-setting functions.

---

# Notes

These facilities are intentionally small.
They are not a full logging framework, but they provide a useful diagnostic foundation for XER itself and for small programs using XER.

---

# `<xer/string.h>`

## Purpose

`<xer/string.h>` provides string-related utilities in XER.

This header brings together several kinds of functionality:

- C-style string operations
- UTF-aware character and substring search helpers
- PHP-inspired utility functions such as split/join and trim
- raw memory helpers grouped with string-oriented facilities
- prefix/suffix checks
- case conversion and dynamic string transformation
- error text helpers

The goal is not to reproduce the C standard library exactly.
Instead, this header reorganizes practical string-related functionality into a form that fits XER's overall design.

---

## Main Role

The main role of `<xer/string.h>` is to provide a practical collection of text and memory helpers centered on XER's UTF-8-oriented public string model.

In particular, it serves the following purposes:

- provide familiar C-style names where that improves approachability
- support UTF-8-oriented public APIs based on `char8_t`
- provide allocation-free helpers where practical
- provide higher-level utility functions that are convenient in ordinary code

This header therefore mixes low-level and high-level facilities more intentionally than the standard C library does.

---

## Main Function Groups

At a high level, `<xer/string.h>` contains the following groups of functionality:

- search and comparison
- case-insensitive search and comparison
- copy and concatenation
- split / join / trim
- raw memory helpers
- prefix/suffix checks
- case conversion and dynamic string transformation
- error text helpers

---

## Search and Comparison

At minimum, this header provides functions such as the following:

```cpp
strlen
strcmp
strncmp
strchr
strrchr
strstr
strrstr
strpos
strrpos
strpbrk
strspn
strcspn
```

### Role of This Group

These functions provide familiar string-search and comparison operations in a form adapted to XER's public string model.

Some are close in spirit to C standard-library functions, while others are influenced more by PHP-style utility naming.

### Notes

* not all functions are exact source-compatible reimplementations of their C or PHP namesakes
* accepted argument forms are designed according to XER's own API policy
* ordinary public APIs are documented in terms of ordinary values rather than `xer::result` arguments

---

## Case-Insensitive Search and Comparison

This header also provides case-insensitive operations such as the following:

```cpp
strcasecmp
strncasecmp
stricmp
strnicmp
strcasechr
strcaserchr
strichr
strirchr
strcasepos
strcaserpos
strcasestr
strcaserstr
stripos
strirpos
stristr
strirstr
```

### Role of This Group

These functions exist to support practical case-insensitive text handling without requiring users to build the operation manually every time.

### Notes

* these facilities are intentionally simple
* they are primarily oriented around ASCII and the normalization currently implemented by the library
* they should not be read as a promise of full locale-sensitive or Unicode-wide collation behavior

---

## Copy and Concatenation

At minimum, this header provides functions such as:

```cpp
strcpy
strncpy
strcat
strncat
```

### Role of This Group

These functions provide familiar copy and concatenation operations, but their exact behavior follows XER's design rather than attempting perfect C compatibility.

### Notes

* XER's design gives priority to natural and practical use in C++ code
* overloads may exist for arrays, pointers, and container-like targets
* when automatic capacity growth is appropriate for dynamic containers, the design may favor that behavior over stricter historical C behavior

---

## Split / Join / Trim

This header also provides higher-level helpers such as:

```cpp
explode
implode
ltrim
rtrim
trim
ltrim_view
rtrim_view
trim_view
```

### Role of This Group

This group provides convenient text-processing helpers inspired in part by PHP.

These functions are intended for cases where ordinary code benefits from a compact utility API rather than from manual loops and range manipulation.

### `*_view` Variants

The `ltrim_view`, `rtrim_view`, and `trim_view` family are especially important because they provide non-allocating trimming operations around UTF-8-oriented string views.

### Notes

* `trim_view`-style functions are intended to be lightweight and convenient
* these helpers are useful both in ordinary code and in executable examples
* code examples are expected to use these functions naturally with explicit `xer::result` checking where required

---

## Raw Memory Helpers

`<xer/string.h>` also groups raw memory helpers through the related memory facilities.

These include operations such as:

```cpp
memcpy
memmove
memchr
memrchr
memcmp
memset
```

### Role of This Group

Although raw memory operations are not text operations in the narrow sense, XER groups them together with string-oriented helpers for practical convenience.

This reflects the historical closeness of string and memory functions in C-style programming, while still keeping the public-header structure compact.

### Notes

* these functions are low-level helpers
* they are still part of the public surface when exposed through `<xer/string.h>`
* their semantics should be read according to XER's own design and test coverage, not assumed solely from the standard library

---

## Error Text Helpers

This header also provides helpers such as:

```cpp
strerror
get_error_name
get_errno_name
```

### Role of This Group

These helpers are intended to convert error categories into human-readable or symbolic forms.

They are useful in diagnostics, debugging output, and user-facing reporting when appropriate.

### Notes

* the exact mapping policy depends on XER's own error model
* `get_error_name` and `get_errno_name` are especially useful where symbolic names are preferable to free-form text

---

## UTF-Oriented Public String Model

`<xer/string.h>` should be understood in the context of XER's general text model.

### Basic Expectations

* public string APIs generally use `char8_t`
* owned strings generally use `std::u8string`
* non-owning text views generally use `std::u8string_view`
* individual Unicode scalar values are generally represented as `char32_t`

This is important because the header name may look similar to C's `<string.h>`, but the actual design direction is different.

### Character Search

UTF-aware character search functions may accept `char32_t` for searching UTF-8, UTF-16, or UTF-32 text.

This allows a single Unicode scalar value to be expressed clearly at the call site.

---

## Relationship to Other Policies

`<xer/string.h>` is closely related to the following policy documents:

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `policy_examples.md`

It also interacts conceptually with:

* `policy_ctype.md`
* `policy_encoding.md`

The exact boundary is as follows:

* `<xer/string.h>` handles string and memory utilities
* `<xer/ctype.h>` handles classification and character transformation
* encoding policy defines the broader text model used by the library

---

## Documentation Notes

When this header is used as part of generated documentation, it is usually enough to explain:

* that it combines C-style string utilities with XER-specific UTF-8-oriented helpers
* that it includes both low-level and higher-level practical utilities
* that trim/split/join helpers are an important user-facing part of the header
* that accepted argument forms follow XER's own API policy

Detailed per-function semantics should be described in the reference manual or generated API sections.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* trimming a UTF-8 string with `trim_view`
* splitting and joining text with `explode` / `implode`
* searching for a Unicode scalar value in UTF-8 text
* performing familiar C-style comparison or copying in XER style

This aligns well with the project direction that executable examples should become the canonical source for user-facing code snippets.

---

## Example

```cpp id="r4yb9n"
#include <xer/stdio.h>
#include <xer/string.h>

auto main() -> int
{
    constexpr std::u8string_view input = u8"  hello  ";

    const auto trimmed = xer::trim_view(input);
    if (!trimmed.has_value()) {
        return 1;
    }

    if (!xer::puts(*trimmed).has_value()) {
        return 1;
    }

    return 0;
}
```

This example shows a typical XER style:

* use UTF-8-oriented string input
* call an ordinary public API with an ordinary value
* check `xer::result` explicitly
* use `<xer/stdio.h>` for text output in examples

---

## See Also

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `policy_examples.md`
* `header_ctype.md`
* `header_stdlib.md`


---

## Prefix and Suffix Checks

`<xer/string.h>` provides prefix and suffix helpers:

```cpp
starts_with
ends_with
```

They accept string-like arguments and are intended to cover common UTF-8 string-view and literal use cases naturally.

---

## Case Conversion and Dynamic String Transformation

The header provides string-level transformation helpers:

```cpp
strtolower
strtoupper
strtoctrans
```

`strtolower` and `strtoupper` apply character conversion to a string and return a transformed string.

`strtoctrans` applies a `ctrans_id` transformation to each code point of a string.
It is the string-level counterpart of `toctrans`.

Current transformation support includes ASCII and Latin-1 case conversion as well as fullwidth/halfwidth transformations where implemented by `toctrans`.

---

# `<xer/ctype.h>`

## Purpose

`<xer/ctype.h>` provides character classification and character conversion facilities in XER.

This header covers two closely related areas:

- classification of characters into categories such as alphabetic, digit, space, and printable
- conversion of characters such as uppercase/lowercase conversion and related transformations

Its role is similar in spirit to C's `<ctype.h>` and `<wctype.h>`, but the design follows XER's own text model and API policy rather than reproducing the standard library structure exactly.

---

## Main Role

The main role of `<xer/ctype.h>` is to provide a simple and explicit character-handling model that fits the rest of XER.

In particular, it aims to provide:

- locale-independent basic behavior
- a clear distinction between ordinary ASCII-oriented operations and more extended operations
- a unified character argument type
- a dynamic mechanism for classification and conversion when fixed function names are not enough

This makes the header suitable both for straightforward checks and for cases where the caller wants to choose a classification or conversion kind dynamically.

---

## Basic Design Direction

### Locale Independence

The basic `is` functions and `to` functions are locale-independent.

Their behavior corresponds to the `"C"` locale rather than to environment-dependent locale rules.

This is important because XER's broader design tries to minimize dependence on locale.

### Character Type

The argument type for individual character classification and conversion functions is unified to `char32_t`.

This matches XER's general policy that individual Unicode scalar values are handled as `char32_t`.

### ASCII as the Basic Scope

The basic individual functions operate only on the ASCII range.

That means:

- classification functions return `false` for non-ASCII input
- conversion functions return failure for non-ASCII input

This is a deliberate design choice rather than a temporary limitation.

---

## Individual Classification Functions

At minimum, `<xer/ctype.h>` provides individual classification functions such as the following:

```cpp
isalpha
isdigit
isalnum
islower
isupper
isspace
isblank
iscntrl
isprint
isgraph
ispunct
isxdigit
isascii
isoctal
isbinary
```

### Role of These Functions

These functions provide direct and readable checks for common categories.

They are intended for the ordinary case where the caller already knows which category should be tested.

### Return Type

Individual classification functions return:

```cpp
bool
```

### Behavior for Non-ASCII Input

These functions classify only ASCII characters.

If a non-ASCII character is passed, they return `false`.

This keeps the meaning of each function simple and predictable.

---

## Individual Conversion Functions

At minimum, `<xer/ctype.h>` provides individual conversion functions such as:

```cpp
tolower
toupper
```

### Role of These Functions

These functions provide direct conversion for common character transformations.

They are intended for the ordinary case where the caller wants a fixed known transformation.

### Return Type

Individual conversion functions return:

```cpp
xer::result<char32_t>
```

### Behavior

These functions operate only on the ASCII range.

Their behavior is:

* if the input is a conversion target, return the converted character
* if the input is not a conversion target, return the original character as a successful result
* if the input is non-ASCII, return failure

### About `toascii`

A traditional C-family `toascii` is not adopted at present.

This is because `toascii` is often understood as a low-level bit-masking operation that extracts the lower 7 bits, and that does not fit naturally with XER's character-handling policy.

---

## Dynamic Character Classification

In addition to fixed `is` functions, XER provides a dynamic classifier:

```cpp
enum class ctype_id;
auto isctype(char32_t c, ctype_id id) noexcept -> bool;
```

### Role of `isctype`

`isctype` allows the classification kind to be chosen dynamically through a value of `ctype_id`.

This is useful when:

* the classification kind is determined at runtime
* a single function should support multiple categories
* code should not branch manually over many fixed `is...` functions

### Design Direction

`ctype_id` may contain both:

* ASCII-limited categories
* extended categories that also cover non-ASCII characters

This allows one unified API to cover both the basic and extended cases.

### Behavior

When an ASCII-limited category is specified, `isctype` behaves like the corresponding individual `is` function.

When an extended category is specified, it may classify non-ASCII characters as well.

This makes it possible, for example, to keep `isdigit` simple while still supporting richer classification through `isctype` when explicitly requested.

---

## Dynamic Character Conversion

XER also provides a dynamic conversion function:

```cpp
enum class ctrans_id;
auto toctrans(char32_t c, ctrans_id id) -> xer::result<char32_t>;
```

### Role of `toctrans`

`toctrans` allows the conversion kind to be selected dynamically through `ctrans_id`.

This is useful when:

* the desired transformation is chosen at runtime
* a single API should support multiple conversion kinds
* extended conversion categories should be available without creating a large number of fixed function names

### Behavior

When an ASCII-limited category is specified, `toctrans` behaves like the corresponding individual conversion function.

When an extended category is specified, it may perform non-ASCII conversion as well.

---

## Extended Conversion Areas

Extended `toctrans` categories may cover at least the following areas:

* fullwidth/halfwidth conversion for Japanese use
* conversion between Hiragana and Katakana
* uppercase/lowercase conversion for Greek and Cyrillic letters

These are intentionally treated as explicit extended features rather than as behavior automatically implied by the basic ASCII functions.

---

## Fullwidth/Halfwidth Conversion

Fullwidth/halfwidth conversion in `toctrans` is intended mainly for Japanese use.

### Target Characters

The target set may include at least:

* ASCII-compatible letters, digits, symbols, and space
* halfwidth Katakana
* fullwidth Katakana
* punctuation related to Kana, dakuten, and handakuten

### Exclusions

Hiragana is excluded from fullwidth/halfwidth conversion.

### Single-Character Limitation

Because `toctrans` returns a single `char32_t`, some information may be dropped in conversions that would otherwise require multiple output characters.

For example, converting a fullwidth Katakana character with dakuten or handakuten into a halfwidth form may drop the mark when a one-character result is required.

---

## Kana Conversion

`toctrans` also supports conversion between fullwidth Hiragana and fullwidth Katakana.

Typical categories include:

* `katakana`
* `hiragana`

### Intended Meaning

* `katakana`: convert fullwidth Hiragana to the corresponding fullwidth Katakana
* `hiragana`: convert fullwidth Katakana to the corresponding fullwidth Hiragana

### Exclusions

Halfwidth Katakana is excluded from Kana conversion.

This keeps the transformation model simpler and avoids mixing script conversion with width conversion.

---

## Latin-1 and Extended Classification

`<xer/ctype.h>` may also provide extended classification helpers for Latin-1-related cases.

These can include functions such as:

```cpp
islatin1_upper
islatin1_lower
islatin1_alpha
islatin1_alnum
islatin1_print
islatin1_graph
```

### Role of These Functions

They provide an intermediate step between:

* strict ASCII-only individual functions
* fully dynamic or more ambitious Unicode-wide classification

This is useful where the project wants a small, explicit expansion of practical character handling without turning the basic API into a fully locale-driven or fully Unicode-property-driven system.

---

## Relationship to Other Policies

`<xer/ctype.h>` should be understood together with the following documents:

* `policy_project_outline.md`
* `policy_encoding.md`

It is also related to:

* `header_string.md`
* `header_stdlib.md`

The rough boundary is:

* `<xer/ctype.h>` handles classification and character transformation
* `<xer/string.h>` handles string and memory utilities
* `<xer/stdlib.h>` handles multibyte conversion and related facilities
* encoding policy defines the broader model within which these character operations make sense

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that the basic individual functions are ASCII-oriented and locale-independent
* that `char32_t` is the standard argument type
* that `isctype` and `toctrans` provide dynamic classification and conversion
* that extended categories exist explicitly rather than being implied automatically by the basic API

Detailed per-category semantics should be described in the reference manual or generated API sections.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* testing a character with `isalpha` or `isdigit`
* converting a character with `tolower` or `toupper`
* performing dynamic classification with `isctype`
* using `toctrans` for Kana or width conversion

These are good candidates for executable examples in `examples/`.

---

## Example

```cpp
#include <xer/ctype.h>

auto main() -> int
{
    const auto lower = xer::tolower(U'A');
    if (!lower.has_value()) {
        return 1;
    }

    if (*lower != U'a') {
        return 1;
    }

    if (!xer::isdigit(U'7')) {
        return 1;
    }

    return 0;
}
```

This example shows the normal style:

* use `char32_t` for individual character values
* call basic classification directly
* check `xer::result` explicitly for conversion

---

## See Also

* `policy_project_outline.md`
* `policy_encoding.md`
* `header_string.md`
* `header_stdlib.md`

---

## Unicode Scalar Classification

The dynamic classifier now also includes Unicode validity checks:

```cpp
is_unicode_scalar_value
is_unicode_bmp_scalar_value

ctype_id::unicode
ctype_id::unicode_bmp
```

`is_unicode_scalar_value` returns true for valid Unicode scalar values.
Surrogate code points are rejected.

`is_unicode_bmp_scalar_value` returns true only for valid Unicode scalar values in the Basic Multilingual Plane.

These functions are code-point validity checks. They are not Unicode identifier checks.

---

## Fullwidth and Halfwidth Classification

`ctype_id` includes explicit fullwidth and halfwidth categories:

```cpp
fullwidth_kana
halfwidth_kana
fullwidth_digit
halfwidth_digit
fullwidth_alpha
halfwidth_alpha
fullwidth_punct
halfwidth_punct
fullwidth_space
halfwidth_space
fullwidth_graph
halfwidth_graph
fullwidth_print
halfwidth_print
fullwidth
halfwidth
```

These categories are intended for Japanese text processing and are available through `isctype`.

---

## Fullwidth and Halfwidth Conversion

`ctrans_id` includes fullwidth and halfwidth conversion categories corresponding to the classification categories.

They are available through:

```cpp
auto toctrans(char32_t c, ctrans_id id) -> xer::result<char32_t>;
```

The conversions are single-code-point transformations.
When a fullwidth Kana character with dakuten or handakuten would require multiple halfwidth code points, the current single-character return model may necessarily lose that mark.

---

# `<xer/stdlib.h>`

## Purpose

`<xer/stdlib.h>` provides a collection of general-purpose utilities in XER.

Its role is similar in spirit to the C standard library `<stdlib.h>`, but it is not intended to be a literal reproduction.
Instead, it gathers practical facilities that fit XER's design, especially in the following areas:

- numeric conversion
- multibyte character conversion
- sorting and searching
- environment access
- pseudo-random generation
- small utility structures related to arithmetic-style APIs

This header therefore functions as a broad utility header whose contents are tied together by practical use rather than by a narrow single abstraction.

---

## Main Role

The main role of `<xer/stdlib.h>` is to provide utility facilities that do not belong more naturally to headers such as:

- `<xer/string.h>`
- `<xer/ctype.h>`
- `<xer/stdio.h>`
- `<xer/path.h>`
- `<xer/time.h>`

In particular, it serves as the home for:

- string-to-number conversion
- multibyte conversion APIs
- search and sort helpers
- pseudo-random utilities
- environment variable access

This matches the broad and somewhat heterogeneous role traditionally associated with `<stdlib.h>`, while adapting the details to XER's own policies.

---

## Main Function Groups

At a high level, `<xer/stdlib.h>` contains the following groups of functionality:

- integer division result structures
- search and sorting
- numeric conversion
- multibyte conversion
- environment access
- pseudo-random generation

---

## Integer Division Result Structures

This header may provide small structures related to quotient/remainder style operations, such as:

```cpp
template <class T>
struct rem_quot;

using div_t;
using ldiv_t;
using lldiv_t;

using i8div_t;
using i16div_t;
using i32div_t;
using i64div_t;

using u8div_t;
using u16div_t;
using u32div_t;
using u64div_t;
```

### Role of This Group

These types provide named structures for cases where quotient and remainder are treated together.

They are useful when a division-style helper should return both values in a structured way rather than through multiple separate outputs.

### Notes

* exact naming and type coverage follow XER's own design
* this is related conceptually to arithmetic helpers, but the utility-structure side belongs naturally in `<xer/stdlib.h>`

---

## Search and Sorting

At minimum, this header provides functions such as:

```cpp
bsearch
qsort
```

### Role of This Group

These functions provide classic general-purpose search and sorting operations in the style of the C standard library.

### Design Direction

Although the names are familiar, they should be understood according to XER's broader style:

* practical usability is preferred over strict historical fidelity
* the surrounding type and error model may differ from traditional C expectations
* documentation should describe actual XER behavior rather than relying on user assumptions from C alone

---

## Numeric Conversion

A major part of `<xer/stdlib.h>` is numeric conversion.

At minimum, this header may provide functions such as:

```cpp
ato
atoi
atol
atoll

strto
strtol
strtoll
strtoul
strtoull

strtof
strtod
strtold
strtof32
strtof64
```

### Role of This Group

These functions convert text into numeric values.

They are especially important because XER uses explicit text and encoding policies rather than relying on locale-driven interpretation.

### Design Direction

These facilities are not intended to reproduce the standard library exactly.
Instead, they are reconstructed to fit XER's model.

Important characteristics may include:

* support for UTF-8-oriented public text handling
* explicit failure reporting
* practical parsing features such as binary prefixes where adopted by XER
* consistent behavior across the library's supported environments

### Notes

Floating-point conversion may recognize textual forms such as:

* `inf`
* `infinity`
* `nan`

Integer conversion may also support practical forms such as:

* decimal
* octal
* hexadecimal
* binary with `0b...` where adopted by XER

The precise accepted grammar belongs in detailed API documentation.

---

## Multibyte Conversion

One of the most important roles of `<xer/stdlib.h>` in XER is multibyte character conversion.

At minimum, this header provides the following state type and related facilities:

```cpp
enum class multibyte_encoding;
struct mbstate_t;
```

and functions such as:

```cpp
mblen
mbtotc
tctomb
mbstotcs
tcstombs
```

### Role of This Group

These functions provide conversion between multibyte text and character-oriented representations.

This is one of the clearest places where XER deliberately redesigns a standard-library area according to its own encoding policy.

### Basic Design Direction

The multibyte conversion model in XER is based on the following ideas:

* supported encodings are limited and explicit
* locale is not the center of the design
* `char`, `unsigned char`, and `char8_t` are distinguished explicitly
* `wchar_t`, `char16_t`, and `char32_t` are supported as destination or source character types
* stateful conversion is expressed explicitly through `xer::mbstate_t*`

### Overload-Based Design

These functions are provided as overload sets rather than as templates.

This allows combinations such as:

* byte-oriented input/output using `char`, `unsigned char`, or `char8_t`
* character-oriented input/output using `wchar_t`, `char16_t`, or `char32_t`

### Relationship to Encoding Policy

This part of `<xer/stdlib.h>` should always be read together with the broader encoding policy.
It is one of the most policy-driven areas in the whole header.

---

## `xer::mbstate_t`

`xer::mbstate_t` stores the intermediate state used for stateful multibyte conversion.

### Purpose

It exists so that conversion state is:

* explicit
* portable across calls
* not hidden in internal static storage

### Design Direction

`xer::mbstate_t` stores at least information such as:

* the effective encoding
* incomplete byte sequences
* the number of retained bytes

This is necessary because incomplete multibyte input cannot be interpreted correctly without remembering the encoding context.

### Notes

* XER does not use hidden internal static conversion state
* if the caller wants independent conversion calls, the caller omits the state argument
* if the caller wants continued stateful conversion, the caller provides `xer::mbstate_t*`

---

## Environment Access

This header may provide:

```cpp
getenv
```

### Role of This Group

This function group provides access to process environment information.

### Notes

As with other facilities in XER, the exact argument and return conventions should be read from XER documentation rather than assumed from the C standard library alone.

---

## Pseudo-Random Generation

This header may provide facilities such as:

```cpp
rand
srand
class rand_context
```

### Role of This Group

These functions and types provide simple pseudo-random number generation facilities.

### Design Direction

The purpose here is not to compete with the full generality of `<random>`.
Instead, the goal is to provide a lighter, easier-to-approach utility layer in the style of traditional C facilities, while still fitting XER's design.

### `rand_context`

A context type such as `rand_context` is useful when:

* random state should be made explicit
* global-state dependence should be reduced
* multiple independent generators are desirable

The exact API belongs in the detailed reference documentation.

---

## Relationship to XER's Text Model

`<xer/stdlib.h>` is closely tied to XER's text and encoding model.

This is especially true for:

* numeric conversion
* multibyte conversion

### Important Assumptions

* UTF-8 is the primary public string representation
* `char8_t`, `std::u8string`, and `std::u8string_view` are central to public text APIs
* `char32_t` is the normal type for individual Unicode scalar values
* multibyte conversion distinguishes `char`, `unsigned char`, and `char8_t` explicitly

This means that `<xer/stdlib.h>` is not just a miscellaneous utility header.
In XER, it is also one of the key headers for explicit encoding-aware text handling.

---

## Relationship to Other Headers

`<xer/stdlib.h>` should be understood together with the following headers and policies:

* `header_string.md`
* `header_ctype.md`
* `policy_multibyte.md`
* `policy_encoding.md`
* `policy_project_outline.md`

The rough boundary is:

* `<xer/string.h>` handles string and memory utilities
* `<xer/ctype.h>` handles character classification and character transformation
* `<xer/stdlib.h>` handles numeric conversion, multibyte conversion, and miscellaneous utility facilities
* encoding and multibyte policy documents define the deeper model behind the API

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that it serves as a utility header rather than a narrowly scoped abstraction
* that numeric conversion and multibyte conversion are the most important parts of the header
* that multibyte conversion follows XER's own encoding policy rather than locale-centered C behavior
* that overloads distinguish byte-oriented and character-oriented types explicitly

Detailed per-function semantics should be described in the reference manual or generated API sections.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* converting UTF-8 text to an integer or floating-point value
* converting one multibyte character with `mbtotc`
* converting one character back with `tctomb`
* converting an entire multibyte string with `mbstotcs` or `tcstombs`
* using `rand` / `srand` or `rand_context` in a minimal example

These are good candidates for executable examples under `examples/`.

---

## Example

```cpp
#include <xer/stdlib.h>

auto main() -> int
{
    const auto result = xer::strtol(u8"123");
    if (!result.has_value()) {
        return 1;
    }

    if (*result != 123) {
        return 1;
    }

    return 0;
}
```

This example shows the general XER style:

* call the conversion API with an ordinary value
* check `xer::result` explicitly
* treat normal failure as part of the ordinary control flow

---

## See Also

* `policy_project_outline.md`
* `policy_encoding.md`
* `policy_multibyte.md`
* `header_string.md`
* `header_ctype.md`

---

# `<xer/json.h>`

## Purpose

`<xer/json.h>` provides JSON value handling and JSON encode/decode facilities in XER.

Its purpose is not merely to treat JSON as a string-processing convenience.
Instead, it provides a structured representation of JSON data together with parsing and serialization functions.

This header is therefore positioned as a data-format facility rather than as part of the ordinary string utility layer.

---

## Main Role

The main role of `<xer/json.h>` is to make it possible to:

- parse JSON text into a structured XER value model
- inspect and manipulate JSON values in memory
- serialize structured JSON values back into text

This makes the header useful for configuration data, simple structured interchange, and JSON-oriented utility processing.

---

## Main Entities

At minimum, `<xer/json.h>` provides the following entities:

```cpp
struct json_value;

using json_array = json_value::array_type;
using json_object = json_value::object_type;

auto json_decode(std::u8string_view text) -> xer::result<json_value>;
auto json_encode(const json_value& value) -> xer::result<std::u8string>;
```

The exact helper members and constructors of `json_value` may evolve, but this is the core public shape.

---

## `json_value`

`json_value` is the central value type for JSON in XER.

It stores one JSON value in structured form.

### Supported Value Kinds

At minimum, `json_value` can represent the following JSON kinds:

* `null`
* boolean
* number
* string
* array
* object

### Concrete Stored Forms

The value model currently corresponds to the following C++ forms:

* `std::nullptr_t`
* `bool`
* `double`
* `std::u8string`
* array of `json_value`
* object represented as `std::vector<std::pair<std::u8string, json_value>>`

This means that objects preserve insertion order rather than being normalized into an associative container.

### Why This Matters

This design keeps the JSON model lightweight and practical:

* strings fit naturally into XER's UTF-8-oriented text model
* numbers use `double`
* arrays remain straightforward recursive containers
* objects preserve source order naturally

---

## `json_array`

`json_array` is an alias for the array type used by `json_value`.

### Role

This alias exists to make array-oriented code more readable and to avoid exposing the implementation form only indirectly through `json_value`.

### Typical Use

A caller may use `json_array` when constructing or inspecting an array value explicitly.

---

## `json_object`

`json_object` is an alias for the object type used by `json_value`.

### Role

This alias exists to make object-oriented code easier to read.

### Current Representation

A JSON object is represented as an ordered sequence of key/value pairs:

```cpp
std::vector<std::pair<std::u8string, json_value>>
```

### Notes

This means:

* key order is preserved
* the representation is simple and predictable
* behavior should not be assumed to match that of a hash map or tree map automatically

---

## `json_decode`

```cpp id="tjh4ki"
auto json_decode(std::u8string_view text) -> xer::result<json_value>;
```

### Purpose

`json_decode` parses UTF-8 JSON text and returns a structured `json_value`.

### Input Model

The input text is provided as:

```cpp id="6ve0b2"
std::u8string_view
```

This matches XER's general UTF-8-oriented public text policy.

### Return Model

On success, `json_decode` returns a `json_value`.

On failure, it returns an error through `xer::result`.

### Role in the Header

This is the main entry point from text into the in-memory JSON model.

---

## `json_encode`

```cpp id="d05h93"
auto json_encode(const json_value& value) -> xer::result<std::u8string>;
```

### Purpose

`json_encode` serializes a structured `json_value` into UTF-8 JSON text.

### Return Model

On success, it returns:

```cpp id="7v7txm"
std::u8string
```

On failure, it returns an error through `xer::result`.

### Role in the Header

This is the main entry point from the in-memory JSON model back into text.

---

## Accessors and Inspection

`json_value` may provide inspection and accessor functions such as:

```cpp id="2z9586"
is_null
is_bool
is_number
is_string
is_array
is_object

as_bool
as_number
as_string
as_array
as_object
```

### Role of These Functions

These functions allow callers to:

* inspect which JSON kind is currently stored
* retrieve the underlying value in typed form

### Basic Direction

The design is intentionally explicit.

A caller should first know or check what kind of value is present, and then access it accordingly.

---

## Number Representation

Numbers in `json_value` are represented as:

```cpp id="enb0ih"
double
```

### Design Direction

This keeps the implementation simple and aligns with the current project policy.

### Implications

* the JSON number model is handled as floating-point data
* integers are represented through the same numeric storage
* the API should not be read as preserving arbitrary-precision numeric structure

---

## String Representation

JSON strings are represented as:

```cpp id="5bbuxu"
std::u8string
```

### Why This Matters

This matches XER's broader text model:

* public text APIs are UTF-8 oriented
* `char8_t`-based strings are the normal representation
* JSON processing remains consistent with the rest of the library

---

## Object Representation

Objects are represented as an ordered vector of key/value pairs rather than as a map-like container.

### Reasons This Is Reasonable

This design has practical advantages:

* preserves insertion order
* avoids committing the public model to hash-based semantics
* keeps the stored structure straightforward
* aligns well with many JSON-oriented use cases

### Important Note

Documentation should make clear that object behavior is based on ordered pairs, not on implicit map semantics.

---

## Error Handling

`<xer/json.h>` follows XER's ordinary failure model.

That means:

* parsing failure is reported through `xer::result`
* serialization failure is reported through `xer::result`
* normal failure is not expressed through exceptions by default

### Design Direction

This keeps JSON processing aligned with the rest of XER's public APIs.

---

## Relationship to Other Headers

`<xer/json.h>` should be understood together with the following documents and headers:

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `header_string.md`

The rough boundary is:

* `<xer/string.h>` handles string and text utilities
* `<xer/json.h>` handles structured JSON data and JSON text conversion

This separation is intentional.
JSON is treated as an independent data-format facility rather than as a mere extension of string helpers. 

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that it provides a structured JSON value model
* that `json_decode` parses UTF-8 JSON text
* that `json_encode` serializes the structured model back to UTF-8 text
* that objects preserve insertion order

Detailed grammar and escaping rules belong in the detailed reference or generated API documentation.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* decoding a simple JSON object
* decoding a JSON array
* reading values from `json_value`
* encoding a constructed JSON value back into text

These are good candidates for executable examples in `examples/`.

---

## Example

```cpp id="q07rge"
#include <xer/json.h>

auto main() -> int
{
    const auto decoded = xer::json_decode(u8"{\"value\":123}");
    if (!decoded.has_value()) {
        return 1;
    }

    if (!decoded->is_object()) {
        return 1;
    }

    const auto encoded = xer::json_encode(*decoded);
    if (!encoded.has_value()) {
        return 1;
    }

    return 0;
}
```

This example shows the general style:

* parse UTF-8 JSON text with `json_decode`
* inspect the resulting `json_value`
* serialize it again with `json_encode`
* check `xer::result` explicitly at each fallible step

---

## See Also

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `header_string.md`

---

# `<xer/stdio.h>`

## Purpose

`<xer/stdio.h>` provides stream-based input and output facilities in XER.

Its role is similar in spirit to the C standard library `<stdio.h>`, but it is not intended to be a literal reproduction.
Instead, it reconstructs practical I/O around explicit stream types, explicit encodings, and XER's ordinary failure model.

This header is one of the most important public headers in XER because it provides the main user-facing path for:

- binary stream I/O
- text stream I/O
- formatted input/output
- file-entry operations
- CSV input/output
- stream state and positioning
- stream rewinding

---

## Main Role

The main role of `<xer/stdio.h>` is to provide a coherent I/O model built on the following principles:

- do not expose `FILE*` directly as the public abstraction
- distinguish binary I/O from text I/O explicitly
- use RAII for stream lifetime management
- handle text encodings explicitly rather than through locale
- report ordinary failure through `xer::result`

This makes the header easier to use safely from modern C++ code while preserving the familiarity of many classic C-style function names.

---

## Core Stream Types

At minimum, `<xer/stdio.h>` provides the following public stream types:

```cpp
class binary_stream;
class text_stream;
```

These are the central abstractions of the header.

### `binary_stream`

`binary_stream` represents binary input/output.

It is used for:

* files opened in binary mode
* memory-backed binary streams
* binary temporary streams
* other byte-oriented stream targets

### `text_stream`

`text_stream` represents text input/output.

It is used for:

* files opened with explicit text encoding
* UTF-8 or CP932 text sources
* string-backed text streams
* text temporary streams
* standard text-oriented input/output targets

### Design Direction

These two stream types are intentionally separate.

XER does not model them as one stream class with a mode switch.
Instead, the distinction between binary and text I/O is made explicit at the type level.

---

## Move-Only RAII Objects

`binary_stream` and `text_stream` are move-only RAII objects.

### Meaning

This implies at least the following:

* they are not copyable
* they are movable
* they acquire and release stream resources through object lifetime
* the destructor performs automatic cleanup

### Why This Matters

This makes stream ownership explicit and avoids many ambiguities associated with raw handle sharing.

It also fits XER's broader design preference for explicit ownership and explicit failure handling.

---

## Stream Opening

`<xer/stdio.h>` provides functions for opening streams from files, memory, and strings.

### File Opening

At minimum, the public file-opening forms are:

```cpp
auto fopen(const path& filename, const char* mode) noexcept -> xer::result<binary_stream>;
auto fopen(const path& filename, const char* mode, encoding_t encoding) noexcept -> xer::result<text_stream>;
```

These two overloads separate binary opening from text opening.

### Memory Opening

For memory-backed streams, the header may provide forms such as:

```cpp
auto memopen(std::span<std::byte> memory, const char* mode) noexcept -> xer::result<binary_stream>;
auto stropen(std::u8string_view text, const char* mode) noexcept -> xer::result<text_stream>;
auto stropen(std::u8string& text, const char* mode) noexcept -> xer::result<text_stream>;
```

### Design Direction

These open functions are designed so that:

* binary streams and text streams are distinguished at open time
* ordinary ownership of the backing container is not silently transferred
* borrowed storage remains explicit in the API shape

---

## Text Encoding Selection

For text streams, `<xer/stdio.h>` provides explicit encoding selection.

At minimum, the public encoding enumeration is:

```cpp
enum class encoding_t {
    utf8,
    cp932,
    auto_detect,
};
```

### Meaning

* `utf8` means UTF-8 text
* `cp932` means CP932 text
* `auto_detect` means automatic input-side detection between supported encodings

### Important Notes

* text I/O in XER is not locale-centered
* encoding is part of the stream-opening model
* `auto_detect` is intended for input, not for general write-side behavior

This is one of the clearest differences from traditional locale-driven C text I/O.

---

## Binary I/O

For binary streams, the header provides byte-oriented operations.

At minimum, this includes functions such as:

```cpp
fread
fwrite
fgetb
fputb
```

### Role of This Group

These functions provide:

* block input/output
* single-byte input/output
* binary-stream operations in byte units

### Design Direction

Binary data is handled as raw byte-oriented data rather than as text.

`fgetc` and `fputc` are therefore not the single-byte binary I/O interface.
Instead, XER uses `fgetb` and `fputb` for that role.

---

## Text I/O

For text streams, the header provides character- and string-oriented operations.

At minimum, this includes functions such as:

```cpp
fgetc
getchar
ungetc

fputc
putchar

fgets
gets
fputs
puts
```

### Role of This Group

These functions provide:

* single-character text input/output
* string-oriented text input/output
* standard-stream convenience operations
* limited push-back support through `ungetc`

### Design Direction

Text streams are normalized internally around XER's text model.

In particular:

* single-character input is centered on `char32_t`
* string-oriented text handling is centered on UTF-8 `char8_t` strings

The exact visible overload set depends on the implementation, but the conceptual model remains the same.

---

## Formatted Input and Output

`<xer/stdio.h>` also provides formatted I/O facilities.

At minimum, this includes the following families:

```cpp
fprintf
sprintf
snprintf
printf
fscanf
sscanf
scanf
```

### Role of This Group

These functions provide familiar formatted I/O in a style approachable to users familiar with C.

### Design Direction

Although the naming resembles the standard library, the surrounding design is XER's own:

* stream types are explicit
* text model is UTF-8-oriented
* ordinary failure is reported through XER-style result handling where applicable
* integration with XER stream abstractions takes priority over strict source-level emulation of C

### printf Format Details

# XER printf Format Specifiers

## Scope

This document describes the format strings used by the XER printf family.

Target functions:

```cpp
printf
fprintf
sprintf
snprintf
```

The scanf family is outside the scope of this document for now.
It will be documented separately after the scanf family is strengthened.

---

## Basic Policy

XER printf-style functions are inspired by C printf, but they are not strict source-compatible reimplementations.

- format strings are UTF-8 strings
- fixed text in the format string is copied as UTF-8
- conversion specifications start with `%`
- ordinary failure is reported through `xer::result`
- XER-specific extensions may exist

A format string may contain ordinary UTF-8 text and conversion specifications.
Ordinary text is copied to the output as-is.

---

## Conversion Specification Syntax

A conversion specification begins with `%`.

The currently supported structure is:

```text
%[position$][flags][width][.precision][length]conversion
```

The positional form is optional.
When it is used, the first argument is numbered `1`.

Examples:

```cpp
xer::printf(u8"%@ %@\n", first, second);
xer::printf(u8"%2$@ %1$@\n", first, second);
```

---

## Flags

The following flags are recognized:

```text
- + space # 0
```

Their meanings follow the usual printf-style interpretation where applicable.
For conversions where a flag has no meaningful effect, it may be ignored.

---

## Width and Precision

A field width may be specified as a decimal integer or by `*`.

A precision may be specified with `.` followed by a decimal integer or by `*`.

Both width and precision may use positional arguments.

Examples:

```cpp
xer::printf(u8"%10@\n", value);
xer::printf(u8"%.*@\n", precision, value);
xer::printf(u8"%2$*1$@\n", width, value);
```

Width is counted in UTF-8 code units in the current implementation.
It is not a display-cell width calculation.

---

## Length Modifiers

The following length modifiers are parsed:

```text
hh h l ll j z t L
```

They are accepted as part of the printf-style grammar.
The actual effect depends on the conversion and on XER's internal argument normalization.

For floating-point conversions, `L` is used when constructing the intermediate narrow format passed to `std::snprintf`.

---

## Supported C-Style Conversions

The following C-style conversion specifiers are supported:

```text
%d %i
%u
%o
%x %X
%c
%s
%p
%e %E
%f %F
%g %G
%a %A
%%
```

`%%` outputs a literal percent sign and does not consume an argument.

---

## XER Generic Display Conversion: `%@`

`%@` is XER's generic display specifier.

It is intended for diagnostics, examples, tracing, and simple output where precise base, padding, or precision control is not the main concern.
When precise formatting is required, ordinary printf-style conversions should be used instead.

### Argument Conversion Rules

Arguments passed to `%@` are normalized to UTF-8 text according to the following rules:

1. `char8_t`, `char8_t*`, `std::u8string`, and `std::u8string_view` are treated directly as UTF-8.
2. `char16_t*`, `std::u16string`, and `std::u16string_view` are converted from UTF-16 to UTF-8.
3. `char32_t*`, `std::u32string`, and `std::u32string_view` are converted from UTF-32 to UTF-8.
4. `wchar_t*`, `std::wstring`, and `std::wstring_view` are converted according to the width of `wchar_t`.
5. `std::string` and `std::string_view` are treated as UTF-8 byte strings.
6. `bool` is formatted as `true` or `false`.
7. `nullptr` is formatted as `null`.
8. Other stream-insertable types are formatted through `std::ostringstream` and the resulting narrow string is treated as UTF-8 bytes.

Invalid UTF-16 or UTF-32 scalar data may be represented by the replacement character in diagnostic-oriented conversions.

### XER Types

The following XER types are intended to be printable through `%@`:

```cpp
xer::error_t
xer::error<Detail>
xer::result<T, Detail>
```

These types provide stream insertion support so that `%@` can display them through the generic stream-based route.

### Notes on `std::ostringstream`

XER does not use iostreams as its primary public I/O model.
However, `%@` may use `std::ostringstream` internally as a practical interoperability mechanism.
This keeps user-facing XER formatted I/O based on `xer::printf` and related functions while allowing types that support `operator<<` to be displayed conveniently.

---

## Error Handling

Format errors, unsupported argument kinds, missing arguments, and out-of-range width or precision arguments are reported through `xer::result`.

The exact error category may be refined as the implementation evolves, but invalid format usage is generally treated as an ordinary formatting failure rather than as undefined behavior.

---

## Implementation Notes

This document is intended to describe the user-visible printf-family behavior.
When implementation details in `xer/bits/printf_format.h` change, this document should be kept in sync.

---

## CSV Support

`<xer/stdio.h>` also includes CSV-oriented helpers:

```cpp
fgetcsv
fputcsv
```

### Role of This Group

These functions provide convenient CSV input and output on top of XER streams.

They are particularly useful because CSV is a text-oriented format that benefits from integration with:

* explicit text encodings
* explicit stream ownership
* UTF-8-oriented strings

### Design Direction

These functions are not merely string helpers.
They belong naturally in the I/O layer because they operate on streams and formatted text records.

---

## Position and Stream State Helpers

At minimum, `<xer/stdio.h>` provides helpers such as:

```cpp
fseek
ftell
fgetpos
fsetpos
feof
ferror
clearerr
```

and the related public types:

```cpp
enum seek_origin_t { seek_set, seek_cur, seek_end };
using fpos_t = std::uint64_t;
```

### Role of This Group

These facilities provide:

* byte- or position-oriented stream movement
* stream status inspection
* stream error-state control
* explicit text-stream position handling

### Binary vs Text Positioning

The basic intended distinction is:

* `fseek` / `ftell` are the ordinary position helpers for `binary_stream`
* `fgetpos` / `fsetpos` are the primary position helpers for `text_stream`

This reflects the fact that text streams may not always map cleanly to simple byte offsets after decoding and buffering.

---

## Closing and Flushing

This header also provides operations such as:

```cpp
fclose
fflush
tmpfile
```

and a text-oriented temporary-file overload or equivalent helper for explicitly encoded text streams.

### Role of This Group

These functions support:

* explicit closing
* explicit flush control
* temporary stream creation

### Design Direction

Even though stream destructors perform automatic cleanup, explicit close and flush operations still matter because:

* callers may want deterministic resource release
* callers may want explicit error observation before destruction
* explicit flushing is part of normal stream control

---

## File-Entry Operations

`<xer/stdio.h>` also provides file-entry operations such as:

```cpp
remove
rename
mkdir
rmdir
copy
```

### Role of This Group

These functions operate on filesystem entries rather than on open stream objects.

They are grouped here because they are operationally close to stream/file handling.

### Design Direction

These functions are intentionally separate from stream objects themselves.

They typically operate on `xer::path`, not on raw native path strings.

This aligns them with XER's own path model.

---

## Native Handle Access

The header may also expose support related to native-handle access.

### Role

This exists for cases where callers need to bridge XER stream abstractions to lower-level platform or runtime facilities.

### Design Direction

Such support is supplementary rather than central.
The normal user-facing abstraction remains `binary_stream` or `text_stream`, not the native handle.

---

## Internal Design Direction

Although `<xer/stdio.h>` is a public header, its conceptual design depends on the following ideas:

* stream objects are lightweight public handles
* internal state is hidden behind those handles
* binary and text streams may use function-pointer-based internal dispatch
* text-stream state may include buffering, encoding resolution, and multibyte intermediate state

These implementation ideas are important to understand the shape of the public API, even though the internal structures themselves are not the public abstraction.

---

## Relationship to XER's Text Model

`<xer/stdio.h>` is one of the headers most tightly coupled to XER's overall text model.

In particular:

* UTF-8 is the primary public string representation
* `char32_t` is used for individual text characters where appropriate
* text streams support UTF-8 and CP932 explicitly
* automatic text input detection is limited and explicit

This means that `<xer/stdio.h>` should always be read together with the broader encoding-related policies.

---

## Relationship to Other Headers and Policies

`<xer/stdio.h>` should be understood together with:

* `policy_project_outline.md`
* `policy_stdio.md`
* `policy_encoding.md`
* `header_path.md`
* `header_stdlib.md`

The rough boundary is:

* `<xer/path.h>` handles lexical path representation and native-path conversion
* `<xer/stdio.h>` handles stream I/O and file-entry operations
* `<xer/stdlib.h>` handles multibyte conversion and related utility facilities
* encoding policy explains the broader text-encoding model behind text streams

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that XER distinguishes `binary_stream` and `text_stream`
* that text encodings are explicit rather than locale-driven
* that stream objects are move-only RAII types
* that both low-level I/O and higher-level facilities such as formatted I/O and CSV are included
* that path-oriented file-entry operations are part of the header

Detailed per-function semantics should be described in the reference manual or generated API sections.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* opening a binary file and reading bytes
* opening a UTF-8 text stream and reading text
* writing text with `puts` or `fputs`
* using `fgetpos` / `fsetpos`
* using `tmpfile`
* reading or writing CSV
* performing `rename`, `remove`, or `copy`

These are good candidates for executable examples under `examples/`.

---

## Example

```cpp
#include <xer/stdio.h>

auto main() -> int
{
    if (!xer::puts(u8"hello").has_value()) {
        return 1;
    }

    return 0;
}
```

This example shows the basic XER style:

* use XER text I/O directly
* work with UTF-8-oriented text
* check `xer::result` explicitly

---

## See Also

* `policy_project_outline.md`
* `policy_stdio.md`
* `policy_encoding.md`
* `header_path.md`
* `header_stdlib.md`


---

## Rewinding

`<xer/stdio.h>` provides `rewind` for both stream kinds:

```cpp
auto rewind(binary_stream& stream) noexcept -> xer::result<void>;
auto rewind(text_stream& stream) noexcept -> xer::result<void>;
```

Unlike the C standard-library function, XER's `rewind` returns `xer::result<void>` so that invalid streams and seek failures can be reported explicitly.

For text streams, rewinding also clears pushed-back characters, lookahead bytes, and partial decoding state. If the stream was opened with `encoding_t::auto_detect`, the concrete encoding is returned to the undecided state.

---

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

---

# `<xer/socket.h>`

## Purpose

`<xer/socket.h>` provides a small socket API for TCP and UDP networking.

The API is intentionally low-level enough to stay understandable, but it wraps platform differences and reports ordinary failure through `xer::result`.

---

## Main Role

This header provides:

- a move-only RAII socket handle
- IPv4 and IPv6 socket creation
- TCP connection, bind, listen, and accept operations
- UDP send/receive operations
- conversion of sockets to XER binary or text streams

---

## Main Types

```cpp
enum class socket_family;
enum class socket_type;
struct socket_address;
struct socket_recvfrom_result;
class socket;
```

### `socket_family`

```cpp
ipv4
ipv6
```

### `socket_type`

```cpp
tcp
udp
```

### `socket_address`

`socket_address` stores a textual address and a port number.

```cpp
std::u8string address;
std::uint16_t port;
```

### `socket_recvfrom_result`

`socket_recvfrom_result` stores the number of bytes read and the remote endpoint address.

---

## Socket Handle

`socket` is a move-only RAII type.

Important operations include:

```cpp
auto is_open() const noexcept -> bool;
auto family() const noexcept -> socket_family;
auto type() const noexcept -> socket_type;
auto close() noexcept -> int;
auto release() noexcept -> native_socket_t;
auto native_handle() const noexcept -> native_socket_t;
```

The destructor closes the socket if it is still open.

---

## Socket Operations

```cpp
auto socket_create(socket_family family, socket_type type) noexcept -> xer::result<socket>;
auto socket_close(socket& s) noexcept -> xer::result<void>;
auto socket_connect(socket& s, std::u8string_view host, std::uint16_t port) noexcept -> xer::result<void>;
auto socket_bind(socket& s, std::uint16_t port) noexcept -> xer::result<void>;
auto socket_getsockname(socket& s) noexcept -> xer::result<socket_address>;
auto socket_listen(socket& s, int backlog = 16) noexcept -> xer::result<void>;
auto socket_accept(socket& s) noexcept -> xer::result<socket>;
auto socket_send(socket& s, std::span<const std::byte> data) noexcept -> xer::result<std::size_t>;
auto socket_recv(socket& s, std::span<std::byte> data) noexcept -> xer::result<std::size_t>;
auto socket_sendto(socket& s, std::u8string_view host, std::uint16_t port, std::span<const std::byte> data) noexcept -> xer::result<std::size_t>;
auto socket_recvfrom(socket& s, std::span<std::byte> data) noexcept -> xer::result<socket_recvfrom_result>;
```

---

## Stream Conversion

Sockets can be converted into XER streams:

```cpp
auto socket_open(socket&& s) noexcept -> xer::result<binary_stream>;
auto socket_open(socket&& s, encoding_t encoding) noexcept -> xer::result<text_stream>;
```

The binary stream form is suitable for byte-oriented protocols.
The text stream form is suitable for UTF-8 or CP932 text-oriented communication.

---

## Notes

- Network-related ordinary failures are represented mainly with `error_t::network_error`.
- The API does not use command shells or external utilities.
- Host names are accepted as UTF-8 strings and converted to ordinary narrow strings for resolver APIs.
- `socket_open` transfers ownership from the socket object into the resulting stream.

---

# `<xer/stdint.h>`

## Purpose

`<xer/stdint.h>` provides fixed-width integer facilities and closely related numeric utilities in XER.

Its role is similar in spirit to the C standard library `<stdint.h>`, but it is not limited to merely re-exporting integer typedefs.
Instead, it also serves as the home for practical integer-oriented helpers that fit XER's overall design.

This header is especially important because it provides:

- fixed-width integer type aliases
- pointer-sized integer types
- optional 128-bit integer aliases where supported
- compile-time numeric helper constants
- integer literal suffixes for convenient typed integer constants

---

## Main Role

The main role of `<xer/stdint.h>` is to provide a stable and explicit integer vocabulary for the rest of XER.

In particular, it exists to make the following easy and clear:

- writing code with explicitly sized integer types
- referring to implementation-sized integer types such as pointer-sized integers
- expressing integer limits and bit widths in a unified XER style
- writing typed integer literals directly in source code

This makes the header useful both as a foundational type header and as a practical utility header for integer-heavy code.

---

## Main Entities

At minimum, `<xer/stdint.h>` provides the following kinds of entities:

- fixed-width signed integer types
- fixed-width unsigned integer types
- least-width and fast-width integer types where appropriate
- pointer-sized integer types
- maximum-width integer types
- optional 128-bit integer types where available
- compile-time helper constants
- user-defined integer literal suffixes

The exact exposed set follows the implementation and project policy, but these are the intended public categories.

---

## Fixed-Width Integer Types

`<xer/stdint.h>` provides the familiar fixed-width integer types, including at least forms such as:

```cpp
int8_t
int16_t
int32_t
int64_t

uint8_t
uint16_t
uint32_t
uint64_t
```

### Role of These Types

These types are used when the width of the integer value matters explicitly.

Typical use cases include:

* binary formats
* protocol definitions
* packed data structures
* arithmetic with explicit size expectations
* cross-platform code that should not depend on implementation-defined plain `int` width

### Notes

These names are meant to be straightforward and familiar to users coming from C and C++.

---

## Pointer-Sized and Maximum-Width Integer Types

This header also provides integer types associated with pointer size or maximum practical width, such as:

```cpp
intptr_t
uintptr_t
intmax_t
uintmax_t
```

### Role of These Types

They are useful when code needs to:

* convert pointers to integer form safely where appropriate
* reason about the widest practical integer category
* write generic numeric code that should adapt to the implementation

### Notes

These types are especially useful in low-level code and implementation-support code.

---

## Optional 128-Bit Integer Types

Where the implementation supports `__int128`, XER may provide:

```cpp
int128_t
uint128_t
```

### Role of These Types

These types are useful when:

* 64-bit range is insufficient
* intermediate arithmetic should avoid overflow
* larger integer constants should remain explicit in type

### Availability

These types are implementation-dependent.

They are available only where the compiler and target support the necessary underlying integer type.

Documentation should therefore describe them as optional rather than universally guaranteed.

---

## Compile-Time Numeric Helpers

`<xer/stdint.h>` may also provide helper constants such as:

```cpp
min_of<T>
max_of<T>
bit_width_of<T>
```

### `min_of<T>`

`min_of<T>` represents the minimum value of the integer type `T`.

### `max_of<T>`

`max_of<T>` represents the maximum value of the integer type `T`.

### `bit_width_of<T>`

`bit_width_of<T>` represents the bit width of `T`.

### Role of These Helpers

These helpers exist so that integer-type metadata can be referred to in a compact, readable, and XER-consistent way.

They are especially useful in:

* compile-time checks
* generic numeric utilities
* range-sensitive code
* documentation examples

### Design Direction

These helpers are intended to be simple compile-time facilities, not large abstraction layers.

---

## Integer Literal Suffixes

One of the most visible user-facing features of `<xer/stdint.h>` is the integer literal suffix set.

At minimum, XER may provide literal suffixes such as:

```cpp
_i8   _i16   _i32   _i64
_u8   _u16   _u32   _u64
_i128 _u128
```

typically under:

```cpp
xer::literals::integer_literals
```

### Role of These Suffixes

These suffixes make it possible to write typed integer constants directly and readably.

For example:

```cpp
using namespace xer::literals::integer_literals;

constexpr auto x = 123_i32;
constexpr auto y = 255_u8;
```

This improves clarity in code where the intended integer type matters.

### Design Direction

These literal suffixes are intended to be:

* explicit
* readable
* convenient in tests and examples
* useful in compile-time contexts

They are especially attractive in a project like XER, which emphasizes explicitness and type clarity.

---

## Supported Literal Forms

The literal-parsing facilities behind the suffixes may support at least the following textual forms:

* decimal
* octal
* hexadecimal
* binary with `0b...`
* digit separators using `'`

### Examples

```cpp
using namespace xer::literals::integer_literals;

constexpr auto a = 123_i32;
constexpr auto b = 0xff_u32;
constexpr auto c = 0b1010_u8;
constexpr auto d = 1'000'000_i64;
```

### Notes

These features are especially useful for:

* tests
* binary and bit-oriented code
* examples where the intended numeric type should remain obvious

---

## Range Checking

An important design point of the integer literal facilities is that range checking is performed explicitly.

### Meaning

When a literal suffix requests a specific destination type, the literal should fit in that type.

If it does not fit, the program should fail to compile rather than silently narrowing the value.

### Why This Matters

This makes typed integer literals trustworthy and avoids hidden truncation.

It also aligns with XER's broader design preference for explicit failure over surprising implicit behavior.

---

## Relationship to Other Headers

`<xer/stdint.h>` should be understood together with:

* `policy_project_outline.md`
* `header_arithmetic.md`

The rough boundary is:

* `<xer/stdint.h>` provides integer types, numeric limits/helpers, and typed integer literals
* `<xer/arithmetic.h>` provides arithmetic and comparison helpers built on top of explicit numeric types

This makes `<xer/stdint.h>` foundational, while `<xer/arithmetic.h>` handles higher-level numeric operations.

---

## Relationship to XER's Numeric Design

Although `<xer/stdint.h>` looks like a basic type header, it plays an important role in XER's numeric design.

In particular, it helps make the following explicit:

* the exact type of integer values
* the intended width of constants
* the range assumptions of numeric code
* implementation-dependent availability of wider integer types

This is especially valuable in a library that also provides mixed-type arithmetic helpers and explicit range checking.

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that it provides fixed-width integer types and related aliases
* that it may provide optional 128-bit aliases where supported
* that it includes simple compile-time integer helpers
* that it provides user-defined literal suffixes for typed integer constants

Detailed parsing rules and edge cases for literal suffixes belong in the detailed reference or generated API sections.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* declaring values with fixed-width integer types
* using `bit_width_of<T>`
* writing typed integer literals with `_i32` or `_u64`
* showing compile-time range-safe constant expression use

These are good candidates for executable examples in `examples/`.

---

## Example

```cpp
#include <xer/stdint.h>

using namespace xer::literals::integer_literals;

auto main() -> int
{
    constexpr auto x = 123_i32;
    constexpr auto y = 255_u16;

    static_assert(std::same_as<decltype(x), const xer::int32_t>);
    static_assert(std::same_as<decltype(y), const xer::uint16_t>);

    return 0;
}
```

This example shows the normal style:

* use explicit XER integer types
* use typed integer literal suffixes
* keep integer width visible in the code itself

---

## See Also

* `policy_project_outline.md`
* `header_arithmetic.md`


---

## Integer Literal Suffixes

Integer literal suffixes are provided in:

```cpp
xer::literals::integer_literals
```

The fixed-width suffixes include:

```cpp
_i8   _i16   _i32   _i64
_u8   _u16   _u32   _u64
_i128 _u128  // when supported
```

The least-width suffixes include:

```cpp
_il8   _il16   _il32   _il64
_ul8   _ul16   _ul32   _ul64
```

The least-width suffixes produce the corresponding `int_leastN_t` or `uint_leastN_t` type and are useful when exact storage width is less important than a guaranteed minimum range.

---

## Numeric Limit Helpers

`min_of<T>` and `max_of<T>` are implemented through a shared numeric-limits helper so that they can also be reused by floating-point facilities such as `<xer/stdfloat.h>`.

`bit_width_of<T>` remains available from `<xer/stdint.h>` for integer-oriented bit width queries.

---

# `<xer/stdfloat.h>`

## Purpose

`<xer/stdfloat.h>` provides floating-point type aliases and floating-point user-defined literals in the same spirit as `<xer/stdint.h>`.

The header is intended to make floating-point width and minimum-width intent explicit while still remaining usable on implementations where C++23 `<stdfloat>` support is incomplete.

---

## Main Role

This header provides:

- fixed-width floating-point aliases where the implementation provides them
- practical fallback aliases for `float32_t` and `float64_t`
- optional aliases for 80-bit and 128-bit floating-point formats
- least-width and fast-width floating-point aliases
- optional decimal floating-point aliases when the implementation provides them
- floating-point user-defined literals under `xer::literals::floating_literals`

---

## Availability Macros

The header defines availability macros for optional types.

Examples include:

```cpp
XER_HAS_FLOAT16_T
XER_HAS_FLOAT32_T
XER_HAS_FLOAT64_T
XER_HAS_FLOAT80_T
XER_HAS_FLOAT128_T
XER_HAS_BFLOAT16_T
XER_HAS_FLOAT_LEAST80_T
XER_HAS_FLOAT_FAST80_T
XER_HAS_DECIMAL32_T
XER_HAS_DECIMAL64_T
XER_HAS_DECIMAL128_T
```

These macros allow code and tests to guard features that depend on implementation support.

---

## Binary Floating-Point Aliases

At minimum, the following aliases are provided when possible:

```cpp
float16_t
float32_t
float64_t
float80_t
float128_t
bfloat16_t
```

`float32_t` and `float64_t` are always available in XER. If the standard `<stdfloat>` aliases are not available, they fall back to `float` and `double` respectively.

`float80_t`, `float128_t`, and `bfloat16_t` are optional and are available only when the implementation provides a suitable underlying type.

---

## Least and Fast Floating-Point Aliases

The header provides least-width and fast-width aliases such as:

```cpp
float_least16_t
float_least32_t
float_least64_t
float_least80_t
float_least128_t

float_fast16_t
float_fast32_t
float_fast64_t
float_fast80_t
float_fast128_t
```

`float_least80_t` uses `float80_t` when available, and otherwise uses `float128_t` when that is available.

---

## Maximum Floating-Point Alias

```cpp
floatmax_t
```

`floatmax_t` is selected from the widest practical binary floating-point type available to XER.

---

## Decimal Floating-Point Aliases

When the implementation provides `<decimal/decimal>`, XER exposes decimal floating-point aliases such as:

```cpp
decimal32_t
decimal64_t
decimal128_t

decimal_least32_t
decimal_least64_t
decimal_least128_t

decimal_fast32_t
decimal_fast64_t
decimal_fast128_t

decimalmax_t
```

These aliases are optional and should be guarded with the corresponding `XER_HAS_DECIMAL...` macros.

---

## Floating-Point Literals

Floating-point user-defined literals are placed under:

```cpp
xer::literals::floating_literals
```

Examples include:

```cpp
_f32
_f64
_f80
_f128
_fl16
_fl32
_fl64
_fl80
_fl128
_bf16
```

Only literals whose destination type is available are provided.

---

## Notes

- This header is intentionally capability-based.
- Optional types are not promised on every compiler or target.
- Code that depends on optional formats should check the corresponding availability macro.
- The least-width literal suffixes are useful when the exact underlying available type may vary by platform.

---

# `<xer/arithmetic.h>`

## Purpose

`<xer/arithmetic.h>` provides arithmetic and comparison helper functions in XER.

Its purpose is not merely to wrap built-in operators with different names.
Instead, it provides a numeric utility layer designed to avoid common problems of ordinary C++ arithmetic, especially in cases such as:

- mixing signed and unsigned integer types
- integrating arithmetic with explicit failure handling
- making range failure visible
- expressing comparisons in a way that follows XER's own numeric rules

This header is therefore a central part of XER's numeric design.

---

## Main Role

The main role of `<xer/arithmetic.h>` is to provide arithmetic and comparison operations that are:

- explicit
- predictable
- easier to chain safely than raw built-in operators
- better aligned with XER's error model

In particular, it exists to make the following easier:

- mixed-type integer arithmetic without surprising implicit conversions
- explicit range-aware arithmetic
- explicit comparison helpers for use in generic code
- utility functions such as `min`, `max`, `clamp`, and `in_range`

---

## Main Function Groups

At a high level, `<xer/arithmetic.h>` contains the following groups of functionality:

- integer arithmetic helpers
- comparison helpers
- range and bounds helpers
- absolute-value helpers
- floating-point and complex-number support within the same general design

---

## Integer Arithmetic Helpers

At minimum, this header provides the following integer-oriented arithmetic helpers:

```cpp
add
uadd
sub
usub
mul
umul
div
udiv
mod
umod
```

### Role of This Group

These functions provide arithmetic operations whose behavior is designed explicitly rather than inherited automatically from C++'s usual arithmetic conversions.

This is especially important when signed and unsigned integer types are mixed.

### Design Direction

The design goals of this group include:

* allow mixed signed/unsigned input where that is mathematically meaningful
* return explicit failure when the result does not fit in the target result domain
* avoid silent narrowing or surprising wraparound
* make division and remainder behavior explicit

---

## `add`, `sub`, and `mul`

### Signed-Domain Variants

The following helpers conceptually operate in the signed result domain:

```cpp
add
sub
mul
```

These functions generally return:

```cpp
xer::result<std::int64_t>
```

when operating on integer values.

### Meaning

* `add(a, b)` performs addition
* `sub(a, b)` performs subtraction
* `mul(a, b)` performs multiplication

### Error Handling

If the result cannot be represented in the intended signed result domain, these functions return failure.

This makes overflow and out-of-range situations explicit.

---

## `uadd`, `usub`, and `umul`

### Unsigned-Domain Variants

The following helpers conceptually operate in the unsigned result domain:

```cpp
uadd
usub
umul
```

These functions generally return:

```cpp
xer::result<std::uint64_t>
```

when operating on integer values.

### Meaning

* `uadd(a, b)` performs addition in the unsigned result domain
* `usub(a, b)` performs subtraction in the unsigned result domain
* `umul(a, b)` performs multiplication in the unsigned result domain

### Error Handling

If the mathematically selected result cannot be represented in the intended unsigned result domain, these functions return failure.

For example, a negative result is an error for `usub` or `umul`.

---

## `div`, `udiv`, `mod`, and `umod`

This header also provides division and remainder helpers:

```cpp
div
udiv
mod
umod
```

### `div`

`div` performs division in the signed result domain.

For integer input:

* the quotient is rounded toward zero
* division by zero is an error
* out-of-range results are errors
* a remainder output may also be supported

### `udiv`

`udiv` performs division in the unsigned result domain.

For integer input:

* division by zero is an error
* out-of-range results are errors
* a remainder output may also be supported

### `mod`

`mod` returns the signed remainder according to the same rule family as `div`.

### `umod`

`umod` returns the unsigned remainder according to the same rule family as `udiv`.

### Why These Matter

These helpers make quotient/remainder behavior explicit and keep it aligned with XER's own arithmetic policy rather than leaving everything to built-in operator behavior.

---

## Comparison Helpers

At minimum, `<xer/arithmetic.h>` provides the following comparison helpers:

```cpp
eq
ne
lt
le
gt
ge
```

### Role of This Group

These functions provide explicit comparison in a way that fits XER's numeric rules.

They are especially useful when:

* mixed integer types are involved
* generic code should not depend directly on built-in operator behavior
* the project wants one consistent comparison layer across arithmetic helpers

### Return Type

Comparison helpers return:

```cpp
bool
```

They do **not** return `xer::result<bool>` in the ordinary design.

### Why `xer::result<bool>` Is Not Used

Returning `xer::result<bool>` from ordinary comparison helpers would make conditional use too awkward and too easy to misuse.

For example, code such as:

```cpp
if (eq(a, b)) {
    ...
}
```

should remain straightforward.

For that reason, comparison helpers use `bool` and instead restrict their intended argument domain appropriately.

---

## Range and Bounds Helpers

This header also provides utility helpers such as:

```cpp
in_range
min
max
clamp
```

### `in_range`

`in_range<T>(value)` checks whether `value` can be represented as type `T`.

Its role is to make explicit range-checking available in ordinary code.

This is especially important before conversion or when generic code works across multiple numeric types.

### `min` and `max`

`min` and `max` return the smaller or larger of two values according to XER's comparison rules.

They are not intended to be mere clones of the standard library forms.
Instead, they are designed for mixed-type use under XER's own numeric policy.

### `clamp`

`clamp(value, lo, hi)` constrains a value to the closed interval `[lo, hi]`.

Its purpose is to provide an explicit and predictable clamping helper that works consistently with XER's comparison model.

---

## Absolute-Value Helpers

At minimum, this header provides:

```cpp
abs
uabs
```

### `abs`

`abs(value)` returns the absolute value in the signed-domain design.

For integer input, it generally returns:

```cpp
xer::result<std::int64_t>
```

and reports failure if the result cannot be represented.

### `uabs`

`uabs(value)` returns the nonnegative absolute value in the unsigned-domain design.

For integer input, it generally returns:

```cpp
xer::result<std::uint64_t>
```

and reports failure if the result cannot be represented.

### Why These Helpers Matter

These helpers are important because even "simple" absolute-value operations can fail in fixed-width signed integer domains.

XER therefore makes that failure explicit.

---

## Acceptance of `xer::result`

One of the most important design points of `<xer/arithmetic.h>` is that arithmetic helpers may accept `xer::result` arguments.

### Why This Header Is Special

In the general XER API policy, ordinary public APIs are not supposed to take `xer::result` as an argument.

However, `<xer/arithmetic.h>` is the main exception.

This is because arithmetic chaining has clear value when intermediate failures should propagate naturally.

### Meaning

If an arithmetic helper receives a `xer::result` argument:

* if it contains a success value, that value is used
* if it contains an error, that error is propagated

This makes it easier to write chained arithmetic expressions without manually unwrapping every intermediate step.

### Important Boundary

This exception is specific to the arithmetic area.
It should not be treated as the default design for ordinary public APIs elsewhere in the library.

---

## Floating-Point Support

`<xer/arithmetic.h>` also covers floating-point arithmetic within the same general design.

### General Direction

For floating-point input:

* results may be represented in `long double`
* explicit failure handling is still used where appropriate
* non-computable cases may be treated as failure

### Why This Matters

This allows XER arithmetic helpers to remain usable across both integer and floating-point code while keeping a unified design direction.

---

## Complex-Number Support

Within a reasonable scope, this header may also support arithmetic on complex values.

### General Direction

For complex-number input:

* addition, subtraction, multiplication, and division may be supported
* the result type may be based on `std::complex<long double>`
* comparison operations are generally not provided

### Why Comparison Is Different

Order comparison is not part of the ordinary complex-number model.
For that reason, the comparison helper family does not extend mechanically to complex numbers.

---

## Relationship Between Arithmetic and Comparison

A central design principle of this header is that arithmetic helpers should use XER's comparison helpers internally where ordering matters.

### Meaning

When arithmetic utilities such as:

* `min`
* `max`
* `clamp`

need ordering, they should use `xer::lt` and related helpers rather than relying directly on built-in `<`.

### Why This Matters

This keeps the numeric model internally consistent, especially for mixed signed/unsigned cases.

---

## Relationship to Other Headers

`<xer/arithmetic.h>` should be understood together with:

* `policy_project_outline.md`
* `policy_arithmetic.md`
* `policy_result_arguments.md`
* `header_stdint.md`
* `header_error.md`

The rough boundary is:

* `<xer/stdint.h>` provides integer types, limits/helpers, and typed integer literals
* `<xer/error.h>` provides `xer::result` and error machinery
* `<xer/arithmetic.h>` provides arithmetic and comparison operations built on top of those foundations

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that it provides explicit arithmetic and comparison helpers
* that mixed integer types are an important design target
* that ordinary failure is represented explicitly through `xer::result`
* that this header is the main public exception to the general "no `xer::result` arguments" rule

Detailed per-function numeric rules belong in the detailed reference or generated API sections.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* adding signed and unsigned integers with `add`
* propagating failure through chained arithmetic helpers
* checking representability with `in_range`
* using `min`, `max`, or `clamp` with mixed numeric types
* using `abs` or `uabs` with explicit result checking

These are good candidates for executable examples in `examples/`.

---

## Example

```cpp
#include <xer/arithmetic.h>

auto main() -> int
{
    const auto sum = xer::add(10u, -3);
    if (!sum.has_value()) {
        return 1;
    }

    const auto limited = xer::clamp(*sum, -5, 5);
    if (!limited.has_value()) {
        return 1;
    }

    if (*limited != 5) {
        return 1;
    }

    return 0;
}
```

This example shows the normal XER style:

* use explicit arithmetic helpers instead of raw operators where policy matters
* check `xer::result` explicitly
* use utility helpers such as `clamp` in the same model

---

## See Also

* `policy_project_outline.md`
* `policy_arithmetic.md`
* `policy_result_arguments.md`
* `header_stdint.md`
* `header_error.md`

---

# `<xer/cyclic.h>`

## Purpose

`<xer/cyclic.h>` provides the `cyclic` type and related helpers for handling circular values in XER.

This header is intended for values such as:

- angles
- phases
- directions
- time-of-day-like circular positions
- other quantities defined relative to one full turn

Its role is not merely to provide modular arithmetic.
Instead, it provides a lightweight value type that makes circular semantics explicit, especially concepts such as clockwise and counterclockwise distance.

---

## Main Role

The main role of `<xer/cyclic.h>` is to provide a compact and explicit model for circular values that:

- are normalized to one full turn
- need wraparound behavior
- benefit from shortest-difference operations
- should expose clockwise and counterclockwise interpretation directly

This makes the header especially useful for code involving:

- angles and rotations
- periodic control values
- UI or graphics direction handling
- other one-turn-based quantities

---

## Main Entities

At minimum, `<xer/cyclic.h>` provides the following entities:

```cpp
template <std::floating_point T>
class cyclic;

template <std::floating_point T>
auto from_degree(T value) noexcept -> cyclic<T>;

template <std::floating_point T>
auto to_degree(cyclic<T> value) noexcept -> T;

template <std::floating_point T>
auto from_radian(T value) noexcept -> cyclic<T>;

template <std::floating_point T>
auto to_radian(cyclic<T> value) noexcept -> T;
```

The exact overload set may grow, but this is the essential public shape.

---

## `cyclic<T>`

`cyclic<T>` is the central type of the header.

It represents a circular value normalized to one full turn.

### Basic Shape

At minimum, the class is expected to have a form like the following:

```cpp id="0r4t03"
template <std::floating_point T>
class cyclic {
public:
    using value_type = T;

    static constexpr T default_epsilon =
        std::numeric_limits<T>::epsilon() * static_cast<T>(16);

    constexpr cyclic() noexcept;
    constexpr explicit cyclic(T value) noexcept;

    constexpr auto value() const noexcept -> T;

    constexpr auto cw(cyclic to) const noexcept -> T;
    constexpr auto ccw(cyclic to) const noexcept -> T;
    constexpr auto diff(cyclic to) const noexcept -> T;

    constexpr auto eq(cyclic to) const noexcept -> bool;
    constexpr auto ne(cyclic to) const noexcept -> bool;

    constexpr auto eq(cyclic to, T epsilon) const noexcept -> bool;
    constexpr auto ne(cyclic to, T epsilon) const noexcept -> bool;

    constexpr auto operator+() const noexcept -> cyclic;
    constexpr auto operator-() const noexcept -> cyclic;

    constexpr auto operator+=(cyclic value) noexcept -> cyclic&;
    constexpr auto operator-=(cyclic value) noexcept -> cyclic&;
};
```

This header is therefore centered on one small, value-oriented class template rather than on a large framework.

---

## Internal Representation

`cyclic<T>` uses a normalized internal representation where one full turn is `1`.

### Basic Rule

The stored value always belongs to the half-open interval:

```text
[0, 1)
```

That means:

* `0` is the reference position
* `1` is identified with `0`
* values are normalized after construction and arithmetic updates

### Why This Matters

By using `[0, 1)` internally, the circular nature of the value is separated cleanly from external units such as:

* degrees
* radians
* any other one-turn-based external scale

This makes the type compact and unit-independent.

---

## Supported Value Types

`cyclic<T>` is parameterized by a floating-point type.

The intended template arguments are:

* `float`
* `double`
* `long double`

Integer types are not accepted.

### Why Floating-Point Types

Circular values are naturally modeled as continuous values rather than discrete modular integers in the main intended use cases.

This is especially appropriate for:

* direction control
* angle interpolation
* phase-related processing
* real-time graphics and UI work

---

## Normalization

A `cyclic<T>` object always stores a normalized value.

### Meaning

Conceptually, normalization means mapping an arbitrary value into the interval `[0, 1)`.

Examples:

* `0.3` stays `0.3`
* `1.3` becomes `0.3`
* `-0.2` becomes `0.8`

### Design Direction

The exact implementation is not the public concern.
What matters is the invariant:

```text
0 <= value < 1
```

This invariant is fundamental to all operations provided by the type.

---

## `value()`

```cpp id="ke9w5i"
auto value() const noexcept -> T;
```

### Purpose

`value()` returns the internal normalized representation.

### Meaning

The returned value is always in `[0, 1)`.

This is the raw circular representation used by the type itself.

### Notes

This is not the same thing as a degree or radian value.
Those conversions are handled by separate helper functions.

---

## Clockwise and Counterclockwise Distance

One of the defining features of `cyclic<T>` is that it makes direction along the circle explicit.

At minimum, this is expressed by:

```cpp id="ca9elv"
auto cw(cyclic to) const noexcept -> T;
auto ccw(cyclic to) const noexcept -> T;
```

### `cw`

`cw(to)` returns the clockwise distance from `this` to `to`.

### `ccw`

`ccw(to)` returns the counterclockwise distance from `this` to `to`.

### Range

These distances are returned in the range:

```text
[0, 1)
```

### Why This Matters

This explicit directional model is one of the main reasons `cyclic<T>` exists as its own type rather than simply using a floating-point value with manual modulo arithmetic.

---

## `diff`

```cpp id="j4di47"
auto diff(cyclic to) const noexcept -> T;
```

### Purpose

`diff(to)` returns the shortest signed difference from `this` to `to`.

### Meaning of the Sign

* positive means counterclockwise
* negative means clockwise

### Range

The returned value lies in:

```text
[-0.5, 0.5)
```

If the difference is exactly half a turn, it is normalized to the `-0.5` side.

### Why This Matters

This operation is especially useful in practical code that wants:

* shortest-angle movement
* compact directional difference logic
* comparison on a circle without manual wraparound handling

---

## Equality Testing

`cyclic<T>` does not use strict bitwise equality as its main equality model.

Instead, it provides explicit approximate equality helpers:

```cpp id="v86flg"
auto eq(cyclic to) const noexcept -> bool;
auto ne(cyclic to) const noexcept -> bool;

auto eq(cyclic to, T epsilon) const noexcept -> bool;
auto ne(cyclic to, T epsilon) const noexcept -> bool;
```

### Why `eq` / `ne` Exist

This design makes it clear that equality is tolerance-based rather than strict.

### Why `==` and `!=` Are Not the Main API

If ordinary comparison operators were used for approximate equality, it would be too easy to misread them as strict equality.

XER therefore prefers explicit named functions.

### Default Tolerance

The default tolerance is stored in:

```cpp id="tvnkmz"
static constexpr T default_epsilon;
```

This provides a practical default width appropriate to the floating-point type.

---

## Arithmetic Operators

At minimum, `cyclic<T>` may provide the following operators:

* unary `+`
* unary `-`
* binary `+`
* binary `-`
* `+=`
* `-=`

### Meaning

These operators are interpreted as arithmetic on a circle.

This means:

* results are always normalized back into `[0, 1)`
* addition means moving forward around the circle
* subtraction means moving backward around the circle

### Important Note

These are not ordinary real-number operators in the abstract mathematical sense.
They are circular operations defined by the type's normalization rule.

---

## Comparison Operators Not Provided

Order-comparison operators such as:

* `<`
* `<=`
* `>`
* `>=`
* `<=>`

are not part of the intended model.

### Why

Order comparison is not intrinsic to circular values in the same way it is for ordinary real numbers.

Similarly, `==` and `!=` are not the preferred public equality model because approximate comparison is the intended design.

---

## Unit Conversion Helpers

`<xer/cyclic.h>` provides free functions for conversion to and from ordinary angular units.

At minimum:

```cpp id="rwmkpt"
template <std::floating_point T>
auto from_degree(T value) noexcept -> cyclic<T>;

template <std::floating_point T>
auto to_degree(cyclic<T> value) noexcept -> T;

template <std::floating_point T>
auto from_radian(T value) noexcept -> cyclic<T>;

template <std::floating_point T>
auto to_radian(cyclic<T> value) noexcept -> T;
```

### Why Free Functions

Unit conversion is not treated as the responsibility of the `cyclic` object itself.

This keeps the type unitless internally while allowing conversion at the API boundary.

### Meaning

These functions translate between:

* external degree/radian values
* the internal one-turn-based representation

---

## Relationship to Mathematical Constants

Radian conversion naturally depends on π.

In XER's design, mathematical constants such as π are not embedded directly into `cyclic<T>` as members.
Instead, they are treated as separate supporting facilities, conceptually associated with dedicated internal constant support.

This keeps `cyclic<T>` itself focused on circular value handling rather than on general constant provision.

---

## Relationship to Other Headers

`<xer/cyclic.h>` should be understood together with:

* `policy_project_outline.md`
* `policy_cyclic.md`
* `header_quantity.md`

The rough boundary is:

* `<xer/cyclic.h>` handles circular values and circular operations
* `<xer/quantity.h>` handles physical quantities and units
* angular quantities may be represented as ordinary quantities, while `cyclic` is used when circular semantics are needed explicitly

This distinction is important in XER's design.

---

## Relationship to Angle Quantities

A central point in XER's design is that `cyclic<T>` is **not** the universal storage model for all angle quantities.

### Meaning

* ordinary angle quantities, including turn counts, are better modeled as quantities with units
* `cyclic<T>` is for circular interpretation
* conversion into `cyclic<T>` happens when shortest-difference, clockwise distance, or counterclockwise distance is the real concern

This makes `cyclic<T>` a focused and practical tool rather than a universal replacement for every angle-like value.

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that `cyclic<T>` stores values normalized to one turn
* that clockwise and counterclockwise distance are explicit operations
* that shortest signed difference is provided by `diff`
* that equality is approximate and expressed by `eq` / `ne`
* that degree/radian conversion is handled by free functions

Detailed numeric edge cases belong in the detailed reference or generated API sections.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* constructing a `cyclic<float>` from a raw turn-based value
* converting from degrees with `from_degree`
* converting to degrees with `to_degree`
* measuring clockwise and counterclockwise distance
* computing the shortest difference with `diff`
* comparing values with `eq`

These are good candidates for executable examples in `examples/`.

---

## Example

```cpp
#include <xer/cyclic.h>

auto main() -> int
{
    const auto a = xer::from_degree(36.0);
    const auto b = xer::from_degree(108.0);

    const auto d = a.diff(b);
    if (d <= 0.0) {
        return 1;
    }

    const auto deg = xer::to_degree(a);
    if (deg != 36.0) {
        return 1;
    }

    return 0;
}
```

This example shows the normal XER style:

* create circular values through free conversion helpers
* use circular operations explicitly
* treat direction on the circle as part of the API itself

---

## See Also

* `policy_project_outline.md`
* `policy_cyclic.md`
* `header_quantity.md`

---

# `<xer/quantity.h>`

## Purpose

`<xer/quantity.h>` provides physical quantity and unit facilities in XER.

Its purpose is to allow quantities with dimensions to be handled in a type-safe and practical way.
This includes:

- preventing meaningless arithmetic between different dimensions
- making unit conversion explicit
- allowing natural notation with unit objects
- keeping the design lightweight and easy to understand

This header is not intended to reproduce an existing quantity library as it is.
Instead, it follows XER's own design priorities.

---

## Main Role

The main role of `<xer/quantity.h>` is to provide a compact framework for:

- dimensions
- units
- quantities
- practical predefined units under `xer::units`

This makes it possible to write code such as:

```cpp
using namespace xer::units;

auto x = 1.5 * km;
auto t = 2.0 * sec;
auto v = x / t;
```

while preserving dimension safety and explicit conversion rules.

---

## Main Entities

At minimum, `<xer/quantity.h>` provides the following entities:

```cpp
template <int L, int M, int T, int I>
struct dimension;

using dimensionless = dimension<0, 0, 0, 0>;

template <typename Dim, typename Scale = std::ratio<1>>
class unit;

template <std::floating_point T, typename Dim>
class quantity;
```

In addition, it provides predefined unit objects under the `xer::units` namespace.

---

## `dimension`

`dimension` represents the dimension of a physical quantity.

### Basic Shape

```cpp
template <int L, int M, int T, int I>
struct dimension;
```

### Meaning of the Parameters

The template arguments represent exponents of the base dimensions:

* `L`: length
* `M`: mass
* `T`: time
* `I`: electric current

### Examples

Typical examples include:

```cpp
dimension<1, 0, 0, 0>   // length
dimension<0, 1, 0, 0>   // mass
dimension<0, 0, 1, 0>   // time
dimension<0, 0, 0, 1>   // electric current
dimension<1, 0, -1, 0>  // velocity
dimension<1, 1, -2, 0>  // force
```

### Role

`dimension` exists to make dimensional correctness part of the type system.

This prevents invalid combinations such as:

* adding length and time
* comparing mass and electric current directly

---

## `dimensionless`

```cpp
using dimensionless = dimension<0, 0, 0, 0>;
```

### Role

`dimensionless` represents a quantity with no physical dimension.

This is useful for values such as:

* pure ratios
* normalized coefficients
* angular-unit-related scale values when treated dimensionlessly

### Notes

Even dimensionless quantities remain quantities in the type system.
They are not automatically the same thing as raw scalar values.

---

## `quantity<T, Dim>`

`quantity<T, Dim>` is the central value type of the header.

It represents a numeric value together with a dimension.

### Basic Shape

```cpp
template <std::floating_point T, typename Dim>
class quantity;
```

### Role

A `quantity<T, Dim>` stores:

* a numeric value
* a physical dimension

This allows arithmetic to preserve dimensional correctness.

### Stored Value Type

At least in the current design direction, `T` is restricted to floating-point types such as:

* `float`
* `double`
* `long double`

Integer storage is not the primary model.

### Why Floating-Point Storage

This is because:

* unit conversion naturally produces fractional values
* some unit scales are non-rational
* internal normalization to base units is not generally integral
* keeping the design simple is more important than supporting every numeric form initially

---

## Internal Quantity Representation

A `quantity<T, Dim>` stores its value normalized to the base unit system.

### Base Unit System

At minimum, the base dimensions are:

* meter
* kilogram
* second
* ampere

This means the internal system is effectively MKSA.

### Examples

Conceptually:

* `1 km` is stored as `1000 m`
* `1 g` is stored as `0.001 kg`
* `1 msec` is stored as `0.001 sec`

### Why This Matters

Normalizing stored values to base units simplifies:

* arithmetic
* comparison
* conversion between units
* reasoning about mixed units of the same dimension

---

## Construction and Value Retrieval

The intended usage model includes at least the following ideas:

* constructing directly from a base-unit value
* constructing from a scalar multiplied by a unit object
* retrieving the normalized base-unit value
* retrieving the value converted to a specified unit

Typical forms include:

```cpp
auto value() const noexcept -> T;
auto value(unit_type u) const noexcept -> T;
```

The exact signatures may vary, but this is the intended public direction.

### Example

```cpp
using namespace xer::units;

auto x = 1.5 * km;
auto a = x.value();    // base-unit value
auto b = x.value(km);  // value expressed in km
```

---

## Raw Value Access for Dimensionless Quantities

Dimensionless quantities sometimes need to be converted back to raw scalars.

### Design Direction

This should be possible, but it should remain explicit.

### Why Explicit

Implicit conversion from a dimensionless quantity to a raw scalar weakens the type system and makes code less clear.

For that reason, explicit conversion or explicit value retrieval is preferred.

---

## `unit<Dim, Scale>`

`unit<Dim, Scale>` represents a unit.

### Basic Shape

```cpp
template <typename Dim, typename Scale = std::ratio<1>>
class unit;
```

### Role

A unit represents:

* a dimension
* a scale relative to the base unit of that dimension

### Nature of `unit`

The intended design is that `unit` should be primarily type-level information.

That means:

* unit information should ideally be carried by template arguments
* unit objects should remain lightweight
* unnecessary runtime data members should be avoided

In practice, predefined unit objects should behave like empty or near-empty compile-time objects.

---

## Scale Representation

One important design point is that unit scales are **not** limited to rational values.

### Rational-Scale Examples

Many units can be represented naturally with rational scales:

* `mm`
* `cm`
* `km`
* `g`
* `mg`
* `msec`
* `kHz`
* `hPa`

### Non-Rational-Scale Example

Some units, such as `rad` relative to `taurad`, are not naturally representable by a purely rational scale.

### Design Direction

Therefore:

* `std::ratio`-like rational scales are the default
* floating-point-based scale representation may also be necessary for some units
* the template default should not be interpreted as a permanent restriction of the entire design

This point is especially important for understanding `unit<Dim, Scale>` correctly.

---

## Unit Arithmetic

Units may be multiplied and divided.

### Meaning

When units are multiplied or divided:

* dimensions are combined
* scales are combined

This allows natural construction of derived quantities such as:

```cpp
using namespace xer::units;

auto v = 3.0 * m / sec;
auto a = 9.8 * m / (sec * sec);
auto f = 2.0 * kg * m / (sec * sec);
```

### Why This Matters

This avoids the need to define every composite unit as a separate fixed name.

---

## `xer::units`

Predefined unit objects are provided under the `xer::units` namespace.

### Role

This namespace groups common unit names in one predictable place.

### Basic Direction

Units are intentionally **not** placed directly under `xer`.

This makes it possible to write:

```cpp
using namespace xer::units;
```

only where needed, without polluting the main namespace.

### Examples of Base Units

At minimum, the following base units are provided:

```cpp
xer::units::m
xer::units::kg
xer::units::sec
xer::units::A
```

---

## Predefined Units

The header is expected to provide a practical set of common units.

### Base Units

At minimum:

* `m`
* `kg`
* `sec`
* `A`

### Selected Prefixed Units

Examples include:

* `mm`
* `cm`
* `km`
* `microm`
* `nm`
* `g`
* `mg`
* `nsec`
* `microsec`
* `msec`
* `mA`
* `kHz`
* `GHz`
* `hPa`

### Selected Derived Units

Examples include:

* `Hz`
* `N`
* `J`
* `W`
* `V`
* `Pa`

### Conventional Units

Examples include:

* `ha`
* `mL`
* `dL`
* `L`
* `kL`
* `cal`
* `kcal`

### Aliases

Examples may include:

* `μm`
* `μsec`
* `cc`

The exact set belongs to the detailed unit reference, but these are the main intended categories.

---

## Angular Units

`<xer/quantity.h>` also covers angle-related units in coordination with the broader XER design.

### Important Units

At minimum:

* `taurad`
* `τrad`
* `rad`

### Design Meaning

* `taurad` is the base unit for angle
* `τrad` is an alias of `taurad`
* `rad` is treated as a dimensionless unit for angle

### Why This Matters

This keeps angle quantities compatible with XER's design where one full turn corresponds naturally to the `cyclic` model.

---

## Relationship to `cyclic`

A key design point is that angle quantities and circular values are **not identical concepts**.

### Quantity Side

`quantity` handles ordinary angle quantities, including values such as:

* multiple turns
* negative turns
* values used in ordinary arithmetic

### `cyclic` Side

`cyclic` handles explicitly circular semantics such as:

* clockwise distance
* counterclockwise distance
* shortest difference on a circle

### Design Boundary

So the design direction is:

* store ordinary angles as quantities
* convert to `cyclic` when circular behavior is actually needed

This keeps both abstractions focused.

---

## User-Defined Literals

User-defined literals are not the primary notation model here.

### Preferred Style

The intended style is:

```cpp
1.23f * msec
```

rather than unit suffix literals.

### Why

This keeps the notation:

* explicit
* easy to read
* consistent with the rest of XER
* easier to extend without creating many special literal forms

---

## Relationship to Other Headers

`<xer/quantity.h>` should be understood together with:

* `policy_project_outline.md`
* `policy_quantity.md`
* `header_cyclic.md`
* `header_arithmetic.md`

The rough boundary is:

* `<xer/quantity.h>` handles units, dimensions, and quantities
* `<xer/cyclic.h>` handles explicitly circular values
* `<xer/arithmetic.h>` handles general arithmetic/comparison helpers not specific to physical dimensions

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that it provides dimensions, units, and quantities
* that quantities are normalized internally to base units
* that unit objects live under `xer::units`
* that scale is not conceptually restricted to rational representation only
* that ordinary angular quantities and `cyclic` values are distinct concepts

Detailed unit catalogs and per-operator rules belong in the detailed reference or generated API sections.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* constructing a quantity from a scalar and a unit
* converting a quantity to base units and to another unit
* dividing distance by time to obtain velocity
* using predefined units from `xer::units`
* handling angle quantities with `taurad` or `rad`

These are good candidates for executable examples in `examples/`.

---

## Example

```cpp
#include <xer/quantity.h>

using namespace xer::units;

auto main() -> int
{
    const auto distance = 1.5 * km;
    const auto seconds = 30.0 * sec;
    const auto speed = distance / seconds;

    const auto meters = distance.value();
    if (meters <= 0.0) {
        return 1;
    }

    const auto kilometers = distance.value(km);
    if (kilometers != 1.5) {
        return 1;
    }

    static_cast<void>(speed);
    return 0;
}
```

This example shows the normal XER style:

* use unit objects from `xer::units`
* construct quantities with scalar × unit
* retrieve values explicitly
* keep dimensional meaning in the type system

---

## See Also

* `policy_project_outline.md`
* `policy_quantity.md`
* `header_cyclic.md`
* `header_arithmetic.md`

---

# `<xer/process.h>`

## Purpose

`<xer/process.h>` provides child process management facilities.

The initial API is deliberately small and focuses on direct process spawning, waiting, and standard stream wiring.

---

## Main Role

This header provides:

- a move-only process handle
- direct child process spawning without a command shell
- standard input, standard output, and standard error configuration
- optional pipes exposed as `binary_stream` objects
- process waiting and exit-code retrieval

---

## Main Types

```cpp
enum class process_stdio;
struct process_options;
struct process_result;
class process;
struct process_spawn_result;
```

### `process_stdio`

```cpp
inherit
null
pipe
```

- `inherit` connects the child stream to the corresponding parent stream.
- `null` connects the child stream to the platform null device.
- `pipe` creates a parent-side pipe represented as a `binary_stream`.

### `process_options`

```cpp
path program;
std::vector<std::u8string> arguments;
process_stdio stdin_mode;
process_stdio stdout_mode;
process_stdio stderr_mode;
```

`arguments` excludes `argv[0]`; the program path is supplied separately.

### `process_result`

```cpp
int exit_code;
```

On POSIX, signal termination is represented as `128 + signal_number`.

### `process_spawn_result`

```cpp
process proc;
std::optional<binary_stream> stdin_stream;
std::optional<binary_stream> stdout_stream;
std::optional<binary_stream> stderr_stream;
```

The optional streams are present only when the corresponding `process_stdio::pipe` mode is requested.

---

## Main Functions

```cpp
auto process_spawn(const process_options& options) noexcept -> xer::result<process_spawn_result>;
auto process_wait(process& value) noexcept -> xer::result<process_result>;
```

`process_spawn` executes the target program directly and passes arguments as separate command-line arguments.
It does not invoke a command shell.

`process_wait` waits for the child process and returns its exit status.

---

## Process Handle

`process` is a move-only handle type.

```cpp
auto is_open() const noexcept -> bool;
```

The destructor releases the native handle owned by the object, but it does not wait for process termination.
Call `process_wait` explicitly when the exit code is needed.

---

## Notes

- Paths use `xer::path` and native path conversion internally.
- Arguments use UTF-8 `std::u8string` values.
- On Windows, command-line quoting is performed internally for direct process creation.
- On POSIX, the child process is created using the platform process facilities and then executed directly.
- Pipe streams are binary streams. Higher-level text handling can be layered separately if needed.

---

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

---

# `<xer/version.h>`

## Purpose

`<xer/version.h>` provides compile-time version information for XER.

Its purpose is to make the library version available in a simple and explicit form for:

- source-level conditional handling
- diagnostics and reporting
- build-time checks
- generated documentation and example output when needed

This header is intentionally small and focused.

---

## Main Role

The main role of `<xer/version.h>` is to provide a stable public way to query the version of the XER library.

In particular, it exists to make the following easy:

- checking the major, minor, and patch version numbers
- checking the current suffix such as an alpha marker
- retrieving the complete version string
- keeping version information available both as macros and as inline constants

This makes the header useful both in preprocessor contexts and in ordinary C++ code.

---

## Main Entities

At minimum, `<xer/version.h>` provides entities such as the following:

```cpp
#define XER_VERSION_MAJOR <major>
#define XER_VERSION_MINOR <minor>
#define XER_VERSION_PATCH <patch>
#define XER_VERSION_SUFFIX "<suffix>"
#define XER_VERSION_STRING "<version>"

inline constexpr int version_major;
inline constexpr int version_minor;
inline constexpr int version_patch;
inline constexpr std::string_view version_suffix;
inline constexpr std::string_view version_string;
```

The exact values naturally change from release to release, but this is the intended public shape.

---

## Preprocessor Macros

`<xer/version.h>` provides preprocessor macros for version information.

### Purpose

The macro form exists for cases where version handling must happen before ordinary C++ constant evaluation or where preprocessor-based branching is desired.

### Main Macros

At minimum, the following macros are expected:

```cpp id="gkdm5v"
XER_VERSION_MAJOR
XER_VERSION_MINOR
XER_VERSION_PATCH
XER_VERSION_SUFFIX
XER_VERSION_STRING
```

### Meaning

* `XER_VERSION_MAJOR`: major version number
* `XER_VERSION_MINOR`: minor version number
* `XER_VERSION_PATCH`: patch version number
* `XER_VERSION_SUFFIX`: suffix such as `a5`
* `XER_VERSION_STRING`: full version string such as `0.2.0a1`

### Notes

These macros are useful in contexts such as:

* conditional compilation
* generated banners
* simple compile-time checks
* embedding version information into other generated artifacts

---

## Inline Constants

`<xer/version.h>` also provides inline C++ constants corresponding to the macro-level version information.

### Purpose

The inline constant form exists so that ordinary C++ code can use version information without needing to rely on macros.

### Main Constants

At minimum, the following constants are expected:

```cpp id="0t63o7"
inline constexpr int version_major;
inline constexpr int version_minor;
inline constexpr int version_patch;
inline constexpr std::string_view version_suffix;
inline constexpr std::string_view version_string;
```

### Meaning

These constants correspond directly to the macro values, but in a form more natural for C++ code.

### Notes

This dual provision of macros and constants is useful because:

* macros remain practical for preprocessing
* constants are better suited to type-safe ordinary code

---

## Version Components

The XER version is divided into several components.

### Major Version

The major version represents a large-scale compatibility or release-line change.

### Minor Version

The minor version represents smaller feature-line progression within the same major line.

### Patch Version

The patch version represents smaller corrective or maintenance progression.

### Suffix

The suffix represents additional release-state information such as alpha-stage notation.

For example:

```text id="uh0aq6"
0.2.0a1
```

may be interpreted as:

* major: `0`
* minor: `1`
* patch: `0`
* suffix: `a5`

### Full Version String

The full version string combines these elements into a single human-readable version identifier.

---

## Design Direction

`<xer/version.h>` is intentionally simple.

### Why It Is Small

Version information is important, but it should remain easy to understand and easy to consume.

This means the header should avoid:

* excessive abstraction
* overly clever compile-time metaprogramming
* unnecessarily complicated parsing facilities

The main goal is simply to expose the library version clearly.

---

## Intended Uses

Typical uses of `<xer/version.h>` include:

* displaying the current XER version in logs or diagnostics
* embedding the version into generated documentation
* checking the library version in test code
* simple conditional handling in source code

### Example Situations

It is especially natural to use this header in:

* documentation generation scripts or related examples
* test output
* release verification code
* feature-gating code when version distinctions matter

---

## Relationship to Documentation

`<xer/version.h>` is especially relevant to generated documentation.

### Why

The reference manual already records the target version explicitly.
For example, if the reference manual records a specific target version, `<xer/version.h>` is the natural source of truth for keeping that information consistent. 

### Design Direction

In the long term, generated documentation should ideally remain aligned with the actual contents of `<xer/version.h>` so that version drift is minimized.

---

## Relationship to Other Headers

`<xer/version.h>` is largely independent from the rest of the public headers.

However, it should be understood together with:

* `policy_project_outline.md`
* `public_headers.md`

The rough relationship is:

* `<xer/version.h>` provides version metadata
* other headers provide functional library facilities

This header does not define behavior of the rest of the library, but it identifies the release state of that library.

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that it provides compile-time version information
* that both macro and inline-constant forms are available
* that the suffix is part of the public version model
* that `version_string` / `XER_VERSION_STRING` provide the full human-readable version

Detailed release-policy interpretation belongs in release notes or higher-level project documentation rather than in the per-header summary.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* reading `version_string`
* checking `version_major`
* printing the current version
* comparing against a known release-line expectation in test code

These are good candidates for executable examples in `examples/`.

---

## Example

```cpp id="yrg8u0"
#include <xer/version.h>

auto main() -> int
{
    if (xer::version_major < 0) {
        return 1;
    }

    if (xer::version_string.empty()) {
        return 1;
    }

    return 0;
}
```

This example shows the normal style:

* include the version header directly
* use the inline constants in ordinary C++ code
* treat the header as a simple source of library metadata

---

## See Also

* `policy_project_outline.md`
* `public_headers.md`
