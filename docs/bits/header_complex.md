# `<xer/complex.h>`

## Purpose

`<xer/complex.h>` provides lightweight complex-number mathematical helpers in xer.

This header contains the complex-number counterparts of selected functions in `<xer/math.h>`. It is separated from `<xer/math.h>` so that code using only real-number helpers does not need to include complex-number facilities.

---

## Provided Functions

### `cquadratic`

```cpp
template<std::floating_point T>
auto cquadratic(T a, T b, T c)
    -> xer::result<std::array<std::complex<T>, 2>>;
```

Solves the quadratic equation:

```text
a * x * x + b * x + c == 0
```

The coefficient `a` must not be zero. If `a == 0`, the function returns `error_t::invalid_argument`.

The returned array contains two complex roots with multiplicity.

### `ccubic`

```cpp
template<std::floating_point T>
auto ccubic(T a, T b, T c, T d)
    -> xer::result<std::array<std::complex<T>, 3>>;
```

Solves the cubic equation:

```text
a * x * x * x + b * x * x + c * x + d == 0
```

The coefficient `a` must not be zero. If `a == 0`, the function returns `error_t::invalid_argument`.

The returned array contains three complex roots with multiplicity.

---

## Design Notes

The `c` prefix means complex-root variant. For example, `cquadratic` is the complex-root counterpart of `quadratic`.

The functions currently accept real coefficients and return complex roots. Complex coefficients can be considered later if a practical need appears.
