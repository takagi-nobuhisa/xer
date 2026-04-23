# Policy for Code Generation and Testing with PHP

## Overview

XER uses PHP for code generation and test support.

Its intended uses include generation of conversion tables, generation of test cases, build and execution control for test programs, and result aggregation.

PHP is often recognized as a language for web application development, but in this project it is used as a general-purpose scripting language.
Particular importance is placed on the fact that its syntax is relatively familiar to C programmers.

PHP is an auxiliary tool used during XER development, and XER users are not required to use PHP.
To use XER itself, an ordinary C++ compiler and build environment are sufficient.

The repository includes PHP scripts and the related data used for generation.
On the other hand, a distribution of the library by itself will not normally include them.

---

## Role of PHP

PHP is used not as part of XER itself, but as a development-only tool.

Its main uses are as follows:

- generation of conversion tables
- generation of test cases
- generation of test programs
- build and execution control for test programs
- aggregation of test results

---

## Classification of Tests

XER tests are classified into at least the following categories:

- successful compilation tests
- expected-failure compilation tests
- execution tests

### Successful Compilation Tests

Successful compilation tests verify that correctly used code can be compiled.

This category also includes combination tests of public headers.

### Expected-Failure Compilation Tests

Expected-failure compilation tests verify that intended misuse results in compilation errors as expected.

In this kind of test, error detection is based mainly on `static_assert`, so that the reason for failure can be identified in a stable way.

The pass condition for such a test is not only that compilation fails, but also that the expected `static_assert` message is included in the diagnostic output.

The method for defining test cases and the method for embedding metadata into source code will be defined separately when the implementation is designed.

### Execution Tests

Execution tests are written using XER-specific assertion macros.

These assertion macros do not terminate the process like the standard `assert`.
Instead, they throw an XER-specific exception type on failure.

Checks covered by execution tests include at least the following:

- truth checks
- negation checks
- two-value comparisons
- ordering comparisons (for the time being, only `<`)
- confirmation that an exception is thrown
- confirmation that no exception is thrown

---

## Placement of Public Headers

Public headers of XER are placed directly under the `xer/` directory.

Headers located under subdirectories of `xer/` are not treated as public headers, but as implementation details or internal support headers.

Whether a header is public is determined by its location, not by its extension.

Accordingly, the list of public headers is extracted automatically from the files located directly under the `xer/` directory.
Files under subdirectories are not included in that target set.

---

## Compilation Tests for Combinations of Public Headers

For public headers, ordered combination tests of two different public headers are performed as successful compilation tests, in order to prevent compilation errors that depend on include order.

If there are `n` public headers, the number of ordered two-header combinations is `nP2`.
Test source files for all of these combinations are generated automatically by PHP and then compiled.

As for whether each public header can be included by itself, ordinary execution tests are treated as covering that requirement as well, and dedicated single-header include tests are not provided as a rule.

However, it is assumed that each public header is included by itself in at least one ordinary test.

---

## Assertion Macros

### Position

XER assertion macros may be exposed not only for execution tests inside the library, but also to XER users.

However, these assertion macros are intended mainly for XER's own development and for lightweight testing.
They are not intended to guarantee complete convenience for every type, output environment, and string representation.

Usage restrictions and limitations related to value reporting should be stated explicitly in documentation and comments.

The library side should not provide excessive special handling beyond what is necessary.

### Macro Names

The assertion macros for execution tests use the `xer_` prefix and should include at least the following:

- `xer_assert`
- `xer_assert_not`
- `xer_assert_eq`
- `xer_assert_ne`
- `xer_assert_lt`
- `xer_assert_throw`
- `xer_assert_nothrow`

### `xer_assert_throw`

The argument order of `xer_assert_throw` places the expression to be evaluated first and the exception type second.

```cpp
xer_assert_throw(expr, exception_type)
````

---

## Summary

* XER uses PHP for code generation and test support
* PHP is treated as a development-only tool, not as part of the library itself
* XER users are not required to use PHP merely to use the library
* the repository may include PHP scripts and generation data, while a library-only distribution normally does not
* XER tests are classified into successful compilation tests, expected-failure compilation tests, and execution tests
* expected-failure compilation tests rely mainly on `static_assert` and diagnostic-message matching
* execution tests are written using XER-specific assertion macros that throw exceptions on failure
* public headers are defined by placement directly under `xer/`
* ordered two-header combination tests are generated automatically for public headers
* XER assertion macros may also be exposed to users, but they are intended mainly for development and lightweight testing
