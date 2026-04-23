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
