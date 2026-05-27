# xer Project Outline

## 1. Purpose

xer is designed as a **C++23 library for programmers of the X generation who are deeply familiar with C**.

It values the clarity inherited from C while adopting the type safety and expressive power of C++23.
It avoids excessive C++-style abstraction and gives priority to being **easy for C programmers to understand and use**.

### 1.1 Practical Quality Target

xer values practical usefulness over theoretical perfection.

For domains where complete correctness is impossible, excessively costly, or dependent on information unavailable to the library, xer should aim for behavior that is:

> **not perfect, but sufficiently good**

This principle applies especially to facilities such as natural-language-oriented Japanese text processing, where readings, word boundaries, and structural interpretation may depend on context, dictionary coverage, or implicit human knowledge.

Such facilities should:

- provide useful and predictable results for ordinary practical use
- document their limitations honestly
- avoid pretending to solve problems that cannot be solved reliably without broader context
- prefer a robust practical baseline over endlessly pursuing exceptional cases

This principle does **not** weaken the expectations for deterministic low-level APIs.
Where exact behavior is reasonably definable, xer should define it clearly and implement it faithfully.

---

## 2. Supported Environment

### 2.1 Supported Compiler

- The primary officially supported compiler is **GCC 13.3.0 or later**
- Support for Visual C++ and Clang may be considered in the future
- At present, only GCC is treated as an officially supported compiler

### 2.2 Supported and Tested Environments

The primary supported and tested environments are:

- **Ubuntu**
- **MSYS2 UCRT64**

### 2.3 Platform Scope

The current platform scope is:

- Linux through Ubuntu
- Windows through MSYS2 UCRT64

### 2.4 Unsupported MSYS2 Environments

The following MSYS2 environments are not supported targets:

- **MSYS2 MSYS**
- **MSYS2 MINGW64**

They are not included in the current or planned test matrix.
If a clear need appears in the future, support for those environments may be reconsidered at that time.

### 2.5 Windows Version

- On Windows, the target baseline is **Windows 11 or later**

---

## 3. Distribution Model

### 3.1 Basic Policy

- xer is **header-only**
- It should be usable simply by including the headers

### 3.2 Directory Structure

- Public headers are placed under `xer/`
- Internal subdivided declarations and definitions are placed under `xer/bits/`
- Whether a header is public is determined by its **location**, not by its extension

### 3.3 Header Extension

- Header files use the **`.h`** extension

### 3.4 External Component Policy

xer avoids being driven by unstable external dependencies.

When xer intentionally integrates an external component, it should prefer components that are:

- mature
- widely used
- operationally simple
- stable in behavior and interface
- unlikely to impose frequent disruptive changes on xer

The goal is to gain practical capability without being forced to chase fast-moving external ecosystems.

Examples of external components that fit this policy include:

- **Tcl/Tk** for lightweight GUI integration
- **MeCab** for Japanese morphological analysis
- **ICU** for Unicode normalization
- **zlib** for ZIP deflate compression and expansion

These components are old, well-established, and sufficiently stable that xer can build practical facilities around them without expecting constant breakage from upstream changes.

External-component details follow this document:

- `policy_external_components.md`

---

## 4. Namespace and Naming Policy

### 4.1 Namespaces

- The namespace for normal APIs is **`xer`**
- The namespace for low-level advanced APIs is **`xer::advanced`**
- Internal implementation identifiers belong to **`xer::detail`**

### 4.2 Function Names

- Reimplementations of C standard library functions and PHP functions use **the same names as the originals**
- However, sharing a name with C or PHP does **not** imply full compatibility with C or PHP

---

## 5. Scope of C Standard Library Reimplementation

### 5.1 Basic Policy

- As a rule, the reimplementation target is the set of **C99 functions**
- However, implementation should proceed incrementally, starting from what is needed

### 5.2 Priorities

In the initial stage, the following areas are prioritized:

1. **character and string processing**
2. **input/output processing**

Mathematical functions are postponed.

### 5.3 Out of Scope

At least in the initial stage, the following are out of scope:

- locale-related facilities
- `inttypes.h`
- `fenv.h`
- compatibility headers that are unnecessary in C++, such as `stdbool.h`

---

## 6. Encoding Policy

### 6.1 Supported Encodings

The character encodings handled by xer are limited to the following four:

- **CP932**
- **UTF-8**
- **UTF-16**
- **UTF-32**

xer does not depend on locale.

### 6.2 Characters and Strings

- **Strings** use UTF-8 as the default representation
- **Characters** use UTF-32 when appropriate
- UTF-16 and UTF-32 may also be used for intermediate data and platform interoperation
- CP932 is also handled for compatibility with existing environments
- Language-oriented text handling primarily targets English and Japanese; other languages and scripts are not default scope unless a clear project need appears

### 6.3 Mapping to Types

- UTF-8 strings: `char8_t`
- UTF-16 strings: `char16_t`
- UTF-32 characters: `char32_t`
- CP932 strings: `unsigned char`
- `char`: not used as the primary internal representation, but accepted for compatibility input

### 6.4 Handling of `char` Strings

- `char` strings are not used as the internal standard representation
- When using GCC, it is assumed that `-fexec-charset` is not used
- Source files use **UTF-8 with BOM**
- Automatic encoding detection for `char` strings is not used in normal operation and is limited to validation of assumptions

### 6.5 Detailed References

Details of character encoding conversion and Unicode text handling follow these documents:

- `policy_encoding.md`
- `policy_ctype.md`
- `policy_unicode_normalize.md`

---

## 7. Path Handling Policy

### 7.1 Basic Policy

- Path handling is primarily **lexical**, independent of the actual filesystem
- The internal representation is UTF-8
- Separators are normalized to `/`
- Windows-style meaning of leading components is preserved

### 7.2 Detailed Reference

Details of path handling follow this document:

- `policy_path.md`

---

## 8. Input/Output Policy

### 8.1 Basic Policy

- Input/output is designed on top of **`FILE`-based functions** from the C standard library
- `iostream` is not used
- The public API is redesigned as a C++ library and does not aim for full C compatibility

### 8.2 Public Types

The public API does not expose `FILE` directly.
Instead, it uses the following types according to purpose:

- `binary_stream`
- `text_stream`

### 8.3 Detailed Reference

Details of input/output follow this document:

- `policy_stdio.md`
- `policy_socket.md`

---

## 9. Arithmetic and Comparison

### 9.1 Basic Policy

- xer does not rely directly on built-in C++ operators alone, and instead provides **dedicated functions**
- The goal is to make error handling easier to incorporate and to return mathematically straightforward and predictable results

### 9.2 Detailed Reference

Details of arithmetic and comparison follow this document:

- `policy_arithmetic.md`
- `policy_stdfloat.md`

---

## 10. Error Handling Policy

### 10.1 Basic Policy

- Exceptions are used as little as possible, and normal failure is represented by **`xer::result`**
- `xer::result<T, Detail>` is implemented as **`std::expected<T, error<Detail>>`**
- However, exceptions are used when they are appropriate by design, such as for non-comparable values or assertion failures

### 10.2 Result Type

- The standard result type in xer is `xer::result<T, Detail = void>`
- `xer::result<T>` is an alias of `std::expected<T, error<void>>`
- `xer::result<T, Detail>` is an alias of `std::expected<T, error<Detail>>`
- As a rule, public APIs and internal implementations that can fail should use `xer::result` as their return type
- This keeps xer return type notation concise and improves readability and consistency

### 10.3 Error Type

- `error<void>` is used when no extra detail is needed
- `error<void>` stores `code` and `location` as common information
- It does not have a `message` member
- `error<Detail>` is an error type that carries additional information
- If `Detail` is a class type, `error<Detail>` inherits from `Detail`
- If `Detail` is a non-class type, `error<Detail>` stores it in a `detail` data member
- `Detail` is specified explicitly as a template argument

### 10.4 Error Construction

- `make_error` is used to construct errors
- `std::source_location::current()` is obtained at the top level as the default argument of `make_error`
- The constructor of `error` does not call `current()` directly, but stores the received `location`
- `make_error(error_t)` constructs `error<void>`
- `make_error<Detail>(error_t, value)` constructs `error<Detail>`
- `value` takes exactly one object, and that object is used directly for the constructor of `Detail` or for initialization of the `detail` member
- If multiple values are needed, they should be bundled into a single object such as a dedicated aggregate or a `tuple`

### 10.5 `error_t`

- `error_t` is designed primarily with **`errno`** in mind
- Positive values correspond to target-environment `errno` values
- Negative values are library-specific errors
- Among them, a common code such as `io_error` may be introduced for failures in the I/O abstraction layer
- It does not include a success value
- The underlying type is currently **`int32_t`**

### 10.6 Assertion Facilities

- Code bugs and violations of internal invariants are treated separately from ordinary I/O or runtime failures
- For that reason, xer provides its own **assertion mechanism**
- Unlike the standard `assert`, it does not terminate the process immediately; instead, **it throws an exception on failure**

---

## 11. Common Header and Implementation-Dependent Elements

### 11.1 Role of the Common Header

The common header is responsible for the following:

- validation of implementation assumptions
- validation of language version assumptions
- validation of character encoding assumptions
- absorption of implementation-dependent macros and extensions
- provision of common abstract names used inside the library

### 11.2 Validation Policy

- Preconditions are checked **continuously in the common header**
- Validation is performed at compile time using **`static_assert`**
- If a condition is not satisfied, compilation fails

### 11.3 Common Macros

- The common macro name for retrieving the C++ version is **`XER_STDCPP_VERSION`**
- The common macro name for retrieving the function signature string is **`XER_PRETTY_FUNCTION`**

---

## 12. Test and Code Generation Policy

### 12.1 Position of PHP

- xer uses **PHP for code generation and test support**
- PHP is not part of xer itself, but is treated as a **development-only tool**

### 12.2 Classification of Tests

xer tests are classified into at least the following categories:

- successful compilation tests
- expected-failure compilation tests
- execution tests

### 12.3 Detailed Reference

Details of testing and code generation follow this document:

- `policy_testing_and_php.md`

---

## 13. Coding Conventions

### 13.1 Basic Position

Coding conventions are defined separately from this outline.

### 13.2 Detailed Reference

Details of coding conventions follow this document:

- `coding_conventions.md`

---

## 14. Documentation Policy

### 14.1 Basic Position

Documentation should be organized so that project-wide primary documents, supporting fragments, executable examples, and development scripts remain clearly separated.

### 14.2 Detailed References

Details of documentation layout and code examples follow these documents:

- `policy_doc_layout.md`
- `policy_examples.md`

---

## 15. Public Headers

### 15.1 Basic Position

Public headers are defined separately as a project-wide list.

### 15.2 Detailed Reference

The list of public headers follows this document:

- `public_headers.md`

---

## 16. Process Handling

Child process handling is provided through `xer/process.h`.

Details follow this document:

- `policy_process.md`

---

## 17. Additional Functional Areas

The following functional areas are already part of xer's current design scope.
They are organized as separate policy documents because they form independent groups of functionality rather than being mere extensions of a single header.

### 17.1 `cyclic`

Circular values such as angles, phases, and directions are handled by `cyclic`.

Details follow this document:

- `policy_cyclic.md`

### 17.2 Bounded Intervals

Values constrained to a compile-time interval are handled by `interval`.
This facility is intended to support domains such as normalized color components and other range-limited scalar values.

Details follow this document:

- `policy_interval.md`

### 17.3 Color Handling

Color-related functionality is provided as a practical foundation for image processing and drawing APIs.
It includes basic color representations, color-space conversion utilities, and grayscale-oriented helpers.

Details follow this document:

- `policy_color.md`

### 17.4 Physical Quantities and Units

Physical quantities and units are handled by the quantity system.

Details follow this document:

- `policy_quantity.md`

### 17.5 Time Handling

Time-related functionality is redesigned as a simple and easy-to-use high-precision `time.h`-style library.

Details follow this document:

- `policy_time.md`

### 17.6 Multibyte Conversion in `stdlib.h`

The multibyte conversion facilities in `stdlib.h` follow a separate policy document.

Details follow this document:

- `policy_multibyte.md`

### 17.7 Tcl/Tk Integration

Tcl/Tk integration provides a lightweight GUI path for xer while preserving the library's explicit-error and low-abstraction style.
The design covers interpreter ownership, command registration, event-loop control, and practical interoperability with Tk objects.

Details follow this document:

- `policy_tk.md`

### 17.8 Image and Canvas Facilities

Image and drawing facilities are organized around a lightweight canvas abstraction.
They cover pixel access, basic drawing primitives, anti-aliased and thick strokes, simple image filters, and interoperability with other xer facilities where appropriate.

Details follow this document:

- `policy_image.md`

### 17.9 Bitmap Font Handling

Bitmap font handling provides a practical text-rendering path for the image subsystem.
It covers BDF-derived font data, font loading, and text drawing on image canvases.

Details follow this document:

- `policy_bitmap_font.md`

### 17.10 MeCab-Based Japanese Text Processing

MeCab-based Japanese text processing provides a practical foundation for higher-level Japanese-language features.
It covers UTF-8 child-process integration with MeCab, access to morphological analysis data, split feature fields, practical bunsetsu-like phrase grouping, symbol-range separation, reading-based kana conversion, kana wakachi-gaki, romaji wakachi-gaki, braille-oriented wakachi-gaki, direct braille-oriented text conversion, and the basis for ruby and related counts.

Independent formatting helpers that do not require morphological analysis may be provided separately.
For example, `xer/furigana.h` formats already-known base text and reading pairs as HTML ruby markup or parenthesized furigana text, and can later be reused by MeCab-based automatic furigana facilities.

Details follow this document:

- `policy_mecab.md`

---

### 17.11 Unicode Utilities

Unicode utilities provide a practical way to handle Unicode text at the code point layer, at the extended grapheme cluster layer, through grapheme-cluster-based string operations, and to normalize UTF-8 text to Unicode NFC.

The code point layer and grapheme cluster layer support traversal of:

- `std::u8string_view`
- `std::u16string_view`
- `std::wstring_view`

The normalization implementation uses ICU C API rather than embedding Unicode normalization tables into xer.

The current normalization scope is deliberately narrow:

- `normalize_nfc`
- `is_normalized_nfc`

Grapheme cluster traversal is built on top of the code point layer. It is intended for practical user-visible character traversal and keeps malformed input explicit through `xer::result`. Grapheme-cluster-based string operations such as length and substring helpers use that traversal layer and return views into the original text.

The default Unicode text-handling scope is English and Japanese. The grapheme cluster implementation may handle common emoji and general Unicode sequences that appear in that scope, but xer does not try to cover every language-specific writing-system rule by default. Users who need broader script coverage can extend the public source code or use dedicated Unicode boundary services in their own programs.

Other normalization forms and ICU-based text services may be considered later only when there is a clear practical need.

Details follow this document:

- `policy_unicode_normalize.md`

---

### 17.12 ZIP Archive Utilities

ZIP archive utilities are provided through `xer/zip.h`.
They are treated as compression-and-archive facilities rather than ordinary file I/O.

The current scope covers ordinary non-ZIP64 single-disk archives: sequential reading, exact-name lookup, whole-entry reads, archive creation, deflated entry writing, explicit commit, and extraction helpers with path-safety checks.

ZIP64, encrypted entries, archive comments, multi-disk archives, and streaming large-entry I/O are deferred until there is a clear practical need.

Details follow these documents:

- `policy_external_components.md`
- `docs/bits/header_zip.md`

### 17.13 Fixed-Schema Binary Serialization

Fixed-schema binary serialization is provided through `xer/serialize.h`.

The design deliberately avoids reflection-based or self-describing serialization.
The low-level archive layer transfers supported scalar values and selected standard containers, while PHP-generated C++ code defines user structures and one `xfer` function per structure.

The binary format stores only data in a fixed field order.
Type names, field names, schema descriptions, object identifiers, byte-order markers, and version records are not written by the low-level archive layer.

Details follow this document:

- `policy_serialize.md`

---

## 18. Summary

- xer is a C++23 library designed for programmers familiar with C
- It values clarity and practicality over excessive abstraction
- In domains where perfect correctness is unrealistic or disproportionally costly, it aims for results that are **not perfect, but sufficiently good**
- It is header-only, with public headers under `xer/` and implementation details under `xer/bits/`
- It limits its supported encodings to CP932, UTF-8, UTF-16, and UTF-32
- It uses UTF-8 as the primary string representation
- It builds I/O on top of `FILE`-based design rather than `iostream`
- It represents normal failure with `xer::result`
- It performs validation through common headers and `static_assert`
- It uses PHP as a development-only tool for code generation and testing
- It avoids unstable external dependencies and prefers mature, stable external components when integration is worthwhile
- It isolates external-component features behind dedicated public headers and uses compile-time detection where appropriate
- It organizes documentation, examples, and development scripts by role
- It includes additional practical facilities such as process handling, socket support, ZIP archive utilities, fixed-schema binary serialization, value-domain utilities, Tcl/Tk integration, image/canvas APIs, bitmap-font-based text rendering, MeCab-based Japanese text processing, and Unicode code point traversal, grapheme cluster traversal, and ICU-based Unicode NFC normalization
- It continues to evolve incrementally, starting from the facilities most needed in practice
