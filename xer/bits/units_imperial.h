/**
 * @file xer/bits/units_imperial.h
 * @brief Internal yard-pound unit definitions for quantity support.
 */

#pragma once

#ifndef XER_BITS_UNITS_IMPERIAL_H_INCLUDED_
#define XER_BITS_UNITS_IMPERIAL_H_INCLUDED_

#include <ratio>

#include <xer/bits/units_si.h>

namespace xer {

namespace units {

/**
 * @brief International inch, exactly 0.0254 m.
 */
inline constexpr unit<length_dim, std::ratio<254, 10000>> inch{};

/**
 * @brief International foot, exactly 0.3048 m.
 */
inline constexpr unit<length_dim, std::ratio<3048, 10000>> ft{};

/**
 * @brief International yard, exactly 0.9144 m.
 */
inline constexpr unit<length_dim, std::ratio<9144, 10000>> yd{};

/**
 * @brief International mile, exactly 1609.344 m.
 */
inline constexpr unit<length_dim, std::ratio<1609344, 1000>> mile{};

/**
 * @brief Avoirdupois ounce, exactly 1/16 lb.
 */
inline constexpr unit<mass_dim, std::ratio<28349523125, 1000000000000>> oz{};

/**
 * @brief International avoirdupois pound, exactly 0.45359237 kg.
 */
inline constexpr unit<mass_dim, std::ratio<45359237, 100000000>> lb{};

} // namespace units

} // namespace xer

#endif /* XER_BITS_UNITS_IMPERIAL_H_INCLUDED_ */
