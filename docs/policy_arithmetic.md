# Policy for Arithmetic and Comparison

## Overview

In XER, arithmetic and comparison are provided through dedicated functions rather than relying directly on C++ built-in operators.

This is because built-in operators are prone to the following problems:

- it is difficult to integrate error handling into them
- mixing signed and unsigned integer types often leads to undesirable behavior
- for floating-point numbers and complex numbers, it is difficult to express cases such as non-comparability and `NaN` explicitly

XER arithmetic functions do not simply follow the usual arithmetic conversions of the C++ standard.
Instead, they are intended to return **mathematically straightforward and predictable results**.

---

## Function Names to Be Provided

### Integer Arithmetic Functions

- `add`
- `uadd`
- `sub`
- `usub`
- `mul`
- `umul`
- `div`
- `udiv`
- `mod`
- `umod`

### Comparison Functions

- `eq`
- `ne`
- `lt`
- `le`
- `gt`
- `ge`

Comparison functions use short names because they are intended to be used in a somewhat operator-like manner.

### Helper Functions

- `in_range`
- `min`
- `max`
- `clamp`
- `abs`
- `uabs`

---

## Common Policy

### Acceptance of `xer::result`

Arithmetic functions accept not only raw values, but also `xer::result` objects returned by other arithmetic functions.

- if a `xer::result` argument holds a success value, arithmetic is performed using that value
- if a `xer::result` argument holds an error, that error is propagated as it is

Since `xer::result<T, Detail>` is implemented as `std::expected<T, error<Detail>>`, the implementation may use the facilities of `std::expected`.

This makes arithmetic functions easier to chain.

As for comparison functions, they do not return `xer::result`, because returning `xer::result<bool>` would make accidental misuse too likely.

### Use `xer::lt`-Family Functions for Comparison

Whenever arithmetic functions inside XER need ordering comparison, they should, as a rule, use `xer::lt` and related functions rather than the built-in `<`.

This is to apply XER's comparison rules consistently, especially when signed and unsigned integer types are mixed.

Accordingly, internal comparisons in functions such as `min`, `max`, and `clamp` should also use `xer::lt`.

---

## Arithmetic for Integer Types

## `add`

`add(a, b)` performs addition of two values.

- it accepts integer operands
- it allows mixing signed and unsigned integer types
- if the result can be represented by `std::int64_t`, it returns `xer::result<std::int64_t>`
- if the result cannot be represented by `std::int64_t`, it returns an error
- passing a `std::uint64_t` value as an argument is allowed
- however, if the result does not fit in `std::int64_t`, it is an error

### Examples

- `add(1u, 2)` → success
- `add(UINT64_C(10), -3)` → may succeed
- `add(UINT64_MAX, 0)` → error
- `add(INT64_MAX, 1)` → error

---

## `uadd`

`uadd(a, b)` performs addition of two values.

- it accepts integer operands
- it allows mixing signed and unsigned integer types
- if the result can be represented by `std::uint64_t`, it returns `xer::result<std::uint64_t>`
- if the result cannot be represented by `std::uint64_t`, it returns an error

---

## `sub`

`sub(a, b)` performs subtraction of two values.

- its basic policy is the same as `add`
- its return type is `xer::result<std::int64_t>`
- if the result does not fit in `std::int64_t`, it is an error

### Examples

- `sub(10u, 3)` → success
- `sub(3u, 10)` → succeeds with `-7`

---

## `usub`

`usub(a, b)` performs subtraction of two values.

- its basic policy is the same as `uadd`
- its return type is `xer::result<std::uint64_t>`
- if the result does not fit in `std::uint64_t`, it is an error

### Examples

- `usub(10, 3u)` → success
- `usub(3, 10u)` → error because the result would be negative

---

## `mul`

`mul(a, b)` performs multiplication of two values.

- its basic policy is the same as `add`
- its return type is `xer::result<std::int64_t>`
- if the result does not fit in `std::int64_t`, it is an error

### Examples

- `mul(3, 4u)` → success
- `mul(-3, 4u)` → succeeds with `-12`
- `mul(INT64_MIN, -1)` → error

---

## `umul`

`umul(a, b)` performs multiplication of two values.

- its basic policy is the same as `uadd`
- its return type is `xer::result<std::uint64_t>`
- if the result does not fit in `std::uint64_t`, it is an error

### Examples

- `umul(3, 4)` → success
- `umul(-3, 4)` → error because the result would be negative

---

## `div`

`div(a, b)` performs division of two values.

### For Integer Types

- the return type is `xer::result<std::int64_t>`
- mixing signed and unsigned integer types is allowed
- the quotient is rounded toward zero
- if the result does not fit in `std::int64_t`, it is an error
- division by zero is an error
- `div(INT64_MIN, -1)` is an error

### Retrieving the Remainder

The remainder output parameter may be omitted.

- when the remainder is retrieved, its type is `std::int64_t*`
- errors are returned only through the main return value, not through the remainder parameter

When the remainder is retrieved, the following rules are used so that it remains symmetric with integer division:

- the quotient is rounded toward zero
- the remainder satisfies `a == b * q + r`
- the sign of the remainder follows the dividend

### Examples

- `div(7, 3)` → quotient `2`, remainder `1`
- `div(-7, 3)` → quotient `-2`, remainder `-1`

---

## `udiv`

`udiv(a, b)` performs division of two values.

### For Integer Types

- the return type is `xer::result<std::uint64_t>`
- mixing signed and unsigned integer types is allowed
- if the result does not fit in `std::uint64_t`, it is an error
- division by zero is an error

### Retrieving the Remainder

- the remainder output parameter may be omitted
- when the remainder is retrieved, its type is `std::uint64_t*`
- errors are returned only through the main return value

---

## `mod`

`mod(a, b)` returns the remainder.

### For Integer Types

- the return type is `xer::result<std::int64_t>`
- the remainder is computed according to the same rules as `div`
- division by zero is an error

### Examples

- `mod(7, 3)` → `1`
- `mod(-7, 3)` → `-1`

---

## `umod`

`umod(a, b)` returns the remainder.

### For Integer Types

- the return type is `xer::result<std::uint64_t>`
- the remainder is computed according to the same rules as `udiv`
- division by zero is an error

---

## Arithmetic for Floating-Point Types

Floating-point types are also designed according to a straightforward policy.

- the return type is `long double`
- `xer::result` arguments are also accepted
- non-computable cases are errors

### Implementation Note

On x86_64 environments, `long double` typically has a 64-bit significand, so it remains fairly practical even when integer types are mixed in terms of precision.

However, since the situation differs on Visual C++ and ARM environments, this should not be treated as a public specification assumption.

---

## `div` and `mod` for Floating-Point Types

To preserve symmetry with integer division, when a floating-point `div` is given a remainder output parameter, the quotient and remainder are defined based on `std::trunc`.

### `div(x, y)`

- when no remainder argument is given, it returns the ordinary division result

### `div(x, y, &r)`

- the return value is `std::trunc(x / y)`
- the remainder is `x - y * q`
- here, `q` is the return value

### `mod(x, y)`

- it returns `x - y * std::trunc(x / y)`

### Note

- `std::remainder` is not adopted
- the reason is that it breaks symmetry with integer division

---

## Arithmetic for Complex Types

For complex types as well, dedicated functions may be provided within a reasonable scope.

- addition, subtraction, multiplication, and division are accepted
- the return type is `std::complex<long double>`
- `xer::result` arguments are also accepted
- comparison operations are not provided as a rule

---

## Policy for Comparison Operations

### Basic Policy

Comparison functions return `bool`.

- `eq(a, b)`
- `ne(a, b)`
- `lt(a, b)`
- `le(a, b)`
- `gt(a, b)`
- `ge(a, b)`

### Why `xer::result` Is Not Returned

If comparison functions returned `xer::result<bool>`, misuse in conditional expressions would become too easy.

For example, the following code may look natural at first glance, but is dangerous:

```cpp
if (eq(a, b)) {
    ...
}
````

For that reason, comparison is not designed to return errors.
Instead, the usable argument domain should be appropriately restricted while returning `bool`.

---

## `min`

`min(a, b)` returns the smaller of two values.

### For Integer Types

For integer types, `min` does not determine its return type according to the built-in usual arithmetic conversions.
Instead, the return type is determined by the following rules:

1. first apply the **integral promotions** to each argument
2. compare the representable ranges of the two promoted types
3. if the representable ranges are the same, use the promoted type of the first argument as the return type
4. if the representable ranges differ, use the larger **signed integer type** as the return type

In implementation, `xer::intmax_t` or `xer::uintmax_t` may be used as intermediate representations if needed.

If the selected value does not fit in the final return type, it is an error.

### When a Floating-Point Type Is Involved

If at least one argument is a floating-point type, the return type may be `std::common_type_t`-like.

### Comparison Method

Ordering is determined by using `xer::lt`.

### Examples

* `min(-3, 10u)` → `-3`
* `min(3u, 10)` → `3u`
* `min(3, 2.5)` → `2.5`

---

## `max`

`max(a, b)` returns the larger of two values.

### For Integer Types

For integer types, the return type determination rule of `max` is the same as that of `min`.

1. apply the integral promotions to each argument
2. if the representable ranges are the same, use the promoted type of the first argument as the return type
3. if the representable ranges differ, use the larger signed integer type as the return type

If the selected value does not fit in the return type, it is an error.

### When a Floating-Point Type Is Involved

If at least one argument is a floating-point type, the return type may be `std::common_type_t`-like.

### Comparison Method

Ordering is determined by using `xer::lt`.

### Examples

* `max(-3, 10u)` → `10`
* `max(3u, 10)` → `10u`
* `max(3, 2.5)` → `3.0`

---

## `clamp`

`clamp(value, lo, hi)` returns the value constrained to the closed interval `[lo, hi]`.

### Return Type

The return type of `clamp` is **always the type of the first argument `value`**.

Unlike `min` and `max`, it is not widened to a common type involving the second or third arguments.

### Rules of Selection

* if `value < lo`, return `lo`
* if `hi < value`, return `hi`
* otherwise, return `value`

### Comparison Method

Comparison uses `xer::lt`.

### Error Conditions

* if `hi < lo`, it is an error
* if `lo` or `hi` cannot be converted to the return type, it is an error
* if the selected value does not fit in the return type, that is also an error

### Examples

* `clamp(20, 0, 10u)` → `10`
* `clamp(-5, 1u, 10u)` → `1`
* `clamp(20u, 0, 10)` → `10u`

---

## `in_range`

`in_range<T>(value)` checks whether a value fits in the representable range of type `T`.

* the return type is `bool`
* `T` is intended for arithmetic types
* using `bool` as the target type is not allowed
* passing `bool` as `value` may be allowed

When converting from floating-point to an integer type, the check must confirm not only the numeric range but also that the value is finite.

---

## `abs`

`abs(value)` returns the absolute value.

### For Integer Types

* the return type is `xer::result<std::int64_t>`
* if the result does not fit in `std::int64_t`, it is an error

### For Floating-Point Types

* the return type is `xer::result<long double>`
* the treatment of `NaN` and infinity is left to implementation, but at minimum non-computable cases may be treated as errors

---

## `uabs`

`uabs(value)` returns the nonnegative absolute value.

### For Integer Types

* the return type is `xer::result<std::uint64_t>`
* if the result does not fit in `std::uint64_t`, it is an error

### For Floating-Point Types

* it is not mandatory in the initial stage
* if introduced, it may be designed on a `long double` basis in the same way as `abs`

---

## Supplement

`min`, `max`, `clamp`, `in_range`, `abs`, and `uabs` are positioned not as simple substitutes for ordinary operators, but as **helper functions for explicitly applying XER's comparison rules, type rules, and error rules**.

In particular, priority is placed on returning mathematically straightforward and predictable results for mixed integer types, and on making failure explicit through `xer::result` when necessary.
