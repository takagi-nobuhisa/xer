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

On Windows, `get_environs` primarily reads `__wenvp` so that environment strings are obtained as UTF-16 and converted to UTF-8.
If `__wenvp` is `nullptr`, it falls back to `__envp` and assumes that those byte strings are UTF-8.

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
