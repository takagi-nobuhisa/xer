/**
 * @file xer/bits/braille_symbols.h
 * @brief Common braille sign constants.
 */

#pragma once

#ifndef XER_BITS_BRAILLE_SYMBOLS_H_INCLUDED_
#define XER_BITS_BRAILLE_SYMBOLS_H_INCLUDED_

#include <string_view>

#include <xer/bits/common.h>

namespace xer::braille {

/**
 * @brief Japanese braille numeric indicator.
 *
 * This is the Unicode braille pattern for dots 3-4-5-6.
 */
inline constexpr std::u8string_view numeric_indicator = u8"⠼";

/**
 * @brief Japanese braille alphabetic indicator.
 *
 * This is the Unicode braille pattern for dots 5-6.
 */
inline constexpr std::u8string_view alphabetic_indicator = u8"⠰";

/**
 * @brief Japanese braille capital indicator.
 *
 * This is the Unicode braille pattern for dot 6.
 */
inline constexpr std::u8string_view capital_indicator = u8"⠠";

/**
 * @brief Japanese braille double capital indicator.
 */
inline constexpr std::u8string_view double_capital_indicator = u8"⠠⠠";

namespace information_processing {

/**
 * @brief Information-processing braille lowercase indicator.
 *
 * This is the Unicode braille pattern for dots 5-6.
 */
inline constexpr std::u8string_view lowercase_indicator = u8"⠰";

/**
 * @brief Information-processing braille uppercase indicator.
 *
 * This is the Unicode braille pattern for dot 6.
 */
inline constexpr std::u8string_view uppercase_indicator = u8"⠠";

/**
 * @brief Information-processing braille single uppercase indicator.
 *
 * In standard notation, this has the same cell as the uppercase indicator.
 */
inline constexpr std::u8string_view single_uppercase_indicator = u8"⠠";

/**
 * @brief Information-processing braille double uppercase indicator.
 */
inline constexpr std::u8string_view double_uppercase_indicator = u8"⠠⠠";

/**
 * @brief Information-processing braille numeric indicator.
 *
 * This is the Unicode braille pattern for dots 3-4-5-6.
 */
inline constexpr std::u8string_view numeric_indicator = u8"⠼";

} // namespace information_processing

} // namespace xer::braille

#endif /* XER_BITS_BRAILLE_SYMBOLS_H_INCLUDED_ */
