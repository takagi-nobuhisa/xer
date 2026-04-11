/**
 * @file xer/assert.h
 * @brief Public assertion macros for xer tests.
 */

#pragma once

#ifndef XER_ASSERT_H_INCLUDED_
#define XER_ASSERT_H_INCLUDED_

#include <source_location>

#include <xer/bits/assert.h>

/**
 * @brief Asserts that the expression is true.
 */
#define xer_assert(expr)                                                     \
    do {                                                                     \
        ::xer::detail::assert_true(                                          \
            static_cast<bool>(expr), #expr, std::source_location::current()); \
    } while (false)

/**
 * @brief Asserts that the expression is false.
 */
#define xer_assert_not(expr)                                                  \
    do {                                                                      \
        ::xer::detail::assert_false(                                          \
            static_cast<bool>(expr), #expr, std::source_location::current()); \
    } while (false)

/**
 * @brief Asserts that two values are equal.
 */
#define xer_assert_eq(lhs, rhs)                     \
    do {                                            \
        auto&& xer_assert_lhs_ = (lhs);             \
        auto&& xer_assert_rhs_ = (rhs);             \
        ::xer::detail::assert_eq(                   \
            xer_assert_lhs_,                        \
            xer_assert_rhs_,                        \
            #lhs,                                   \
            #rhs,                                   \
            std::source_location::current());       \
    } while (false)

/**
 * @brief Asserts that two values are not equal.
 */
#define xer_assert_ne(lhs, rhs)                     \
    do {                                            \
        auto&& xer_assert_lhs_ = (lhs);             \
        auto&& xer_assert_rhs_ = (rhs);             \
        ::xer::detail::assert_ne(                   \
            xer_assert_lhs_,                        \
            xer_assert_rhs_,                        \
            #lhs,                                   \
            #rhs,                                   \
            std::source_location::current());       \
    } while (false)

/**
 * @brief Asserts that the left value is less than the right value.
 */
#define xer_assert_lt(lhs, rhs)                     \
    do {                                            \
        auto&& xer_assert_lhs_ = (lhs);             \
        auto&& xer_assert_rhs_ = (rhs);             \
        ::xer::detail::assert_lt(                   \
            xer_assert_lhs_,                        \
            xer_assert_rhs_,                        \
            #lhs,                                   \
            #rhs,                                   \
            std::source_location::current());       \
    } while (false)

/**
 * @brief Asserts that the expression throws the specified exception type.
 */
#define xer_assert_throw(expr, exception_type)                       \
    do {                                                             \
        bool xer_assert_caught_ = false;                             \
        try {                                                        \
            try {                                                    \
                expr;                                                \
            } catch (exception_type) {                               \
                xer_assert_caught_ = true;                           \
            }                                                        \
        } catch (...) {                                              \
            ::xer::detail::throw_assertion_failure(                  \
                "xer_assert_throw",                                 \
                #expr,                                               \
                "unexpected exception type",                        \
                std::source_location::current());                    \
        }                                                            \
        if (!xer_assert_caught_) {                                   \
            ::xer::detail::throw_assertion_failure(                  \
                "xer_assert_throw",                                 \
                #expr,                                               \
                "no exception thrown",                              \
                std::source_location::current());                    \
        }                                                            \
    } while (false)

/**
 * @brief Asserts that the expression does not throw.
 */
#define xer_assert_nothrow(expr)                                     \
    do {                                                             \
        try {                                                        \
            expr;                                                    \
        } catch (const ::std::exception& xer_assert_exception_) {    \
            ::xer::detail::throw_assertion_failure(                  \
                "xer_assert_nothrow",                               \
                #expr,                                               \
                xer_assert_exception_.what(),                        \
                std::source_location::current());                    \
        } catch (...) {                                              \
            ::xer::detail::throw_assertion_failure(                  \
                "xer_assert_nothrow",                               \
                #expr,                                               \
                "unexpected exception thrown",                      \
                std::source_location::current());                    \
        }                                                            \
    } while (false)

#endif /* XER_ASSERT_H_INCLUDED_ */
