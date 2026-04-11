#ifndef XER_VERSION_H
#define XER_VERSION_H

#include <string_view>

#define XER_VERSION_MAJOR 0
#define XER_VERSION_MINOR 1
#define XER_VERSION_PATCH 0
#define XER_VERSION_SUFFIX "a1"
#define XER_VERSION_STRING "0.1.0a1"

namespace xer {

inline constexpr int version_major = XER_VERSION_MAJOR;
inline constexpr int version_minor = XER_VERSION_MINOR;
inline constexpr int version_patch = XER_VERSION_PATCH;
inline constexpr std::string_view version_suffix = XER_VERSION_SUFFIX;
inline constexpr std::string_view version_string = XER_VERSION_STRING;

} // namespace xer

#endif
