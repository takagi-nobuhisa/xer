/**
 * @file xer/bits/print.h
 * @brief Simple diagnostic print support.
 */

#pragma once

#ifndef XER_BITS_PRINT_H_INCLUDED_
#define XER_BITS_PRINT_H_INCLUDED_

#include <cstddef>
#include <expected>
#include <string_view>
#include <type_traits>
#include <utility>

#include <xer/bits/printf.h>
#include <xer/bits/standard_streams.h>
#include <xer/bits/text_stream.h>
#include <xer/error.h>

namespace xer::detail {

inline text_stream* print_output_stream = nullptr;

template<class T>
struct is_xer_result : std::false_type {};

template<class T, class Detail>
struct is_xer_result<std::expected<T, error<Detail>>> : std::true_type {};

template<class T>
inline constexpr bool is_xer_result_v =
    is_xer_result<std::remove_cvref_t<T>>::value;

template<class T>
struct xer_result_value_type;

template<class T, class Detail>
struct xer_result_value_type<std::expected<T, error<Detail>>> {
    using type = T;
};

template<class T>
using xer_result_value_type_t =
    typename xer_result_value_type<std::remove_cvref_t<T>>::type;

template<class T>
concept xer_print_result = is_xer_result_v<T>;

} // namespace xer::detail

namespace xer {

/**
 * @brief Sets the destination stream for simple diagnostic print output.
 *
 * The stream is borrowed and must outlive subsequent `xer_print` use.
 *
 * @param stream New print output stream.
 */
inline auto set_print_stream(text_stream& stream) noexcept -> void
{
    detail::print_output_stream = &stream;
}

/**
 * @brief Resets simple diagnostic print output to the default standard output.
 */
inline auto reset_print_stream() noexcept -> void
{
    detail::print_output_stream = nullptr;
}

/**
 * @brief Returns the current simple diagnostic print output stream.
 * @return Print output stream, or standard output when no override is set.
 */
inline auto get_print_stream() noexcept -> text_stream&
{
    if (detail::print_output_stream != nullptr) {
        return *detail::print_output_stream;
    }

    return standard_output;
}

/**
 * @brief Prints a labelled value for examples and quick diagnostics.
 *
 * Non-result values are formatted through xer's `%@` printf conversion.
 * `result<T>` values print the contained value on success and
 * `error(name)` on failure. `result<void>` prints `ok` on success.
 *
 * @tparam T Value type.
 * @param label Display label.
 * @param separator Separator written between the label and the value.
 * @param value Value to print.
 * @return Written byte count on success.
 */
template<class T>
inline auto print_expression(
    std::u8string_view label,
    std::u8string_view separator,
    T&& value) -> result<std::size_t>
{
    if constexpr (detail::xer_print_result<T>) {
        using value_type = detail::xer_result_value_type_t<T>;

        if (!value.has_value()) {
            return fprintf(
                get_print_stream(),
                u8"%@%@error(%@)\n",
                label,
                separator,
                value.error().code);
        }

        if constexpr (std::same_as<value_type, void>) {
            return fprintf(get_print_stream(), u8"%@%@ok\n", label, separator);
        } else {
            return fprintf(get_print_stream(), u8"%@%@%@\n", label, separator, *value);
        }
    } else {
        return fprintf(
            get_print_stream(),
            u8"%@%@%@\n",
            label,
            separator,
            std::forward<T>(value));
    }
}

} // namespace xer

#define XER_DETAIL_PRINT_1(expr) \
    ::xer::print_expression(u8"" #expr, u8" = ", (expr))

#define XER_DETAIL_PRINT_2(label, expr) \
    ::xer::print_expression((label), u8": ", (expr))

#define XER_DETAIL_PRINT_SELECT(_1, _2, NAME, ...) NAME

#define xer_print(...) \
    XER_DETAIL_PRINT_SELECT(__VA_ARGS__, XER_DETAIL_PRINT_2, XER_DETAIL_PRINT_1)(__VA_ARGS__)

#endif /* XER_BITS_PRINT_H_INCLUDED_ */
