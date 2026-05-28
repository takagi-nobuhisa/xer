# `<xer/math.h>`

## Purpose

`<xer/math.h>` provides lightweight real-number mathematical helpers in xer.

The initial scope is intentionally practical. It is not a replacement for a numerical-analysis library. It provides small, commonly useful helpers whose behavior and error handling are explicit.

---

## Provided Functions

### `quadratic`

```cpp
template<std::floating_point T>
auto quadratic(T a, T b, T c)
    -> xer::result<std::array<std::optional<T>, 2>>;
```

Solves the quadratic equation:

```text
a * x * x + b * x + c == 0
```

The coefficient `a` must not be zero. If `a == 0`, the function returns `error_t::invalid_argument`.

The returned array stores distinct real roots from the first element:

| Result | Meaning |
|---|---|
| `{ nullopt, nullopt }` | no real root |
| `{ x, nullopt }` | one real root, including a double root |
| `{ x1, x2 }` | two distinct real roots |

When two real roots are returned, they are sorted in ascending order.

### `cubic`

```cpp
template<std::floating_point T>
auto cubic(T a, T b, T c, T d)
    -> xer::result<std::array<std::optional<T>, 3>>;
```

Solves the cubic equation:

```text
a * x * x * x + b * x * x + c * x + d == 0
```

The coefficient `a` must not be zero. If `a == 0`, the function returns `error_t::invalid_argument`.

The returned array stores distinct real roots from the first element. Empty elements are represented by `std::nullopt`. Returned roots are sorted in ascending order.

---

## Design Notes

`quadratic` and `cubic` return only real roots. Non-real roots are not errors; they are simply not stored in the returned optional array.

For complex roots, use `<xer/complex.h>`.

Repeated roots are represented once in the real-number functions. This keeps the result useful for common practical checks such as intersections, hit times, and real-domain constraints.
