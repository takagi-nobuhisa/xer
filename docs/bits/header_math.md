# `<xer/math.h>`

## Purpose

`<xer/math.h>` provides lightweight real-number mathematical helpers in xer.

The initial scope is intentionally practical. It is not a replacement for a numerical-analysis library. It provides small, commonly useful helpers whose behavior and error handling are explicit.

---


## Types

### `vec`

```cpp
template<class T, std::size_t N = 2>
struct vec;
```

Represents a small position vector or mathematical vector.

Only `N == 2`, `N == 3`, and `N == 4` are provided. The primary template is intentionally not defined, because `<xer/math.h>` is not intended to provide a general-purpose arbitrary-dimensional vector type.

The specializations provide named coordinate members:

| Type | Members |
|---|---|
| `vec<T, 2>` | `x`, `y` |
| `vec<T, 3>` | `x`, `y`, `z` |
| `vec<T, 4>` | `x`, `y`, `z`, `w` |

`vec<T>` is equivalent to `vec<T, 2>`.

Each specialization also provides unchecked subscript access:

```cpp
auto operator[](std::size_t index) noexcept -> T&;
auto operator[](std::size_t index) const noexcept -> const T&;
```

`operator[]` does not perform range checking. Passing an out-of-range index has undefined behavior.

For checked access, use `at`:

```cpp
auto at(std::size_t index) noexcept
    -> xer::result<std::reference_wrapper<T>>;

auto at(std::size_t index) const noexcept
    -> xer::result<std::reference_wrapper<const T>>;
```

If `index` is out of range, `at` returns `error_t::out_of_range`.

### `polar`

```cpp
template<class T, std::size_t N = 2>
struct polar;
```

Represents polar coordinates.

Currently, only `polar<T, 2>` is provided:

```cpp
template<class T>
struct polar<T, 2> {
    T r;
    cyclic<T> theta;
};
```

`theta` is a cyclic angle expressed in τrad, where `1` means one full turn.

---

## Provided Functions




### Trigonometric Functions

```cpp
template<std::floating_point T>
auto sin(T theta) noexcept -> T;

template<std::floating_point T>
auto sin(cyclic<T> theta) noexcept -> T;

template<std::floating_point T>
auto cos(T theta) noexcept -> T;

template<std::floating_point T>
auto cos(cyclic<T> theta) noexcept -> T;

template<std::floating_point T>
auto tan(T theta) noexcept -> T;

template<std::floating_point T>
auto tan(cyclic<T> theta) noexcept -> T;
```

Computes the ordinary trigonometric functions using xer angle units.

The scalar overloads interpret `theta` as a τrad value, where `1` means one full turn. The `cyclic<T>` overloads use the normalized value of the cyclic angle.

Examples:

```text
sin(0.25) == 1
cos(0.5) == -1
tan(0.125) == 1
```

Hyperbolic functions are intentionally not provided by `<xer/math.h>`. Use the C++ standard library directly when those functions are needed.

### Inverse Trigonometric Functions

```cpp
template<std::floating_point T>
auto asin(T value) noexcept -> T;

template<std::floating_point T>
auto acos(T value) noexcept -> T;

template<std::floating_point T>
auto atan(T value) noexcept -> T;

template<std::floating_point T>
auto atan2(T y, T x) noexcept -> T;
```

Computes the inverse trigonometric functions and returns τrad scalar values.

Typical return ranges are inherited from the corresponding standard-library functions after conversion from radians to τrad:

| Function | Typical return range |
|---|---|
| `asin` | `[-0.25, 0.25]` |
| `acos` | `[0, 0.5]` |
| `atan` | `(-0.25, 0.25)` |
| `atan2` | `[-0.5, 0.5]` |

For domain errors, these functions follow the behavior of the underlying standard-library functions, such as returning NaN for invalid floating-point inputs. They do not return `xer::result`.


### `to_polar`

```cpp
template<std::floating_point T>
auto to_polar(vec<T, 2> v) noexcept -> polar<T, 2>;
```

Converts a two-dimensional Cartesian vector to polar coordinates.

The returned radius is computed with `std::hypot(v.x, v.y)`. The returned angle is computed from `std::atan2(v.y, v.x)` and converted to a cyclic τrad value.

### `to_cartesian`

```cpp
template<std::floating_point T>
auto to_cartesian(polar<T, 2> p) noexcept -> vec<T, 2>;
```

Converts two-dimensional polar coordinates to a Cartesian vector.

The coordinate components are computed as:

```text
x = r * cos(theta * τ)
y = r * sin(theta * τ)
```


### `dot`

```cpp
template<class T, std::size_t N>
auto dot(vec<T, N> a, vec<T, N> b) noexcept -> T;
```

Computes the dot product of two vectors.

`T` must be an arithmetic type. `N` must be one of the supported `vec` dimensions: `2`, `3`, or `4`.

The function returns the sum of the products of corresponding components. For integral vectors, the returned value is also integral.

### `length`

```cpp
template<class T, std::size_t N>
auto length(vec<T, N> v) noexcept -> std::common_type_t<T, double>;
```

Computes the Euclidean length of a vector.

`T` must be an arithmetic type. `N` must be one of the supported `vec` dimensions: `2`, `3`, or `4`.

The return type is `std::common_type_t<T, double>`, so integer vectors produce a floating-point length.

### `distance`

```cpp
template<class T, std::size_t N>
auto distance(vec<T, N> a, vec<T, N> b) noexcept -> std::common_type_t<T, double>;
```

Computes the Euclidean distance between two vectors. Positions are represented as position vectors, so this function can also be used to compute the distance between two points.

`T` must be an arithmetic type. `N` must be one of the supported `vec` dimensions: `2`, `3`, or `4`.

The return type is `std::common_type_t<T, double>`, so integer vectors produce a floating-point distance.

### `normalize`

```cpp
template<class T, std::size_t N>
auto normalize(vec<T, N> v) noexcept
    -> xer::result<vec<std::common_type_t<T, double>, N>>;
```

Returns a vector with the same direction as `v` and length `1`.

`T` must be an arithmetic type. `N` must be one of the supported `vec` dimensions: `2`, `3`, or `4`.

The returned vector uses `std::common_type_t<T, double>` as its component type, so integer vectors can be normalized without losing fractional components.

If `v` is the zero vector, the function returns `error_t::invalid_argument`.

### `angle`

```cpp
template<class T, std::size_t N>
auto angle(vec<T, N> a, vec<T, N> b) noexcept
    -> xer::result<std::common_type_t<T, double>>;
```

Computes the unsigned angle between two vectors in τrad.

`T` must be an arithmetic type. `N` must be one of the supported `vec` dimensions: `2`, `3`, or `4`.

The return type is `std::common_type_t<T, double>`, so integer vectors produce a floating-point angle. The returned value is not `cyclic<T>` because this function returns an angle magnitude. It is in the range `0` to `0.5`, where `0.25` is a right angle and `0.5` is a straight angle.

If either vector is the zero vector, the function returns `error_t::invalid_argument`.

### `rotate`

```cpp
template<class T, std::floating_point Angle>
auto rotate(vec<T, 2> v, cyclic<Angle> theta) noexcept
    -> vec<std::common_type_t<T, Angle, double>, 2>;
```

Rotates a two-dimensional vector around the origin.

`T` must be an arithmetic type. `theta` is a cyclic angle expressed in τrad, where `1` means one full turn.

The return type uses `std::common_type_t<T, Angle, double>` as its component type, so integer vectors can be rotated without losing fractional components.

Positive angles rotate counterclockwise in the usual mathematical coordinate system.

### `cross`

```cpp
template<class T>
auto cross(vec<T, 3> a, vec<T, 3> b) noexcept -> vec<T, 3>;
```

Computes the three-dimensional cross product of two vectors.

`T` must be an arithmetic type. The function currently supports only `vec<T, 3>`.

### `heron`

```cpp
template<std::floating_point T>
auto heron(T a, T b, T c) -> xer::result<T>;
```

Computes the area of a triangle from its three side lengths using Heron's formula.

The side lengths must be non-negative and must be able to form a triangle. If a side length is negative, or if the side lengths violate the triangle inequality, the function returns `error_t::invalid_argument`. Degenerate triangles are accepted and return zero.

The implementation uses a rearranged form of Heron's formula after sorting the side lengths. This avoids some avoidable cancellation compared with the most direct `s * (s - a) * (s - b) * (s - c)` form.

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


## Coordinate and Vector Policy

Positions are represented as position vectors. `<xer/math.h>` does not provide a separate point type for mathematical coordinates.

The `vec` type is limited to two, three, and four dimensions. This keeps the facility focused on practical geometry, graphics, coordinate conversion, and simple physical calculations rather than becoming a general linear-algebra package.
