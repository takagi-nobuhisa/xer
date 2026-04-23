# Policy for Code Examples

## Overview

XER places user-facing code examples in the `examples/` directory.

The purpose of a code example is not merely to show a fragment that happens to compile, but to present a **minimal and natural usage example that users can actually try as it is**.

Accordingly, code examples should satisfy the following properties:

- they can be compiled independently
- they can be run independently
- their intended behavior is understandable to the reader
- they show natural usage of XER's public API
- they can be verified by automatic execution

Code examples are different from test programs.
A test program exists to verify the correctness of the implementation, whereas a code example exists to **show users how to use the library**.

---

## Placement Policy

### Location

Source files for code examples are placed in the `examples/` directory.

`examples/` is placed at the same level as `docs/` and `php/`.

### Target Files

As a rule, code examples are placed as `.cpp` files.

In the initial stage, the basic structure is one independent code example per file.

---

## Purpose of Code Examples

A code example should satisfy at least one of the following:

- show the basic usage of a public API
- show a typical and commonly used call pattern
- show a natural usage example including error handling
- serve as a minimal example that is easy to reference from the reference manual or tutorials

On the other hand, the following are **not** the main purpose of code examples:

- exhaustive verification of internal implementation details
- thorough checking of boundary conditions
- abnormal-case testing based on intentional misuse
- presentation of test-style code that heavily relies on assertion macros

Those should, as a rule, be handled under `tests/`.

---

## Independent Compilation and Execution

### Basic Policy

Each code example source file must be compilable and runnable on its own.

Here, “independent” means at least the following:

- it does not depend on another example source file
- it can be compiled by specifying that single file alone
- the minimum processing required for execution is contained in that file

### `main` Function

As a rule, a code example source file contains a `main` function.

```cpp
auto main() -> int;
````

The return type should be written in trailing form, in accordance with XER's overall policy.

### Exit Status

A code example should return `0` on success and a nonzero value on failure.

In a minimal example, failure may simply be written as `return 1;`.

---

## Policy for the APIs Used

### Prefer Public APIs

Code examples should, as a rule, use only APIs from public headers located directly under `xer/`.

Internal implementation APIs under `xer/bits/` should not, as a rule, be used directly in code examples.

### Prefer Natural Style

A code example should prioritize a natural style that library users can imitate directly.

Accordingly, unnatural explicit typing or awkward workarounds written only for the sake of the example should be avoided.

If a natural style cannot be written, the answer should not be to force a workaround in the example.
Instead, improvement of the public API design should be considered.

### Handling of `xer::result`

Code examples follow XER's overall policy that ordinary public APIs do not take `xer::result` as a function argument.

Accordingly, if a code example holds a `xer::result`, it should explicitly extract the success value at the call site before passing it to the next function.

A code example should show this explicit flow of error handling in a natural way.

---

## Input/Output Policy

### Basic Policy

When displaying strings or characters in code examples, the public APIs in `xer/stdio.h` should generally be used instead of `std::cout`.

This is to match XER's policy for string handling and text input/output.

### Reason

XER uses `char8_t`-based UTF-8 strings as its primary string representation, and uses `char32_t` for characters where appropriate.
These do not always fit naturally with `std::cout`.

For that reason, it is more natural for code examples to use XER's own text I/O APIs.

### Recommended Functions

For output to standard output in code examples, at least the following functions are recommended:

* `xer::puts`
* `xer::fputs`
* `xer::putchar`
* `xer::fputc`

### Standard Input

Examples that require input may be created, but in the initial stage examples that do not require input should be preferred as much as possible.

If examples that require input are introduced, it is assumed that standard input can be controlled from PHP-side scripts.

---

## Comment Policy

### Comments Are Written in English

Comments inside code examples are written in English, in accordance with XER's overall coding conventions.

### Purpose of Comments

Comments in code examples are used not merely to restate the code, but to communicate the following information to the reader:

* what the example demonstrates
* the relationship between input and output
* assumptions that should be noted
* the expected result

### Avoid Excessive Explanation

It is important that code examples remain short and easy to read, so overly verbose embedded explanation should be avoided.

Detailed explanation should be given in the main documentation when necessary.

---

## Marker Policy

### Purpose

A code example source file should contain explicit markers in comments so that it can later be extracted mechanically.

These markers are intended at least for the following uses:

* extraction during documentation generation
* identification of individual code examples
* future automatic processing or verification

### Required Markers

Each code example file must contain at least the following two markers:

* `XER_EXAMPLE_BEGIN`
* `XER_EXAMPLE_END`

The format is as follows:

```cpp
// XER_EXAMPLE_BEGIN: <example_id>
...
// XER_EXAMPLE_END: <example_id>
```

### `example_id`

`<example_id>` is an identifier used to identify the code example.

The identifier should satisfy the following conditions:

* it should basically consist of lowercase English letters, digits, and underscores
* it must not contain spaces
* it must match exactly between `BEGIN` and `END`
* it should be a name whose meaning is understandable from its content

Examples:

* `trim_basic`
* `json_decode_simple`
* `path_join_basic`

### Placement

`XER_EXAMPLE_BEGIN` should, as a rule, be placed near the beginning of the file, at the start of the initial comment block.

`XER_EXAMPLE_END` should, as a rule, be placed near the end of the file.

### One Example per File

In the initial stage, one file should contain one code example, with only one pair of `BEGIN`/`END` markers.

A style that embeds multiple examples in one file is not adopted, at least in the initial stage.

---

## Description of Expected Output

### Purpose

A code example should state its expected output in comments so that a user can understand the intended behavior visually.

This serves a different purpose from future machine-verification metadata.

### Format

When appropriate, the leading comment block of a code example should contain `Expected output:`.

Example:

```cpp
// XER_EXAMPLE_BEGIN: trim_basic
//
// This example trims leading and trailing ASCII spaces from a UTF-8 string.
//
// Expected output:
// hello
```

### Policy

* `Expected output:` is written as human-oriented explanation
* it should be brief, while making the actual displayed content understandable
* if the example prints multiple lines, those lines should be listed in comments
* if the output is environment-dependent, it should not be forced into a fixed form

### When Input Exists

For an example that takes input, `Input:` may also be shown in comments if necessary.

However, in the initial stage, input-free examples should be preferred.

---

## Difference Between Code Examples and Test Programs

### Not Test Programs

A code example is not, as a rule, a test program.

Accordingly, detailed verification like that found in `tests/` and extensive boundary-value checking should not be carried into code examples.

### Assertion Macros

As a rule, code examples do not use `xer_assert`-family macros.

The reasons are as follows:

* they can harm readability for users
* they can make the code look like a test
* they make the example less natural as an API usage example

However, if the purpose is specifically to show the typical use of `xer_assert` itself as a public API, a dedicated code example for that purpose may be provided.

---

## Style of Error Handling

### Basic Policy

A code example should not ignore errors, and should include at least minimal explicit error handling.

### Recommended Form

In a minimal code example, the following concise form is sufficient:

```cpp
if (!result.has_value()) {
    return 1;
}
```

### Avoid Excessive Branching

The main purpose of a code example is to show usage, so detailed error reporting and complex recovery logic should generally not be included.

If needed, those should be explained in a separate code example or in the main documentation.

---

## File Naming Policy

### Basic Policy

File names of code examples should use lower snake case and should make the subject matter understandable.

### Prefix

In the initial stage, `example_` is used as a prefix.

Examples:

* `example_trim.cpp`
* `example_json_decode.cpp`
* `example_path_join.cpp`

### Uniqueness

The file name alone should make the rough topic understandable.

---

## Relationship to Automatic Execution

### Basic Policy

Code examples placed in `examples/` should be compilable and runnable automatically by PHP scripts.

### Target

At least in the initial stage, when `examples/` is targeted by `php/run_tests.php`, it should be possible to verify that each code example builds and runs.

### Success Conditions

In the initial stage, the success conditions are at least the following:

* compilation succeeds
* execution succeeds
* the program does not terminate abnormally

### Future Extensions

If necessary, the following may be introduced in the future:

* comparison against expected standard output
* automatic supply of standard input
* automatic extraction based on markers
* automatic embedding into documentation

However, these are not required in the initial stage.

---

## Relationship to Documentation

### Basic Policy

Code examples should be written in a form that can later be referenced or republished easily from the reference manual or tutorials.

### Avoid Example-Only Tricks

Complex tricks intended only for documentation extraction, or special notation that harms readability, should be avoided.

Only the minimum necessary markers and comments should be used, and the source file itself should remain easy to read.

### Treat Code Examples as Canonical

As far as possible, the goal is to treat the code examples placed in `examples/` as the canonical source, rather than manually rewritten code fragments embedded in the documentation.

This makes it easier to reflect examples that have actually been compiled and run into the documentation.

---

## Recommended Template

A code example file should use at least a structure like the following:

```cpp
// XER_EXAMPLE_BEGIN: trim_basic
//
// This example trims leading and trailing ASCII spaces from a UTF-8 string.
//
// Expected output:
// hello

#include <xer/stdio.h>
#include <xer/string.h>

auto main() -> int
{
    const auto trimmed = xer::trim_view(std::u8string_view(u8"  hello  "));
    if (!trimmed.has_value()) {
        return 1;
    }

    if (!xer::puts(*trimmed).has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: trim_basic
```

The specific API names and input values may vary from example to example, but the following points should be preserved as much as possible:

* `BEGIN`/`END` markers
* English comments
* `Expected output:`
* a standalone `main`
* minimal explicit error handling

---

## Summary

* code examples are placed in `examples/`
* each file should be compilable and runnable independently
* in the initial stage, one file should normally contain one example
* priority is given to showing natural usage of the public API
* comments are written in English
* examples should contain explicit extraction markers
* `Expected output:` should be written when appropriate
* code examples are distinct from test programs
* assertion-heavy verification belongs under `tests/`, not in examples
* code examples should include minimal explicit error handling
* file names should use lower snake case with the `example_` prefix
* examples should be suitable for automatic build and execution
* as far as possible, code examples in `examples/` should become the canonical source for user-facing example snippets
