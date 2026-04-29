# `<xer/matrix.h>`

## Purpose

`<xer/matrix.h>` provides fixed-size matrix and affine transform helpers in XER.

The initial purpose of this header is deliberately practical and limited.
It is not intended to become a full linear algebra framework at the beginning.
Instead, it provides enough functionality to express common 2D and 3D affine transforms and their inverse transforms in a clear, lightweight way.

---

## Main Role

The main role of `<xer/matrix.h>` is to provide:

- fixed-size row-major matrices
- 3x3 and 4x4 matrix aliases
- 3x1 and 4x1 column-vector aliases
- ordinary matrix multiplication
- identity matrix creation
- inverse calculation for 3x3 and 4x4 matrices
- helper functions for 2D and 3D affine transforms

This makes it possible to write transform code such as:

```cpp
const xer::vector3<double> point{2.0, 3.0, 1.0};

const auto transform =
    xer::translate2(10.0, 20.0) *
    xer::scale2(1.5, 4.0);

const auto transformed = transform * point;
```

---

## Main Entities

At minimum, `<xer/matrix.h>` provides the following entities:

```cpp
template <std::floating_point T, std::size_t Rows, std::size_t Cols>
class matrix;

template <std::floating_point T>
using matrix3 = matrix<T, 3, 3>;

template <std::floating_point T>
using matrix4 = matrix<T, 4, 4>;

template <std::floating_point T>
using vector3 = matrix<T, 3, 1>;

template <std::floating_point T>
using vector4 = matrix<T, 4, 1>;
```

It also provides multiplication, identity matrix creation, inverse calculation, and affine transform helper functions.

---

## `matrix<T, Rows, Cols>`

`matrix<T, Rows, Cols>` is the fundamental fixed-size matrix type.

### Basic Shape

```cpp
template <std::floating_point T, std::size_t Rows, std::size_t Cols>
class matrix;
```

### Element Type

The element type is restricted to floating-point types.

The main intended types are:

* `float`
* `double`
* `long double`

This keeps the first implementation focused on geometric transforms and numeric operations where fractional values are normal.

### Storage Model

The matrix is stored as a fixed-size row-major value.

Conceptually, the element at row `r` and column `c` is accessed as:

```cpp
m(r, c)
```

Rows and columns are zero-based.
Bounds are not checked by `operator()`.

### Construction

A default-constructed matrix is a zero matrix.

A matrix may also be constructed from exactly `Rows * Cols` values in row-major order:

```cpp
xer::matrix<double, 2, 3> value{
    1.0, 2.0, 3.0,
    4.0, 5.0, 6.0
};
```

The exact number of values is required so that incomplete or excessive matrix literals are detected at compile time.

---

## Matrix Aliases

The header provides aliases for the matrix sizes used by the initial affine-transform functionality.

```cpp
template <std::floating_point T>
using matrix3 = matrix<T, 3, 3>;

template <std::floating_point T>
using matrix4 = matrix<T, 4, 4>;
```

### Role

* `matrix3<T>` is primarily used for 2D homogeneous affine transforms.
* `matrix4<T>` is primarily used for 3D homogeneous affine transforms.

---

## Column Vector Aliases

The header also provides aliases for homogeneous column vectors.

```cpp
template <std::floating_point T>
using vector3 = matrix<T, 3, 1>;

template <std::floating_point T>
using vector4 = matrix<T, 4, 1>;
```

### Role

* `vector3<T>` is typically used as a 2D homogeneous column vector: `(x, y, 1)`.
* `vector4<T>` is typically used as a 3D homogeneous column vector: `(x, y, z, 1)`.

They are aliases of `matrix`, not separate vector classes.
This keeps the first implementation simple and makes matrix-vector multiplication the ordinary matrix multiplication operation.

---

## Matrix Multiplication

`<xer/matrix.h>` provides ordinary row-by-column matrix multiplication.

```cpp
auto operator*(
    const matrix<T, R, C>& left,
    const matrix<T, C, K>& right) noexcept
    -> matrix<T, R, K>;
```

### Role

This single operation covers:

* matrix × matrix
* matrix × column vector
* affine transform composition
* applying an affine transform to a homogeneous point

### Transform Composition Order

XER uses column vectors in this matrix facility.
Therefore, in an expression such as:

```cpp
const auto transform = translate2<double>(10.0, 20.0) * scale2<double>(2.0, 3.0);
const auto result = transform * point;
```

`point` is transformed by the rightmost transform first.
In this example, scaling is applied first, and translation is applied afterward.

---

## Identity Matrices

The header provides a generic identity matrix helper:

```cpp
template <std::floating_point T, std::size_t N>
auto identity_matrix() noexcept -> matrix<T, N, N>;
```

It also provides convenience helpers for the two main affine-transform sizes:

```cpp
template <std::floating_point T>
auto identity3() noexcept -> matrix3<T>;

template <std::floating_point T>
auto identity4() noexcept -> matrix4<T>;
```

---

## Inverse Matrices

The header provides inverse calculation for 3x3 and 4x4 matrices:

```cpp
template <std::floating_point T>
auto inverse(const matrix<T, 3, 3>& value) noexcept
    -> xer::result<matrix<T, 3, 3>>;

template <std::floating_point T>
auto inverse(const matrix<T, 4, 4>& value) noexcept
    -> xer::result<matrix<T, 4, 4>>;
```

### Error Handling

If the matrix is singular or too close to singular for the implemented calculation, `inverse` returns failure.

The current implementation reports this as `error_t::divide_by_zero`.
This expresses the fact that the inverse operation requires division by a usable pivot value.

---

## 2D Affine Transform Helpers

For 2D homogeneous column vectors, the header provides 3x3 transform helpers.

```cpp
template <std::floating_point T>
auto translate2(T tx, T ty) noexcept -> matrix3<T>;

template <std::floating_point T>
auto scale2(T sx, T sy) noexcept -> matrix3<T>;

template <std::floating_point T>
auto rotate2(T radian) noexcept -> matrix3<T>;
```

### Rotation Direction

`rotate2` uses radians and follows the ordinary mathematical convention: positive angles rotate counterclockwise.

---

## 3D Affine Transform Helpers

For 3D homogeneous column vectors, the header provides 4x4 transform helpers.

```cpp
template <std::floating_point T>
auto translate3(T tx, T ty, T tz) noexcept -> matrix4<T>;

template <std::floating_point T>
auto scale3(T sx, T sy, T sz) noexcept -> matrix4<T>;

template <std::floating_point T>
auto rotate_x(T radian) noexcept -> matrix4<T>;

template <std::floating_point T>
auto rotate_y(T radian) noexcept -> matrix4<T>;

template <std::floating_point T>
auto rotate_z(T radian) noexcept -> matrix4<T>;
```

### Rotation Units

The rotation helpers take raw radian values.

Angle quantities, `cyclic`, and other higher-level angle abstractions are not mixed into the first matrix API.
Callers may convert into radians before calling these functions when needed.

---

## Scope of the Initial Matrix Facility

The initial matrix facility is intentionally small.

It focuses on:

* 2D affine transforms using 3x3 matrices
* 3D affine transforms using 4x4 matrices
* homogeneous column vectors
* inverse transforms for 3x3 and 4x4 matrices

It does not initially try to provide a complete linear algebra library.

Deferred or intentionally omitted items include:

* dynamic-size matrices
* decomposition algorithms
* eigenvalues or eigenvectors
* specialized vector classes
* determinant APIs
* full numerical linear algebra facilities

These may be considered later only if they become necessary.

---

## Relationship to Other Headers

`<xer/matrix.h>` should be understood together with:

* `policy_project_outline.md`
* `policy_arithmetic.md`
* `header_arithmetic.md`
* `header_cyclic.md`
* `header_quantity.md`

The rough boundary is:

* `<xer/arithmetic.h>` handles scalar arithmetic and comparison helpers
* `<xer/cyclic.h>` handles circular values such as normalized angles and directions
* `<xer/quantity.h>` handles physical quantities and units
* `<xer/matrix.h>` handles fixed-size matrices and affine transforms

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that the matrix type is fixed-size and row-major
* that column vectors are represented as `matrix<T, N, 1>` aliases
* that the initial focus is 2D and 3D affine transforms
* that inverse calculation is provided for 3x3 and 4x4 matrices
* that rotation helpers take radians

Detailed numerical behavior and future linear algebra expansion should be documented separately when those features are added.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* applying a 2D affine transform to a point
* composing translation, scaling, and rotation transforms
* applying a 3D affine transform to a point
* computing an inverse transform and restoring the original point

These are good candidates for executable examples in `examples/`.

---

## Example

```cpp
#include <xer/matrix.h>

auto main() -> int
{
    const xer::vector3<double> point{2.0, 3.0, 1.0};

    const auto transform =
        xer::translate2(10.0, 20.0) *
        xer::scale2(1.5, 4.0);

    const auto transformed = transform * point;
    const auto inverse = xer::inverse(transform);
    if (!inverse.has_value()) {
        return 1;
    }

    const auto restored = *inverse * transformed;

    static_cast<void>(restored);
    return 0;
}
```

This example shows the normal XER style:

* represent points as homogeneous column vectors
* compose transforms with matrix multiplication
* apply a transform by multiplying the matrix and the point
* check `xer::result` explicitly when computing an inverse matrix

---

## See Also

* `policy_project_outline.md`
* `policy_arithmetic.md`
* `header_arithmetic.md`
* `header_cyclic.md`
* `header_quantity.md`
