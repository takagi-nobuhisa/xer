/**
 * @file xer/bits/trim.h
 * @brief Internal trim function implementations.
 */

#pragma once

#ifndef XER_BITS_TRIM_H_INCLUDED_
#define XER_BITS_TRIM_H_INCLUDED_

#include <array>
#include <cstddef>
#include <string>
#include <string_view>

#include <xer/bits/common.h>
#include <xer/error.h>

namespace xer::detail {

/**
 * @brief Creates a default trim mask compatible with PHP trim().
 *
 * The default character set is:
 * - space
 * - horizontal tab
 * - line feed
 * - carriage return
 * - vertical tab
 * - NUL
 *
 * @return Trim mask.
 */
[[nodiscard]] constexpr auto make_default_trim_mask()
    -> std::array<bool, 256>
{
    std::array<bool, 256> mask{};

    mask[static_cast<unsigned char>(' ')] = true;
    mask[static_cast<unsigned char>('\t')] = true;
    mask[static_cast<unsigned char>('\n')] = true;
    mask[static_cast<unsigned char>('\r')] = true;
    mask[static_cast<unsigned char>('\v')] = true;
    mask[static_cast<unsigned char>('\0')] = true;

    return mask;
}

/**
 * @brief Builds a trim mask from a PHP-style character list.
 *
 * This function supports plain byte listing and range notation using `..`,
 * such as `a..z` or `0..9`.
 *
 * @param characters Character list.
 * @return Trim mask.
 */
[[nodiscard]] inline auto make_trim_mask(
    const std::u8string_view characters) -> std::array<bool, 256>
{
    if (characters.empty()) {
        return make_default_trim_mask();
    }

    std::array<bool, 256> mask{};

    std::size_t index = 0;
    while (index < characters.size()) {
        if ((index + 3) < characters.size() &&
            characters[index + 1] == u8'.' &&
            characters[index + 2] == u8'.') {
            const unsigned int first =
                static_cast<unsigned char>(characters[index]);
            const unsigned int last =
                static_cast<unsigned char>(characters[index + 3]);

            if (first <= last) {
                for (unsigned int value = first; value <= last; ++value) {
                    mask[value] = true;
                }
            } else {
                for (unsigned int value = last; value <= first; ++value) {
                    mask[value] = true;
                }
            }

            index += 4;
            continue;
        }

        mask[static_cast<unsigned char>(characters[index])] = true;
        ++index;
    }

    return mask;
}

/**
 * @brief Checks whether a byte is included in the trim mask.
 *
 * @param mask Trim mask.
 * @param ch Byte to test.
 * @return true if the byte is trimmable.
 */
[[nodiscard]] constexpr auto is_trim_char(
    const std::array<bool, 256>& mask,
    const char8_t ch) noexcept -> bool
{
    return mask[static_cast<unsigned char>(ch)];
}

/**
 * @brief Returns the left-trimmed view.
 *
 * @param value Source string.
 * @param characters Character list. Empty means PHP default trim set.
 * @return Trimmed view.
 */
[[nodiscard]] inline auto ltrim_view_impl(
    const std::u8string_view value,
    const std::u8string_view characters) -> result<std::u8string_view>
{
    const auto mask = make_trim_mask(characters);

    std::size_t first = 0;
    while (first < value.size() && is_trim_char(mask, value[first])) {
        ++first;
    }

    return value.substr(first);
}

/**
 * @brief Returns the right-trimmed view.
 *
 * @param value Source string.
 * @param characters Character list. Empty means PHP default trim set.
 * @return Trimmed view.
 */
[[nodiscard]] inline auto rtrim_view_impl(
    const std::u8string_view value,
    const std::u8string_view characters) -> result<std::u8string_view>
{
    const auto mask = make_trim_mask(characters);

    std::size_t last = value.size();
    while (last > 0 && is_trim_char(mask, value[last - 1])) {
        --last;
    }

    return value.substr(0, last);
}

/**
 * @brief Returns the both-side trimmed view.
 *
 * @param value Source string.
 * @param characters Character list. Empty means PHP default trim set.
 * @return Trimmed view.
 */
[[nodiscard]] inline auto trim_view_impl(
    const std::u8string_view value,
    const std::u8string_view characters) -> result<std::u8string_view>
{
    const auto mask = make_trim_mask(characters);

    std::size_t first = 0;
    std::size_t last = value.size();

    while (first < last && is_trim_char(mask, value[first])) {
        ++first;
    }

    while (last > first && is_trim_char(mask, value[last - 1])) {
        --last;
    }

    return value.substr(first, last - first);
}

/**
 * @brief Returns the left-trimmed owning string.
 *
 * @param value Source string.
 * @param characters Character list. Empty means PHP default trim set.
 * @return Trimmed string.
 */
[[nodiscard]] inline auto ltrim_impl(
    const std::u8string_view value,
    const std::u8string_view characters) -> result<std::u8string>
{
    const auto view = ltrim_view_impl(value, characters);
    if (!view.has_value()) {
        return std::unexpected(view.error());
    }

    return std::u8string(*view);
}

/**
 * @brief Returns the right-trimmed owning string.
 *
 * @param value Source string.
 * @param characters Character list. Empty means PHP default trim set.
 * @return Trimmed string.
 */
[[nodiscard]] inline auto rtrim_impl(
    const std::u8string_view value,
    const std::u8string_view characters) -> result<std::u8string>
{
    const auto view = rtrim_view_impl(value, characters);
    if (!view.has_value()) {
        return std::unexpected(view.error());
    }

    return std::u8string(*view);
}

/**
 * @brief Returns the both-side trimmed owning string.
 *
 * @param value Source string.
 * @param characters Character list. Empty means PHP default trim set.
 * @return Trimmed string.
 */
[[nodiscard]] inline auto trim_impl(
    const std::u8string_view value,
    const std::u8string_view characters) -> result<std::u8string>
{
    const auto view = trim_view_impl(value, characters);
    if (!view.has_value()) {
        return std::unexpected(view.error());
    }

    return std::u8string(*view);
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Removes trim characters from the beginning of a UTF-8 string.
 *
 * When @p characters is empty, this function uses the PHP-compatible default
 * trim set: space, tab, LF, CR, vertical tab, and NUL.
 *
 * @param value Source string.
 * @param characters Character list or ranges using `..`.
 * @return Trimmed owning string.
 */
[[nodiscard]] inline auto ltrim(
    const std::u8string_view value,
    const std::u8string_view characters = {}) -> result<std::u8string>
{
    return detail::ltrim_impl(value, characters);
}

/**
 * @brief Removes trim characters from the end of a UTF-8 string.
 *
 * When @p characters is empty, this function uses the PHP-compatible default
 * trim set: space, tab, LF, CR, vertical tab, and NUL.
 *
 * @param value Source string.
 * @param characters Character list or ranges using `..`.
 * @return Trimmed owning string.
 */
[[nodiscard]] inline auto rtrim(
    const std::u8string_view value,
    const std::u8string_view characters = {}) -> result<std::u8string>
{
    return detail::rtrim_impl(value, characters);
}

/**
 * @brief Removes trim characters from both ends of a UTF-8 string.
 *
 * When @p characters is empty, this function uses the PHP-compatible default
 * trim set: space, tab, LF, CR, vertical tab, and NUL.
 *
 * @param value Source string.
 * @param characters Character list or ranges using `..`.
 * @return Trimmed owning string.
 */
[[nodiscard]] inline auto trim(
    const std::u8string_view value,
    const std::u8string_view characters = {}) -> result<std::u8string>
{
    return detail::trim_impl(value, characters);
}

/**
 * @brief Removes trim characters from the beginning and returns a view.
 *
 * The returned view refers to the storage of @p value.
 *
 * @param value Source string.
 * @param characters Character list or ranges using `..`.
 * @return Trimmed string view.
 */
[[nodiscard]] inline auto ltrim_view(
    const std::u8string_view value,
    const std::u8string_view characters = {}) -> result<std::u8string_view>
{
    return detail::ltrim_view_impl(value, characters);
}

/**
 * @brief Removes trim characters from the end and returns a view.
 *
 * The returned view refers to the storage of @p value.
 *
 * @param value Source string.
 * @param characters Character list or ranges using `..`.
 * @return Trimmed string view.
 */
[[nodiscard]] inline auto rtrim_view(
    const std::u8string_view value,
    const std::u8string_view characters = {}) -> result<std::u8string_view>
{
    return detail::rtrim_view_impl(value, characters);
}

/**
 * @brief Removes trim characters from both ends and returns a view.
 *
 * The returned view refers to the storage of @p value.
 *
 * @param value Source string.
 * @param characters Character list or ranges using `..`.
 * @return Trimmed string view.
 */
[[nodiscard]] inline auto trim_view(
    const std::u8string_view value,
    const std::u8string_view characters = {}) -> result<std::u8string_view>
{
    return detail::trim_view_impl(value, characters);
}

} // namespace xer

#endif /* XER_BITS_TRIM_H_INCLUDED_ */
