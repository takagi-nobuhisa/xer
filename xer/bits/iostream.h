/**
 * @file xer/bits/iostream.h
 * @brief Defines iostream operators for selected XER value types.
 */

#pragma once

#ifndef XER_BITS_IOSTREAM_H_INCLUDED_
#define XER_BITS_IOSTREAM_H_INCLUDED_

#include <concepts>
#include <ios>
#include <istream>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>

#include <xer/bits/strerror.h>
#include <xer/cyclic.h>
#include <xer/error.h>
#include <xer/color.h>
#include <xer/interval.h>
#include <xer/matrix.h>
#include <xer/path.h>
#include <xer/quantity.h>
#include <xer/typeinfo.h>

namespace xer::detail {

/**
 * @brief Writes a UTF-8 string view to a narrow output stream as raw bytes.
 *
 * XER uses `char8_t` for UTF-8 text, while ordinary iostreams are based on
 * `char`. This helper deliberately copies the underlying UTF-8 code units to
 * the stream without locale-dependent conversion.
 *
 * @param stream Output stream.
 * @param value UTF-8 text to write.
 * @return Output stream.
 */
inline auto write_u8_to_ostream(
    std::ostream& stream,
    std::u8string_view value) -> std::ostream&
{
    return stream.write(reinterpret_cast<const char*>(value.data()),
                        static_cast<std::streamsize>(value.size()));
}

/**
 * @brief Converts a narrow token read from an input stream to UTF-8 storage.
 *
 * The byte sequence is copied as-is. This follows XER's current GCC-oriented
 * assumption that ordinary source and execution narrow strings are UTF-8 when
 * no explicit execution-charset option is used.
 *
 * @param value Narrow byte string.
 * @return UTF-8 string containing the same bytes.
 */
[[nodiscard]] inline auto iostream_bytes_to_u8string(std::string_view value)
    -> std::u8string
{
    return std::u8string(reinterpret_cast<const char8_t*>(value.data()),
                         value.size());
}

/**
 * @brief Parses an `error_t` enumerator name.
 *
 * @param name Enumerator name without `error_t::`.
 * @param code Destination error code.
 * @return true if the name was recognized.
 */
[[nodiscard]] inline auto parse_error_name(
    std::string_view name,
    error_t& code) noexcept -> bool
{
#define XER_DETAIL_PARSE_ERROR_NAME(name_)                                    \
    if (name == #name_) {                                                     \
        code = error_t::name_;                                                \
        return true;                                                          \
    }

    XER_DETAIL_PARSE_ERROR_NAME(perm)
    XER_DETAIL_PARSE_ERROR_NAME(noent)
    XER_DETAIL_PARSE_ERROR_NAME(srch)
    XER_DETAIL_PARSE_ERROR_NAME(intr)
    XER_DETAIL_PARSE_ERROR_NAME(io)
    XER_DETAIL_PARSE_ERROR_NAME(nxio)
    XER_DETAIL_PARSE_ERROR_NAME(toobig)
    XER_DETAIL_PARSE_ERROR_NAME(noexec)
    XER_DETAIL_PARSE_ERROR_NAME(badf)
    XER_DETAIL_PARSE_ERROR_NAME(child)
    XER_DETAIL_PARSE_ERROR_NAME(again)
    XER_DETAIL_PARSE_ERROR_NAME(nomem)
    XER_DETAIL_PARSE_ERROR_NAME(acces)
    XER_DETAIL_PARSE_ERROR_NAME(fault)
    XER_DETAIL_PARSE_ERROR_NAME(busy)
    XER_DETAIL_PARSE_ERROR_NAME(exist)
    XER_DETAIL_PARSE_ERROR_NAME(xdev)
    XER_DETAIL_PARSE_ERROR_NAME(nodev)
    XER_DETAIL_PARSE_ERROR_NAME(notdir)
    XER_DETAIL_PARSE_ERROR_NAME(isdir)
    XER_DETAIL_PARSE_ERROR_NAME(inval)
    XER_DETAIL_PARSE_ERROR_NAME(nfile)
    XER_DETAIL_PARSE_ERROR_NAME(mfile)
    XER_DETAIL_PARSE_ERROR_NAME(notty)
    XER_DETAIL_PARSE_ERROR_NAME(fbig)
    XER_DETAIL_PARSE_ERROR_NAME(nospc)
    XER_DETAIL_PARSE_ERROR_NAME(spipe)
    XER_DETAIL_PARSE_ERROR_NAME(rofs)
    XER_DETAIL_PARSE_ERROR_NAME(mlink)
    XER_DETAIL_PARSE_ERROR_NAME(pipe)
    XER_DETAIL_PARSE_ERROR_NAME(dom)
    XER_DETAIL_PARSE_ERROR_NAME(range)

#ifdef EDEADLK
    XER_DETAIL_PARSE_ERROR_NAME(deadlk)
#endif
#ifdef ENAMETOOLONG
    XER_DETAIL_PARSE_ERROR_NAME(nametoolong)
#endif
#ifdef ENOLCK
    XER_DETAIL_PARSE_ERROR_NAME(nolck)
#endif
#ifdef ENOSYS
    XER_DETAIL_PARSE_ERROR_NAME(nosys)
#endif
#ifdef ENOTEMPTY
    XER_DETAIL_PARSE_ERROR_NAME(notempty)
#endif
#ifdef EILSEQ
    XER_DETAIL_PARSE_ERROR_NAME(ilseq)
#endif

    XER_DETAIL_PARSE_ERROR_NAME(logic_error)
    XER_DETAIL_PARSE_ERROR_NAME(domain_error)
    XER_DETAIL_PARSE_ERROR_NAME(invalid_argument)
    XER_DETAIL_PARSE_ERROR_NAME(length_error)
    XER_DETAIL_PARSE_ERROR_NAME(out_of_range)
    XER_DETAIL_PARSE_ERROR_NAME(runtime_error)
    XER_DETAIL_PARSE_ERROR_NAME(range_error)
    XER_DETAIL_PARSE_ERROR_NAME(overflow_error)
    XER_DETAIL_PARSE_ERROR_NAME(underflow_error)
    XER_DETAIL_PARSE_ERROR_NAME(io_error)
    XER_DETAIL_PARSE_ERROR_NAME(encoding_error)
    XER_DETAIL_PARSE_ERROR_NAME(not_found)
    XER_DETAIL_PARSE_ERROR_NAME(divide_by_zero)
    XER_DETAIL_PARSE_ERROR_NAME(network_error)
    XER_DETAIL_PARSE_ERROR_NAME(process_error)
    XER_DETAIL_PARSE_ERROR_NAME(user_error)

#undef XER_DETAIL_PARSE_ERROR_NAME

    return false;
}

} // namespace xer::detail

namespace xer {


/**
 * @brief Reads an error code name.
 *
 * The accepted input form is the enumerator name without `error_t::`, such as
 * `invalid_argument` or `not_found`.
 *
 * @param stream Input stream.
 * @param value Destination error code.
 * @return Input stream.
 */
inline auto operator>>(std::istream& stream, error_t& value) -> std::istream&
{
    std::string token;
    if (!(stream >> token)) {
        return stream;
    }

    error_t parsed{};
    if (!detail::parse_error_name(token, parsed)) {
        stream.setstate(std::ios::failbit);
        return stream;
    }

    value = parsed;
    return stream;
}


/**
 * @brief Writes a type information display name.
 *
 * @param stream Output stream.
 * @param value Type information object.
 * @return Output stream.
 */
inline auto operator<<(
    std::ostream& stream,
    const type_info& value) -> std::ostream&
{
    return detail::write_u8_to_ostream(stream, value.name());
}

/**
 * @brief Writes a path in its normalized UTF-8 form.
 *
 * @param stream Output stream.
 * @param value Path value.
 * @return Output stream.
 */
inline auto operator<<(std::ostream& stream, const path& value) -> std::ostream&
{
    return detail::write_u8_to_ostream(stream, value.str());
}

/**
 * @brief Reads a whitespace-delimited path token.
 *
 * Paths containing whitespace are not handled by this formatted extraction
 * operator. Use line-oriented input and construct `xer::path` explicitly when
 * such paths are needed.
 *
 * @param stream Input stream.
 * @param value Destination path.
 * @return Input stream.
 */
inline auto operator>>(std::istream& stream, path& value) -> std::istream&
{
    std::string token;
    if (!(stream >> token)) {
        return stream;
    }

    value = path(detail::iostream_bytes_to_u8string(token));
    return stream;
}

/**
 * @brief Writes the normalized scalar value of a cyclic value.
 *
 * @tparam T Floating-point type.
 * @param stream Output stream.
 * @param value Cyclic value.
 * @return Output stream.
 */
template<std::floating_point T>
inline auto operator<<(
    std::ostream& stream,
    cyclic<T> value) -> std::ostream&
{
    return stream << value.value();
}

/**
 * @brief Reads a scalar value and constructs a cyclic value from it.
 *
 * @tparam T Floating-point type.
 * @param stream Input stream.
 * @param value Destination cyclic value.
 * @return Input stream.
 */
template<std::floating_point T>
inline auto operator>>(
    std::istream& stream,
    cyclic<T>& value) -> std::istream&
{
    T scalar{};
    if (!(stream >> scalar)) {
        return stream;
    }

    value = cyclic<T>(scalar);
    return stream;
}

/**
 * @brief Writes the stored scalar value of an interval value.
 *
 * @tparam T Floating-point type.
 * @tparam Min Inclusive lower bound.
 * @tparam Max Inclusive upper bound.
 * @param stream Output stream.
 * @param value Interval value.
 * @return Output stream.
 */
template<std::floating_point T, T Min, T Max>
inline auto operator<<(
    std::ostream& stream,
    interval<T, Min, Max> value) -> std::ostream&
{
    return stream << value.value();
}

/**
 * @brief Reads a scalar value and constructs an interval value from it.
 *
 * If interval construction rejects the input value, the stream fail bit is set.
 *
 * @tparam T Floating-point type.
 * @tparam Min Inclusive lower bound.
 * @tparam Max Inclusive upper bound.
 * @param stream Input stream.
 * @param value Destination interval value.
 * @return Input stream.
 */
template<std::floating_point T, T Min, T Max>
inline auto operator>>(
    std::istream& stream,
    interval<T, Min, Max>& value) -> std::istream&
{
    T scalar{};
    if (!(stream >> scalar)) {
        return stream;
    }

    try {
        value = interval<T, Min, Max>(scalar);
    } catch (...) {
        stream.setstate(std::ios::failbit);
    }

    return stream;
}

/**
 * @brief Writes the base-unit scalar value of a quantity.
 *
 * @tparam T Floating-point type.
 * @tparam Dim Dimension type.
 * @param stream Output stream.
 * @param value Quantity value.
 * @return Output stream.
 */
template<std::floating_point T, class Dim>
inline auto operator<<(
    std::ostream& stream,
    quantity<T, Dim> value) -> std::ostream&
{
    return stream << value.value();
}

/**
 * @brief Reads a base-unit scalar value and constructs a quantity.
 *
 * The input value is interpreted as already normalized to the base unit system
 * of the quantity dimension.
 *
 * @tparam T Floating-point type.
 * @tparam Dim Dimension type.
 * @param stream Input stream.
 * @param value Destination quantity value.
 * @return Input stream.
 */
template<std::floating_point T, class Dim>
inline auto operator>>(
    std::istream& stream,
    quantity<T, Dim>& value) -> std::istream&
{
    T scalar{};
    if (!(stream >> scalar)) {
        return stream;
    }

    value = quantity<T, Dim>(scalar);
    return stream;
}

/**
 * @brief Writes a fixed-size matrix in a compact row-major form.
 *
 * The output form is `[[a, b], [c, d]]` for a 2x2 matrix. This operator is
 * intended for diagnostics and default formatted output, not for stable
 * serialization.
 *
 * @tparam T Floating-point element type.
 * @tparam Rows Number of rows.
 * @tparam Cols Number of columns.
 * @param stream Output stream.
 * @param value Matrix value.
 * @return Output stream.
 */
template<std::floating_point T, std::size_t Rows, std::size_t Cols>
inline auto operator<<(
    std::ostream& stream,
    const matrix<T, Rows, Cols>& value) -> std::ostream&
{
    stream << '[';

    for (std::size_t row = 0; row < Rows; ++row) {
        if (row != 0) {
            stream << ", ";
        }

        stream << '[';
        for (std::size_t col = 0; col < Cols; ++col) {
            if (col != 0) {
                stream << ", ";
            }

            stream << value(row, col);
        }
        stream << ']';
    }

    stream << ']';
    return stream;
}

/**
 * @brief Writes an RGB color.
 */
template<std::floating_point T>
inline auto operator<<(
    std::ostream& stream,
    basic_rgb<T> value) -> std::ostream&
{
    return stream << "rgb(" << value.r.value() << ", " << value.g.value()
                  << ", " << value.b.value() << ')';
}

/**
 * @brief Writes a grayscale color.
 */
template<std::floating_point T>
inline auto operator<<(
    std::ostream& stream,
    basic_gray<T> value) -> std::ostream&
{
    return stream << "gray(" << value.y.value() << ')';
}

/**
 * @brief Writes a CMY color.
 */
template<std::floating_point T>
inline auto operator<<(
    std::ostream& stream,
    basic_cmy<T> value) -> std::ostream&
{
    return stream << "cmy(" << value.c.value() << ", " << value.m.value()
                  << ", " << value.y.value() << ')';
}

/**
 * @brief Writes an HSV color.
 */
template<std::floating_point T>
inline auto operator<<(
    std::ostream& stream,
    basic_hsv<T> value) -> std::ostream&
{
    return stream << "hsv(" << value.h.value() << ", " << value.s.value()
                  << ", " << value.v.value() << ')';
}

/**
 * @brief Writes an XYZ color.
 */
template<std::floating_point T>
inline auto operator<<(
    std::ostream& stream,
    basic_xyz<T> value) -> std::ostream&
{
    return stream << "xyz(" << value.x << ", " << value.y << ", " << value.z
                  << ')';
}

/**
 * @brief Writes a Lab color.
 */
template<std::floating_point T>
inline auto operator<<(
    std::ostream& stream,
    basic_lab<T> value) -> std::ostream&
{
    return stream << "lab(" << value.l << ", " << value.a << ", " << value.b
                  << ')';
}

/**
 * @brief Writes a Luv color.
 */
template<std::floating_point T>
inline auto operator<<(
    std::ostream& stream,
    basic_luv<T> value) -> std::ostream&
{
    return stream << "luv(" << value.l << ", " << value.u << ", " << value.v
                  << ')';
}

} // namespace xer

#endif /* XER_BITS_IOSTREAM_H_INCLUDED_ */
