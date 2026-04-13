#ifndef XER_VERSION_H
#define XER_VERSION_H

#include <string_view>

/**
 * @file
 * @brief Version information for XER.
 */

/**
 * @defgroup xer_version xer/version.h
 * @brief Macros and constants that describe the XER library version.
 * @{
 */

/**
 * @brief Major version number of XER.
 */
#define XER_VERSION_MAJOR 0

/**
 * @brief Minor version number of XER.
 */
#define XER_VERSION_MINOR 1

/**
 * @brief Patch version number of XER.
 */
#define XER_VERSION_PATCH 0

/**
 * @brief Version suffix string of XER.
 *
 * This macro holds the non-numeric suffix part of the version string.
 */
#define XER_VERSION_SUFFIX "a1"

/**
 * @brief Full version string of XER.
 *
 * This macro holds the complete version string.
 */
#define XER_VERSION_STRING "0.1.0a1"

namespace xer {

/**
 * @brief Major version number of XER.
 */
inline constexpr int version_major = XER_VERSION_MAJOR;

/**
 * @brief Minor version number of XER.
 */
inline constexpr int version_minor = XER_VERSION_MINOR;

/**
 * @brief Patch version number of XER.
 */
inline constexpr int version_patch = XER_VERSION_PATCH;

/**
 * @brief Version suffix string of XER.
 *
 * This constant holds the non-numeric suffix part of the version string.
 */
inline constexpr std::string_view version_suffix = XER_VERSION_SUFFIX;

/**
 * @brief Full version string of XER.
 *
 * This constant holds the complete version string.
 */
inline constexpr std::string_view version_string = XER_VERSION_STRING;

} // namespace xer

/** @} */

#endif
