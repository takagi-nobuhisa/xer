# xer Reference Manual

Version: **v0.1.0a5**

## 1. Overview

**xer** is a header-only C++23 library that rethinks selected C standard-library style facilities with modern types, UTF-8-oriented text handling, and explicit error reporting.

The current release line is intentionally pragmatic:

- public headers live directly under `xer/`
- implementation details live under `xer/bits/`
- normal failure is expressed with `xer::result`
- UTF-8 is the primary string representation for public text APIs
- text I/O is built around explicit encodings instead of locale-dependent behavior
- the library prefers simple, C-like function names where that improves approachability

This manual is aligned with the current contents of `xer.zip` and is intended to be the English reference for **v0.1.0a5**.

## 2. Supported environment

Current project assumptions:

- Language: **C++23**
- Primary compiler target: **GCC 13.3.0 or later**
- Platforms: **Windows** and **Linux**
- Windows target baseline: **Windows 11 or later**

xer is header-only. Typical test builds in the repository use:

```bash
g++ -std=gnu++23 -I.. tests/test_xxx.cpp
````

## 3. Public headers

```text
xer/error.h
xer/assert.h
xer/string.h
xer/ctype.h
xer/stdlib.h
xer/json.h
xer/stdio.h
xer/path.h
xer/stdint.h
xer/arithmetic.h
xer/cyclic.h
xer/quantity.h
xer/time.h
xer/version.h
```

## 4. Common conventions

### 4.1 Error handling

Most fallible APIs return:

```cpp
xer::result<T>
```

which is an alias of:

```cpp
std::expected<T, xer::error<void>>
```

Additional detail can be attached with `xer::result<T, Detail>`.

As a general rule, normal public APIs take normal values, not `xer::result` arguments.
If the caller already has a `xer::result`, the caller is expected to unwrap the success value explicitly before passing it to the next API. The main exception is `<xer/arithmetic.h>`, where chaining and propagation through arithmetic helpers is an intentional part of the design. This policy keeps ordinary APIs simpler and avoids overload ambiguity. 

### 4.2 Text model

* Public string APIs generally use `char8_t`, `std::u8string`, and `std::u8string_view`
* Individual Unicode scalar values are generally represented as `char32_t`
* Multibyte conversion APIs explicitly distinguish `char`, `unsigned char`, and `char8_t`
* Text streams support **UTF-8** and **CP932**, with optional auto detection on input

### 4.3 Header organization

Only headers directly under `xer/` are public. Files under `xer/bits/` are implementation detail headers even when they contain reusable templates or inline functions.

### 4.4 Documentation direction

The current English reference manual is still maintained, but the long-term direction is to generate documentation from structured documentation fragments under `docs/` / `docs/bits/` together with executable examples under `examples/`. Code examples are intended to become the primary canonical source for user-facing example snippets.  

---

## 5. `<xer/error.h>`

Core error and result facilities.

### Main entities

```cpp
enum class xer::error_t : std::int32_t;
template<class Detail = void> struct xer::error;
template<class T, class Detail = void> using xer::result = std::expected<T, error<Detail>>;
constexpr auto xer::make_error(error_t code, std::source_location location = std::source_location::current()) noexcept -> error<void>;
template<class Detail, class T>
constexpr auto xer::make_error(error_t code, T&& value, std::source_location location = std::source_location::current()) noexcept -> error<Detail>;
```

### Notes

* Positive `error_t` values track `errno`-style codes where practical.
* Negative values are XER-specific extensions such as `logic_error`, `invalid_argument`, `io_error`, `encoding_error`, `not_found`, and `divide_by_zero`.
* `error<void>` stores the error code and creation source location.
* `error<Detail>` adds extra payload while keeping the same base error information.

---

## 6. `<xer/assert.h>`

Assertion macros used by the test suite and also available to library users.

### Main entities

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

### Notes

* Failures throw `xer::assertion_error` instead of terminating the process.
* Diagnostics include expression text and source location.

---

## 7. `<xer/stdint.h>`

Fixed-width integer aliases, numeric helpers, and integer literal suffixes.

### Main entities

* Re-exports of standard integer types such as `int8_t`, `uint32_t`, `intptr_t`, and `uintptr_t`
* `int128_t` and `uint128_t` when `__int128` is available
* `min_of<T>`, `max_of<T>`, `bit_width_of<T>`
* Integer literal suffixes in `xer::literals::integer_literals`

### Literal suffixes

```cpp
_i8  _i16  _i32  _i64
_u8  _u16  _u32  _u64
_i128  _u128   // when supported
```

### Notes

* Literal parsing supports decimal, octal, hexadecimal, binary (`0b...`), and digit separators (`'`).
* Range checking is performed at compile time.

---

## 8. `<xer/arithmetic.h>`

Arithmetic helper functions that avoid surprising mixed-type behavior and report failure explicitly where needed.

### Main function groups

#### Integer arithmetic

```cpp
add   uadd
sub   usub
mul   umul
div   udiv
mod   umod
```

#### Comparison

```cpp
eq  ne  lt  le  gt  ge
```

#### Range and bounds helpers

```cpp
in_range
min
max
clamp
```

#### Absolute-value helpers

```cpp
abs
uabs
```

### Notes

* Integer arithmetic accepts mixed signed and unsigned operands.
* `<xer/arithmetic.h>` is the main public-header exception to the general “no `xer::result` arguments” rule. Arithmetic helpers may accept `xer::result` operands and propagate errors in order to support chained numeric expressions naturally.  
* Floating-point arithmetic returns `xer::result<long double>`.
* Division helpers can also store remainders.
* `min`, `max`, and `clamp` are designed for mixed-type use rather than mirroring the standard library exactly.
* `in_range<T>(value)` checks whether a numeric value can be represented as `T`.

---

## 9. `<xer/string.h>`

UTF-aware string helpers, C-style byte-string utilities, and PHP-inspired split/join/trim helpers.

### Main groups

#### Search and comparison

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

#### Case-insensitive comparison and search

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

#### Copy and concatenation

```cpp
strcpy
strncpy
strcat
strncat
```

#### Raw memory helpers

The header also exposes memory-oriented helpers via `xer/bits/mem.h`.

#### Split / join / trim

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

#### Error text

```cpp
strerror
get_error_name
get_errno_name
```

### Notes

* The header groups both string-oriented helpers and raw memory helpers.
* Raw memory helpers currently include `memcpy`, `memmove`, `memchr`, `memrchr`, `memcmp`, and `memset`.
* UTF-aware overloads are available for several search operations, notably for UTF-8, UTF-16, and UTF-32 character search with `char32_t` input.
* Case-insensitive operations are intentionally simple and primarily oriented around ASCII plus the currently implemented normalization helpers.
* `trim_view` family is intended as non-allocating trimming helpers around UTF-8-oriented string views.
* Pointer-taking `strlen` uses an explicit size, while string literals are accepted naturally through array-reference overloads.
* As with other ordinary public APIs, these functions are documented as taking ordinary values rather than `xer::result` inputs.

---

## 10. `<xer/ctype.h>`

Character classification and character conversion.

### Classification functions

ASCII-oriented individual predicates:

```cpp
isascii  isupper  islower  isdigit  isalpha  isalnum
isblank  isspace  iscntrl  isprint  isgraph  ispunct
isxdigit isoctal  isbinary
```

Latin-1 extensions:

```cpp
islatin1_upper
islatin1_lower
islatin1_alpha
islatin1_alnum
islatin1_print
islatin1_graph
```

Dynamic classifier:

```cpp
enum class ctype_id;
auto isctype(char32_t c, ctype_id id) noexcept -> bool;
```

### Conversion functions

```cpp
enum class ctrans_id;
auto tolower(char32_t c) -> xer::result<char32_t>;
auto toupper(char32_t c) -> xer::result<char32_t>;
auto toctrans(char32_t c, ctrans_id id) -> xer::result<char32_t>;
```

### Notes

* Individual ASCII functions reject non-ASCII input by returning `false` for classification or an error for conversion.
* Latin-1 conversion currently includes reversible handling for sharp s (`U+00DF` / `U+1E9E`).

---

## 11. `<xer/stdlib.h>`

A collection of stdlib-style utilities, numeric conversion, multibyte conversion, sorting/searching, environment access, and pseudo-random generation.

### Main groups

#### Integer division structures

```cpp
template<class T> struct rem_quot;
using div_t, ldiv_t, lldiv_t;
using i8div_t, i16div_t, i32div_t, i64div_t;
using u8div_t, u16div_t, u32div_t, u64div_t;
```

#### Search and sorting

```cpp
bsearch
qsort
```

#### Numeric conversion

```cpp
ato
atoi   atol   atoll
strto
strtol  strtoll  strtoul  strtoull
strtof  strtod   strtold
strtof32  strtof64
```

#### Multibyte conversion state

```cpp
enum class multibyte_encoding;
struct mbstate_t;
```

#### Multibyte conversion functions

```cpp
mblen
mbtotc
tctomb
mbstotcs
tcstombs
```

These are provided as overload sets for combinations of:

* input/output byte types: `char`, `unsigned char`, `char8_t`
* text character types: `wchar_t`, `char16_t`, `char32_t`
* optional `mbstate_t*` state objects

#### Miscellaneous

```cpp
getenv
rand
srand
class rand_context
```

### Notes

* `char`-based multibyte APIs use the platform default encoding policy (`CP932` on Windows, `UTF-8` on Linux).
* `unsigned char` is treated as CP932-oriented multibyte input/output.
* `char8_t` is treated as UTF-8.
* Floating conversion recognizes forms such as `inf`, `infinity`, and `nan`.

---

## 12. `<xer/json.h>`

JSON value model and JSON encode/decode helpers.

### Main entities

```cpp
struct json_value;
using json_array = json_value::array_type;
using json_object = json_value::object_type;

auto json_decode(std::u8string_view text) -> xer::result<json_value>;
auto json_encode(const json_value& value) -> xer::result<std::u8string>;
```

### `json_value`

`json_value` stores one of:

* `std::nullptr_t`
* `bool`
* `double`
* `std::u8string`
* array of `json_value`
* object represented as `std::vector<std::pair<std::u8string, json_value>>`

### Accessors

```cpp
is_null  is_bool  is_number  is_string  is_array  is_object
as_bool  as_number  as_string  as_array  as_object
```

### Notes

* Objects preserve insertion order because they are stored as a vector of key/value pairs.
* The numeric representation is `double`.

---

## 13. `<xer/path.h>`

UTF-8 lexical path type and native-path conversion helpers.

### Main entities

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

### `path`

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

### Notes

* Internal representation is UTF-8.
* Internal separators are normalized to `'/'`.
* Windows-specific leading-part meaning is preserved lexically, including distinctions such as `C:foo`, `C:/foo`, `/foo`, and `//server/share/foo`.
* Path joining is lexical and follows the project path-policy rules rather than `std::filesystem::path` semantics.
* `basename`, `extension`, `stem`, `parent_path`, `is_absolute`, and `is_relative` are free functions.
* `extension` returns the part beginning with the **first** `'.'` in the basename, so `archive.tar.gz` yields `.tar.gz`.

---

## 14. `<xer/stdio.h>`

Stream-based I/O facilities built on explicit binary and text stream types.

### Core stream types

```cpp
class binary_stream;
class text_stream;

enum seek_origin_t { seek_set, seek_cur, seek_end };
using fpos_t = std::uint64_t;
```

### Stream opening

```cpp
auto fopen(const path& filename, const char* mode) noexcept -> xer::result<binary_stream>;
auto fopen(const path& filename, const char* mode, encoding_t encoding) noexcept -> xer::result<text_stream>;
auto memopen(std::span<std::byte> memory, const char* mode) noexcept -> xer::result<binary_stream>;
auto stropen(std::u8string_view text, const char* mode) noexcept -> xer::result<text_stream>;
auto stropen(std::u8string& text, const char* mode) noexcept -> xer::result<text_stream>;
```

### Text encoding selection

```cpp
enum class encoding_t {
    utf8,
    cp932,
    auto_detect,
};
```

### Closing and flushing

```cpp
fclose
fflush
tmpfile        // binary temporary file
tmpfile(enc)   // text temporary file
```

### File-entry operations

```cpp
remove
rename
mkdir
rmdir
copy
```

### Binary I/O

```cpp
fread
fwrite
fgetb
fputb
```

### Text I/O

```cpp
fgetc   getchar   ungetc
fputc   putchar
fgets   gets
fputs   puts
```

### Position / state helpers

```cpp
fseek
ftell
fgetpos
fsetpos
feof
ferror
clearerr
```

### Formatted I/O

```cpp
printf family via xer::fprintf / xer::sprintf / xer::snprintf / ...
scanf family via xer::fscanf / xer::sscanf / xer::scanf
```

### CSV

```cpp
fgetcsv
fputcsv
```

### Native handle access

The header also includes `to_native_handle` support for exposing underlying native handles where available.

### Notes

* `binary_stream` and `text_stream` are move-only RAII objects.
* Text streams normalize input/output around `char32_t` for single characters and UTF-8 for strings.
* Text file I/O supports UTF-8 and CP932; input can optionally use `auto_detect`.
* `fseek` / `ftell` are the normal position helpers for `binary_stream`.
* `fgetpos` / `fsetpos` are the primary position helpers for `text_stream`.
* `ungetc` support for `text_stream` is intentionally limited to one pushed-back character.
* File-entry helpers operate on `xer::path` and are intentionally separate from stream objects.

---

## 15. `<xer/cyclic.h>`

A lightweight cyclic value type for normalized circular quantities.

### Main entities

```cpp
template<std::floating_point T>
class cyclic;

template<std::floating_point T>
auto from_degree(T value) noexcept -> cyclic<T>;

template<std::floating_point T>
auto to_degree(cyclic<T> value) noexcept -> T;

template<std::floating_point T>
auto from_radian(T value) noexcept -> cyclic<T>;

template<std::floating_point T>
auto to_radian(cyclic<T> value) noexcept -> T;
```

### `cyclic<T>` summary

```cpp
using value_type = T;
static constexpr T default_epsilon;

auto value() const noexcept -> T;
auto cw(cyclic to) const noexcept -> T;
auto ccw(cyclic to) const noexcept -> T;
auto diff(cyclic to) const noexcept -> T;
auto eq(cyclic to) const noexcept -> bool;
auto ne(cyclic to) const noexcept -> bool;
auto eq(cyclic to, T epsilon) const noexcept -> bool;
auto ne(cyclic to, T epsilon) const noexcept -> bool;
```

### Notes

* Stored values are normalized to the half-open interval `[0, 1)`.
* Positive shortest differences correspond to counterclockwise movement.
* Equality is approximate by design; use `eq` and `ne` instead of expecting an ordering model.

---

## 16. `<xer/quantity.h>`

Physical quantity and unit support using MKSA-style base dimensions.

### Main entities

```cpp
template<int L, int M, int Ti, int I>
struct dimension;

using dimensionless = dimension<0, 0, 0, 0>;

template<class Dim, class Scale = std::ratio<1>>
class unit;

template<std::floating_point T, class Dim>
class quantity;
```

### Quantity model

* `quantity<T, Dim>` stores a value normalized to the base unit system.
* `value()` returns the base-unit value.
* `value(unit)` converts to the specified unit.
* Dimensionless quantities support explicit conversion back to the raw scalar.

### Unit model

* `unit<Dim, Scale>` is a zero-size type-level unit tag.
* Units can be multiplied and divided to form derived units.
* A scalar multiplied by a unit produces a quantity.

### Provided base dimensions and units

#### Base dimensions

* length
* mass
* time
* electric current

#### Base units

```cpp
xer::units::m
xer::units::kg
xer::units::sec
xer::units::A
```

#### Selected predefined units

```cpp
mm  cm  km  microm  nm  μm
g   mg
nsec  microsec  msec  μsec
mA
Hz  kHz  GHz
N  J  W  V  Pa  hPa
ha
mL  dL  L  kL  cc
cal  kcal
taurad  τrad  rad
```

### Notes

* Storage types are currently restricted to floating-point types.
* Quantity comparisons are only defined for matching dimensions.
* `rad` is represented as a dimensionless unit scaled from `taurad`.

---

## 17. `<xer/time.h>`

Simple time utilities centered on a `double`-based `time_t` and a `tm` structure with microsecond precision.

### Main entities

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
```

### Main functions

```cpp
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

### Notes

* `time_t` counts seconds from the POSIX epoch and may include a fractional part.
* `tm_microsec` stores microseconds in the range `0..999999`.
* The implementation rejects invalid microsecond fields and other invalid inputs through `xer::result`.

---

## 18. `<xer/version.h>`

Compile-time version information.

### Main entities

```cpp
#define XER_VERSION_MAJOR 0
#define XER_VERSION_MINOR 1
#define XER_VERSION_PATCH 0
#define XER_VERSION_SUFFIX "a5"
#define XER_VERSION_STRING "0.1.0a5"

inline constexpr int version_major;
inline constexpr int version_minor;
inline constexpr int version_patch;
inline constexpr std::string_view version_suffix;
inline constexpr std::string_view version_string;
```

---

## 19. Repository test and example coverage snapshot

The current archive includes focused tests for at least the following areas:

* arithmetic helpers
* string and text processing
* ctype and Latin-1 conversion
* path handling and native-path conversion
* JSON
* stream I/O, CSV, printf, scanf, and stream state
* multibyte conversion
* cyclic values
* quantity and units
* time conversion and formatting
* public-header combination checks

The repository also now contains a broad `examples/` set for user-facing executable examples. These examples are single-file, directly compilable, directly runnable, and aligned with the project policy that examples should show natural use of the public API rather than workaround-heavy code. They now cover, among other areas:

* arithmetic helpers (`abs`, `uabs`, `min` / `max` / `clamp`, `div`)
* string search, comparison, copying, trimming, split/join, and memory helpers
* ctype / to-functions
* multibyte conversion
* path handling and native-path conversion
* binary and text stream I/O, position helpers, stream state, `tmpfile`, `rename`, `remove`, `mkdir`, `rmdir`, and `copy`
* `bsearch`, `qsort`, `getenv`, random generation, JSON, `cyclic`, `quantity`, and time formatting/conversion

This does not replace line-by-line API documentation, but it is a useful indicator of the currently exercised feature set and of the examples that can later be extracted into generated documentation.

---

## 20. Practical starting points

For first use, these headers are usually the most relevant:

* `<xer/error.h>` for `xer::result`
* `<xer/string.h>` for UTF-8-friendly string helpers
* `<xer/stdio.h>` for text and binary streams
* `<xer/stdlib.h>` for numeric and multibyte conversion
* `<xer/arithmetic.h>` for safer mixed-type arithmetic
* `<xer/time.h>` for simple wall-clock and formatting utilities

A minimal example in the current style is:

```cpp
#include <string_view>

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

This style reflects the current documentation direction:

* ordinary APIs are called with ordinary values
* `xer::result` is checked explicitly by the caller
* text output examples prefer `<xer/stdio.h>` rather than `std::cout`
* executable examples are expected to live under `examples/` and to be reusable for future documentation generation  
