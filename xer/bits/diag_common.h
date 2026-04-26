/**
 * @file xer/bits/diag_common.h
 * @brief Common definitions for XER diagnostics.
 */

#pragma once

#ifndef XER_BITS_DIAG_COMMON_H_INCLUDED_
#define XER_BITS_DIAG_COMMON_H_INCLUDED_

#include <cstdint>
#include <string_view>

#include <xer/bits/common.h>

namespace xer {

/**
 * @brief Diagnostic output level type.
 *
 * Smaller values represent more important messages. Larger values represent
 * more detailed diagnostic output.
 */
using diag_level_t = int;

inline constexpr diag_level_t diag_error = 10;
inline constexpr diag_level_t diag_warning = 20;
inline constexpr diag_level_t diag_info = 30;
inline constexpr diag_level_t diag_debug = 40;
inline constexpr diag_level_t diag_verbose = 50;

/**
 * @brief Diagnostic category shared by tracing and logging.
 */
enum class diag_category : std::uint32_t {
    general = 0,
    io,
    path,
    process,
    socket,
    user,
};

/**
 * @brief Returns the symbolic name of a diagnostic category.
 *
 * @param category Diagnostic category.
 * @return UTF-8 category name.
 */
[[nodiscard]] constexpr auto get_diag_category_name(diag_category category) noexcept
    -> std::u8string_view
{
    switch (category) {
    case diag_category::general:
        return u8"general";
    case diag_category::io:
        return u8"io";
    case diag_category::path:
        return u8"path";
    case diag_category::process:
        return u8"process";
    case diag_category::socket:
        return u8"socket";
    case diag_category::user:
        return u8"user";
    }

    return u8"unknown";
}

/**
 * @brief Tests whether a requested diagnostic level is enabled.
 *
 * @param current Currently configured maximum level.
 * @param requested Requested message level.
 * @return true if requested should be emitted.
 */
[[nodiscard]] constexpr auto is_diag_level_enabled(
    diag_level_t current,
    diag_level_t requested) noexcept -> bool
{
    return requested <= current;
}

} // namespace xer

#endif /* XER_BITS_DIAG_COMMON_H_INCLUDED_ */
