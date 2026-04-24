/**
 * @file xer/bits/string_prefix_suffix.h
 * @brief Internal string prefix and suffix predicate implementations.
 */

#pragma once

#ifndef XER_BITS_STRING_PREFIX_SUFFIX_H_INCLUDED_
#define XER_BITS_STRING_PREFIX_SUFFIX_H_INCLUDED_

#include <cstddef>
#include <string_view>
#include <type_traits>

#include <xer/bits/common.h>
#include <xer/bits/string_character.h>

namespace xer::detail {

/**
 * @brief Returns the effective length of an array string.
 *
 * If the last element is NUL, it is treated as a terminator and excluded from
 * the effective length. Otherwise, the whole array is used. This keeps string
 * literals natural while still allowing non-NUL-terminated arrays to be used.
 *
 * @tparam CharT Character type.
 * @tparam N Array size.
 * @param source Source array.
 * @return Effective code-unit length.
 */
template<typename CharT, std::size_t N>
    requires supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto array_string_length(const CharT (&source)[N]) noexcept
    -> std::size_t
{
    if constexpr (N == 0) {
        return 0;
    } else {
        using bare_char_t = std::remove_cv_t<CharT>;

        if (source[N - 1] == static_cast<bare_char_t>(0)) {
            return N - 1;
        }

        return N;
    }
}

/**
 * @brief Creates a string view from an array string.
 *
 * A trailing NUL code unit is excluded when present. Any other NUL code units
 * are treated as ordinary code units.
 *
 * @tparam CharT Character type.
 * @tparam N Array size.
 * @param source Source array.
 * @return String view over the effective array contents.
 */
template<typename CharT, std::size_t N>
    requires supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto array_string_view(const CharT (&source)[N]) noexcept
    -> std::basic_string_view<std::remove_cv_t<CharT>>
{
    using bare_char_t = std::remove_cv_t<CharT>;
    return std::basic_string_view<bare_char_t>(source, array_string_length(source));
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Tests whether a string starts with the specified prefix.
 *
 * @tparam CharT Character type.
 * @param source Source string.
 * @param prefix Prefix to test.
 * @return true if source starts with prefix, otherwise false.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] constexpr auto starts_with(
    const std::basic_string_view<CharT> source,
    const std::basic_string_view<CharT> prefix) noexcept -> bool
{
    if (prefix.size() > source.size()) {
        return false;
    }

    for (std::size_t i = 0; i < prefix.size(); ++i) {
        if (source[i] != prefix[i]) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Tests whether a pointer-sized string starts with the specified prefix.
 *
 * The supplied sizes are used directly. NUL code units are treated as ordinary
 * code units.
 *
 * @tparam CharT Character type.
 * @param source Source pointer.
 * @param source_size Source size in code units.
 * @param prefix Prefix pointer.
 * @param prefix_size Prefix size in code units.
 * @return true if source starts with prefix, otherwise false.
 */
template<typename CharT>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto starts_with(
    const CharT* source,
    const std::size_t source_size,
    const CharT* prefix,
    const std::size_t prefix_size) noexcept -> bool
{
    if ((source == nullptr && source_size != 0) ||
        (prefix == nullptr && prefix_size != 0)) {
        return false;
    }

    using bare_char_t = std::remove_cv_t<CharT>;

    return xer::starts_with(
        std::basic_string_view<bare_char_t>(source, source_size),
        std::basic_string_view<bare_char_t>(prefix, prefix_size));
}

/**
 * @brief Tests whether an array string starts with the specified array prefix.
 *
 * A trailing NUL code unit in each array is excluded when present.
 *
 * @tparam CharT Character type.
 * @tparam N1 Source array size.
 * @tparam N2 Prefix array size.
 * @param source Source array.
 * @param prefix Prefix array.
 * @return true if source starts with prefix, otherwise false.
 */
template<typename CharT, std::size_t N1, std::size_t N2>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto starts_with(
    const CharT (&source)[N1],
    const CharT (&prefix)[N2]) noexcept -> bool
{
    return xer::starts_with(
        detail::array_string_view(source),
        detail::array_string_view(prefix));
}

/**
 * @brief Tests whether a string view starts with the specified array prefix.
 *
 * A trailing NUL code unit in the array prefix is excluded when present.
 *
 * @tparam CharT Character type.
 * @tparam N Prefix array size.
 * @param source Source string.
 * @param prefix Prefix array.
 * @return true if source starts with prefix, otherwise false.
 */
template<detail::supported_string_character CharT, std::size_t N>
[[nodiscard]] constexpr auto starts_with(
    const std::basic_string_view<CharT> source,
    const CharT (&prefix)[N]) noexcept -> bool
{
    return xer::starts_with(source, detail::array_string_view(prefix));
}

/**
 * @brief Tests whether an array string starts with the specified string view prefix.
 *
 * A trailing NUL code unit in the source array is excluded when present.
 *
 * @tparam CharT Character type.
 * @tparam N Source array size.
 * @param source Source array.
 * @param prefix Prefix string.
 * @return true if source starts with prefix, otherwise false.
 */
template<detail::supported_string_character CharT, std::size_t N>
[[nodiscard]] constexpr auto starts_with(
    const CharT (&source)[N],
    const std::basic_string_view<CharT> prefix) noexcept -> bool
{
    return xer::starts_with(detail::array_string_view(source), prefix);
}

/**
 * @brief Tests whether a string ends with the specified suffix.
 *
 * @tparam CharT Character type.
 * @param source Source string.
 * @param suffix Suffix to test.
 * @return true if source ends with suffix, otherwise false.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] constexpr auto ends_with(
    const std::basic_string_view<CharT> source,
    const std::basic_string_view<CharT> suffix) noexcept -> bool
{
    if (suffix.size() > source.size()) {
        return false;
    }

    const std::size_t offset = source.size() - suffix.size();
    for (std::size_t i = 0; i < suffix.size(); ++i) {
        if (source[offset + i] != suffix[i]) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Tests whether a pointer-sized string ends with the specified suffix.
 *
 * The supplied sizes are used directly. NUL code units are treated as ordinary
 * code units.
 *
 * @tparam CharT Character type.
 * @param source Source pointer.
 * @param source_size Source size in code units.
 * @param suffix Suffix pointer.
 * @param suffix_size Suffix size in code units.
 * @return true if source ends with suffix, otherwise false.
 */
template<typename CharT>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto ends_with(
    const CharT* source,
    const std::size_t source_size,
    const CharT* suffix,
    const std::size_t suffix_size) noexcept -> bool
{
    if ((source == nullptr && source_size != 0) ||
        (suffix == nullptr && suffix_size != 0)) {
        return false;
    }

    using bare_char_t = std::remove_cv_t<CharT>;

    return xer::ends_with(
        std::basic_string_view<bare_char_t>(source, source_size),
        std::basic_string_view<bare_char_t>(suffix, suffix_size));
}

/**
 * @brief Tests whether an array string ends with the specified array suffix.
 *
 * A trailing NUL code unit in each array is excluded when present.
 *
 * @tparam CharT Character type.
 * @tparam N1 Source array size.
 * @tparam N2 Suffix array size.
 * @param source Source array.
 * @param suffix Suffix array.
 * @return true if source ends with suffix, otherwise false.
 */
template<typename CharT, std::size_t N1, std::size_t N2>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto ends_with(
    const CharT (&source)[N1],
    const CharT (&suffix)[N2]) noexcept -> bool
{
    return xer::ends_with(
        detail::array_string_view(source),
        detail::array_string_view(suffix));
}

/**
 * @brief Tests whether a string view ends with the specified array suffix.
 *
 * A trailing NUL code unit in the array suffix is excluded when present.
 *
 * @tparam CharT Character type.
 * @tparam N Suffix array size.
 * @param source Source string.
 * @param suffix Suffix array.
 * @return true if source ends with suffix, otherwise false.
 */
template<detail::supported_string_character CharT, std::size_t N>
[[nodiscard]] constexpr auto ends_with(
    const std::basic_string_view<CharT> source,
    const CharT (&suffix)[N]) noexcept -> bool
{
    return xer::ends_with(source, detail::array_string_view(suffix));
}

/**
 * @brief Tests whether an array string ends with the specified string view suffix.
 *
 * A trailing NUL code unit in the source array is excluded when present.
 *
 * @tparam CharT Character type.
 * @tparam N Source array size.
 * @param source Source array.
 * @param suffix Suffix string.
 * @return true if source ends with suffix, otherwise false.
 */
template<detail::supported_string_character CharT, std::size_t N>
[[nodiscard]] constexpr auto ends_with(
    const CharT (&source)[N],
    const std::basic_string_view<CharT> suffix) noexcept -> bool
{
    return xer::ends_with(detail::array_string_view(source), suffix);
}

} // namespace xer

#endif /* XER_BITS_STRING_PREFIX_SUFFIX_H_INCLUDED_ */
