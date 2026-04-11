# xer

[日本語版 README / Japanese README](README.ja.md)

xer is a header-only C++23 library designed for programmers who are familiar with C and want a simpler, more predictable alternative to overly abstract C++ APIs.

It keeps the spirit of the C standard library where that style is still practical, but redesigns the APIs as a modern C++ library with stronger typing, explicit error handling, and Unicode-aware text processing.

## Status

xer is under active development.

The current focus is on rebuilding practical parts of the C standard library in a way that fits the design goals of this project, especially:

- string and character handling
- I/O
- path handling
- arithmetic helpers
- time utilities

This project does **not** aim at full source-level compatibility with the C standard library, even when function names are reused.

## Design goals

- Familiar to C programmers
- Header-only
- C++23-based
- Explicit and predictable behavior
- Minimal reliance on locale
- Practical Unicode support
- Avoid unnecessary abstraction
- Prefer simple APIs over clever APIs

## Supported environments

Current official target:

- GCC 13.3.0 or later

Target platforms:

- Windows
- Linux

Current Windows scope:

- Command Prompt
- PowerShell
- MSYS2 (`msys`, `mingw64`, `ucrt64`)

Current Windows version target:

- Windows 11 or later

Visual C++ and Clang may be considered later, but they are not official targets yet.

## Key characteristics

### 1. Header-only

xer is intended to be usable by including headers only.

### 2. Error handling based on `std::expected`

Normal failures are represented with `std::expected`.
Internal invariant violations are handled separately through XER's assert mechanism.

### 3. Clear separation of API layers

- `xer` for regular public APIs
- `xer::advanced` for lower-level advanced APIs
- `xer::detail` for internal implementation details

### 4. Unicode and encoding policy

xer intentionally limits its text-encoding scope to:

- CP932
- UTF-8
- UTF-16
- UTF-32

In regular APIs:

- strings are primarily UTF-8
- individual text characters may use `char32_t`
- CP932 is supported for interoperability with existing environments

### 5. Locale-independent design

xer does not place locale-dependent behavior at the center of its design.
For example, character classification and text conversion are designed around explicit project rules rather than the host locale.

### 6. I/O built on `FILE`-style foundations

xer does not use `iostream` as its foundation.
Instead, it builds on `FILE`-style I/O and exposes redesigned stream types such as:

- `binary_stream`
- `text_stream`

## Public headers

Current public headers:

- `xer/error.h`
- `xer/assert.h`
- `xer/string.h`
- `xer/ctype.h`
- `xer/stdlib.h`
- `xer/stdio.h`
- `xer/path.h`
- `xer/stdint.h`
- `xer/arithmetic.h`
- `xer/time.h`
- `xer/version.h`

Headers under `xer/bits/` are internal implementation details.

## Repository layout

```text
xer/
  xer/        public headers
  xer/bits/   internal implementation headers
  tests/      test programs
  php/        development-time helper scripts
  doc/        design documents
```

## Example

```cpp
#include <iostream>
#include <xer/path.h>
#include <xer/arithmetic.h>

int main()
{
    xer::path base(u8"C:/work");
    xer::path file(u8"docs/readme.txt");

    auto joined = base / file;
    auto sum = xer::add(10u, -3);

    std::cout << reinterpret_cast<const char*>(joined.str().data()) << '\n';

    if (sum.has_value()) {
        std::cout << *sum << '\n';
    }
}
```

## Development notes

The repository contains PHP scripts used for development tasks such as:

- generating conversion tables
- generating test cases
- driving compile/run tests
- collecting test results

PHP is a development tool for xer itself.
It is **not required** for users who only want to use the library.

## Non-goals for the current stage

At least for now, xer does not aim to provide:

- full locale support
- full reproduction of every C standard header
- `iostream`-based design
- complete compatibility with all host-specific behavior
- immediate support for every compiler

Some headers are intentionally not provided as standalone public headers, and some areas such as `math.h` are postponed.

## Documentation

Design documents are available under `doc/`.

These documents describe the current direction of the project, including:

- project overview
- encoding policy
- path handling
- I/O design
- arithmetic behavior
- coding conventions
- test and code-generation policy

## License

Boost Software License 1.0.
See [LICENSE](LICENSE).

## Why the name “xer”?

The project is designed as a C++23 library for programmers of the X generation who grew up with C-style programming.
