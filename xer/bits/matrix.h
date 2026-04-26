/**
 * @file xer/bits/matrix.h
 * @brief Internal fixed-size matrix and affine transform helpers.
 */

#pragma once

#ifndef XER_BITS_MATRIX_H_INCLUDED_
#define XER_BITS_MATRIX_H_INCLUDED_

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <expected>
#include <limits>
#include <utility>

#include <xer/bits/math_constants.h>
#include <xer/error.h>

namespace xer {

/**
 * @brief Fixed-size row-major matrix.
 *
 * The matrix type is intentionally small and direct. It stores values in a
 * row-major fixed-size array and exposes element access through
 * `operator()(row, column)`.
 *
 * @tparam T Floating-point element type.
 * @tparam Rows Number of rows.
 * @tparam Cols Number of columns.
 */
template<std::floating_point T, std::size_t Rows, std::size_t Cols>
class matrix {
    static_assert(Rows > 0);
    static_assert(Cols > 0);

public:
    using value_type = T;
    using size_type = std::size_t;

    static constexpr size_type row_count = Rows;
    static constexpr size_type col_count = Cols;

    /**
     * @brief Constructs a zero matrix.
     */
    constexpr matrix() noexcept = default;

    /**
     * @brief Constructs a matrix from row-major values.
     *
     * The number of arguments must match the number of matrix elements. This
     * avoids silently ignoring excess initializers or silently accepting an
     * incomplete matrix where a complete literal was intended.
     */
    template<class... Args>
        requires(sizeof...(Args) == Rows * Cols &&
                 (std::convertible_to<Args, T> && ...))
    constexpr explicit matrix(Args... args) noexcept
        : values_{static_cast<T>(args)...}
    {}

    /**
     * @brief Returns a mutable reference to an element.
     *
     * Bounds are not checked. The caller is responsible for passing valid
     * indices.
     */
    [[nodiscard]] constexpr auto operator()(size_type row, size_type col) noexcept
        -> T&
    {
        return values_[row * Cols + col];
    }

    /**
     * @brief Returns an element value.
     *
     * Bounds are not checked. The caller is responsible for passing valid
     * indices.
     */
    [[nodiscard]] constexpr auto operator()(size_type row, size_type col) const noexcept
        -> T
    {
        return values_[row * Cols + col];
    }

private:
    std::array<T, Rows * Cols> values_{};
};

/**
 * @brief 3x3 matrix alias.
 */
template<std::floating_point T>
using matrix3 = matrix<T, 3, 3>;

/**
 * @brief 4x4 matrix alias.
 */
template<std::floating_point T>
using matrix4 = matrix<T, 4, 4>;

/**
 * @brief 3x1 column vector alias, typically used for 2D homogeneous vectors.
 */
template<std::floating_point T>
using vector3 = matrix<T, 3, 1>;

/**
 * @brief 4x1 column vector alias, typically used for 3D homogeneous vectors.
 */
template<std::floating_point T>
using vector4 = matrix<T, 4, 1>;

/**
 * @brief Multiplies two fixed-size matrices.
 *
 * The matrices are multiplied using ordinary row-by-column multiplication.
 */
template<std::floating_point T,
         std::size_t LeftRows,
         std::size_t Shared,
         std::size_t RightCols>
[[nodiscard]] constexpr auto operator*(
    const matrix<T, LeftRows, Shared>& left,
    const matrix<T, Shared, RightCols>& right) noexcept
    -> matrix<T, LeftRows, RightCols>
{
    matrix<T, LeftRows, RightCols> result;

    for (std::size_t row = 0; row < LeftRows; ++row) {
        for (std::size_t col = 0; col < RightCols; ++col) {
            T sum = static_cast<T>(0);

            for (std::size_t k = 0; k < Shared; ++k) {
                sum += left(row, k) * right(k, col);
            }

            result(row, col) = sum;
        }
    }

    return result;
}

/**
 * @brief Creates an identity matrix.
 *
 * @tparam T Floating-point element type.
 * @tparam N Matrix size.
 * @return N x N identity matrix.
 */
template<std::floating_point T, std::size_t N>
[[nodiscard]] constexpr auto identity_matrix() noexcept -> matrix<T, N, N>
{
    matrix<T, N, N> result;

    for (std::size_t i = 0; i < N; ++i) {
        result(i, i) = static_cast<T>(1);
    }

    return result;
}

/**
 * @brief Creates a 3x3 identity matrix.
 */
template<std::floating_point T>
[[nodiscard]] constexpr auto identity3() noexcept -> matrix3<T>
{
    return identity_matrix<T, 3>();
}

/**
 * @brief Creates a 4x4 identity matrix.
 */
template<std::floating_point T>
[[nodiscard]] constexpr auto identity4() noexcept -> matrix4<T>
{
    return identity_matrix<T, 4>();
}

namespace detail {

template<std::floating_point T>
[[nodiscard]] constexpr auto matrix_inverse_epsilon() noexcept -> T
{
    return std::numeric_limits<T>::epsilon() * static_cast<T>(1024);
}

template<std::floating_point T, std::size_t N>
constexpr auto swap_matrix_rows(
    matrix<T, N, N>& value,
    std::size_t left,
    std::size_t right) noexcept -> void
{
    if (left == right) {
        return;
    }

    for (std::size_t col = 0; col < N; ++col) {
        std::swap(value(left, col), value(right, col));
    }
}

template<std::floating_point T, std::size_t N>
[[nodiscard]] auto inverse_square_matrix(const matrix<T, N, N>& value) noexcept
    -> result<matrix<T, N, N>>
{
    auto left = value;
    auto right = identity_matrix<T, N>();
    const auto epsilon = matrix_inverse_epsilon<T>();

    for (std::size_t col = 0; col < N; ++col) {
        std::size_t pivot_row = col;
        auto pivot_abs = std::fabs(left(col, col));

        for (std::size_t row = col + 1; row < N; ++row) {
            const auto candidate_abs = std::fabs(left(row, col));
            if (candidate_abs > pivot_abs) {
                pivot_abs = candidate_abs;
                pivot_row = row;
            }
        }

        if (pivot_abs <= epsilon) {
            return std::unexpected(make_error(error_t::divide_by_zero));
        }

        swap_matrix_rows(left, col, pivot_row);
        swap_matrix_rows(right, col, pivot_row);

        const auto pivot = left(col, col);
        for (std::size_t k = 0; k < N; ++k) {
            left(col, k) /= pivot;
            right(col, k) /= pivot;
        }

        for (std::size_t row = 0; row < N; ++row) {
            if (row == col) {
                continue;
            }

            const auto factor = left(row, col);
            if (factor == static_cast<T>(0)) {
                continue;
            }

            for (std::size_t k = 0; k < N; ++k) {
                left(row, k) -= factor * left(col, k);
                right(row, k) -= factor * right(col, k);
            }
        }
    }

    return right;
}

} // namespace detail

/**
 * @brief Computes the inverse of a 3x3 matrix.
 *
 * If the matrix is singular, this function returns `error_t::divide_by_zero`.
 */
template<std::floating_point T>
[[nodiscard]] auto inverse(const matrix<T, 3, 3>& value) noexcept
    -> result<matrix<T, 3, 3>>
{
    return detail::inverse_square_matrix(value);
}

/**
 * @brief Computes the inverse of a 4x4 matrix.
 *
 * If the matrix is singular, this function returns `error_t::divide_by_zero`.
 */
template<std::floating_point T>
[[nodiscard]] auto inverse(const matrix<T, 4, 4>& value) noexcept
    -> result<matrix<T, 4, 4>>
{
    return detail::inverse_square_matrix(value);
}

/**
 * @brief Creates a 2D translation matrix for homogeneous column vectors.
 */
template<std::floating_point T>
[[nodiscard]] constexpr auto translate2(T tx, T ty) noexcept -> matrix3<T>
{
    return matrix3<T>{
        static_cast<T>(1), static_cast<T>(0), tx,
        static_cast<T>(0), static_cast<T>(1), ty,
        static_cast<T>(0), static_cast<T>(0), static_cast<T>(1)};
}

/**
 * @brief Creates a 2D scale matrix for homogeneous column vectors.
 */
template<std::floating_point T>
[[nodiscard]] constexpr auto scale2(T sx, T sy) noexcept -> matrix3<T>
{
    return matrix3<T>{
        sx, static_cast<T>(0), static_cast<T>(0),
        static_cast<T>(0), sy, static_cast<T>(0),
        static_cast<T>(0), static_cast<T>(0), static_cast<T>(1)};
}

/**
 * @brief Creates a 2D counterclockwise rotation matrix for radians.
 */
template<std::floating_point T>
[[nodiscard]] auto rotate2(T radian) noexcept -> matrix3<T>
{
    const auto c = std::cos(radian);
    const auto s = std::sin(radian);

    return matrix3<T>{
        c, -s, static_cast<T>(0),
        s, c, static_cast<T>(0),
        static_cast<T>(0), static_cast<T>(0), static_cast<T>(1)};
}

/**
 * @brief Creates a 3D translation matrix for homogeneous column vectors.
 */
template<std::floating_point T>
[[nodiscard]] constexpr auto translate3(T tx, T ty, T tz) noexcept -> matrix4<T>
{
    return matrix4<T>{
        static_cast<T>(1), static_cast<T>(0), static_cast<T>(0), tx,
        static_cast<T>(0), static_cast<T>(1), static_cast<T>(0), ty,
        static_cast<T>(0), static_cast<T>(0), static_cast<T>(1), tz,
        static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1)};
}

/**
 * @brief Creates a 3D scale matrix for homogeneous column vectors.
 */
template<std::floating_point T>
[[nodiscard]] constexpr auto scale3(T sx, T sy, T sz) noexcept -> matrix4<T>
{
    return matrix4<T>{
        sx, static_cast<T>(0), static_cast<T>(0), static_cast<T>(0),
        static_cast<T>(0), sy, static_cast<T>(0), static_cast<T>(0),
        static_cast<T>(0), static_cast<T>(0), sz, static_cast<T>(0),
        static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1)};
}

/**
 * @brief Creates a 3D rotation matrix around the X axis for radians.
 */
template<std::floating_point T>
[[nodiscard]] auto rotate_x(T radian) noexcept -> matrix4<T>
{
    const auto c = std::cos(radian);
    const auto s = std::sin(radian);

    return matrix4<T>{
        static_cast<T>(1), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0),
        static_cast<T>(0), c, -s, static_cast<T>(0),
        static_cast<T>(0), s, c, static_cast<T>(0),
        static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1)};
}

/**
 * @brief Creates a 3D rotation matrix around the Y axis for radians.
 */
template<std::floating_point T>
[[nodiscard]] auto rotate_y(T radian) noexcept -> matrix4<T>
{
    const auto c = std::cos(radian);
    const auto s = std::sin(radian);

    return matrix4<T>{
        c, static_cast<T>(0), s, static_cast<T>(0),
        static_cast<T>(0), static_cast<T>(1), static_cast<T>(0), static_cast<T>(0),
        -s, static_cast<T>(0), c, static_cast<T>(0),
        static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1)};
}

/**
 * @brief Creates a 3D rotation matrix around the Z axis for radians.
 */
template<std::floating_point T>
[[nodiscard]] auto rotate_z(T radian) noexcept -> matrix4<T>
{
    const auto c = std::cos(radian);
    const auto s = std::sin(radian);

    return matrix4<T>{
        c, -s, static_cast<T>(0), static_cast<T>(0),
        s, c, static_cast<T>(0), static_cast<T>(0),
        static_cast<T>(0), static_cast<T>(0), static_cast<T>(1), static_cast<T>(0),
        static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1)};
}

} // namespace xer

#endif /* XER_BITS_MATRIX_H_INCLUDED_ */
