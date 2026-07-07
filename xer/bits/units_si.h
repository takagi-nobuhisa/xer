/**
 * @file xer/bits/units_si.h
 * @brief Internal SI and common unit definitions for quantity support.
 */

#pragma once

#ifndef XER_BITS_UNITS_SI_H_INCLUDED_
#define XER_BITS_UNITS_SI_H_INCLUDED_

#include <ratio>

#include <xer/bits/math_constants.h>
#include <xer/bits/quantity.h>

namespace xer {

namespace units {

using length_dim = dimension<1, 0, 0, 0>;
using mass_dim = dimension<0, 1, 0, 0>;
using time_dim = dimension<0, 0, 1, 0>;
using current_dim = dimension<0, 0, 0, 1>;

inline constexpr unit<length_dim> m{};
inline constexpr auto m2 = sq(m);
inline constexpr auto m3 = cb(m);

inline constexpr unit<mass_dim> kg{};

inline constexpr unit<time_dim> sec{};
inline constexpr auto sec2 = sq(sec);
inline constexpr auto sec3 = cb(sec);

#if defined(XER_ENABLE_NON_STANDARD_IDENTIFIERS)
#    include <xer/bits/units_si_non_standard_identifiers.h>
#endif

inline constexpr unit<current_dim> A{};

inline constexpr unit<length_dim, std::milli> mm{};
inline constexpr unit<length_dim, std::centi> cm{};
inline constexpr unit<length_dim, std::kilo> km{};
inline constexpr unit<length_dim, std::micro> microm{};
inline constexpr unit<length_dim, std::nano> nm{};
inline constexpr unit<length_dim, std::micro> μm{};

inline constexpr unit<mass_dim, std::ratio<1, 1000>> g{};
inline constexpr unit<mass_dim, std::ratio<1, 1000000>> mg{};

inline constexpr unit<time_dim, std::nano> nsec{};
inline constexpr unit<time_dim, std::micro> microsec{};
inline constexpr unit<time_dim, std::milli> msec{};
inline constexpr unit<time_dim, std::micro> μsec{};

inline constexpr unit<current_dim, std::milli> mA{};

inline constexpr auto Hz = unit<dimension<0, 0, -1, 0>>{};
inline constexpr auto kHz = unit<dimension<0, 0, -1, 0>, std::kilo>{};
inline constexpr auto GHz = unit<dimension<0, 0, -1, 0>, std::giga>{};

inline constexpr auto N = kg * m / sec2;
inline constexpr auto J = N * m;
inline constexpr auto W = J / sec;
inline constexpr auto V = W / A;
inline constexpr auto Pa = N / m2;
inline constexpr auto hPa = unit<dimension<-1, 1, -2, 0>, std::hecto>{};

inline constexpr auto ha = unit<dimension<2, 0, 0, 0>, std::ratio<10000>>{};
inline constexpr auto mL = unit<dimension<3, 0, 0, 0>, std::ratio<1, 1000000>>{};
inline constexpr auto dL = unit<dimension<3, 0, 0, 0>, std::ratio<1, 10000>>{};
inline constexpr auto L = unit<dimension<3, 0, 0, 0>, std::ratio<1, 1000>>{};
inline constexpr auto kL = unit<dimension<3, 0, 0, 0>>{};
inline constexpr auto cc = mL;

inline constexpr auto cal = unit<dimension<2, 1, -2, 0>, std::ratio<523, 125>>{};
inline constexpr auto kcal = unit<dimension<2, 1, -2, 0>, std::ratio<4184>>{};

inline constexpr unit<dimensionless> taurad{};
inline constexpr unit<dimensionless> τrad{};
inline constexpr unit<dimensionless,
                      detail::floating_scale<1.0L / tau_v<long double>>> rad{};

} // namespace units

} // namespace xer

#endif /* XER_BITS_UNITS_SI_H_INCLUDED_ */
