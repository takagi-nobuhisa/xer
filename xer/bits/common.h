/**
 * @file xer/bits/common.h
 * @brief Common compiler/environment checks and portability macros.
 */

#pragma once

#ifndef XER_BITS_COMMON_H_INCLUDED_
#define XER_BITS_COMMON_H_INCLUDED_

#include <climits>

#if !defined(__cplusplus)
#    error "xer requires C++."
#endif

/**
 * @brief Common name for the active C++ language version.
 */
#define XER_STDCPP_VERSION __cplusplus

/**
 * @brief Common name for the current function signature string.
 */
#define XER_PRETTY_FUNCTION __PRETTY_FUNCTION__

#if !defined(__GNUC__)
static_assert(false, "xer requires GCC.");
#endif

#if defined(__clang__)
static_assert(false, "xer currently does not support Clang.");
#endif

static_assert(
    (__GNUC__ > 13) ||
    (__GNUC__ == 13 && __GNUC_MINOR__ > 3) ||
    (__GNUC__ == 13 && __GNUC_MINOR__ == 3 && __GNUC_PATCHLEVEL__ >= 0),
    "xer requires GCC 13.3.0 or later.");

static_assert(XER_STDCPP_VERSION >= 202100L, "xer requires C++23 or later.");

static_assert(CHAR_BIT == 8, "xer requires CHAR_BIT == 8.");

static_assert(sizeof(short) == 2, "xer requires 16-bit short.");
static_assert(sizeof(int) == 4, "xer requires 32-bit int.");
static_assert(sizeof(long long) == 8, "xer requires 64-bit long long.");

static_assert(sizeof(float) == 4, "xer requires 32-bit float.");
static_assert(sizeof(double) == 8, "xer requires 64-bit double.");

#endif /* XER_BITS_COMMON_H_INCLUDED_ */
