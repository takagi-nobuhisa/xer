# XER Project Outline

## 1. Purpose

XER is designed as a **C++23 library for programmers of the X generation who are deeply familiar with C**.

It values the clarity inherited from C while adopting the type safety and expressive power of C++23.
It avoids excessive C++-style abstraction and gives priority to being **easy for C programmers to understand and use**.

---

## 2. Supported Environment

### 2.1 Supported Compilers

- The primary officially supported compiler is **GCC 13.3.0 or later**
- Support for Visual C++ and Clang may be considered in the future
- At present, only GCC is treated as an officially supported compiler

### 2.2 Supported Platforms

- **Windows**
- **Linux**

### 2.3 Supported Windows Environments

- Command Prompt
- PowerShell
- MSYS2

Under MSYS2, at least the following environments are supported:

- `msys`
- `mingw64`
- `ucrt64`

### 2.4 Windows Version

- On Windows, the target baseline is **Windows 11 or later**

---

## 3. Distribution Model

### 3.1 Basic Policy

- XER is **header-only**
- It should be usable simply by including the headers

### 3.2 Directory Structure

- Public headers are placed under `xer/`
- Internal subdivided declarations and definitions are placed under `xer/bits/`
- Whether a header is public is determined by its **location**, not by its extension

### 3.3 Header Extension

- Header files use the **`.h`** extension

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

The character encodings handled by XER are limited to the following four:

- **CP932**
- **UTF-8**
- **UTF-16**
- **UTF-32**

XER does not depend on locale.

### 6.2 Characters and Strings

- **Strings** use UTF-8 as the default representation
- **Characters** use UTF-32 when appropriate
- UTF-16 and UTF-32 may also be used for intermediate data and platform interoperation
- CP932 is also handled for compatibility with existing environments

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

Details of character encoding conversion follow these documents:

- `policy_encoding.md`
- `policy_ctype.md`

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

---

## 9. Arithmetic and Comparison

### 9.1 Basic Policy

- XER does not rely directly on built-in C++ operators alone, and instead provides **dedicated functions**
- The goal is to make error handling easier to incorporate and to return mathematically straightforward and predictable results

### 9.2 Detailed Reference

Details of arithmetic and comparison follow this document:

- `policy_arithmetic.md`

---

## 10. Error Handling Policy

### 10.1 Basic Policy

- Exceptions are used as little as possible, and normal failure is represented by **`xer::result`**
- `xer::result<T, Detail>` is implemented as **`std::expected<T, error<Detail>>`**
- However, exceptions are used when they are appropriate by design, such as for non-comparable values or assertion failures

### 10.2 Result Type

- The standard result type in XER is `xer::result<T, Detail = void>`
- `xer::result<T>` is an alias of `std::expected<T, error<void>>`
- `xer::result<T, Detail>` is an alias of `std::expected<T, error<Detail>>`
- As a rule, public APIs and internal implementations that can fail should use `xer::result` as their return type
- This keeps XER return type notation concise and improves readability and consistency

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
- For that reason, XER provides its own **assertion mechanism**
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

- XER uses **PHP for code generation and test support**
- PHP is not part of XER itself, but is treated as a **development-only tool**

### 12.2 Classification of Tests

XER tests are classified into at least the following categories:

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

## 16. Future Functional Areas

### 16.1 `cyclic`

Circular values such as angles, phases, and directions are handled by `cyclic`.

Details follow this document:

- `policy_cyclic.md`

### 16.2 Physical Quantities and Units

Physical quantities and units are handled by the quantity system.

Details follow this document:

- `policy_quantity.md`

### 16.3 Time Handling

Time-related functionality is redesigned as a simple and easy-to-use high-precision `time.h`-style library.

Details follow this document:

- `policy_time.md`

### 16.4 Multibyte Conversion in `stdlib.h`

The multibyte conversion facilities in `stdlib.h` follow a separate policy document.

Details follow this document:

- `policy_multibyte.md`

---

## 17. Summary

- XER is a C++23 library designed for programmers familiar with C
- It values clarity and practicality over excessive abstraction
- It is header-only, with public headers under `xer/` and implementation details under `xer/bits/`
- It limits its supported encodings to CP932, UTF-8, UTF-16, and UTF-32
- It uses UTF-8 as the primary string representation
- It builds I/O on top of `FILE`-based design rather than `iostream`
- It represents normal failure with `xer::result`
- It performs validation through common headers and `static_assert`
- It uses PHP as a development-only tool for code generation and testing
- It organizes documentation, examples, and development scripts by role
- It continues to evolve incrementally, starting from the facilities most needed in practice
