# XER Reference Manual

Target version: **v0.3.0a4**

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

# `<xer/diag.h>`

## Purpose

`<xer/diag.h>` provides lightweight diagnostic facilities for XER.

It groups tracing and logging support under one public diagnostic header while keeping the shared category and level vocabulary common to both facilities.

---

## Main Entities

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

## Design Role

This header is intended for diagnostics, development-time tracing, and simple runtime logging.

Tracing and logging share:

* `diag_category`
* `diag_level_t`
* the named level constants

Their output destinations and current levels are configured independently.

---

## Trace

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

## Log

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

## Output Streams

Trace and log output default to the standard error text stream.
They can be changed independently through the corresponding stream-setting functions.

---

## Notes

These facilities are intentionally small.
They are not a full logging framework, but they provide a useful diagnostic foundation for XER itself and for small programs using XER.

---

# `<xer/scope.h>`

## Purpose

`<xer/scope.h>` provides small scope-based utility facilities for XER.

At present, this header provides `xer::scope_exit`, a lightweight scope guard that invokes a registered callable when the guard object is destroyed.

This is useful for cleanup actions that should be performed automatically when leaving a block.

---

## Main Entity

At minimum, `<xer/scope.h>` provides the following entity:

```cpp
template <class F>
class scope_exit;
```

It also provides a deduction guide so that ordinary lambdas and function objects can be passed naturally:

```cpp
template <class F>
scope_exit(F&&) -> scope_exit<std::decay_t<F>>;
```

---

## `scope_exit`

`scope_exit` is a move-only RAII helper.

It stores a callable object and invokes it from the destructor while the guard is active.

```cpp
auto guard = xer::scope_exit([&] noexcept {
    cleanup();
});
```

### Basic Behavior

A `scope_exit` object is active immediately after construction.

When the object is destroyed:

* if it is active, the registered callable is invoked
* if it has been released, nothing is invoked

This makes it suitable for registering cleanup logic close to the resource or state change that needs cleanup.

---

## Typical Use Cases

`scope_exit` is useful for actions such as:

* restoring the current working directory after `chdir`
* removing a temporary file or temporary directory
* unlocking a resource
* restoring a global or process-wide setting
* rolling back a local state change
* closing or releasing a non-RAII resource when no better wrapper exists

It is especially useful when cleanup must happen along multiple return paths.

---

## Move-Only Semantics

`scope_exit` is copy-disabled and move-constructible.

Copying is not allowed because copying a guard would make it unclear which object owns the cleanup action.

Move construction transfers the cleanup responsibility to the new object.
The moved-from guard is released so that the registered callable is not invoked twice.

Move assignment is not provided.

---

## Releasing a Guard

A guard can be released explicitly:

```cpp
guard.release();
```

After `release()` is called, the registered callable will not be invoked when the guard is destroyed.

This is useful when the cleanup action is only needed if a later operation does not complete successfully.

---

## Checking Whether a Guard Is Active

The current active state can be checked:

```cpp
auto active() const noexcept -> bool;
```

This returns `true` if the callable will be invoked on destruction.

---

## Exception Policy

The destructor of `scope_exit` is `noexcept`.

The registered callable is expected not to throw.

If the callable throws while the `scope_exit` destructor is running, `std::terminate` is called.

This policy follows the usual expectation for cleanup code executed from destructors.
A scope guard is not a good place to report ordinary recoverable failure, because destructors cannot return `xer::result`.

If the cleanup action can fail in a meaningful way, callers should normally handle that failure explicitly before leaving the scope, or intentionally ignore it when failure is not actionable.

---

## Relationship to Standard and Experimental Facilities

XER provides its own `scope_exit`.

This is not a wrapper around a standard C++ `<scope>` header.
Standard C++ does not currently provide such a header as part of the ordinary standard library.

Some similar facilities exist in experimental or library-extension contexts, but XER keeps this utility small and self-contained so that it fits the library's header-only and GCC-oriented portability policy.

---

## Design Role

`scope_exit` is intentionally small.

It does not attempt to provide a complete scope-guard framework.

In particular, this header does not currently provide:

* `scope_success`
* `scope_fail`
* `unique_resource`

The initial purpose is only to support the common “run this cleanup action when leaving the current scope” pattern.

---

## Relationship to XER's Error Model

XER generally represents ordinary fallible operations through `xer::result`.

However, `scope_exit` itself does not return a result from its destructor.

For that reason, cleanup actions registered with `scope_exit` should normally be actions whose failure can be safely ignored or actions that are known not to fail in the intended context.

A common pattern is to explicitly discard the result of a cleanup operation:

```cpp
auto guard = xer::scope_exit([&] noexcept {
    static_cast<void>(xer::chdir(original));
});
```

This makes the decision to ignore cleanup failure visible.

---

## Example

```cpp
#include <xer/scope.h>
#include <xer/stdio.h>

auto main() -> int
{
    const auto original = xer::getcwd();
    if (!original.has_value()) {
        return 1;
    }

    {
        auto restore = xer::scope_exit([&] noexcept {
            static_cast<void>(xer::chdir(*original));
        });

        if (!xer::chdir(xer::path(u8"work")).has_value()) {
            return 1;
        }

        // Work inside another current directory.
    }

    return 0;
}
```

This example shows the typical XER style:

* acquire or record state explicitly
* register cleanup with `scope_exit`
* use `noexcept` for the cleanup lambda when practical
* explicitly discard cleanup results when failure is not actionable

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that `scope_exit` is a move-only scope guard
* that it calls the registered callable from its destructor
* that `release()` disables the cleanup action
* that the registered callable should not throw
* that `scope_success` and `scope_fail` are intentionally not provided at this stage

Detailed examples should focus on practical cleanup patterns rather than abstract RAII theory.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* restoring the current working directory after `chdir`
* deleting a temporary file on scope exit
* releasing a guard after successful completion
* moving a guard to transfer cleanup responsibility

These are good candidates for executable examples under `examples/`.

---

## See Also

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `header_stdio.md`

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
- string replacement
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

## String Replacement

`<xer/string.h>` provides a PHP-inspired string replacement helper:

```cpp
auto str_replace(
    std::u8string_view search,
    std::u8string_view replace,
    std::u8string_view subject,
    std::size_t* count = nullptr) -> xer::result<std::u8string>;
```

### Role of This Function

`str_replace` replaces all non-overlapping occurrences of `search` in `subject` with `replace`.

The argument order follows PHP's `str_replace` naming tradition: search string, replacement string, and subject string.

### Behavior

Replacement is performed on UTF-8 code units. The function does not attempt grapheme-cluster processing.

If `search` is empty, no replacement is performed and `subject` is returned unchanged.

If `count` is not `nullptr`, the number of performed replacements is stored there.

### Notes

This function is useful for simple text substitution. More advanced text processing, such as regular-expression replacement or locale-sensitive transformation, is outside the scope of this helper.

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
* replacing text with `str_replace`
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

This header provides environment-variable access facilities such as:

```cpp
struct environ_entry;
using environ_arg = std::pair<std::u8string_view, std::u8string_view>;

class environs;

auto getenv(std::u8string_view name) -> xer::result<std::u8string>;
auto get_environs() -> xer::result<environs>;
```

### Role of This Group

This function group provides access to process environment information.

`getenv` obtains one environment variable by name.
`get_environs` obtains a UTF-8 snapshot of all environment variables and returns it as an `environs` object.

### `environs`

`environs` owns environment-variable entries as UTF-8 strings.
It is a snapshot of the process environment at the time `get_environs` is called.
Later changes to the process environment do not update an already-created `environs` object.

At minimum, `environs` provides:

```cpp
auto size() const noexcept -> std::size_t;
auto empty() const noexcept -> bool;
auto entries() const noexcept -> std::span<const environ_entry>;
auto at(std::size_t index) const -> xer::result<environ_arg>;
auto find(std::u8string_view name) const -> xer::result<std::u8string_view>;
```

`at` returns name and value views for one entry.
`find` returns the first value whose name matches the requested name.
If the requested name is empty, `find` fails with `error_t::invalid_argument`.
If the name is not present, it fails with `error_t::not_found`.

### Platform Encoding Policy

On Windows, `get_environs` uses `GetEnvironmentStringsW` so that environment strings are obtained as UTF-16 and converted to UTF-8.

On Linux, `get_environs` reads the process environment array and requires each name and value to be valid UTF-8.

Entries without `=` and entries whose name part is empty are ignored.

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

---

# `<xer/base64.h>`

## Purpose

`<xer/base64.h>` provides Base64 encode and decode facilities in XER.

Base64 is treated as a small binary-to-text conversion facility. It is not a structured data format like JSON, INI, or TOML, and it is not ordinary string processing either. Its role is to convert binary byte sequences into UTF-8 text that can be embedded in text-based data and to convert that text representation back into bytes.

The initial implementation deliberately supports only the standard Base64 form. Variants such as URL-safe Base64 and unpadded Base64 are deferred.

---

## Main Role

The main role of `<xer/base64.h>` is to make it possible to:

- encode binary data into standard Base64 text
- decode standard Base64 text back into binary data
- handle invalid encoded text through XER's ordinary `xer::result` error model
- provide a compact public API that can be extended later without changing its basic shape

This makes the header useful for simple binary payload handling, text-based interchange, configuration data, diagnostics, and small utility programs.

---

## Main Entities

At minimum, `<xer/base64.h>` provides the following functions:

```cpp
auto base64_encode(std::span<const std::byte> data)
    -> xer::result<std::u8string>;

auto base64_decode(std::u8string_view text)
    -> xer::result<std::vector<std::byte>>;
```

The current API is intentionally small. Additional overloads or options may be added later.

---

## `base64_encode`

```cpp
auto base64_encode(std::span<const std::byte> data)
    -> xer::result<std::u8string>;
```

### Purpose

`base64_encode` converts a binary byte sequence into standard Base64 text.

### Input Model

The input is provided as:

```cpp
std::span<const std::byte>
```

This makes the function explicitly byte-oriented. The input is not interpreted as text and is not validated as UTF-8.

### Output Model

On success, the function returns:

```cpp
std::u8string
```

The output contains only ASCII Base64 characters and is therefore valid UTF-8.

### Return Model

The return type is `xer::result<std::u8string>`.

The current minimal encoder normally has no content-based failure condition. However, the function still returns `xer::result` so that future variants can report ordinary failures without changing the public API shape.

Possible future failure cases include output-size limits, formatting options, stream-oriented output failures, or invalid option combinations.

---

## `base64_decode`

```cpp
auto base64_decode(std::u8string_view text)
    -> xer::result<std::vector<std::byte>>;
```

### Purpose

`base64_decode` converts standard Base64 text back into binary data.

### Input Model

The input is provided as:

```cpp
std::u8string_view
```

Only ASCII Base64 characters, `=`, and ignored ASCII whitespace are meaningful. Non-ASCII input is rejected because it is not part of the supported Base64 alphabet.

### Output Model

On success, the function returns:

```cpp
std::vector<std::byte>
```

The output is binary data. It is not interpreted as text.

---

## Supported Base64 Form

The initial implementation supports standard Base64 with the following alphabet:

```text
ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/
```

Padding uses:

```text
=
```

### Encoding Behavior

`base64_encode` currently behaves as follows:

- emits standard Base64 text
- always emits padding when padding is needed
- does not insert line breaks
- does not insert spaces
- does not support URL-safe output
- does not support unpadded output

Examples:

```text
f      -> Zg==
fo     -> Zm8=
foo    -> Zm9v
foobar -> Zm9vYmFy
```

### Decoding Behavior

`base64_decode` currently behaves as follows:

- accepts the standard Base64 alphabet
- accepts `=` padding only in valid final positions
- ignores ASCII whitespace
- rejects non-Base64 characters
- rejects malformed padding
- rejects inputs whose effective length is not a multiple of four
- rejects non-canonical padding bits

ASCII whitespace means the following characters:

```text
space, tab, LF, CR, FF, VT
```

Whitespace is ignored only while decoding. The encoder does not generate whitespace.

---

## Error Handling

`<xer/base64.h>` follows XER's ordinary failure model.

That means:

- normal failure is reported through `xer::result`
- invalid encoded text is not handled by exceptions
- callers explicitly check whether the returned `xer::result` has a value

The initial decoder reports malformed Base64 input as:

```cpp
error_t::invalid_argument
```

This includes at least the following cases:

- invalid Base64 character
- invalid effective input length
- padding before the final quartet
- malformed padding pattern
- non-canonical unused padding bits

At this stage, detailed error positions are not reported. If position-aware diagnostics become useful later, the error detail type can be extended separately.

---

## Deferred Items and Limitations

The following items are intentionally deferred from the initial implementation.

### URL-Safe Base64

URL-safe Base64, which uses `-` and `_` instead of `+` and `/`, is not supported yet.

A future API may add an option or separate variant for URL-safe Base64.

### Unpadded Base64

Unpadded Base64 is not supported yet.

The current decoder requires the effective input length after whitespace removal to be a multiple of four. Therefore, input that relies on omitted `=` padding is rejected.

A future API may add an option to accept or emit unpadded Base64.

### MIME-Style Line Wrapping on Encode

The encoder does not insert line breaks.

The decoder ignores ASCII whitespace, so it can read many line-wrapped Base64 strings. However, line-wrapped output generation is not provided yet.

A future API may add a line-width option.

### Streaming Encode and Decode

The initial API operates on complete input data and returns complete output data.

Streaming Base64 processing is deferred. This includes direct integration with `binary_stream` and `text_stream`.

### Custom Alphabets

Custom Base64 alphabets are not supported.

Only the standard alphabet is accepted in the initial implementation.

### Detailed Error Positions

The initial decoder reports invalid input as `error_t::invalid_argument`, but it does not report the exact position of the invalid character or malformed padding.

Detailed position information may be added later through `xer::error<Detail>`.

### Text Encoding Conversion

Base64 text itself is ASCII and therefore valid UTF-8, but `<xer/base64.h>` does not perform character encoding conversion.

The input to `base64_decode` is treated as UTF-8-oriented text storage, but only the ASCII Base64 subset is meaningful. Binary output is returned as bytes and is not interpreted as UTF-8.

---

## Relationship to Other Headers

`<xer/base64.h>` is related to the following headers and policies:

- `<xer/error.h>`
- `<xer/stdio.h>`
- `policy_project_outline.md`
- `policy_result_arguments.md`
- `policy_encoding.md`

The rough boundary is:

- `<xer/string.h>` handles general string and memory utilities
- `<xer/stdio.h>` handles stream I/O
- `<xer/json.h>`, `<xer/ini.h>`, and `<xer/toml.h>` handle structured or semi-structured data formats
- `<xer/base64.h>` handles byte-oriented binary-to-text conversion

This separation is intentional. Base64 is useful together with text formats, but it is not itself a structured data format.

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

- that Base64 encoding is byte-oriented
- that encoded output is UTF-8 text containing only ASCII characters
- that decoding returns binary bytes
- that both encode and decode return `xer::result`
- that the initial implementation supports only standard padded Base64
- that URL-safe, unpadded, wrapped-output, and streaming variants are deferred

Detailed option behavior should be added when such options are actually introduced.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

- encoding a small byte sequence into Base64
- decoding the Base64 result back into bytes
- showing explicit `xer::result` checking
- printing encoded text with `<xer/stdio.h>`

These are good candidates for executable examples in `examples/`.

---

## Example

```cpp
#include <array>
#include <cstddef>

#include <xer/base64.h>
#include <xer/stdio.h>

auto main() -> int
{
    const std::array<std::byte, 5> data = {
        std::byte{'h'},
        std::byte{'e'},
        std::byte{'l'},
        std::byte{'l'},
        std::byte{'o'},
    };

    const auto encoded = xer::base64_encode(data);
    if (!encoded.has_value()) {
        return 1;
    }

    if (!xer::printf(u8"Encoded: %@\n", *encoded).has_value()) {
        return 1;
    }

    const auto decoded = xer::base64_decode(*encoded);
    if (!decoded.has_value()) {
        return 1;
    }

    return 0;
}
```

This example shows the general style:

- pass ordinary byte data to `base64_encode`
- check `xer::result` explicitly
- pass the encoded text to `base64_decode`
- treat decoded data as bytes rather than as text unless the caller knows its content

---

## See Also

- `policy_project_outline.md`
- `policy_result_arguments.md`
- `policy_encoding.md`
- `header_error.md`
- `header_stdio.md`

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

auto json_decode(std::u8string_view text) -> xer::result<json_value, parse_error_detail>;
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
auto json_decode(std::u8string_view text) -> xer::result<json_value, parse_error_detail>;
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

## JSON find and load/save helpers

This header also provides helper functions for inspecting decoded JSON values and for loading or saving JSON files.

```cpp
auto json_find(json_value& value, std::u8string_view key) noexcept
    -> json_value*;

auto json_find(const json_value& value, std::u8string_view key) noexcept
    -> const json_value*;

auto json_load(const path& filename)
    -> xer::result<json_value, parse_error_detail>;

auto json_save(const path& filename, const json_value& value)
    -> xer::result<void>;
```

The `json_find` helpers inspect an already-decoded JSON object and search only its direct child entries. They return a pointer to the existing value when the key is found. They return `nullptr` when the searched value is not an object or when the key is not present.

The load helper combines UTF-8 file reading with decoding and returns `xer::result<json_value, parse_error_detail>`. If file I/O fails before parsing begins, the returned error uses `parse_error_reason::none` and leaves `offset`, `line`, and `column` at zero.

The save helper combines encoding with UTF-8 file writing and returns `xer::result<void>`.

---

# `<xer/ini.h>`

## Purpose

`<xer/ini.h>` provides INI decode and encode facilities in XER.

INI is treated as a small configuration-file format rather than as a general structured data language.
The purpose of this header is to support practical reading and writing of simple UTF-8 INI text while preserving the parts of the format that are important for ordinary configuration files.

The initial implementation deliberately supports a small and predictable subset.
It does not try to define every INI dialect.

---

## Main Role

The main role of `<xer/ini.h>` is to make it possible to:

- parse UTF-8 INI text into a simple ordered in-memory representation
- inspect global entries and section entries
- serialize that representation back into UTF-8 INI text
- preserve duplicate keys and duplicate sections instead of silently discarding information

This makes the header useful for simple configuration files and for data that does not require the stricter type system of formats such as TOML.

---

## Main Entities

At minimum, `<xer/ini.h>` provides the following entities:

```cpp
struct ini_entry;
struct ini_section;
struct ini_file;

auto ini_decode(std::u8string_view text) -> xer::result<ini_file, parse_error_detail>;
auto ini_encode(const ini_file& value) -> xer::result<std::u8string>;
````

The exact helper functions may expand later, but these are the core public entities of the initial INI facility.

---

## `ini_entry`

```cpp
struct ini_entry {
    std::u8string key;
    std::u8string value;
};
```

`ini_entry` represents one key-value pair.

INI does not have a single strong standard type system.
For that reason, both keys and values are represented as UTF-8 strings.

### Notes

* keys are stored after ASCII whitespace trimming during decoding
* values are stored after ASCII whitespace trimming during decoding
* duplicate keys are preserved
* the value is not interpreted as a number, boolean, string literal, or other typed value

---

## `ini_section`

```cpp
struct ini_section {
    std::u8string name;
    std::vector<ini_entry> entries;
};
```

`ini_section` represents one section and its entries.

Section names are stored as UTF-8 strings.
The section's entries are stored in source order.

### Notes

* duplicate section names are preserved
* sections are not automatically merged
* section entries preserve their order
* an empty section is valid

---

## `ini_file`

```cpp
struct ini_file {
    std::vector<ini_entry> entries;
    std::vector<ini_section> sections;
};
```

`ini_file` represents a decoded INI document.

Entries before the first section are stored in `entries`.
Sectioned entries are stored in `sections`.

### Why Global Entries Are Separate

Many INI files allow key-value entries before the first section.
XER represents these entries explicitly instead of inventing an artificial section name.

### Preservation Policy

The representation preserves:

* global entry order
* section order
* entry order within each section
* duplicate keys
* duplicate sections

This is intentional.
INI has many dialects, and silently overwriting or merging data can lose information.

---

## Supported INI Subset

The initial implementation supports the following INI elements:

```ini
; comment
# comment

key = value

[section]
name = xer
version = example
```

### Supported Forms

* blank lines
* full-line comments beginning with `;`
* full-line comments beginning with `#`
* global key-value entries
* section headers
* section key-value entries

### Line Endings

The decoder accepts the following line endings:

* LF
* CRLF
* CR

---

## Whitespace Handling

During decoding, leading and trailing ASCII whitespace is removed from:

* each logical line
* section names
* keys
* values

Only ASCII whitespace is treated as trimming whitespace.

This keeps the rule simple and avoids locale dependence.

---

## Comments

Only full-line comments are recognized.

A line is treated as a comment when its first non-trimmed character is either:

```text
;
#
```

Inline comments are not recognized.

For example:

```ini
name = xer ; this remains part of the value
```

is decoded as the value:

```text
xer ; this remains part of the value
```

This rule avoids guessing dialect-specific inline-comment behavior.

---

## Sections

A section line has the following form:

```ini
[section]
```

The section name is trimmed as ASCII whitespace.

For example:

```ini
[ main ]
```

is decoded as the section name:

```text
main
```

### Invalid Section Lines

A section line is invalid if:

* the closing `]` is missing
* the section name is empty after trimming

Such cases are reported as ordinary parse failures.

---

## Entries

An entry line has the following form:

```ini
key=value
```

The first `=` separates the key and value.

For example:

```ini
path = a=b
```

is decoded as:

* key: `path`
* value: `a=b`

### Invalid Entry Lines

An entry line is invalid if:

* it does not contain `=`
* the key is empty after trimming

Such cases are reported as ordinary parse failures.

---

## Quoting and Escaping

The initial INI implementation does not interpret quotes or escape sequences.

For example:

```ini
name = "xer"
```

is decoded as the value:

```text
"xer"
```

including the quote characters.

This is intentional.
INI dialects differ widely in quoting and escaping behavior.
XER keeps the initial INI feature small and predictable, and leaves typed or strongly escaped configuration syntax to formats such as TOML.

---

## `ini_decode`

```cpp
auto ini_decode(std::u8string_view text) -> xer::result<ini_file, parse_error_detail>;
```

### Purpose

`ini_decode` parses UTF-8 INI text and returns an `ini_file`.

### Input Model

The input text is provided as:

```cpp
std::u8string_view
```

This follows XER's UTF-8-oriented public text model.

### Return Model

On success, `ini_decode` returns:

```cpp
ini_file
```

On failure, it returns an error through `xer::result`.

### Failure Conditions

At minimum, decoding fails when:

* the input contains invalid UTF-8
* a section line is malformed
* a section name is empty
* an entry line has no `=`
* an entry key is empty

Invalid UTF-8 is treated as an encoding error.
Malformed INI structure is treated as an invalid argument.

---

## `ini_encode`

```cpp
auto ini_encode(const ini_file& value) -> xer::result<std::u8string>;
```

### Purpose

`ini_encode` serializes an `ini_file` into UTF-8 INI text.

### Output Model

On success, it returns:

```cpp
std::u8string
```

The generated text uses `\n` line endings.

### Serialization Form

The encoder emits a compact deterministic form.

Global entries are written first:

```ini
key=value
```

Sections are then written as:

```ini
[section]
key=value
```

A blank line is inserted before each section when there is previous output.

### Representability

Because the initial INI subset has no quoting or escaping rules, the encoder rejects values that cannot be represented without changing their meaning when decoded again.

For example, the encoder rejects:

* empty keys
* keys containing `=`
* keys with leading or trailing ASCII whitespace
* values with leading or trailing ASCII whitespace
* keys, values, or section names containing line breaks
* section names containing `]`
* invalid UTF-8

This keeps `ini_encode` honest and avoids silently emitting ambiguous INI text.

---

## Error Handling

`<xer/ini.h>` follows XER's ordinary failure model.

That means:

* parsing failure is reported through `xer::result`
* serialization failure is reported through `xer::result`
* normal failure is not expressed through exceptions by default

The usual pattern is:

```cpp
const auto decoded = xer::ini_decode(text);
if (!decoded.has_value()) {
    return 1;
}
```

---

## Relationship to Other Headers

`<xer/ini.h>` should be understood together with the following documents and headers:

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `policy_encoding.md`
* `header_string.md`
* `header_json.md`

The rough boundary is:

* `<xer/string.h>` handles general string and text utilities
* `<xer/json.h>` handles JSON as a structured data format
* `<xer/ini.h>` handles INI as a simple configuration data format
* `<xer/toml.h>` handles a stricter typed configuration format

This separation is intentional.
INI is treated as an independent data-format facility rather than as a string helper or stream helper.

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that INI processing uses UTF-8 text
* that INI values are stored as strings
* that global entries and section entries are separated
* that duplicate keys and duplicate sections are preserved
* that only full-line comments are recognized
* that quotes and escapes are not interpreted in the initial implementation

Detailed dialect-specific behavior should not be implied unless it is actually implemented.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* decoding simple INI text
* reading a value from a section
* encoding an `ini_file` back into text
* preserving duplicate keys or duplicate sections where relevant

These are good candidates for executable examples in `examples/`.

---

## Example

```cpp
#include <xer/ini.h>

auto main() -> int
{
    const auto decoded = xer::ini_decode(
        u8"title = sample\n"
        u8"\n"
        u8"[project]\n"
        u8"name = xer\n"
        u8"version = example\n");

    if (!decoded.has_value()) {
        return 1;
    }

    if (decoded->entries.empty()) {
        return 1;
    }

    if (decoded->sections.empty()) {
        return 1;
    }

    const auto encoded = xer::ini_encode(*decoded);
    if (!encoded.has_value()) {
        return 1;
    }

    return 0;
}
```

This example shows the general style:

* parse UTF-8 INI text with `ini_decode`
* inspect the resulting `ini_file`
* serialize it again with `ini_encode`
* check `xer::result` explicitly at each fallible step

---

## See Also

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `policy_encoding.md`
* `header_json.md`

---

## INI find and load/save helpers

This header also provides helper functions for inspecting decoded INI files and for loading or saving INI files.

```cpp
auto ini_find(ini_file& file, std::u8string_view key) noexcept
    -> ini_entry*;

auto ini_find(const ini_file& file, std::u8string_view key) noexcept
    -> const ini_entry*;

auto ini_find(
    ini_file& file,
    std::u8string_view section,
    std::u8string_view key) noexcept -> ini_entry*;

auto ini_find(
    const ini_file& file,
    std::u8string_view section,
    std::u8string_view key) noexcept -> const ini_entry*;

auto ini_load(const path& filename)
    -> xer::result<ini_file, parse_error_detail>;

auto ini_save(const path& filename, const ini_file& value)
    -> xer::result<void>;
```

The two-argument `ini_find` helpers search global entries. The three-argument overloads search the first section whose name matches `section`, and then the first entry whose key matches `key` within that section.

These helpers return pointers to existing entries. They return `nullptr` when the requested item is not present. Because the INI representation preserves duplicate keys and duplicate sections, these helpers return the first matching entry. A future `ini_find_all`-style helper may be added if bulk retrieval becomes necessary.

The load helper combines UTF-8 file reading with decoding and returns `xer::result<ini_file, parse_error_detail>`. If file I/O fails before parsing begins, the returned error uses `parse_error_reason::none` and leaves `offset`, `line`, and `column` at zero.

The save helper combines encoding with UTF-8 file writing and returns `xer::result<void>`.

---

# `<xer/toml.h>`

## Purpose

`<xer/toml.h>` provides TOML decode and encode facilities in XER.

TOML is treated as a typed configuration data format.
The purpose of this header is to support practical reading and writing of simple UTF-8 TOML text while keeping the implementation small enough to fit XER's incremental development policy.

The current implementation supports a practical subset of TOML.
It does not claim complete TOML v1.0.0 compatibility.

---

## Main Role

The main role of `<xer/toml.h>` is to make it possible to:

- parse UTF-8 TOML text into a structured XER value model
- inspect booleans, integers, floating-point numbers, strings, arrays, and tables
- serialize the supported value model back into UTF-8 TOML text
- use TOML as a typed configuration format distinct from INI

This makes the header useful for configuration data that needs more structure than INI but does not require the full TOML feature set at the initial stage.

---

## Main Entities

At minimum, `<xer/toml.h>` provides the following entities:

```cpp
struct toml_value;

using toml_array = std::vector<toml_value>;
using toml_table = std::vector<std::pair<std::u8string, toml_value>>;

auto toml_decode(std::u8string_view text)
    -> xer::result<toml_value, parse_error_detail>;
auto toml_encode(const toml_value& value) -> xer::result<std::u8string>;
````

The exact helper functions and supported TOML syntax may expand later, but these are the core public entities of the initial TOML facility.

---

## `toml_value`

`toml_value` is the central value type for TOML in XER.

It stores one TOML value in structured form.

### Supported Value Kinds

The current implementation supports the following value kinds:

* boolean
* integer
* floating-point number
* string
* local date
* local time
* local date-time
* offset date-time
* array
* table

The internal representation corresponds to:

```cpp
std::variant<
    bool,
    std::int64_t,
    double,
    std::u8string,
    toml_local_date,
    toml_local_time,
    toml_local_datetime,
    toml_offset_datetime,
    toml_array,
    toml_table
>
```

### Notes

* integer values are stored as `std::int64_t`
* floating-point values are stored as `double`
* strings are stored as UTF-8 `std::u8string`
* date/time values are stored in small TOML-specific value structs
* arrays are stored as `std::vector<toml_value>`
* array-of-tables is represented as an array whose elements are tables
* tables are stored as ordered key-value pairs

---

## `toml_array`

```cpp
using toml_array = std::vector<toml_value>;
```

`toml_array` represents an array of TOML values.

The current implementation stores arrays as ordinary ordered vectors.

### Notes

* array elements preserve order
* arrays may contain values of different supported kinds
* arrays may contain table values
* array-of-tables syntax is represented as an array of table values

---

## `toml_table`

```cpp
using toml_table = std::vector<std::pair<std::u8string, toml_value>>;
```

`toml_table` represents a TOML table.

Tables are represented as ordered key-value pairs rather than as a map-like container.

### Why Ordered Pairs Are Used

This representation keeps the value model simple and preserves source order.

It also matches the general XER tendency to avoid prematurely committing public data-format models to hash-map or tree-map semantics.

### Duplicate Keys

TOML does not allow duplicate keys in the same table.

The decoder rejects duplicate keys as invalid input.

---

## Accessors and Inspection

`toml_value` provides inspection and accessor functions.

At minimum, the following inspection functions are provided:

```cpp
is_bool
is_integer
is_float
is_string
is_array
is_table
```

At minimum, the following accessor functions are provided:

```cpp
as_bool
as_integer
as_float
as_string
as_array
as_table
```

The accessor functions return pointers to the stored value when the value has the requested kind, and `nullptr` otherwise.

### Example

```cpp
const auto* table = value.as_table();
if (table == nullptr) {
    return 1;
}
```

This style keeps type inspection explicit and avoids throwing exceptions for ordinary kind checks.

---

## Supported TOML Subset

The current implementation supports the following TOML-style input:

```toml
title = "xer"
enabled = true
count = 123
hex = 0xFF
mask = 0b1010_0101
ratio = 1.5
large = 1_000_000
ports = [8000, 8001, 8002]
point = { x = 1, y = 2 }
items = [{ name = "one" }, { name = "two" }]
released = 2026-04-30
created = 2026-04-30T23:59:58+09:00

[project]
name = "xer"
version = "example"
```

### Supported Forms

The initial decoder supports:

* blank lines
* comments beginning with `#`
* end-of-line comments outside strings
* bare keys
* quoted keys
* dotted keys
* key-value pairs
* ordinary and nested tables
* basic double-quoted strings
* literal strings
* multiline basic and literal strings
* booleans
* signed decimal integers
* hexadecimal, octal, and binary integers
* numeric separators between digits
* finite and special floating-point numbers
* arrays
* inline tables
* local date, local time, local date-time, and offset date-time values
* array-of-tables

### Line Endings

The decoder accepts the following line endings:

* LF
* CRLF
* CR

---

## Deferred TOML Features and Limitations

The current implementation supports a practical subset of TOML, including date/time values and array-of-tables.

The following areas remain intentionally limited or deferred:

* complete TOML v1.0.0 conformance
* preservation of original formatting and comments during encoding
* timezone names beyond TOML offset date-time syntax
* full validation of every semantic rule required by TOML v1.0.0

The implementation should therefore be described as a practical TOML subset, not as a complete TOML implementation.

---

## Keys

The implementation supports bare keys, quoted keys, and dotted keys.

A bare key may contain:

* ASCII letters
* ASCII digits
* `_`
* `-`

Examples:

```toml
name = "xer"
build-target = "ucrt64"
version_1 = "example"
```

Quoted keys use the same single-line basic-string or literal-string syntax as values.

```toml
"site.name" = "xer"
'literal.key' = 10
```

Dotted keys create nested tables implicitly when necessary.

```toml
server.port = 8080
database."connection.pool".size = 4
```

In this representation, each dotted-key segment is stored as an ordinary table key. Quoted segments may contain dots without being split.

---

## Tables

A table line has the following form:

```toml
[project]
```

Nested table names are also supported through dotted key syntax.

```toml
[project.package]
name = "xer"

[project."build.target"]
name = "ucrt64"
```

The table name is parsed as a key path. The table becomes the destination for subsequent key-value entries until another table is declared.

### Notes

* duplicate explicit table declarations are rejected
* tables created implicitly by dotted keys may later be declared explicitly
* array-of-tables syntax such as `[[project]]` is supported and represented as arrays whose elements are tables

---

## Strings

The implementation supports basic strings, literal strings, multiline basic strings, and multiline literal strings.

```toml
name = "xer"
path = 'C:\\Users\\xer'
description = """
first line
second line"""
literal_block = '''
C:\\Users\\xer
'''
```

Basic strings support the following escapes:

```text
\"
\\
\b
\t
\n
\f
\r
\uXXXX
\UXXXXXXXX
```

Literal strings do not process escape sequences. Multiline strings remove the first newline immediately following the opening delimiter, following TOML-style behavior.

---

## Booleans

The following boolean values are supported:

```toml
enabled = true
disabled = false
```

They are stored as `bool`.

---

## Integers

The current implementation supports signed decimal integers and prefixed non-decimal integers. Numeric separators may be used between digits.

```toml
count = 123
offset = -5
positive = +123
large = 1_000_000
hex = 0xFF
octal = 0o755
binary = 0b1010_0101
```

They are stored as `std::int64_t`.

The supported integer prefixes are `0x` for hexadecimal, `0o` for octal, and `0b` for binary. A leading `+` or `-` may be used before the prefix. Separators are valid only between digits of the corresponding base.

---

## Floating-Point Numbers

The implementation supports finite decimal floating-point numbers and TOML special floating-point values. Numeric separators may be used between decimal digits of finite decimal values.

```toml
ratio = 1.5
scale = 1e-3
large = 1_000.25_5
positive = +1.5
negative = -1.5
positive_inf = inf
negative_inf = -inf
not_a_number = nan
```

They are stored as `double`.

Special values `inf`, `+inf`, `-inf`, `nan`, `+nan`, and `-nan` are accepted. Separators are valid only between digits of finite decimal forms; forms such as `1_.0`, `1._0`, and `1.0e_3` are rejected.

---

## Date and Time Values

The TOML decoder supports local dates, local times, local date-times, and offset date-times.

Examples:

```toml
date = 2026-04-30
time = 23:59:58.123456
local = 2026-04-30T23:59:58
offset = 2026-04-30T23:59:58+09:00
utc = 2026-04-30T14:59:58Z
```

The value model stores these as `toml_local_date`, `toml_local_time`, `toml_local_datetime`, and `toml_offset_datetime`. Fractional seconds are stored with microsecond precision.

---

## Arrays

The implementation supports arrays.

```toml
ports = [8000, 8001, 8002]
mixed = ["xer", true, 3]
```

Arrays preserve element order.

The implementation stores arrays as `toml_array`.

### Notes

* arrays can contain supported scalar values, arrays, and inline tables
* array-of-tables syntax is represented as arrays whose elements are tables

---

## Inline Tables

The implementation supports inline tables.

```toml
point = { x = 1, y = 2 }
package = { name = "xer", metadata.version = "example" }
items = [{ name = "one" }, { name = "two" }]
```

Inline tables are stored as ordinary `toml_table` values. Dotted keys inside inline tables create nested table values in the same representation used by ordinary dotted keys.

Trailing commas in inline tables are rejected.

---

## Array-of-Tables

The decoder supports TOML array-of-tables syntax.

```toml
[[products]]
name = "Hammer"

[[products]]
name = "Nail"
```

In the value model, this is represented as a `toml_array` whose elements are table values. Nested array-of-tables are attached to the latest table element in the parent array.

The encoder emits an array whose elements are tables as `[[...]]` when it appears in table context.

---

## Comments

A `#` starts a comment when it appears outside a string.

```toml
name = "xer" # comment
```

A `#` inside a string is treated as part of the string.

```toml
name = "x#r"
```

---

## `toml_decode`

```cpp
auto toml_decode(std::u8string_view text)
    -> xer::result<toml_value, parse_error_detail>;
```

### Purpose

`toml_decode` parses UTF-8 TOML text and returns a `toml_value`.

### Input Model

The input text is provided as:

```cpp
std::u8string_view
```

This follows XER's UTF-8-oriented public text model.

### Return Model

On success, `toml_decode` returns a `toml_value`.

The returned value is a table value.

### Failure Conditions

At minimum, decoding fails when:

* the input contains invalid UTF-8
* a key-value line does not contain `=`
* a key is malformed
* a table declaration is malformed
* a key is duplicated
* a table is duplicated
* a value uses unsupported syntax
* a value is malformed

Invalid UTF-8 is treated as an encoding error.
Malformed TOML structure is treated as an invalid argument.

---

## Parse Error Detail

`toml_decode` returns `xer::result<toml_value, parse_error_detail>`.

On parse failure, the error object contains the ordinary XER error code together with `offset`, `line`, `column`, and `reason` fields from `<xer/parse.h>`.

The position fields are counted in UTF-8 code units. `line` and `column` are one-based, while `offset` is zero-based.

---

## `toml_encode`

```cpp
auto toml_encode(const toml_value& value) -> xer::result<std::u8string>;
```

### Purpose

`toml_encode` serializes a supported TOML value model into UTF-8 TOML text.

### Input Model

The value to encode must be a table value.

### Output Model

On success, it returns:

```cpp
std::u8string
```

The generated text uses `\n` line endings.

### Serialization Form

The encoder emits ordinary key-value entries first.

```toml
title = "sample"
enabled = true
count = 123
```

Then it emits table sections.

```toml
[project]
name = "xer"
version = "example"
```

A blank line is inserted before a table when there is preceding output.

### Representability

Because the current TOML implementation supports only a subset, `toml_encode` rejects values that cannot be represented by that subset.

For example, it rejects:

* a top-level value that is not a table
* unsupported key forms
* invalid UTF-8 strings

---

## Error Handling

`<xer/toml.h>` follows XER's ordinary failure model.

That means:

* parsing failure is reported through `xer::result`
* serialization failure is reported through `xer::result`
* normal failure is not expressed through exceptions by default

The usual pattern is:

```cpp
const auto decoded = xer::toml_decode(text);
if (!decoded.has_value()) {
    return 1;
}
```

This follows XER's general policy that fallible public APIs return `xer::result`, while ordinary public APIs should accept ordinary values rather than `xer::result` arguments.

---

## Relationship to Other Headers

`<xer/toml.h>` should be understood together with the following documents and headers:

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `policy_encoding.md`
* `header_string.md`
* `header_json.md`
* `header_ini.md`

The rough boundary is:

* `<xer/string.h>` handles general string and text utilities
* `<xer/json.h>` handles JSON as a structured data format
* `<xer/ini.h>` handles INI as a simple string-valued configuration data format
* `<xer/toml.h>` handles TOML as a typed configuration data format

This separation is intentional.
TOML is treated as an independent data-format facility rather than as a string helper or stream helper.

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that TOML processing uses UTF-8 text
* that the current implementation is a practical TOML subset
* that top-level decode results are table values
* that TOML values are typed
* that duplicate keys and duplicate tables are rejected
* that hexadecimal, octal, and binary integers are supported
* that numeric separators are supported only between digits
* that the remaining limitations should be described as limitations of a practical subset

Detailed feature coverage should be kept in sync with the implementation as support expands.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* decoding simple TOML text
* reading a value from the top-level table
* reading a value from a section table
* encoding a `toml_value` table back into TOML text

These are good candidates for executable examples in `examples/`.

---

## Example

```cpp
#include <xer/toml.h>

auto main() -> int
{
    const auto decoded = xer::toml_decode(
        u8"title = \"sample\"\n"
        u8"enabled = true\n"
        u8"\n"
        u8"[project]\n"
        u8"name = \"xer\"\n"
        u8"version = \"example\"\n");

    if (!decoded.has_value()) {
        return 1;
    }

    const auto* root = decoded->as_table();
    if (root == nullptr) {
        return 1;
    }

    const auto encoded = xer::toml_encode(*decoded);
    if (!encoded.has_value()) {
        return 1;
    }

    return 0;
}
```

This example shows the general style:

* parse UTF-8 TOML text with `toml_decode`
* inspect the resulting top-level table
* serialize it again with `toml_encode`
* check `xer::result` explicitly at each fallible step

---

## See Also

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `policy_encoding.md`
* `header_json.md`
* `header_ini.md`

---

## TOML find and load/save helpers

This header also provides helper functions for inspecting decoded TOML values and for loading or saving TOML files.

```cpp
auto toml_find(toml_value& value, std::u8string_view path) noexcept
    -> toml_value*;

auto toml_find(const toml_value& value, std::u8string_view path) noexcept
    -> const toml_value*;

auto toml_load(const path& filename)
    -> xer::result<toml_value, parse_error_detail>;

auto toml_save(const path& filename, const toml_value& value)
    -> xer::result<void>;
```

The `toml_find` helpers inspect already-decoded in-memory values and return pointers to existing values. They return `nullptr` when the requested item is not present or when the searched value has the wrong shape.

The `path` argument to `toml_find` is a simple dot-separated lookup path such as `project.name` or `database.port`. It is not a full TOML key parser. In particular, TOML quoted-key syntax is not interpreted by this helper; quoted dots are not treated specially in the lookup path.

The load helper combines UTF-8 file reading with decoding and returns `xer::result<toml_value, parse_error_detail>`. If file I/O fails before parsing begins, the returned error uses `parse_error_reason::none` and leaves `offset`, `line`, and `column` at zero.

The save helper combines encoding with UTF-8 file writing and returns `xer::result<void>`.

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
- stream content convenience operations
- whole-file content convenience operations

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

### scanf Format Details

# XER scanf Format Specifiers

## Scope

This document describes the format strings used by the XER scanf family.

Target functions:

```cpp
scanf
fscanf
sscanf
```

The printf family is documented separately in `stdio_printf_format.md`.

---

## Basic Policy

XER scanf-style functions are inspired by C scanf, but they are not strict source-compatible reimplementations.

- format strings are UTF-8 strings
- input text is read as XER text and is processed as Unicode scalar values where appropriate
- ordinary fixed text in the format string must match the input
- ASCII whitespace in the format string matches zero or more ASCII whitespace characters in the input
- conversion specifications start with `%`
- ordinary failure is reported through `xer::result`
- match failure returns the number of successful assignments already completed
- XER-specific extensions may exist

A format string may contain ordinary UTF-8 text, whitespace, control tokens, and conversion specifications.

---

## Function Result

The scanf family returns the number of successful assignments.

```cpp
auto result = xer::sscanf(input, format, &a, &b);
```

On success, the returned value is the number of output arguments that were assigned.

If input matching fails after some assignments have already succeeded, the function returns the partial assignment count as a successful result.
This follows the general scanf-style model where an ordinary mismatch is not necessarily a format error.

If the format string is invalid, a type mismatch is detected, input decoding fails, or another ordinary runtime error occurs, the function returns failure through `xer::result`.

---

## Format String Structure

A scanf format string consists of the following kinds of items:

```text
ordinary UTF-8 literal text
ASCII whitespace
conversion specifications beginning with %
XER control tokens such as %@
```

Ordinary literal text must match the input exactly.

ASCII whitespace in the format string consumes zero or more ASCII whitespace characters in the input.
Consecutive whitespace in the format string is treated as a single whitespace-matching item.

---

## Conversion Specification Syntax

A conversion specification begins with `%`.

The currently supported structure is:

```text
%[position$][*][width][length]conversion
```

The positional form is optional.
When it is used, the first output argument is numbered `1`.

Examples:

```cpp
xer::sscanf(u8"10 abc", u8"%d %s", &value, &text);
xer::sscanf(u8"10 abc", u8"%2$s %1$d", &value, &text);
```

---

## Positional Arguments

A conversion may specify an output argument position:

```text
%1$d
%2$s
```

Argument positions are one-based.

When positional arguments are used, the format string is treated as positional.
Sequential and positional argument selection must not be mixed in the same format string, except through the XER `%@` control token rules described below.

Examples:

```cpp
int number = 0;
std::u8string text;

xer::sscanf(u8"hello 123", u8"%2$s %1$d", &number, &text);
```

Here `%2$s` stores into the second output argument and `%1$d` stores into the first output argument.

---

## Assignment Suppression

A conversion may suppress assignment by using `*` after `%` or after the optional positional prefix.

```text
%*d
%*s
```

The input is still matched and consumed, but no output argument is assigned and the assignment count is not incremented.

Example:

```cpp
int value = 0;
xer::sscanf(u8"10 20", u8"%*d %d", &value);
```

This stores `20` in `value`.

---

## Field Width

A field width may be specified as a positive decimal integer.

```text
%3s
%2d
%1c
```

The width limits the number of input characters considered by the conversion.

For `%s` and `%[...]`, the width limits the number of Unicode scalar values collected, not the number of UTF-8 code units.

A width of `0` is not accepted as an explicit field width.
When no field width is present, the conversion reads as much as its own matching rule allows.

---

## Length Modifiers

The following length modifiers are parsed:

```text
hh h l ll j z t L
```

They are accepted as part of the scanf-style grammar.
Their effect is applied to the intermediate value used by numeric conversions.
The actual output type is still determined by the pointer type passed by the caller.

For `%[...]`, length modifiers are currently invalid.

---

## Whitespace Handling Around Conversions

For most conversions, leading ASCII whitespace in the input is skipped before matching.

The following conversions skip leading ASCII whitespace:

```text
%d %u %o %x %X
%f %F %e %E %g %G
%s
```

The following conversions do not skip leading whitespace automatically:

```text
%c
%[...]
%%
```

This follows the usual scanf-style distinction: `%c` and scansets read the next input character according to their own matching rule.

---

## Supported Conversions

The following conversion specifiers are supported:

```text
%d
%u
%o
%x %X
%f %F
%e %E
%g %G
%c
%s
%[...]
%%
```

The `%@` token is also supported as an XER-specific control token.
It is described separately below.

---

## Integer Conversions

### `%d`

`%d` reads a signed decimal integer.

It accepts an optional leading sign followed by decimal digits.

### `%u`

`%u` reads an unsigned decimal integer.

### `%o`

`%o` reads an unsigned octal integer.

### `%x` and `%X`

`%x` and `%X` read an unsigned hexadecimal integer.

The implementation accepts hexadecimal digits using either lowercase or uppercase letters.

### Output Targets

Integer conversions can be stored into ordinary integer scalar targets when the target type is compatible with the intermediate value.

The implementation first parses into an intermediate integer value and then stores into the caller-provided output object.
If the destination pointer type is not compatible with the conversion result, the scan operation reports an error.

---

## Floating-Point Conversions

The following floating-point conversions are supported:

```text
%f %F
%e %E
%g %G
```

They read a floating-point lexeme and store the value into a floating-point target.

The accepted input form follows the implementation's current floating parser.
It includes ordinary decimal forms and exponent forms used by typical scanf-style input.

---

## Character Conversion: `%c`

`%c` reads one input character and stores it as a character-like value.

Unlike `%s`, `%c` does not skip leading whitespace automatically.

In the current implementation, `%c` accepts a field width only when the width is `1` or omitted.
A larger width is treated as invalid.

Typical output targets include:

```cpp
char32_t
char16_t
wchar_t
char8_t
char
signed char
unsigned char
```

The input is read as a Unicode scalar value and then stored into the destination character type.
When the destination is a single-byte character type, the caller is responsible for using it only for values that make sense for that type.

---

## String Conversion: `%s`

`%s` reads a non-empty sequence of non-whitespace characters.

Before matching, leading ASCII whitespace is skipped.
The conversion then collects characters until one of the following occurs:

- end of input
- ASCII whitespace
- field width is reached

The collected text is stored as UTF-8 internally and can be assigned to supported string targets.

Supported string targets include:

```cpp
std::u8string
std::u16string
std::u32string
std::wstring
```

The input text is UTF-8 in the XER text model.
When the destination is `std::u16string`, `std::u32string`, or `std::wstring`, the collected UTF-8 text is converted to the corresponding character-string representation.

For `std::wstring`, conversion follows the width of `wchar_t`:

- when `wchar_t` is effectively UTF-16, UTF-16 code units are produced
- when `wchar_t` is effectively UTF-32, UTF-32 code units are produced

---

## Scanset Conversion: `%[...]`

`%[...]` reads a non-empty sequence of characters that match a scanset.

Unlike `%s`, a scanset does not skip leading whitespace automatically.
The first input character must match the scanset for the conversion to succeed.

The collected text is stored as UTF-8 internally and can be assigned to the same string targets as `%s`:

```cpp
std::u8string
std::u16string
std::u32string
std::wstring
```

### Basic Form

```text
%[abc]
```

This matches one or more characters from the set `a`, `b`, and `c`.

### Negated Form

```text
%[^,]
```

A leading `^` negates the scanset.
This example reads characters until a comma is encountered.

### Including `]`

If `]` appears immediately after `[` or after `[^`, it is treated as a member of the scanset.

Examples:

```text
%[]x]
%[^]x]
```

### Ranges

ASCII ranges are supported.

```text
%[a-z]
%[0-9]
```

Ranges are interpreted over ASCII byte values.
For non-ASCII characters, each UTF-8 code point is handled as an individual scanset item rather than as part of a range.

---

## Literal Percent: `%%`

`%%` matches one literal percent sign in the input.

It does not assign to an output argument and does not increment the assignment count.

---

## XER Control Token: `%@`

`%@` is an XER-specific scanf control token.

It does not read input by itself.
Instead, it controls argument selection for the following conversion specification.

The main purpose is to make a following conversion use a specific output argument while keeping the conversion itself written in the ordinary form.

### Sequential Form

```text
%@ %d
```

In sequential mode, `%@` marks the following conversion as controlled by the current argument-selection flow.
This form is mainly useful as a building block for the same mechanism that also supports positional control.

### Positional Form

```text
%1$@ %d
```

The positional form applies the specified argument position to the following conversion.

Example:

```cpp
int a = 0;
int b = 0;

xer::sscanf(u8"10 20", u8"%2$d %1$@ %d", &a, &b);
```

The behavior is:

```text
%2$d   reads 10 into the second output argument
%1$@   selects the first output argument for the next conversion
%d     reads 20 into the first output argument
```

After the call:

```text
a == 20
b == 10
```

### Restrictions

A `%@` control token must be followed by a conversion specification.
A format string ending with pending `%@` is invalid.

Two consecutive control tokens are invalid.

When positional control is used, the format's argument-selection mode rules still apply.
Mixing incompatible sequential and positional forms is treated as an invalid format.

---

## Generic Stream-Extraction Storage

When a destination type is not one of the explicitly supported scalar, character, or string target categories, the implementation may store through a generic stream-extraction route.

Conceptually, the intermediate scanned value is first converted to UTF-8 text, then to a narrow byte string, and then read through:

```cpp
std::istringstream stream(text);
stream >> value;
```

This route is intended for types that naturally support `operator>>`.

Special string and wide-string targets such as `std::u16string`, `std::u32string`, and `std::wstring` are not handled through this generic route; they are handled explicitly from UTF-8 text.

---

## Assignment Count

The returned assignment count is incremented only when a conversion succeeds and actually assigns to a non-null output pointer.

The count is not incremented for:

- `%%`
- suppressed assignments such as `%*d`
- output arguments passed as `nullptr`
- control tokens such as `%@`

---

## Null Output Pointers

If an output pointer is `nullptr`, the conversion still reads and consumes input normally, but the value is discarded.

A successful conversion with a null output pointer does not increment the assignment count.

This allows callers to ignore selected values without changing the input-matching behavior.

---

## Match Failure vs Error

XER scanf-style functions distinguish ordinary match failure from errors.

### Match Failure

A match failure occurs when the input does not match the next literal or conversion.
In this case, the function returns the assignment count already completed.

Example:

```cpp
int a = 0;
int b = 0;

const auto result = xer::sscanf(u8"10 xx", u8"%d %d", &a, &b);
```

The first conversion succeeds, the second conversion fails to match, and the returned count is `1`.

### Error

An error is reported through `xer::result` failure.

Typical error cases include:

- invalid format syntax
- unsupported conversion syntax
- incompatible argument-selection mode
- invalid UTF-8 input where decoding is required
- type mismatch between a conversion and an output target
- invalid use of field width or length modifier

---

## Encoding Notes

XER scanf-style input works in XER's text model.

For `sscanf`, the input is a UTF-8 string.
For `fscanf` and `scanf`, the source is a `text_stream`, whose external encoding is handled by the stream layer and whose characters are read as XER text characters.

Collected string values are stored internally as UTF-8 before being assigned to the destination string type.

---

## Examples

### Basic scanning

```cpp
int value = 0;
std::u8string text;

const auto result = xer::sscanf(u8"123 hello", u8"%d %s", &value, &text);
```

After success:

```text
value == 123
text == u8"hello"
```

### Reading UTF-8 text into wide string targets

```cpp
std::u16string a;
std::u32string b;
std::wstring c;

xer::sscanf(u8"猫 犬 鳥", u8"%s %s %s", &a, &b, &c);
```

Each `%s` reads UTF-8 input and stores it in the destination string type.

### Reading a scanset

```cpp
std::u8string field;

xer::sscanf(u8"abc,rest", u8"%[^,]", &field);
```

This stores `u8"abc"` in `field`.

---

## Implementation Notes

This document is intended to describe the user-visible scanf-family behavior.
When implementation details in `xer/bits/scanf_format.h` or `xer/bits/scanf.h` change, this document should be kept in sync.

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

## Rewinding

`<xer/stdio.h>` provides `rewind` for both stream kinds:

```cpp
auto rewind(binary_stream& stream) noexcept -> xer::result<void>;
auto rewind(text_stream& stream) noexcept -> xer::result<void>;
```

Unlike the C standard-library function, XER's `rewind` returns `xer::result<void>` so that invalid streams and seek failures can be reported explicitly.

For text streams, rewinding also clears pushed-back characters, lookahead bytes, and partial decoding state. If the stream was opened with `encoding_t::auto_detect`, the concrete encoding is returned to the undecided state.

---

## Whole-Stream Convenience Operations

`<xer/stdio.h>` provides whole-stream convenience operations:

```cpp
auto stream_get_contents(
    binary_stream& stream,
    std::uint64_t length = std::numeric_limits<std::uint64_t>::max())
    -> xer::result<std::vector<std::byte>>;

auto stream_get_contents(text_stream& stream)
    -> xer::result<std::u8string>;

auto stream_put_contents(
    binary_stream& stream,
    std::span<const std::byte> contents)
    -> xer::result<void>;

auto stream_put_contents(
    text_stream& stream,
    std::u8string_view contents)
    -> xer::result<void>;
```

### Purpose

`stream_get_contents` and `stream_put_contents` provide compact helpers for reading from and writing to an already-open XER stream.

They are the stream-level counterparts of `file_get_contents` and `file_put_contents`.

Because they operate on streams rather than file names, they can be used with any stream source or destination supported by XER, including files, temporary files, memory streams, string streams, process pipes, and socket-derived streams where applicable.

### Binary `stream_get_contents`

```cpp
auto stream_get_contents(
    binary_stream& stream,
    std::uint64_t length = std::numeric_limits<std::uint64_t>::max())
    -> xer::result<std::vector<std::byte>>;
```

This overload reads binary data from the current position of `stream`.

It reads at most `length` bytes, or stops earlier if EOF is reached.

If `length` is zero, the function succeeds and returns an empty byte vector.

### No Offset Argument

XER intentionally does not provide an offset parameter for `stream_get_contents`.

A stream already has a current position. If the caller needs to choose the starting position, the caller should use `fseek`, `fsetpos`, or another appropriate positioning function explicitly before calling `stream_get_contents`.

This also avoids the confusing argument-order difference found in PHP, where `file_get_contents` and `stream_get_contents` place offset and length differently.

In XER, the rule is simple:

* `file_get_contents` may take an offset because it opens the file internally
* `stream_get_contents` reads from the stream's current position

### Text `stream_get_contents`

```cpp
auto stream_get_contents(text_stream& stream)
    -> xer::result<std::u8string>;
```

This overload reads text from the current position of `stream` until EOF.

The returned string is UTF-8 text.

The stream's own encoding state controls how external bytes are decoded.

Text-mode `stream_get_contents` does not provide `offset` or `length` arguments because byte offsets, decoded characters, line ending behavior, and encoding state can otherwise become ambiguous.

### Binary `stream_put_contents`

```cpp
auto stream_put_contents(
    binary_stream& stream,
    std::span<const std::byte> contents)
    -> xer::result<void>;
```

This overload writes all bytes in `contents` to the current position of `stream`.

The exact placement behavior is determined by the stream's current position and open mode.

For example, if the stream was opened in append mode, the write follows the stream's append behavior.

### Text `stream_put_contents`

```cpp
auto stream_put_contents(
    text_stream& stream,
    std::u8string_view contents)
    -> xer::result<void>;
```

This overload writes the UTF-8 text in `contents` to the current position of `stream`.

The stream's encoding controls how the UTF-8 text is encoded externally.

### Relationship to File Convenience Functions

`file_get_contents` and `file_put_contents` are file-opening convenience wrappers around these stream-level helpers.

Conceptually:

```cpp
auto stream = xer::fopen(filename, "r");
return xer::stream_get_contents(*stream);
```

or:

```cpp
auto stream = xer::fopen(filename, "w");
return xer::stream_put_contents(*stream, contents);
```

The stream-level functions contain the reusable read/write logic, while the file-level functions handle opening the file and applying file-specific options such as the binary `offset` argument.

### Error Handling

These functions follow XER's ordinary failure model.

On success:

* `stream_get_contents` returns the read data
* `stream_put_contents` returns an empty success value

On failure, they return an error through `xer::result`.

Typical failure conditions include:

* the stream is not readable or writable for the requested operation
* reading or writing fails
* text decoding or encoding fails
* memory allocation fails while collecting the result

### Example

```cpp
std::u8string buffer;

auto stream = xer::stropen(buffer, "w+");
if (!stream.has_value()) {
    return 1;
}

const auto written = xer::stream_put_contents(
    *stream,
    std::u8string_view(u8"hello XER"));

if (!written.has_value()) {
    return 1;
}

const auto rewound = xer::rewind(*stream);
if (!rewound.has_value()) {
    return 1;
}

const auto text = xer::stream_get_contents(*stream);
if (!text.has_value()) {
    return 1;
}
```

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
file_exists
is_file
is_dir
is_readable
is_writable

remove
rename
mkdir
rmdir
copy
touch

fileatime
filemtime
filectime

chdir
getcwd
realpath

file_get_contents
file_put_contents
```

### Role of This Group

These functions operate on filesystem entries rather than on open stream objects.

They are grouped here because they are operationally close to stream/file handling.

### Design Direction

These functions are intentionally separate from stream objects themselves.

They typically operate on `xer::path`, not on raw native path strings.

This aligns them with XER's own path model, where path values are represented internally as UTF-8 strings with `/` as the normalized separator.

Some functions in this group are simple predicates, while others perform actual filesystem operations.

Predicate functions such as `file_exists`, `is_file`, `is_dir`, `is_readable`, and `is_writable` return `bool`.

Operations that can fail normally return `xer::result`.

### File Time Helpers

`fileatime`, `filemtime`, and `filectime` return file time fields as seconds since the POSIX epoch. They use the platform's ordinary path status operation and may follow symbolic links when the platform's normal stat-like operation does so.

`filectime` returns the same ctime field as `xer::stat::ctime`. The platform-specific meaning of that field is documented on `xer::stat`.

```cpp
auto fileatime(const path& filename) -> xer::result<time_t>;
auto filemtime(const path& filename) -> xer::result<time_t>;
auto filectime(const path& filename) -> xer::result<time_t>;
```

### `touch`

```cpp
auto touch(
    const path& filename,
    time_t mtime = -1,
    time_t atime = -1) -> xer::result<void>;
```

`touch` changes the target's modification and access times. If the target does not exist, it creates an empty regular file.

A negative `mtime` means that the current time is used. A negative `atime` means that the resolved `mtime` is also used as the access time. Non-finite time values are rejected as invalid arguments.

---

## Current Working Directory Operations

`<xer/stdio.h>` provides current-working-directory helpers:

```cpp
auto chdir(const path& target) -> xer::result<void>;
auto getcwd() -> xer::result<path>;
```

### `chdir`

`chdir` changes the process-wide current working directory.

```cpp
auto chdir(const path& target) -> xer::result<void>;
```

The argument is a `xer::path`.

On success, the function returns an empty success value.
On failure, it returns an error through `xer::result`.

Because the current working directory is process-wide state, callers should use this function carefully in programs where multiple components or threads may depend on the current directory.

### `getcwd`

`getcwd` returns the current working directory.

```cpp
auto getcwd() -> xer::result<path>;
```

The returned value is a `xer::path`.

The path is converted into XER's internal UTF-8 representation and uses `/` as the normalized separator.

The result is a snapshot of the process-wide current working directory at the time of the call.

---

## `realpath`

```cpp
auto realpath(const path& filename) -> xer::result<path>;
```

### Purpose

`realpath` returns the canonicalized absolute path of an existing filesystem entry.

It queries the actual filesystem through the platform path canonicalization mechanism.

### Behavior

The target path must exist.

Relative path components are resolved.
Symbolic links and other filesystem-level indirections are resolved according to the behavior of the underlying platform.

On POSIX-like environments, the behavior follows the platform `realpath` facility.
On Windows, the implementation uses Windows path canonicalization facilities and converts the result back into XER's path representation.

### Return Value

On success, `realpath` returns a `xer::path`.

The returned path:

* is absolute
* refers to an existing filesystem entry
* is converted to XER's UTF-8 path representation
* uses `/` as the internal separator

On failure, it returns an error through `xer::result`.

Typical failure conditions include:

* the target path does not exist
* the caller lacks permission to access the path
* native path conversion fails
* platform path canonicalization fails

### Difference from Lexical Path Operations

`realpath` is not a purely lexical path operation.

It depends on the actual filesystem and may observe filesystem state such as symbolic links, mounted volumes, permissions, and existing entries.

For purely lexical path manipulation, use path helpers such as `basename`, `parent_path`, `extension`, `stem`, `is_absolute`, and `is_relative`.

### Example

```cpp
const auto resolved = xer::realpath(xer::path(u8"."));
if (!resolved.has_value()) {
    return 1;
}
```

After success, `resolved` contains the canonicalized absolute path of the current directory.

---

## Whole-File Convenience Operations

`<xer/stdio.h>` provides PHP-inspired whole-file convenience operations:

```cpp
auto file_get_contents(
    const path& filename,
    std::uint64_t offset = 0,
    std::uint64_t length = std::numeric_limits<std::uint64_t>::max())
    -> xer::result<std::vector<std::byte>>;

auto file_get_contents(
    const path& filename,
    encoding_t encoding)
    -> xer::result<std::u8string>;

auto file_put_contents(
    const path& filename,
    std::span<const std::byte> contents)
    -> xer::result<void>;

auto file_put_contents(
    const path& filename,
    std::u8string_view contents,
    encoding_t encoding)
    -> xer::result<void>;
```

### Purpose

`file_get_contents` and `file_put_contents` provide compact helpers for reading and writing an entire file without manually opening a stream.

They are file-opening convenience wrappers around `stream_get_contents` and `stream_put_contents`. The reusable read/write behavior belongs to the stream-level helpers, while the file-level helpers additionally open the target file and apply file-specific options.

They are inspired by PHP functions of the same names, but their behavior follows XER's stream and encoding model.

### Binary and Text Selection

The overload set uses the presence or absence of an `encoding_t` argument to select binary or text behavior.

* when no encoding is specified, the file is handled through `binary_stream`
* when an encoding is specified, the file is handled through `text_stream`

This keeps the call site explicit without introducing a separate mode flag.

### Binary `file_get_contents`

```cpp
auto file_get_contents(
    const path& filename,
    std::uint64_t offset = 0,
    std::uint64_t length = std::numeric_limits<std::uint64_t>::max())
    -> xer::result<std::vector<std::byte>>;
```

This overload opens the file as binary and returns its contents as `std::vector<std::byte>`.

The optional `offset` and `length` arguments are byte-based.

If `offset` is greater than the file size, the function returns `error_t::invalid_argument`.

If `offset` is exactly equal to the file size, the function succeeds and returns an empty byte vector.

If `length` is zero, the function succeeds and returns an empty byte vector.

### Text `file_get_contents`

```cpp
auto file_get_contents(
    const path& filename,
    encoding_t encoding)
    -> xer::result<std::u8string>;
```

This overload opens the file as text and returns its contents as UTF-8 text.

The specified encoding controls how the external file bytes are decoded.

`encoding_t::auto_detect` is valid for this input-side operation.

Text-mode `file_get_contents` does not provide `offset` or `length` arguments because byte offsets, decoded characters, line ending behavior, and encoding state can otherwise become ambiguous.

### Binary `file_put_contents`

```cpp
auto file_put_contents(
    const path& filename,
    std::span<const std::byte> contents)
    -> xer::result<void>;
```

This overload opens the file as binary and writes all bytes in `contents`.

Existing file contents are replaced.

### Text `file_put_contents`

```cpp
auto file_put_contents(
    const path& filename,
    std::u8string_view contents,
    encoding_t encoding)
    -> xer::result<void>;
```

This overload opens the file as text and writes the UTF-8 text in `contents` using the specified output encoding.

`encoding_t::auto_detect` is invalid for writing and results in `error_t::invalid_argument`.

### Why PHP-Style Flags Are Not Provided

XER intentionally does not provide PHP-style `flags` arguments for these functions.

In particular, append behavior and locking behavior are not hidden inside `file_put_contents`.

If file locking is required, the caller should perform it explicitly with an outer operation such as `flock`.

If append-style output is required, the caller should use stream APIs directly, such as opening a stream with append mode and writing with `fwrite` or `fputs`.

If append-style output is required, the caller should use stream APIs directly, such as opening a stream with append mode and writing with `fwrite`, `fputs`, or `stream_put_contents`.

### Error Handling

These functions follow XER's ordinary failure model.

On success:

* `file_get_contents` returns the read data
* `file_put_contents` returns an empty success value

On failure, they return an error through `xer::result`.

Typical failure conditions include:

* the file cannot be opened
* seeking fails
* reading or writing fails
* `offset` is invalid
* `encoding_t::auto_detect` is used for text output
* text decoding or encoding fails

### Example

```cpp
const auto text = xer::file_get_contents(
    xer::path(u8"sample.txt"),
    xer::encoding_t::utf8);

if (!text.has_value()) {
    return 1;
}

const auto written = xer::file_put_contents(
    xer::path(u8"copy.txt"),
    *text,
    xer::encoding_t::utf8);

if (!written.has_value()) {
    return 1;
}
```

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
* that `realpath` is filesystem-dependent and distinct from lexical path operations
* that `stream_get_contents` reads from the current stream position and intentionally does not provide an offset argument
* that `file_get_contents` and `file_put_contents` are convenience APIs whose binary/text behavior is selected by the presence of an encoding argument
* that `file_get_contents` and `file_put_contents` are file-opening wrappers around the stream-level content helpers

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
* changing and restoring the current working directory with `chdir` and `getcwd`
* canonicalizing an existing path with `realpath`
* reading and writing already-open streams with `stream_get_contents` and `stream_put_contents`
* reading and writing whole files with `file_get_contents` and `file_put_contents`

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

---

# `<xer/iostream.h>`

## Purpose

`<xer/iostream.h>` provides formatted iostream insertion and extraction operators for selected XER value types.

This header does not make iostreams the main input/output model of XER. XER's primary I/O model remains based on `binary_stream`, `text_stream`, and the functions provided by `<xer/stdio.h>`. The role of `<xer/iostream.h>` is narrower: it provides a bridge for diagnostics, tests, examples, and the implementation of generic `%@` formatting and scanning support.

The header is intentionally opt-in. Users include it only when they want ordinary C++ iostream operators for XER value types.

---

## Main Entities

At minimum, `<xer/iostream.h>` makes `operator<<` available for the following types. The `error_t` and `error<void>` insertion operators are defined by `<xer/error.h>` and are available because this header includes it:

```cpp
xer::error_t
xer::error<void>
xer::type_info
xer::path
xer::cyclic<T>
xer::interval<T, Min, Max>
xer::quantity<T, Dim>
xer::matrix<T, Rows, Cols>
xer::basic_rgb<T>
xer::basic_gray<T>
xer::basic_cmy<T>
xer::basic_hsv<T>
xer::basic_xyz<T>
xer::basic_lab<T>
xer::basic_luv<T>
```

It also provides `operator>>` for the following types where formatted extraction is straightforward:

```cpp
xer::error_t
xer::path
xer::cyclic<T>
xer::interval<T, Min, Max>
xer::quantity<T, Dim>
```

Formatted extraction for matrices and color values is intentionally deferred because their insertion format is meant for diagnostics rather than as a stable serialized grammar.

---

## Design Role

The main design role of this header is to make XER-provided value types usable by generic stream-based formatting paths.

In particular, it supports facilities that internally rely on stream insertion or extraction for default formatting, such as enhanced `%@` handling in the printf and scanf families.

The operators are not intended to replace XER's own text I/O APIs.

---

## UTF-8 Handling

XER uses `char8_t` and `std::u8string_view` for UTF-8 text. Ordinary iostreams use `char` streams.

For this reason, `<xer/iostream.h>` writes UTF-8 text to `std::ostream` by copying the underlying UTF-8 bytes to the stream without locale-dependent conversion. This applies to types such as `path` and `type_info`, whose display strings are UTF-8-oriented.

Formatted extraction from `std::istream` reads ordinary narrow tokens and copies the bytes into UTF-8 storage where appropriate.

---

## `error_t`

`operator<<` for `error_t` writes the enumerator name returned by `get_error_name`.

Example output:

```text
invalid_argument
not_found
io_error
```

`operator>>` accepts enumerator names without the `error_t::` prefix. Unknown names set the stream fail bit.

---

## `error<void>`

`operator<<` for `error<void>` writes the compact diagnostic representation already provided by `<xer/error.h>`:

```text
xer::error{code=invalid_argument}
```

The source location stored in the error object is not written. This keeps the default representation short and suitable for assertion messages, trace output, and `%@` formatting.

There is no extraction operator for `error<void>`. Error objects are normally created by failed operations, not read from formatted input.

---

## `type_info`

`operator<<` for `type_info` writes the display name returned by `type_info::name()`.

This is intended for diagnostics. The spelling remains implementation-dependent, just like the underlying type information facility.

There is no extraction operator for `type_info`.

---

## `path`

`operator<<` for `path` writes the normalized UTF-8 path returned by `path::str()`.

`operator>>` reads one whitespace-delimited token and constructs a `path` from it. As usual for formatted extraction, paths containing whitespace are not handled by this operator. Use line-oriented input and construct `xer::path` explicitly when such paths are needed.

---

## `cyclic<T>`

`operator<<` for `cyclic<T>` writes the normalized scalar value returned by `value()`.

`operator>>` reads a scalar value and constructs `cyclic<T>` from it. The ordinary `cyclic` normalization rules apply.

Example:

```text
-0.25
```

is read as the normalized cyclic value:

```text
0.75
```

---

## `interval<T, Min, Max>`

`operator<<` for `interval<T, Min, Max>` writes the stored scalar value returned by `value()`.

`operator>>` reads a scalar value and constructs an interval value from it. The ordinary `interval` rules apply: finite out-of-range values are clamped, while invalid floating-point values such as NaN or infinity cause extraction to set the stream fail bit if interval construction rejects them.

---

## `quantity<T, Dim>`

`operator<<` for `quantity<T, Dim>` writes the stored value in the base unit system.

`operator>>` reads a scalar value and constructs a quantity of the destination dimension. The input value is interpreted as already normalized to the base unit system.

For example, if the destination type is a length quantity, the input value is interpreted as meters, not as kilometers or centimeters.

---

## `matrix<T, Rows, Cols>`

`operator<<` for `matrix<T, Rows, Cols>` writes a compact row-major diagnostic form.

Example output for a 2x2 matrix:

```text
[[1, 2], [3, 4]]
```

There is no extraction operator for matrices at this stage. Parsing a matrix would require committing the diagnostic output form to a stable input grammar, which is intentionally avoided for now.

---

## Color Types

`operator<<` is provided for the basic color value types.

Example output:

```text
rgb(1, 0.5, 0)
gray(0.25)
cmy(0, 0.5, 1)
hsv(0.25, 0.5, 1)
xyz(0.1, 0.2, 0.3)
lab(50, 10, -20)
luv(50, 10, -20)
```

There are no extraction operators for color values at this stage. The insertion format is intended for diagnostics and `%@` formatting, not for stable serialization.

---

## Deferred Items

The following are intentionally deferred from the current implementation:

- `operator<<` and `operator>>` for `error<Detail>`
- `operator>>` for `matrix`
- `operator>>` for color types
- `operator<<` and `operator>>` for JSON, INI, and TOML values
- stream insertion for resource handles such as `binary_stream`, `text_stream`, `process`, and `socket`

These types either need additional formatting policy or are not ordinary value types suitable for default formatted extraction.

---

## Relationship to Other Headers

`<xer/iostream.h>` is related to the following headers:

- `<xer/error.h>`
- `<xer/typeinfo.h>`
- `<xer/path.h>`
- `<xer/cyclic.h>`
- `<xer/interval.h>`
- `<xer/quantity.h>`
- `<xer/matrix.h>`
- `<xer/color.h>`
- `<xer/stdio.h>`

The rough boundary is:

- `<xer/stdio.h>` remains the ordinary XER text I/O header
- `<xer/iostream.h>` provides opt-in compatibility with standard iostream formatting
- individual value-type headers remain usable without pulling in iostream support

---

## Example

```cpp
#include <iostream>
#include <sstream>

#include <xer/color.h>
#include <xer/cyclic.h>
#include <xer/interval.h>
#include <xer/iostream.h>
#include <xer/matrix.h>
#include <xer/path.h>
#include <xer/quantity.h>

auto main() -> int
{
    using namespace xer::units;

    const auto path = xer::path(u8"work/file.txt");
    const auto angle = xer::cyclic<double>(1.25);
    const auto gain = xer::interval<double>(1.25);
    const auto distance = 1.5 * km;
    const auto transform = xer::matrix<double, 2, 2>(1.0, 2.0, 3.0, 4.0);
    const auto color = xer::rgb(1.0f, 0.5f, 0.0f);

    std::cout << path << '\n';
    std::cout << angle << '\n';
    std::cout << gain << '\n';
    std::cout << distance << '\n';
    std::cout << transform << '\n';
    std::cout << color << '\n';

    std::istringstream input("logs/output.txt -0.25 0.5 2.5");
    xer::path read_path;
    xer::cyclic<double> read_angle;
    xer::interval<double> read_gain;
    xer::quantity<double, xer::units::length_dim> read_distance;

    input >> read_path >> read_angle >> read_gain >> read_distance;
    return input ? 0 : 1;
}
```

---

## See Also

- `header_error.md`
- `header_typeinfo.md`
- `header_path.md`
- `header_cyclic.md`
- `header_interval.md`
- `header_quantity.md`
- `header_matrix.md`
- `header_color.md`
- `header_stdio.md`

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

# `<xer/dirent.h>`

## Purpose

`<xer/dirent.h>` provides directory stream operations in XER.

This header covers PHP/POSIX-style directory traversal facilities such as opening a directory, reading entry names, rewinding the directory stream, and closing it.

The purpose is not to reproduce POSIX `dirent.h` exactly.
Instead, XER provides a small C++23-friendly directory stream API that uses:

- `xer::path` for path names
- UTF-8 strings for directory entry names
- `xer::result` for ordinary failure
- a move-only RAII handle for directory streams

---

## Main Entities

At minimum, `<xer/dirent.h>` provides the following entities:

```cpp
class xer::dir;

auto xer::opendir(const path& dirname) noexcept -> result<dir>;
auto xer::closedir(dir& directory) noexcept -> result<void>;
auto xer::readdir(dir& directory) noexcept -> result<std::u8string>;
auto xer::rewinddir(dir& directory) noexcept -> result<void>;
````

---

## Design Role

This header exists for directory stream traversal.

It is separated from `<xer/stdio.h>` because directory streams are stateful traversal handles rather than ordinary file streams.
Although the names are familiar from POSIX and PHP, the API is adapted to XER's own path, string, and error-handling model.

---

## `xer::dir`

`xer::dir` is a move-only directory stream handle.

It owns a native directory stream handle internally and closes it automatically when destroyed.

```cpp
class xer::dir;
```

### Basic Properties

* move-only
* non-copyable
* RAII-based
* represents either an open directory stream or an empty/closed state

### Explicit Close

The destructor closes the directory stream automatically, but failures from a destructor cannot be observed.

When the caller needs to observe close errors, `xer::closedir` should be called explicitly.

---

## `xer::opendir`

```cpp
auto opendir(const path& dirname) noexcept -> result<dir>;
```

`opendir` opens a directory stream for the specified path.

The path is converted from XER's UTF-8 `xer::path` representation to the platform-native path representation before the underlying directory API is called.

### Return Value

On success, it returns an open `xer::dir`.

On failure, it returns `xer::result` failure.

### Notes

The returned directory stream is a snapshot-like traversal handle.
If the directory contents are modified while the directory is being read, the observed behavior is platform- and filesystem-dependent.

---

## `xer::closedir`

```cpp
auto closedir(dir& directory) noexcept -> result<void>;
```

`closedir` closes a directory stream.

After this function is called, the `xer::dir` object is treated as closed.

### Return Value

On success, it returns an empty success value.

On failure, it returns `xer::result` failure.

### Notes

Calling `closedir` for an already closed or empty `xer::dir` is treated as a no-op success.

The destructor of `xer::dir` also closes the directory stream, but explicit `closedir` is useful when the caller wants to observe close errors.

---

## `xer::readdir`

```cpp
auto readdir(dir& directory) noexcept -> result<std::u8string>;
```

`readdir` reads the next entry name from a directory stream.

The returned string is a UTF-8 directory entry name.

### Return Value

On success, it returns the next entry name.

At the end of the directory stream, it returns failure with:

```cpp
error_t::not_found
```

Other failures are reported through `xer::result` in the usual way.

### Important Notes

`readdir` returns only the entry name.

It does not return a full path.

For example, if the directory contains:

```text
example.txt
```

then `readdir` returns:

```text
example.txt
```

not:

```text
directory/example.txt
```

The special entries `"."` and `".."` are not filtered out.
This follows the PHP/POSIX-style behavior more closely.
Callers that do not want these entries should skip them explicitly.

Entry order is platform- and filesystem-dependent.
Code must not rely on a specific order unless it sorts the entries itself.

---

## `xer::rewinddir`

```cpp
auto rewinddir(dir& directory) noexcept -> result<void>;
```

`rewinddir` rewinds a directory stream to the beginning.

After this function succeeds, subsequent calls to `readdir` read entries again from the beginning of the directory stream.

### Return Value

On success, it returns an empty success value.

On failure, it returns `xer::result` failure.

### Notes

The order after rewinding is still platform- and filesystem-dependent.
XER does not guarantee a stable ordering of directory entries.

---

## End-of-Directory Handling

In XER, reaching the end of a directory stream is represented as:

```cpp
error_t::not_found
```

Typical usage is:

```cpp
for (;;) {
    auto entry = xer::readdir(directory);
    if (!entry.has_value()) {
        if (entry.error().code == xer::error_t::not_found) {
            break;
        }

        return 1;
    }

    // Use *entry.
}
```

This keeps end-of-directory separate from ordinary successful reads while still using the normal `xer::result` failure channel.

---

## Relationship to Path Handling

`<xer/dirent.h>` uses `xer::path` for directory paths.

The `path` object stores a UTF-8 path in XER's normalized internal form.
When `opendir` is called, the path is converted to the platform-native representation before being passed to the underlying directory API.

The names returned by `readdir` are converted back into UTF-8 strings.

---

## Relationship to Other Headers

`<xer/dirent.h>` is related to:

* `<xer/path.h>`
* `<xer/stdio.h>`
* `<xer/error.h>`

The rough boundary is:

* `<xer/path.h>` handles lexical path representation and path utilities
* `<xer/stdio.h>` handles ordinary file streams and file-related operations
* `<xer/dirent.h>` handles directory stream traversal

---

## Documentation Notes

When documenting this header, the most important points are:

* `xer::dir` is a move-only RAII directory stream handle
* `readdir` returns entry names, not full paths
* `"."` and `".."` are not filtered out
* end-of-directory is represented by `error_t::not_found`
* entry order is filesystem-dependent
* modifications during traversal have platform-dependent results

---

## Example

```cpp
#include <xer/dirent.h>
#include <xer/error.h>
#include <xer/stdio.h>

auto main() -> int
{
    auto directory = xer::opendir(u8".");
    if (!directory.has_value()) {
        return 1;
    }

    for (;;) {
        auto entry = xer::readdir(*directory);
        if (!entry.has_value()) {
            if (entry.error().code == xer::error_t::not_found) {
                break;
            }

            return 1;
        }

        if (*entry == u8"." || *entry == u8"..") {
            continue;
        }

        if (!xer::puts(*entry).has_value()) {
            return 1;
        }
    }

    if (!xer::closedir(*directory).has_value()) {
        return 1;
    }

    return 0;
}
```

---

## See Also

* `<xer/path.h>`
* `<xer/stdio.h>`
* `<xer/error.h>`
* `policy_path.md`
* `policy_examples.md`

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
- approximate floating-point comparison
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

## Approximate Floating-Point Comparison

`<xer/arithmetic.h>` provides an approximate comparison helper for floating-point-oriented checks:

```cpp
is_close
```

### `is_close`

```cpp
template<typename A, typename B, typename E>
constexpr auto is_close(A lhs, B rhs, E epsilon) noexcept -> bool;

template<typename A, typename B, typename E>
constexpr auto is_close(
    const result<A>& lhs,
    B rhs,
    E epsilon) noexcept -> result<bool>;

template<typename A, typename B, typename E>
constexpr auto is_close(
    A lhs,
    const result<B>& rhs,
    E epsilon) noexcept -> result<bool>;

template<typename A, typename B, typename E>
constexpr auto is_close(
    const result<A>& lhs,
    const result<B>& rhs,
    E epsilon) noexcept -> result<bool>;
```

`is_close(lhs, rhs, epsilon)` tests whether two arithmetic values are close enough under an absolute tolerance.

Conceptually, the comparison is:

```text
abs(lhs - rhs) <= epsilon
```

The comparison is inclusive. Therefore, values exactly at the specified tolerance boundary are treated as close.

### Rounding Margin

For positive tolerance values, the implementation may apply a small rounding margin based on the common arithmetic type.

This is intended to avoid rejecting values that are mathematically on the tolerance boundary but become slightly larger due to floating-point representation and subtraction rounding.

For example, a difference that is mathematically `0.05` should not be rejected merely because the computed floating-point difference is slightly greater than `0.05`.

When `epsilon` is zero, this extra rounding margin is not applied.
A zero tolerance therefore behaves as an exact comparison after conversion to the internal comparison type.

### Invalid Values

`is_close` returns `false` when any of the following apply:

- `lhs` is NaN or infinity
- `rhs` is NaN or infinity
- `epsilon` is NaN or infinity
- `epsilon` is negative
- the computed difference is not finite

This keeps approximate comparison simple and avoids treating invalid floating-point states as ordinary closeness.

### `xer::result` Arguments

As part of `<xer/arithmetic.h>`, `is_close` may accept `xer::result` operands for `lhs` and `rhs`.

If a result operand contains a success value, that value is compared.
If a result operand contains an error, the error is propagated and the return type is `xer::result<bool>`.

The tolerance argument is intentionally kept as an ordinary value.
The tolerance is normally a fixed decision made by the caller, and accepting a result for it would add little practical value.

### Naming

The name `is_close` is used instead of shorter names such as `near`.

This avoids collision with legacy platform macros while keeping the purpose of the function clear at the call site.

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

## Square and Cube Helpers

`<xer/arithmetic.h>` also provides small power helpers:

```cpp
sq
cb
```

### `sq`

`sq(value)` returns the square of `value`.

For integer input, it generally returns:

```cpp
xer::result<xer::int64_t>
```

and reports failure if the squared value cannot be represented in the signed result domain.

For floating-point input, it follows the floating-point arithmetic rules of this header and returns a result whose success value is `long double`.

### `cb`

`cb(value)` returns the cube of `value`.

It follows the same result and error-handling policy as `sq`.
For integer input, overflow and out-of-range results are reported explicitly.

### Chained Use

As arithmetic helpers, `sq` and `cb` may also accept `xer::result` arguments.
This allows forms such as:

```cpp
const auto value = xer::sq(xer::add(2, 3));
```

If the argument result already contains an error, that error is propagated.
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
* using `sq` or `cb` with explicit result checking
* comparing floating-point values with `is_close`
* comparing floating-point values with `is_close`

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

## Ratio Conversion

`cyclic<T>` provides ratio-oriented member functions for symmetry with `interval`.

```cpp
constexpr auto ratio() const noexcept -> T;
static constexpr auto from_ratio(T ratio) noexcept -> cyclic;
```

`ratio()` returns the normalized internal position in `[0, 1)`.
It is an alias of `value()`.

`from_ratio()` constructs a cyclic value from a turn-based ratio and applies normal cyclic normalization.

```cpp
auto a = xer::cyclic<float>::from_ratio(1.25f);
// a.value() == 0.25f
```

---

## Explicit Conversion with `interval`

Implicit conversion between `cyclic` and `interval` is not provided.
The endpoint semantics are different, so conversion should be visible in the source code.

The interval header provides explicit helpers:

```cpp
auto to_cyclic(interval<T, Min, Max> value) noexcept -> cyclic<T>;
auto to_interval(cyclic<T> value) -> interval<T>;
```

`to_cyclic` maps an interval through its ratio.
`to_interval` maps a cyclic value to the default interval `[0, 1]`.

For custom interval bounds, use `interval<T, Min, Max>::from_ratio(value.ratio())`.

---

# `<xer/interval.h>`

## Purpose

`<xer/interval.h>` provides bounded floating-point value types.

The main entity is `xer::interval<T, Min, Max>`, a lightweight value type that stores a finite scalar value constrained to a fixed closed interval.

The default interval is `[0, 1]`, which is useful for values such as color components, alpha values, normalized ratios, opacity, brightness, gain, and other bounded control values.

---

## Main Entity

At minimum, `<xer/interval.h>` provides the following entity:

```cpp
template <
    std::floating_point T,
    T Min = static_cast<T>(0),
    T Max = static_cast<T>(1)>
class interval;
```

The implementation is provided through the corresponding internal header:

```cpp
#include <xer/bits/interval.h>
```

Users should normally include the public header:

```cpp
#include <xer/interval.h>
```

---

## Design Role

`interval` is a small numeric value type whose main purpose is to preserve an invariant.

For `interval<T, Min, Max>`, the stored value always satisfies:

```text
Min <= value() <= Max
```

The stored value is always finite.

Finite out-of-range values are clamped to the nearest bound.
Invalid floating-point values such as `NaN` and infinity are rejected by throwing an exception.

This makes `interval` useful for values that should never escape a known range during ordinary use.

---

## Relationship to `cyclic`

`interval` is closely related to `cyclic`, but the two types represent different concepts.

`cyclic<T>` represents a circular value normalized into `[0, 1)`.

Typical examples include:

- hue
- angle
- phase
- direction

`interval<T, Min, Max>` represents a linear bounded value in `[Min, Max]`.

Typical examples include:

- red, green, and blue components
- alpha values
- grayscale values
- brightness
- gain
- opacity
- normalized ratios

This distinction is especially important in color handling.
Hue naturally wraps around, while color components do not.

---

## Template Parameters

```cpp
template <
    std::floating_point T,
    T Min = static_cast<T>(0),
    T Max = static_cast<T>(1)>
class interval;
```

### `T`

`T` is the stored floating-point type.

The main intended types are:

- `float`
- `double`
- `long double`

Integer types are not accepted.

### `Min`

`Min` is the inclusive lower bound.

### `Max`

`Max` is the inclusive upper bound.

The type requires:

```cpp
Min < Max
```

An empty interval or reversed interval is not accepted.

---

## Default Interval

The default form:

```cpp
xer::interval<float>
```

means:

```cpp
xer::interval<float, 0.0f, 1.0f>
```

This is the most common form for normalized values.

Example:

```cpp
using component = xer::interval<float>;

auto r = component(1.25f);  // stored as 1.0f
auto g = component(0.5f);   // stored as 0.5f
auto b = component(-0.25f); // stored as 0.0f
```

---

## Custom Intervals

Custom bounds can be specified as floating-point non-type template parameters.

Example:

```cpp
using gain = xer::interval<float, -1.0f, 1.0f>;

auto center = gain(0.0f);
auto upper = gain(2.0f);   // clamped to 1.0f
auto lower = gain(-2.0f);  // clamped to -1.0f
```

This is useful when a value has a natural range other than `[0, 1]`.

---

## Construction

### Default Construction

Default construction initializes the stored value to `Min`.

```cpp
xer::interval<float> x;
// x.value() == 0.0f

xer::interval<float, -1.0f, 1.0f> y;
// y.value() == -1.0f
```

### Construction from a Raw Value

Construction from a raw scalar is explicit.

```cpp
explicit constexpr interval(T value);
```

Finite values are accepted and clamped into the interval.

For `xer::interval<float>`:

```cpp
auto a = xer::interval<float>(0.5f);   // 0.5f
auto b = xer::interval<float>(-0.5f);  // 0.0f
auto c = xer::interval<float>(1.5f);   // 1.0f
```

`NaN` and infinity are rejected by exception.

---

## Exception Policy

`interval` throws `std::domain_error` for values that cannot be represented as valid finite interval values.

At minimum, the following cases throw:

- construction from `NaN`
- construction from positive infinity
- construction from negative infinity
- assignment from `NaN`
- assignment from infinity
- arithmetic that produces `NaN`
- arithmetic that produces infinity
- division by zero

This is intentional.
Silently clamping `NaN` or infinity would hide a serious numeric error.

---

## Member Types and Constants

`interval` provides the following public members:

```cpp
using value_type = T;

static constexpr T min_value = Min;
static constexpr T max_value = Max;
```

`value_type` is the stored floating-point type.

`min_value` and `max_value` expose the compile-time interval bounds.

---

## Value Access

### `value`

```cpp
constexpr auto value() const noexcept -> T;
```

Returns the stored scalar value.

The returned value is always finite and always inside `[Min, Max]`.

---

## Assignment

### `assign`

```cpp
constexpr auto assign(T value) -> void;
```

Assigns a raw scalar value.

Finite values are clamped into `[Min, Max]`.
`NaN` and infinity throw `std::domain_error`.

### Assignment from `T`

```cpp
constexpr auto operator=(T value) -> interval&;
```

Assigns a raw scalar value and returns `*this`.

This behaves the same as `assign`.

Example:

```cpp
auto x = xer::interval<float>();

x = 0.75f; // stored as 0.75f
x = 2.0f;  // stored as 1.0f
```

---

## Ratio Conversion

### `ratio`

```cpp
constexpr auto ratio() const noexcept -> T;
```

Returns the relative position of the stored value in the interval.

The result is in `[0, 1]`.

Conceptually:

```text
(value() - Min) / (Max - Min)
```

Example:

```cpp
using level = xer::interval<float, 10.0f, 20.0f>;

auto x = level(15.0f);
auto r = x.ratio(); // 0.5f
```

### `from_ratio`

```cpp
static constexpr auto from_ratio(T ratio) -> interval;
```

Creates an interval value from a relative position.

The input ratio is treated as a bounded value in `[0, 1]`.

Finite input is clamped into `[0, 1]`.
`NaN` and infinity throw `std::domain_error`.

Conceptually:

```text
Min + ratio * (Max - Min)
```

Example:

```cpp
using gain = xer::interval<float, -1.0f, 1.0f>;

auto center = gain::from_ratio(0.5f);
// center.value() == 0.0f
```

---

## Comparison

`interval` represents a linear bounded value, so comparison operators are provided.

At minimum, the type supports:

```cpp
operator==
operator<=>
```

The remaining comparison operators are available through ordinary C++ comparison rewriting.

Comparison is based on the stored scalar value.

Unlike `cyclic`, `interval` does not use tolerance-based equality.
Since `interval` rejects `NaN`, ordinary linear ordering is meaningful.

Example:

```cpp
auto a = xer::interval<float>(0.25f);
auto b = xer::interval<float>(0.75f);

if (a < b) {
    // true
}
```

---

## Arithmetic Between Interval Values

Arithmetic between values of the same `interval` type is supported.

```cpp
operator+
operator-
operator*
operator/
```

The operation is ordinary scalar arithmetic on the stored values, followed by validation and clamping.

Example:

```cpp
using component = xer::interval<float>;

auto a = component(0.8f);
auto b = component(0.5f);

auto sum = a + b;       // 1.0f
auto product = a * b;   // 0.4f
auto diff = b - a;      // 0.0f
```

Division by an interval value whose stored value is zero throws `std::domain_error`.

---

## Arithmetic with Right-Hand Scalar Values

The following forms are supported:

```cpp
interval + scalar
interval - scalar
interval * scalar
interval / scalar
```

They are useful for increasing, decreasing, scaling, and dividing bounded values.

Example:

```cpp
using component = xer::interval<float>;

auto brightness = component(0.5f);

brightness += 0.25f; // 0.75f
brightness *= 2.0f;  // 1.0f
brightness -= 2.0f;  // 0.0f
```

The scalar is converted to the interval's value type and then validated.

`NaN`, infinity, and division by zero throw `std::domain_error`.

---

## Left-Hand Scalar Multiplication

Scalar multiplication is also supported in the left-hand form:

```cpp
scalar * interval
```

Example:

```cpp
using component = xer::interval<float>;

auto brightness = component(0.75f);
auto dimmed = 0.5f * brightness;
// dimmed.value() == 0.375f
```

This form is provided because multiplication is natural in either order.

---

## Unsupported Left-Hand Scalar Forms

The following forms are intentionally not provided:

```cpp
scalar + interval
scalar - interval
scalar / interval
```

Scalar addition and subtraction are intended to express increasing or decreasing the interval value.
For readability, the interval value should appear on the left-hand side.

Scalar division by an interval value has a reciprocal-like meaning and is not considered a common bounded-value operation.

If such behavior is needed, callers can use `value()` explicitly.

---

## Compound Assignment

`interval` provides compound assignment operators.

With another interval value:

```cpp
operator+=
operator-=
operator*=
operator/=
```

With a right-hand scalar value:

```cpp
operator+=
operator-=
operator*=
operator/=
```

Each operation preserves the interval invariant.

Example:

```cpp
using component = xer::interval<float>;

auto x = component(0.5f);

x += 0.2f; // 0.7f
x *= 2.0f; // 1.0f
x /= 4.0f; // 0.25f
```

---

## Unary Operators

Unary plus and unary minus are provided.

```cpp
+x
-x
```

Unary plus returns the value unchanged.

Unary minus negates the stored value and then constructs a new interval value from the result.
For the default `[0, 1]` interval, this usually clamps to `0`.

Example:

```cpp
auto x = xer::interval<float>(0.25f);
auto y = -x;
// y.value() == 0.0f
```

For a symmetric interval, unary minus behaves more naturally.

```cpp
using gain = xer::interval<float, -1.0f, 1.0f>;

auto x = gain(0.25f);
auto y = -x;
// y.value() == -0.25f
```

---

## Error Handling Model

`interval` uses exceptions only for exceptional numeric conditions.

This differs from ordinary XER APIs that return `xer::result` for normal recoverable failures.

The reason is that `interval` is a value type with a simple invariant.
`NaN`, infinity, and division by zero are treated as invalid numeric states rather than ordinary input failures.

This design keeps normal arithmetic expressions readable:

```cpp
auto x = xer::interval<float>(0.5f);
auto y = x + 0.25f;
auto z = 0.5f * y;
```

---

## Typical Uses

### Color Components

```cpp
using component = xer::interval<float>;

auto r = component(1.25f);  // 1.0f
auto g = component(0.5f);   // 0.5f
auto b = component(-0.25f); // 0.0f
```

### Gain

```cpp
using gain = xer::interval<float, -1.0f, 1.0f>;

auto center = gain::from_ratio(0.5f);
// center.value() == 0.0f
```

### Brightness Adjustment

```cpp
using component = xer::interval<float>;

auto brightness = component(0.5f);
brightness += 0.25f;
brightness *= 2.0f;
```

---

## Relationship to Other Headers

`<xer/interval.h>` is related to the following headers:

- `<xer/cyclic.h>`
- `<xer/arithmetic.h>`
- `<xer/stdfloat.h>`

The rough boundary is:

- `<xer/cyclic.h>` handles circular normalized values
- `<xer/interval.h>` handles linear bounded values
- `<xer/arithmetic.h>` provides arithmetic helper functions
- `<xer/stdfloat.h>` provides floating-point type aliases and literals

`interval` is not absorbed into `<xer/arithmetic.h>` because it is a value type with an invariant, not merely an arithmetic helper function group.

---

## Documentation Notes

When documenting `interval`, it is important to make the following points explicit:

- the interval is closed
- the default interval is `[0, 1]`
- finite out-of-range values are clamped
- `NaN` and infinity throw
- division by zero throws
- arithmetic is scalar arithmetic followed by clamping
- this is not mathematical interval arithmetic
- `interval` is linear and non-wrapping, unlike `cyclic`

---

## Example

```cpp
#include <xer/interval.h>
#include <xer/stdio.h>

auto main() -> int
{
    using component = xer::interval<float>;

    const auto r = component(1.25f);
    const auto g = component(0.5f);
    const auto b = component(-0.25f);

    if (!xer::printf(u8"r = %g\n", static_cast<double>(r.value())).has_value()) {
        return 1;
    }
    if (!xer::printf(u8"g = %g\n", static_cast<double>(g.value())).has_value()) {
        return 1;
    }
    if (!xer::printf(u8"b = %g\n", static_cast<double>(b.value())).has_value()) {
        return 1;
    }

    auto brightness = component(0.5f);
    brightness += 0.25f;

    if (!xer::printf(
            u8"brightness = %g\n",
            static_cast<double>(brightness.value()))
            .has_value()) {
        return 1;
    }

    return 0;
}
```

This example shows the basic XER style:

- use the public header
- construct bounded values naturally
- let finite out-of-range input clamp
- use XER formatted output for examples
- check fallible output operations explicitly

---

## See Also

- `policy_interval.md`
- `policy_cyclic.md`
- `header_cyclic.md`
- `policy_arithmetic.md`
- `header_arithmetic.md`

---

## Explicit Conversion with `cyclic`

`interval` provides explicit helper functions for conversion with `cyclic`.

```cpp
template <std::floating_point T, T Min, T Max>
constexpr auto to_cyclic(interval<T, Min, Max> value) noexcept -> cyclic<T>;

template <std::floating_point T>
constexpr auto to_interval(cyclic<T> value) -> interval<T>;
```

`to_cyclic(interval)` first maps the interval value to `[0, 1]` using `ratio()`, then constructs a cyclic value from that ratio.
Because `cyclic` uses `[0, 1)`, the upper endpoint of an interval maps to the zero position of the cycle.

```cpp
using level = xer::interval<float, 10.0f, 20.0f>;

auto a = xer::to_cyclic(level(10.0f)); // 0.0f
auto b = xer::to_cyclic(level(15.0f)); // 0.5f
auto c = xer::to_cyclic(level(20.0f)); // 0.0f
```

`to_interval(cyclic)` maps a cyclic value to the default interval `[0, 1]`.

For custom interval bounds, use `from_ratio` explicitly.

```cpp
using gain = xer::interval<float, -1.0f, 1.0f>;

auto value = gain::from_ratio(hue.ratio());
```

Implicit conversion constructors are intentionally not provided.

---

# `<xer/color.h>`

## Purpose

`<xer/color.h>` provides color-system value types and color conversion functions.

The purpose of this header is to support practical formula-based color representation and conversion in a lightweight XER style.

The initial supported color systems are:

- RGB
- Grayscale
- CMY
- HSV
- CIE 1931 XYZ
- CIE 1976 L*a*b*
- CIE 1976 L*u*v*

This header does not attempt to become a complete color management system.
It does not handle ICC profiles, chromatic adaptation, spectral data, named colors, or color palette management.

---

## Main Entities

At minimum, `<xer/color.h>` provides the following class templates:

```cpp
template <std::floating_point T>
struct basic_rgb;

template <std::floating_point T>
struct basic_gray;

template <std::floating_point T>
struct basic_cmy;

template <std::floating_point T>
struct basic_hsv;

template <std::floating_point T>
struct basic_xyz;

template <std::floating_point T>
struct basic_lab;

template <std::floating_point T>
struct basic_luv;
```

It also provides ordinary `float` aliases:

```cpp
using rgb = basic_rgb<float>;
using gray = basic_gray<float>;
using cmy = basic_cmy<float>;
using hsv = basic_hsv<float>;
using xyz = basic_xyz<float>;
using lab = basic_lab<float>;
using luv = basic_luv<float>;
```

The public header is:

```cpp
#include <xer/color.h>
```

The implementation is provided through:

```cpp
#include <xer/bits/color.h>
```

Users should include the public header.

---

## Design Role

`<xer/color.h>` provides small value types and conversion functions for common color systems.

The design is based on these ideas:

- color values are simple structs with public data members
- ordinary aliases use `float`
- normalized bounded components use `xer::interval<T>`
- hue uses `xer::cyclic<T>`
- colorimetric spaces such as XYZ, Lab, and Luv use raw floating-point members
- conversion functions are free functions
- deterministic arithmetic conversions return the destination value directly

---

## Float Aliases

Although the templates can use `float`, `double`, or `long double`, practical color handling usually uses `float`.

For this reason, the ordinary aliases use `float`.

```cpp
xer::rgb color(0.25f, 0.5f, 0.75f);
```

If a different precision is needed, use the corresponding template directly.

```cpp
xer::basic_rgb<double> color(0.25, 0.5, 0.75);
```

---

## RGB

## `basic_rgb`

```cpp
template <std::floating_point T>
struct basic_rgb {
    using value_type = T;
    using component_type = interval<T>;

    component_type r;
    component_type g;
    component_type b;
};
```

`basic_rgb<T>` represents an RGB color with normalized red, green, and blue components.

Each component is represented by `interval<T>` and is therefore kept in `[0, 1]`.

```cpp
auto color = xer::rgb(1.25f, 0.5f, -0.25f);

// color.r.value() == 1.0f
// color.g.value() == 0.5f
// color.b.value() == 0.0f
```

### sRGB Assumption

The public type name is `rgb`, not `srgb`.

However, conversion between RGB and XYZ assumes sRGB with the D65 white point.

That means:

- `to_xyz(rgb)` treats RGB components as nonlinear sRGB components
- `to_rgb(xyz)` produces nonlinear sRGB components

This assumption must be kept in mind when using RGB together with XYZ, Lab, or Luv.

### Alpha

Alpha is not part of `basic_rgb`.

Alpha is mainly useful for graphics, compositing, and image processing.
It is not a general component of color itself and is unnecessary for areas such as printing, coating, lighting, and colorimetry.

If alpha support becomes necessary later, it should be provided as a separate type such as `basic_rgba<T>`.
It should not be mixed into `basic_rgb<T>`.

---

## Grayscale

## `basic_gray`

```cpp
template <std::floating_point T>
struct basic_gray {
    using value_type = T;
    using component_type = interval<T>;

    component_type y;
};
```

`basic_gray<T>` represents a display-oriented grayscale value. The component is represented by `interval<T>` and is kept in `[0, 1]`.

`to_luma_gray` computes simple luma directly from nonlinear sRGB components. `to_luminance_gray` computes relative luminance after sRGB decoding and then encodes it back to a display grayscale value. `to_gray` is an alias for `to_luma_gray`. `to_rgb(gray)` duplicates the grayscale component into RGB.

## CMY

## `basic_cmy`

```cpp
template <std::floating_point T>
struct basic_cmy {
    using value_type = T;
    using component_type = interval<T>;

    component_type c;
    component_type m;
    component_type y;
};
```

`basic_cmy<T>` represents a simple normalized CMY color.

Each component is represented by `interval<T>` and is therefore kept in `[0, 1]`.

CMY in XER is the simple complement model of RGB.

Conceptually:

```text
C = 1 - R
M = 1 - G
Y = 1 - B
```

and:

```text
R = 1 - C
G = 1 - M
B = 1 - Y
```

This is not a complete printer color-management model.
It does not represent CMYK, ink behavior, ICC profiles, or device-specific calibration.

---

## HSV

## `basic_hsv`

```cpp
template <std::floating_point T>
struct basic_hsv {
    using value_type = T;
    using hue_type = cyclic<T>;
    using component_type = interval<T>;

    hue_type h;
    component_type s;
    component_type v;
};
```

`basic_hsv<T>` represents hue, saturation, and value.

The components are:

- `h`: hue, represented by `cyclic<T>` in `[0, 1)`
- `s`: saturation, represented by `interval<T>` in `[0, 1]`
- `v`: value, represented by `interval<T>` in `[0, 1]`

Hue is circular, so it uses `cyclic<T>` rather than `interval<T>`.

For grayscale colors, where saturation is zero, hue is not meaningful.
The current conversion from RGB to HSV sets hue to zero in that case.

---

## XYZ

## `basic_xyz`

```cpp
template <std::floating_point T>
struct basic_xyz {
    using value_type = T;

    T x;
    T y;
    T z;
};
```

`basic_xyz<T>` represents CIE 1931 XYZ tristimulus values.

XYZ is used as the central connection point between RGB and the CIE Lab/Luv spaces.

XYZ components are raw floating-point values.
They are not represented by `interval<T>` because XYZ values are colorimetric quantities rather than normalized UI components.

The initial implementation uses D65 as the reference white for conversions.

---

## Lab

## `basic_lab`

```cpp
template <std::floating_point T>
struct basic_lab {
    using value_type = T;

    T l;
    T a;
    T b;
};
```

`basic_lab<T>` represents CIE 1976 L*a*b* values.

The member names are lowercase ASCII identifiers:

- `l`
- `a`
- `b`

The public API does not use identifiers such as `L*`, `a*`, or `b*`.

Lab components are raw floating-point values.
They are not represented by `interval<T>`.

Although `L*` is commonly in `[0, 100]`, the initial implementation does not force it into an interval wrapper.
The `a*` and `b*` components are signed and do not have a simple universal fixed range.

---

## Luv

## `basic_luv`

```cpp
template <std::floating_point T>
struct basic_luv {
    using value_type = T;

    T l;
    T u;
    T v;
};
```

`basic_luv<T>` represents CIE 1976 L*u*v* values.

The member names are lowercase ASCII identifiers:

- `l`
- `u`
- `v`

The public API does not use identifiers such as `L*`, `u*`, or `v*`.

Luv components are raw floating-point values.
They are not represented by `interval<T>`.

---

## Constructors

Each color type supports default construction and construction from component values.

### RGB

```cpp
constexpr basic_rgb();
constexpr basic_rgb(T r, T g, T b);
constexpr basic_rgb(component_type r, component_type g, component_type b) noexcept;
```

Default construction creates black.

```cpp
xer::rgb color;
// r == 0, g == 0, b == 0
```

Construction from raw component values clamps finite out-of-range values through `interval<T>`.

### CMY

```cpp
constexpr basic_cmy();
constexpr basic_cmy(T c, T m, T y);
constexpr basic_cmy(component_type c, component_type m, component_type y) noexcept;
```

Construction from raw component values clamps finite out-of-range values through `interval<T>`.

### HSV

```cpp
constexpr basic_hsv();
constexpr basic_hsv(T h, T s, T v);
constexpr basic_hsv(hue_type h, component_type s, component_type v) noexcept;
```

Hue is normalized through `cyclic<T>`.
Saturation and value are clamped through `interval<T>`.

### XYZ, Lab, and Luv

XYZ, Lab, and Luv store raw floating-point values.

```cpp
constexpr basic_xyz(T x, T y, T z) noexcept;
constexpr basic_lab(T l, T a, T b) noexcept;
constexpr basic_luv(T l, T u, T v) noexcept;
```

The initial implementation does not add special validation to these raw colorimetric values.

---

## Conversion Functions

Conversions are provided as free functions.

## RGB and CMY

```cpp
template <std::floating_point T>
constexpr auto to_cmy(basic_rgb<T> value) -> basic_cmy<T>;

template <std::floating_point T>
constexpr auto to_rgb(basic_cmy<T> value) -> basic_rgb<T>;
```

RGB and CMY conversion uses simple normalized complement conversion.

Example:

```cpp
const auto cmy = xer::to_cmy(xer::rgb(0.25f, 0.5f, 0.75f));

// cmy.c.value() == 0.75f
// cmy.m.value() == 0.5f
// cmy.y.value() == 0.25f
```

---

## RGB and HSV

```cpp
template <std::floating_point T>
constexpr auto to_hsv(basic_rgb<T> value) -> basic_hsv<T>;

template <std::floating_point T>
constexpr auto to_rgb(basic_hsv<T> value) -> basic_rgb<T>;
```

RGB and HSV conversion uses the common normalized HSV model.

- RGB components are in `[0, 1]`
- hue is normalized to `[0, 1)`
- saturation is in `[0, 1]`
- value is in `[0, 1]`

For grayscale RGB colors, `to_hsv` sets hue to zero.

Example:

```cpp
const auto hsv = xer::to_hsv(xer::rgb(0.25f, 0.5f, 0.75f));
const auto rgb = xer::to_rgb(hsv);
```

---

## RGB and XYZ

```cpp
template <std::floating_point T>
auto to_xyz(basic_rgb<T> value) -> basic_xyz<T>;

template <std::floating_point T>
auto to_rgb(basic_xyz<T> value) -> basic_rgb<T>;
```

RGB and XYZ conversion assumes sRGB with the D65 white point.

`to_xyz(rgb)` performs:

1. sRGB transfer-function decoding from nonlinear RGB to linear RGB
2. matrix conversion from linear RGB to XYZ

`to_rgb(xyz)` performs:

1. matrix conversion from XYZ to linear RGB
2. sRGB transfer-function encoding
3. clamping into RGB component intervals

The public names are `to_xyz(rgb)` and `to_rgb(xyz)`, not `to_xyz(srgb)` or `to_srgb(xyz)`.

Example:

```cpp
const auto xyz = xer::to_xyz(xer::rgb(1.0f, 1.0f, 1.0f));

// approximately D65 white:
// x == 0.95047f
// y == 1.0f
// z == 1.08883f
```

---

## XYZ and Lab

```cpp
template <std::floating_point T>
auto to_lab(basic_xyz<T> value) -> basic_lab<T>;

template <std::floating_point T>
constexpr auto to_xyz(basic_lab<T> value) -> basic_xyz<T>;
```

XYZ and Lab conversion uses CIE 1976 L*a*b* formulas with D65 as the reference white.

Example:

```cpp
const auto lab = xer::to_lab(xer::xyz(0.95047f, 1.0f, 1.08883f));

// approximately:
// l == 100
// a == 0
// b == 0
```

---

## XYZ and Luv

```cpp
template <std::floating_point T>
auto to_luv(basic_xyz<T> value) -> basic_luv<T>;

template <std::floating_point T>
auto to_xyz(basic_luv<T> value) -> basic_xyz<T>;
```

XYZ and Luv conversion uses CIE 1976 L*u*v* formulas with D65 as the reference white.

Example:

```cpp
const auto luv = xer::to_luv(xer::xyz(0.95047f, 1.0f, 1.08883f));

// approximately:
// l == 100
// u == 0
// v == 0
```

---

## Direct Conversion Policy

The initial API does not provide direct conversion functions for every possible pair of supported color systems.

For example, RGB to Lab can be written explicitly through XYZ:

```cpp
const auto xyz = xer::to_xyz(color);
const auto lab = xer::to_lab(xyz);
```

This keeps the public API small and avoids unnecessary duplication.

Direct conversion functions may be added later if they become clearly useful.

---

## Error and Exception Policy

Color conversion functions generally return the destination color value directly.

They do not return `xer::result`, because the supported conversions are deterministic arithmetic operations and do not have ordinary recoverable failure modes.

However:

- RGB, grayscale, CMY, and HSV normalized components use `interval<T>`
- hue uses `cyclic<T>`

Therefore, invalid finite-state cases such as `NaN` or infinity may be rejected according to the policies of `interval<T>` and `cyclic<T>`.

For raw colorimetric types such as XYZ, Lab, and Luv, the initial implementation stores raw floating-point values directly and does not add special validation.

---

## Supported and Unsupported Color Systems

### Supported

The initial supported color systems are:

- RGB
- Grayscale
- CMY
- HSV
- CIE 1931 XYZ
- CIE 1976 L*a*b*
- CIE 1976 L*u*v*

### Unsupported

The following systems are outside the scope of XER:

- Munsell color system
- PCCS
- Ostwald color system
- NCS
- ABC tone system

This is not merely a temporary omission.
Unless there is a major reason to reconsider, these systems should remain outside the scope of XER.

The main reason is that they are color-order, color-notation, perceptual, or tone-classification systems rather than lightweight formula-based numeric color spaces suitable for XER's core API.

---

## Deferred Items

The following items are deferred:

- alpha / RGBA
- CMYK
- HSL
- HWB
- linear RGB
- configurable white points
- chromatic adaptation
- ICC profile handling
- color appearance models
- spectral color representation
- color difference formulas such as Delta E
- color temperature
- named colors
- palette utilities
- direct conversion functions for every pair of supported spaces

Some of these may be useful later, but they are not part of the initial color-system facility.

---

## Relationship to Other Headers

`<xer/color.h>` is related to:

- `<xer/interval.h>`
- `<xer/cyclic.h>`
- `<xer/stdfloat.h>`

The rough boundary is:

- `<xer/interval.h>` handles bounded linear scalar values
- `<xer/cyclic.h>` handles circular scalar values such as hue
- `<xer/color.h>` composes these into color-system value types and conversions
- `<xer/stdfloat.h>` provides floating-point type aliases and literals

---

## Documentation Notes

When documenting `<xer/color.h>`, it is important to state:

- `rgb` is used as the code name, not `srgb`
- RGB/XYZ conversion assumes sRGB/D65
- RGB, grayscale, CMY, and HSV normalized components use `interval<T>`
- HSV hue uses `cyclic<T>`
- XYZ, Lab, and Luv use raw floating-point members
- alpha is not part of RGB
- the facility is not a complete color management system
- unsupported color systems are intentionally outside the scope

---

## Example

```cpp
#include <xer/color.h>
#include <xer/stdio.h>

auto main() -> int
{
    const xer::rgb color(0.25f, 0.5f, 0.75f);

    const auto cmy = xer::to_cmy(color);
    const auto hsv = xer::to_hsv(color);
    const auto xyz = xer::to_xyz(color);
    const auto lab = xer::to_lab(xyz);

    if (!xer::printf(
             u8"RGB: r=%g, g=%g, b=%g\n",
             static_cast<double>(color.r.value()),
             static_cast<double>(color.g.value()),
             static_cast<double>(color.b.value()))
             .has_value()) {
        return 1;
    }

    if (!xer::printf(
             u8"CMY: c=%g, m=%g, y=%g\n",
             static_cast<double>(cmy.c.value()),
             static_cast<double>(cmy.m.value()),
             static_cast<double>(cmy.y.value()))
             .has_value()) {
        return 1;
    }

    if (!xer::printf(
             u8"HSV: h=%g, s=%g, v=%g\n",
             static_cast<double>(hsv.h.value()),
             static_cast<double>(hsv.s.value()),
             static_cast<double>(hsv.v.value()))
             .has_value()) {
        return 1;
    }

    if (!xer::printf(
             u8"Lab: l=%g, a=%g, b=%g\n",
             static_cast<double>(lab.l),
             static_cast<double>(lab.a),
             static_cast<double>(lab.b))
             .has_value()) {
        return 1;
    }

    return 0;
}
```

This example shows the basic XER style:

- use the public header
- construct an `rgb` value directly
- convert through free functions
- use `value()` when printing interval components
- use `<xer/stdio.h>` for output in examples
- check fallible output operations explicitly

---

## See Also

- `policy_color.md`
- `policy_interval.md`
- `policy_cyclic.md`
- `header_interval.md`
- `header_cyclic.md`

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

sq
cb
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
auto a = 9.8 * m / sq(sec);
auto f = 2.0 * kg * m / sq(sec);
```

### Why This Matters

This avoids the need to define every composite unit as a separate fixed name.

---

## Square and Cube Helpers for Units and Quantities

`<xer/quantity.h>` provides `sq` and `cb` for units and quantities.

```cpp
sq(unit)
cb(unit)
sq(quantity)
cb(quantity)
```

### Role

These helpers make repeated multiplication easier to read in unit expressions.
For example:

```cpp
using namespace xer::units;

auto acceleration = 9.8 * m / sq(sec);
```

This is equivalent in meaning to:

```cpp
auto acceleration = 9.8 * m / (sec * sec);
```

For quantities, `sq` and `cb` multiply the stored value and combine the dimension exponents accordingly.

### Symbolic Unit Aliases

For common base units, symbolic aliases are also provided under `xer::units`:

```cpp
m²
m³
sec²
sec³
```

These are aliases for the corresponding square or cube unit expressions:

```cpp
m²   // sq(m)
m³   // cb(m)
sec² // sq(sec)
sec³ // cb(sec)
```

They are intended as readable symbolic notation.
The ASCII forms `sq(m)`, `cb(m)`, `sq(sec)`, and `cb(sec)` remain available as the portable spelling.
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

### Squared and Cubed Base Units

At minimum:

* `m²`
* `m³`
* `sec²`
* `sec³`

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
* dividing distance by time to obtain velocity\n* using `sq`, `cb`, `m²`, `m³`, and `sec²` in unit expressions
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

# `<xer/matrix.h>`

## Purpose

`<xer/matrix.h>` provides fixed-size matrix and affine transform helpers in XER.

The initial purpose of this header is deliberately practical and limited.
It is not intended to become a full linear algebra framework at the beginning.
Instead, it provides enough functionality to express common 2D and 3D affine transforms and their inverse transforms in a clear, lightweight way.

---

## Main Role

The main role of `<xer/matrix.h>` is to provide:

- fixed-size row-major matrices
- 3x3 and 4x4 matrix aliases
- 3x1 and 4x1 column-vector aliases
- ordinary matrix multiplication
- identity matrix creation
- inverse calculation for 3x3 and 4x4 matrices
- helper functions for 2D and 3D affine transforms

This makes it possible to write transform code such as:

```cpp
const xer::vector3<double> point{2.0, 3.0, 1.0};

const auto transform =
    xer::translate2(10.0, 20.0) *
    xer::scale2(1.5, 4.0);

const auto transformed = transform * point;
```

---

## Main Entities

At minimum, `<xer/matrix.h>` provides the following entities:

```cpp
template <std::floating_point T, std::size_t Rows, std::size_t Cols>
class matrix;

template <std::floating_point T>
using matrix3 = matrix<T, 3, 3>;

template <std::floating_point T>
using matrix4 = matrix<T, 4, 4>;

template <std::floating_point T>
using vector3 = matrix<T, 3, 1>;

template <std::floating_point T>
using vector4 = matrix<T, 4, 1>;
```

It also provides multiplication, identity matrix creation, inverse calculation, and affine transform helper functions.

---

## `matrix<T, Rows, Cols>`

`matrix<T, Rows, Cols>` is the fundamental fixed-size matrix type.

### Basic Shape

```cpp
template <std::floating_point T, std::size_t Rows, std::size_t Cols>
class matrix;
```

### Element Type

The element type is restricted to floating-point types.

The main intended types are:

* `float`
* `double`
* `long double`

This keeps the first implementation focused on geometric transforms and numeric operations where fractional values are normal.

### Storage Model

The matrix is stored as a fixed-size row-major value.

Conceptually, the element at row `r` and column `c` is accessed as:

```cpp
m(r, c)
```

Rows and columns are zero-based.
Bounds are not checked by `operator()`.

### Construction

A default-constructed matrix is a zero matrix.

A matrix may also be constructed from exactly `Rows * Cols` values in row-major order:

```cpp
xer::matrix<double, 2, 3> value{
    1.0, 2.0, 3.0,
    4.0, 5.0, 6.0
};
```

The exact number of values is required so that incomplete or excessive matrix literals are detected at compile time.

---

## Matrix Aliases

The header provides aliases for the matrix sizes used by the initial affine-transform functionality.

```cpp
template <std::floating_point T>
using matrix3 = matrix<T, 3, 3>;

template <std::floating_point T>
using matrix4 = matrix<T, 4, 4>;
```

### Role

* `matrix3<T>` is primarily used for 2D homogeneous affine transforms.
* `matrix4<T>` is primarily used for 3D homogeneous affine transforms.

---

## Column Vector Aliases

The header also provides aliases for homogeneous column vectors.

```cpp
template <std::floating_point T>
using vector3 = matrix<T, 3, 1>;

template <std::floating_point T>
using vector4 = matrix<T, 4, 1>;
```

### Role

* `vector3<T>` is typically used as a 2D homogeneous column vector: `(x, y, 1)`.
* `vector4<T>` is typically used as a 3D homogeneous column vector: `(x, y, z, 1)`.

They are aliases of `matrix`, not separate vector classes.
This keeps the first implementation simple and makes matrix-vector multiplication the ordinary matrix multiplication operation.

---

## Matrix Multiplication

`<xer/matrix.h>` provides ordinary row-by-column matrix multiplication.

```cpp
auto operator*(
    const matrix<T, R, C>& left,
    const matrix<T, C, K>& right) noexcept
    -> matrix<T, R, K>;
```

### Role

This single operation covers:

* matrix × matrix
* matrix × column vector
* affine transform composition
* applying an affine transform to a homogeneous point

### Transform Composition Order

XER uses column vectors in this matrix facility.
Therefore, in an expression such as:

```cpp
const auto transform = translate2<double>(10.0, 20.0) * scale2<double>(2.0, 3.0);
const auto result = transform * point;
```

`point` is transformed by the rightmost transform first.
In this example, scaling is applied first, and translation is applied afterward.

---

## Identity Matrices

The header provides a generic identity matrix helper:

```cpp
template <std::floating_point T, std::size_t N>
auto identity_matrix() noexcept -> matrix<T, N, N>;
```

It also provides convenience helpers for the two main affine-transform sizes:

```cpp
template <std::floating_point T>
auto identity3() noexcept -> matrix3<T>;

template <std::floating_point T>
auto identity4() noexcept -> matrix4<T>;
```

---

## Inverse Matrices

The header provides inverse calculation for 3x3 and 4x4 matrices:

```cpp
template <std::floating_point T>
auto inverse(const matrix<T, 3, 3>& value) noexcept
    -> xer::result<matrix<T, 3, 3>>;

template <std::floating_point T>
auto inverse(const matrix<T, 4, 4>& value) noexcept
    -> xer::result<matrix<T, 4, 4>>;
```

### Error Handling

If the matrix is singular or too close to singular for the implemented calculation, `inverse` returns failure.

The current implementation reports this as `error_t::divide_by_zero`.
This expresses the fact that the inverse operation requires division by a usable pivot value.

---

## 2D Affine Transform Helpers

For 2D homogeneous column vectors, the header provides 3x3 transform helpers.

```cpp
template <std::floating_point T>
auto translate2(T tx, T ty) noexcept -> matrix3<T>;

template <std::floating_point T>
auto scale2(T sx, T sy) noexcept -> matrix3<T>;

template <std::floating_point T>
auto rotate2(T radian) noexcept -> matrix3<T>;
```

### Rotation Direction

`rotate2` uses radians and follows the ordinary mathematical convention: positive angles rotate counterclockwise.

---

## 3D Affine Transform Helpers

For 3D homogeneous column vectors, the header provides 4x4 transform helpers.

```cpp
template <std::floating_point T>
auto translate3(T tx, T ty, T tz) noexcept -> matrix4<T>;

template <std::floating_point T>
auto scale3(T sx, T sy, T sz) noexcept -> matrix4<T>;

template <std::floating_point T>
auto rotate_x(T radian) noexcept -> matrix4<T>;

template <std::floating_point T>
auto rotate_y(T radian) noexcept -> matrix4<T>;

template <std::floating_point T>
auto rotate_z(T radian) noexcept -> matrix4<T>;
```

### Rotation Units

The rotation helpers take raw radian values.

Angle quantities, `cyclic`, and other higher-level angle abstractions are not mixed into the first matrix API.
Callers may convert into radians before calling these functions when needed.

---

## Scope of the Initial Matrix Facility

The initial matrix facility is intentionally small.

It focuses on:

* 2D affine transforms using 3x3 matrices
* 3D affine transforms using 4x4 matrices
* homogeneous column vectors
* inverse transforms for 3x3 and 4x4 matrices

It does not initially try to provide a complete linear algebra library.

Deferred or intentionally omitted items include:

* dynamic-size matrices
* decomposition algorithms
* eigenvalues or eigenvectors
* specialized vector classes
* determinant APIs
* full numerical linear algebra facilities

These may be considered later only if they become necessary.

---

## Relationship to Other Headers

`<xer/matrix.h>` should be understood together with:

* `policy_project_outline.md`
* `policy_arithmetic.md`
* `header_arithmetic.md`
* `header_cyclic.md`
* `header_quantity.md`

The rough boundary is:

* `<xer/arithmetic.h>` handles scalar arithmetic and comparison helpers
* `<xer/cyclic.h>` handles circular values such as normalized angles and directions
* `<xer/quantity.h>` handles physical quantities and units
* `<xer/matrix.h>` handles fixed-size matrices and affine transforms

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that the matrix type is fixed-size and row-major
* that column vectors are represented as `matrix<T, N, 1>` aliases
* that the initial focus is 2D and 3D affine transforms
* that inverse calculation is provided for 3x3 and 4x4 matrices
* that rotation helpers take radians

Detailed numerical behavior and future linear algebra expansion should be documented separately when those features are added.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* applying a 2D affine transform to a point
* composing translation, scaling, and rotation transforms
* applying a 3D affine transform to a point
* computing an inverse transform and restoring the original point

These are good candidates for executable examples in `examples/`.

---

## Example

```cpp
#include <xer/matrix.h>

auto main() -> int
{
    const xer::vector3<double> point{2.0, 3.0, 1.0};

    const auto transform =
        xer::translate2(10.0, 20.0) *
        xer::scale2(1.5, 4.0);

    const auto transformed = transform * point;
    const auto inverse = xer::inverse(transform);
    if (!inverse.has_value()) {
        return 1;
    }

    const auto restored = *inverse * transformed;

    static_cast<void>(restored);
    return 0;
}
```

This example shows the normal XER style:

* represent points as homogeneous column vectors
* compose transforms with matrix multiplication
* apply a transform by multiplying the matrix and the point
* check `xer::result` explicitly when computing an inverse matrix

---

## See Also

* `policy_project_outline.md`
* `policy_arithmetic.md`
* `header_arithmetic.md`
* `header_cyclic.md`
* `header_quantity.md`

---

# `<xer/image.h>`

## Purpose

`<xer/image.h>` provides lightweight image and framebuffer facilities.

The initial purpose of this header is not full photo editing or complete image-file handling. It is a small framebuffer-oriented layer for fixed-size canvases, VRAM-style emulation, simple drawing, and later integration with Tcl/Tk photo images.

Pure image processing and drawing belong in `<xer/image.h>`. Tcl/Tk photo integration belongs in `<xer/tk.h>`.

---

## Namespace

Image-related types and functions are placed in the `xer::image` namespace.

The previous top-level image type has been renamed to `xer::image::canvas` so that `xer::image` can be used as the namespace for image storage, drawing, and image processing.

---

## Main Entities

At minimum, `<xer/image.h>` provides the following entities:

```cpp
namespace xer::image {

struct point;
struct pointf;
struct size;
struct sizef;
struct rect;
struct rectf;

struct pixel;

struct argb32_policy;
struct rgba32_policy;
struct rgb24_policy;
struct bgr24_policy;

template <std::size_t Width,
          std::size_t Height,
          class Policy = argb32_policy>
class canvas;

template <class Policy = argb32_policy>
using dynamic_canvas = canvas<0, 0, Policy>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_hline(canvas<Width, Height, Policy>& img,
                int x,
                int y,
                int length,
                pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_hline(canvas<Width, Height, Policy>& img,
                point p,
                int length,
                pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_vline(canvas<Width, Height, Policy>& img,
                int x,
                int y,
                int length,
                pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_vline(canvas<Width, Height, Policy>& img,
                point p,
                int length,
                pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line(canvas<Width, Height, Policy>& img,
               int x0,
               int y0,
               int x1,
               int y1,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line(canvas<Width, Height, Policy>& img,
               point p0,
               point p1,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line_aa(canvas<Width, Height, Policy>& img,
                  float x0,
                  float y0,
                  float x1,
                  float y1,
                  pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line_aa(canvas<Width, Height, Policy>& img,
                  float x0,
                  float y0,
                  float x1,
                  float y1,
                  float width,
                  pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line_aa(canvas<Width, Height, Policy>& img,
                  pointf p0,
                  pointf p1,
                  pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line_aa(canvas<Width, Height, Policy>& img,
                  pointf p0,
                  pointf p1,
                  float width,
                  pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_rect(canvas<Width, Height, Policy>& img,
               int x,
               int y,
               int width,
               int height,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_rect(canvas<Width, Height, Policy>& img,
               point origin,
               size extent,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_rect(canvas<Width, Height, Policy>& img,
               rect area,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_rect(canvas<Width, Height, Policy>& img,
               int x,
               int y,
               int width,
               int height,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_rect(canvas<Width, Height, Policy>& img,
               point origin,
               size extent,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_rect(canvas<Width, Height, Policy>& img,
               rect area,
               pixel color) noexcept -> void;

}
```

---

## Geometry Types

The geometry helper types are simple aggregate types used for future drawing and image-processing APIs.

```cpp
struct point {
    int x;
    int y;
};

struct pointf {
    float x;
    float y;
};

struct size {
    int width;
    int height;
};

struct sizef {
    float width;
    float height;
};

struct rect {
    int x;
    int y;
    int width;
    int height;
};

struct rectf {
    float x;
    float y;
    float width;
    float height;
};
```

Integer geometry types are intended for pixel-grid operations and clipping. Floating-point geometry types are intended for subpixel drawing, antialiasing, and future transformations. Drawing functions provide overloads for these helper types so that callers can pass `point`, `size`, `rect`, and `pointf` values directly instead of spelling out every coordinate component.

---

## Logical Pixel

`xer::image::pixel` represents a logical color value.

It is not the same thing as the physical framebuffer storage element. The physical storage format is controlled by the canvas policy.

The logical representation is ARGB in a 32-bit integer:

```text
0xAARRGGBB
```

The conceptual shape is:

```cpp
struct pixel {
    std::uint32_t argb = 0xff000000u;

    constexpr pixel() noexcept = default;
    constexpr explicit pixel(std::uint32_t value) noexcept;
    constexpr pixel(std::uint8_t red,
                    std::uint8_t green,
                    std::uint8_t blue) noexcept;
    constexpr pixel(std::uint8_t alpha,
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
The four-argument constructor follows ARGB order:

```text
alpha, red, green, blue
```

---

## Framebuffer Storage Policies

A canvas policy controls the physical framebuffer storage format.

A policy provides:

```cpp
using storage_type = /* physical storage element type */;

static constexpr auto get(const storage_type& value) noexcept -> pixel;
static constexpr auto encode(pixel value) noexcept -> storage_type;
static constexpr auto set(storage_type& dst, pixel value) noexcept -> void;
```

The initial policies are:

```cpp
argb32_policy
rgba32_policy
rgb24_policy
bgr24_policy
```

`argb32_policy` stores `0xAARRGGBB` and therefore matches the logical `pixel` representation directly.

`rgba32_policy` stores `0xRRGGBBAA`.

`rgb24_policy` and `bgr24_policy` store three 8-bit components and do not preserve alpha. Reading through these policies returns a logical pixel with alpha set to `0xff`.

---

## `canvas`

The primary canvas type is:

```cpp
template <std::size_t Width,
          std::size_t Height,
          class Policy = argb32_policy>
class canvas;
```

Fixed-size canvases are the main model because the initial use case is framebuffer-style handling such as VRAM emulation.

Examples:

```cpp
using screen = xer::image::canvas<256, 192>;
using sprite = xer::image::canvas<16, 16>;
using rgba_screen = xer::image::canvas<256, 192, xer::image::rgba32_policy>;
```

A dynamic-size canvas is represented as:

```cpp
canvas<0, 0, Policy>
```

The convenience alias is:

```cpp
template <class Policy = argb32_policy>
using dynamic_canvas = canvas<0, 0, Policy>;
```

Only these dimension forms are valid:

```text
Width > 0 && Height > 0
Width == 0 && Height == 0
```

Partial dynamic dimensions such as `canvas<0, 192>` and `canvas<256, 0>` are invalid.

---

## Public Pixel Access

The public pixel API uses logical pixels:

```cpp
auto get_pixel(std::size_t x, std::size_t y) const noexcept -> pixel;
auto set_pixel(int x, int y, pixel value) noexcept -> void;
auto set_pixel(int x, int y, pixel value, float coverage) noexcept -> void;
auto set_pixel_unchecked(std::size_t x,
                         std::size_t y,
                         pixel value) noexcept -> void;
auto set_pixel_unchecked(std::size_t x,
                         std::size_t y,
                         pixel value,
                         float coverage) noexcept -> void;
```

`canvas::at()` is intentionally not provided.

Returning a reference to the physical storage element would expose the framebuffer layout and would be incorrect when the storage policy is not ARGB. `pixel` is logical. `Policy::storage_type` is physical.

`get_pixel` expects coordinates that are inside the canvas.

`set_pixel` accepts signed coordinates and does nothing when the coordinates are outside the canvas boundary.

The coverage overloads blend the source pixel over the destination. Coverage is clamped to `[0.0f, 1.0f]`. A coverage value of `0.0f` leaves the destination unchanged. A coverage value of `1.0f` applies the source pixel alpha normally.

`set_pixel_unchecked` does not perform boundary checks. The caller must guarantee that `x < width()` and `y < height()`. It is intended for code that has already performed clipping or bounds checks outside the inner drawing loop.

---

## Basic Member Functions

`canvas` provides basic size and utility operations:

```cpp
auto width() const noexcept -> std::size_t;
auto height() const noexcept -> std::size_t;
auto size() const noexcept -> std::size_t;
auto empty() const noexcept -> bool;
auto contains(int x, int y) const noexcept -> bool;
auto contains(point p) const noexcept -> bool;
auto get_pixel(point p) const noexcept -> pixel;
auto set_pixel(point p, pixel value) noexcept -> void;
auto set_pixel(point p, pixel value, float coverage) noexcept -> void;
auto fill(pixel value) noexcept -> void;
auto clear() noexcept -> void;
```

`clear()` fills the canvas with opaque black.

---

## Drawing Functions

The initial drawing functions are simple framebuffer helpers:

```cpp
draw_hline
draw_vline
draw_line
draw_line_aa
draw_rect
fill_rect
```

Integer drawing coordinates use `int` rather than `std::size_t` because drawing often benefits from clipping negative coordinates.

Drawing operations clip to the canvas bounds. If the target area is fully outside the canvas, nothing is drawn.

After clipping, `draw_hline`, `draw_vline`, and `fill_rect` write directly to framebuffer storage. They do not call `set_pixel` for every pixel. This keeps inner loops based on simple pointer or stride increments instead of repeated coordinate-to-offset calculation.

`draw_line` uses a simple Bresenham-style integer line algorithm. It still checks each generated point against the canvas boundary, but writes through `set_pixel_unchecked` after that check.

`draw_line_aa` uses floating-point pixel-center coordinates and draws an antialiased capsule-shaped stroke. The overload without a width argument draws a one-pixel-wide antialiased line. The width overload takes the width before the color argument. The `pointf` overloads are equivalent to the scalar-coordinate overloads.

The `draw_rect` and `fill_rect` overloads accept either `point` plus `size`, or a single `rect`. The scalar-coordinate overloads remain available for callers that already have separate coordinate values.

---

## Relationship to Tcl/Tk

`<xer/image.h>` does not depend on Tcl/Tk.

Tk photo bridge functions should live in `<xer/tk.h>`. They may convert between Tk photo image blocks and `xer::image::canvas` or `xer::image::dynamic_canvas` later, but pure image storage, drawing, and image processing remain in `<xer/image.h>`.

---

## Deferred Items

The following items are deferred from the first implementation:

- blur
- mosaic
- affine transformation
- raster scroll
- grayscale conversion
- image flipping
- circle drawing
- file format loading and saving
- direct Tk photo conversion helpers

These can be added once the basic framebuffer type is stable.

---

## Example

```cpp
#include <xer/image.h>
#include <xer/stdio.h>

auto main() -> int
{
    xer::image::canvas<4, 4> img;

    img.clear();

    // This line intentionally starts outside the canvas.
    // XER clips it to the framebuffer boundary.
    xer::image::draw_hline(
        img,
        -2,
        1,
        4,
        xer::image::pixel(0xffu, 0x00u, 0x00u));

    const auto value = img.get_pixel(0, 1);
    return value.argb == 0xffff0000u ? 0 : 1;
}
```

---

## See Also

- `policy_image.md`
- `header_tk.md`
- `header_color.md`

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

# `<xer/cmdline.h>`

## Purpose

`<xer/cmdline.h>` provides command-line argument handling facilities for the current process.

The purpose of this header is to make command-line arguments available as UTF-8 strings without requiring the caller to pass `main`'s `argc` and `argv` around manually.

This is useful in situations such as:

* code that runs outside `main`
* non-local object initialization
* code running on threads other than the main thread
* utility functions where carrying `argc` and `argv` explicitly would be awkward

---

## Main Entities

At minimum, `<xer/cmdline.h>` provides the following entities:

```cpp
using cmdline_arg =
    std::pair<std::u8string_view, std::u8string_view>;

class cmdline;

auto parse_arg(std::u8string_view value) noexcept -> cmdline_arg;

auto get_cmdline() -> xer::result<cmdline>;
````

---

## `cmdline`

`cmdline` owns an argv-like sequence of UTF-8 strings.

```cpp
class cmdline;
```

Internally, it stores command-line arguments as:

```cpp
std::vector<std::u8string>
```

The class itself does not interpret options.
It is responsible only for owning and exposing the argument sequence.

### Basic Operations

```cpp
auto size() const noexcept -> std::size_t;
auto empty() const noexcept -> bool;

auto args() const noexcept -> std::span<const std::u8string>;

auto at(std::size_t index) const -> xer::result<std::u8string_view>;
```

### `size`

`size()` returns the number of stored arguments.

### `empty`

`empty()` returns whether the argument list is empty.

In normal successful use of `get_cmdline`, the command-line list is expected to contain at least the program name, but callers should not rely on that in manually constructed `cmdline` objects.

### `args`

`args()` returns a span over the raw stored UTF-8 arguments.

The returned span and its string references are valid as long as the `cmdline` object remains alive and is not modified.

### `at`

`at(index)` returns one raw argument as `std::u8string_view`.

If `index` is out of range, it returns an error through `xer::result`.

---

## `parse_arg`

```cpp
auto parse_arg(std::u8string_view value) noexcept -> cmdline_arg;
```

`parse_arg` parses one raw command-line argument according to XER's simple command-line rule.

The return value is a pair:

```cpp
{ option_name, value }
```

The meaning is:

* if `first` is not empty, the argument is an option
* if `first` is empty, the argument is an ordinary value
* `second` contains the option value or the ordinary value

---

## Supported Argument Forms

XER recognizes only simple long-option forms.

Supported option forms are:

```text
--option
--option=value
```

Ordinary values are also accepted:

```text
value
```

A single-leading-hyphen form such as `-x` is not treated as an option.
It is treated as an ordinary value.

### Examples

```text
--name        -> { "name", "" }
--name=       -> { "name", "" }
--name=value  -> { "name", "value" }
value         -> { "", "value" }
-name         -> { "", "-name" }
--            -> { "", "--" }
--=value      -> { "", "--=value" }
```

`--name` and `--name=` are intentionally treated the same.

Distinguishing “no value” from “empty value” would require a more complex representation, and XER's initial command-line helper deliberately avoids that complexity.

---

## Why Short Options Are Not Special

`parse_arg` does not treat single-leading-hyphen arguments as options.

For example:

```text
-x
```

is parsed as:

```text
{ "", "-x" }
```

This is intentional.

The initial command-line model supports only:

* `--option`
* `--option=value`
* ordinary values

This keeps the rule simple and avoids introducing a larger command-line parser at this stage.

---

## `get_cmdline`

```cpp
auto get_cmdline() -> xer::result<cmdline>;
```

`get_cmdline` obtains the current process command-line arguments and returns them as a `cmdline` object.

The returned arguments are UTF-8 strings.

### Windows Behavior

On Windows, the implementation obtains the raw command line through:

```cpp
GetCommandLineW
```

and splits it through:

```cpp
CommandLineToArgvW
```

This avoids relying on CRT-specific globals such as `__wargv`.

That choice is intentional because command-line access should not depend on details of how the C runtime library is linked.

The resulting UTF-16 strings are converted to UTF-8.

### Linux Behavior

On Linux, the implementation reads:

```text
/proc/self/cmdline
```

This file contains the current process command-line arguments as NUL-separated byte strings.

The byte strings are interpreted as UTF-8 according to XER's Linux text assumptions.
If an argument is not valid UTF-8, `get_cmdline` fails.

Reading `/proc/self/cmdline` can theoretically fail in unusual environments.
In that case, `get_cmdline` returns an error through `xer::result`.

---

## Lifetime of Views

`cmdline::at` and `parse_arg` return `std::u8string_view` values.

These views do not own the underlying text.

For views obtained from a `cmdline` object, the referenced data remains valid only while the `cmdline` object remains alive and unchanged.

Example:

```cpp
const auto line = xer::get_cmdline();
if (!line.has_value()) {
    return 1;
}

const auto raw = line->at(1);
if (!raw.has_value()) {
    return 1;
}

const auto parsed = xer::parse_arg(*raw);
```

Here, `parsed.first` and `parsed.second` refer to the string owned by `line`.

---

## Error Handling

`<xer/cmdline.h>` follows XER's ordinary failure model.

`parse_arg` itself does not fail.
It is a simple view-based parser and returns an ordinary `cmdline_arg`.

`cmdline::at` can fail when the requested index is out of range.

`get_cmdline` can fail when the platform-specific command-line retrieval fails or when command-line data cannot be converted to XER's UTF-8 representation.

Typical failure conditions include:

* out-of-range argument access
* failure to retrieve the platform command line
* failure to read `/proc/self/cmdline`
* invalid UTF-8 in Linux command-line byte strings
* failure to convert Windows UTF-16 command-line strings to UTF-8

---

## Relationship to `main`

The ordinary C and C++ way to receive command-line arguments is through `main`.

```cpp
auto main(int argc, char** argv) -> int;
```

XER does not reject that approach.

However, `<xer/cmdline.h>` exists for cases where explicit `argc` / `argv` propagation is inconvenient or unavailable.

This means `get_cmdline` is a convenience facility for the current process, not a replacement for all uses of `main` arguments.

---

## Relationship to Process Handling

`<xer/cmdline.h>` handles the current process command line.

`<xer/process.h>` handles child process creation and management.

These are related topics, but they are intentionally separate:

* `cmdline.h` observes how the current process was launched
* `process.h` launches and controls child processes

This separation keeps each header focused.

---

## Design Role

`<xer/cmdline.h>` is intentionally small.

It is not a full command-line option parser.

In particular, it does not currently provide:

* short option parsing such as `-x`
* grouped short options such as `-abc`
* option terminator handling such as `--`
* automatic type conversion
* required-option validation
* help text generation
* subcommand handling

The initial feature is only a small argv retrieval and simple long-option parsing facility.

---

## Example

```cpp
#include <xer/cmdline.h>
#include <xer/stdio.h>

auto main() -> int
{
    const auto line = xer::get_cmdline();
    if (!line.has_value()) {
        return 1;
    }

    for (std::size_t i = 1; i < line->size(); ++i) {
        const auto raw = line->at(i);
        if (!raw.has_value()) {
            return 1;
        }

        const auto parsed = xer::parse_arg(*raw);

        if (!parsed.first.empty()) {
            if (!xer::printf(
                    u8"option %@ = %@\n",
                    parsed.first,
                    parsed.second)
                     .has_value()) {
                return 1;
            }
        } else {
            if (!xer::printf(u8"value %@\n", parsed.second).has_value()) {
                return 1;
            }
        }
    }

    return 0;
}
```

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that `cmdline` owns UTF-8 command-line arguments
* that `get_cmdline` obtains the current process arguments without using `main` parameters
* that Windows uses `GetCommandLineW` and `CommandLineToArgvW`
* that Linux reads `/proc/self/cmdline`
* that `parse_arg` recognizes only simple `--option` and `--option=value` forms
* that single-leading-hyphen arguments are treated as ordinary values

Detailed command-line parser behavior should not be implied.
This header is intentionally not a full option parsing framework.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* listing raw command-line arguments
* parsing `--option` and `--option=value`
* treating ordinary values separately from options
* showing that `-x` is treated as a value

These are good candidates for executable examples under `examples/`.

---

## See Also

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `policy_process.md`
* `header_process.md`

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

### XER-Specific Fractional-Second Extensions

In addition to the conversion specifications delegated to the underlying C library, XER provides the following fractional-second extensions:

* `%f`: microseconds as exactly six decimal digits
* `%L`: milliseconds as exactly three decimal digits

These specifiers are based on `tm_microsec`. They are supported only in their simple forms `%f` and `%L`; width, flag, and modifier forms such as `%3f` are rejected.

### Current Design Limits

At least in the current stage:

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
* advanced timezone features in `strftime`
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

    const auto text = xer::strftime(u8"%Y-%m-%d %H:%M:%S.%f", *utc);
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
* `XER_VERSION_SUFFIX`: suffix such as `a3`
* `XER_VERSION_STRING`: full version string such as `1.2.3-beta`

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
1.2.3-beta
```

may be interpreted as:

* major: `0`
* minor: `2`
* patch: `0`
* suffix: `a3`

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
