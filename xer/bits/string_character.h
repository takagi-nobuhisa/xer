/**
 * @file xer/bits/string_character.h
 * @brief Internal string character type constraints.
 */

#pragma once

#ifndef XER_BITS_STRING_CHARACTER_H_INCLUDED_
#define XER_BITS_STRING_CHARACTER_H_INCLUDED_

#include <concepts>

#include <xer/bits/common.h>

namespace xer::detail {

/**
 * @brief Checks whether the specified character type is supported by the
 *        XER string functions.
 *
 * @tparam CharT Character type.
 */
template<typename CharT>
concept supported_string_character =
    std::same_as<CharT, char> || std::same_as<CharT, unsigned char> ||
    std::same_as<CharT, char8_t> || std::same_as<CharT, char16_t> ||
    std::same_as<CharT, char32_t>;

} // namespace xer::detail

#endif /* XER_BITS_STRING_CHARACTER_H_INCLUDED_ */
