/**
 * @file xer/bits/arithmetic_concepts.h
 * @brief Internal arithmetic-related concepts.
 */

#pragma once

#ifndef XER_BITS_ARITHMETIC_CONCEPTS_H_INCLUDED_
#define XER_BITS_ARITHMETIC_CONCEPTS_H_INCLUDED_

#include <concepts>
#include <type_traits>

#include <xer/bits/common.h>

namespace xer {

/**
 * @brief Concept for arithmetic types.
 *
 * This concept follows `std::is_arithmetic_v`, so `bool` is included.
 *
 * @tparam T Type to test.
 */
template<typename T>
concept arithmetic = std::is_arithmetic_v<std::remove_cvref_t<T>>;

/**
 * @brief Concept for arithmetic types except `bool`.
 *
 * @tparam T Type to test.
 */
template<typename T>
concept non_bool_arithmetic =
    arithmetic<T> &&
    !std::same_as<std::remove_cvref_t<T>, bool>;

} // namespace xer

#endif /* XER_BITS_ARITHMETIC_CONCEPTS_H_INCLUDED_ */
