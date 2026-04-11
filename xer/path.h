/**
 * @file xer/path.h
 * @brief Public path class and native path conversion functions.
 */

#pragma once

#ifndef XER_PATH_H_INCLUDED_
#define XER_PATH_H_INCLUDED_

#include <expected>
#include <string>
#include <string_view>

#include <xer/bits/common.h>
#include <xer/bits/text_encoding_common.h>
#include <xer/error.h>

namespace xer {

#if defined(_WIN32)
/**
 * @brief Native path character type for the current platform.
 */
using native_path_char_t = wchar_t;

/**
 * @brief Native path string type for the current platform.
 */
using native_path_string = std::wstring;

/**
 * @brief Native path string view type for the current platform.
 */
using native_path_view = std::wstring_view;
#else
/**
 * @brief Native path character type for the current platform.
 */
using native_path_char_t = char;

/**
 * @brief Native path string type for the current platform.
 */
using native_path_string = std::string;

/**
 * @brief Native path string view type for the current platform.
 */
using native_path_view = std::string_view;
#endif

/**
 * @brief UTF-8 based lexical path.
 *
 * The internal representation is UTF-8 and uses '/' as the separator.
 * On construction, '\' is normalized to '/'.
 * This type performs lexical storage only and does not resolve relative paths,
 * symbolic links, or the actual file system.
 */
class path {
public:
    /**
     * @brief Internal string type.
     */
    using string_type = std::u8string;

    /**
     * @brief Internal string view type.
     */
    using view_type = std::u8string_view;

    /**
     * @brief Constructs an empty path.
     */
    path() = default;

    /**
     * @brief Constructs a path from a UTF-8 string view.
     *
     * @param value Source UTF-8 path string.
     */
    explicit path(std::u8string_view value)
        : value_(value)
    {
        normalize_separators(value_);
    }

    /**
     * @brief Constructs a path from a UTF-8 null-terminated string.
     *
     * A null pointer is treated as an empty path.
     *
     * @param value Source UTF-8 path string.
     */
    explicit path(const char8_t* value)
        : value_(value == nullptr ? std::u8string_view() : std::u8string_view(value))
    {
        normalize_separators(value_);
    }

    /**
     * @brief Returns the normalized internal UTF-8 representation.
     *
     * @return Normalized UTF-8 path string.
     */
    [[nodiscard]] view_type str() const noexcept
    {
        return value_;
    }

private:
    /**
     * @brief Normalizes path separators in place.
     *
     * @param value Target UTF-8 string.
     */
    static void normalize_separators(string_type& value) noexcept
    {
        for (auto& ch : value) {
            if (ch == u8'\\') {
                ch = u8'/';
            }
        }
    }

    string_type value_;
};

namespace detail {

#ifdef _WIN32

/**
 * @brief Replaces path separators in a wide string for XER internal storage.
 *
 * @param value Target wide string.
 */
inline void normalize_native_to_internal(std::wstring& value) noexcept
{
    for (auto& ch : value) {
        if (ch == L'\\') {
            ch = L'/';
        }
    }
}

/**
 * @brief Replaces path separators in a wide string for native Windows APIs.
 *
 * @param value Target wide string.
 */
inline void normalize_internal_to_native(std::wstring& value) noexcept
{
    for (auto& ch : value) {
        if (ch == L'/') {
            ch = L'\\';
        }
    }
}

#else

/**
 * @brief Replaces path separators in a byte string for XER internal storage.
 *
 * @param value Target byte string.
 */
inline void normalize_native_to_internal(std::string& value) noexcept
{
    for (auto& ch : value) {
        if (ch == '\\') {
            ch = '/';
        }
    }
}

#endif

} // namespace detail

/**
 * @brief Converts a XER path to the native path string type.
 *
 * This function performs only encoding and separator conversion.
 * It does not resolve relative paths, symbolic links, or the actual file system.
 *
 * @param value Source path.
 * @return Native path string on success.
 */
[[nodiscard]] inline std::expected<native_path_string, error<void>> to_native_path(
    const path& value)
{
    const std::u8string_view source = value.str();

#ifdef _WIN32
    const auto converted = detail::utf8_to_wstring(detail::to_byte_string(source));
    if (!converted.has_value()) {
        return std::unexpected(converted.error());
    }

    native_path_string result = *converted;
    detail::normalize_internal_to_native(result);
    return result;
#else
    const std::string result = detail::to_byte_string(source);

    if (!detail::is_valid_utf8(result)) {
        return std::unexpected(make_error(error_t::ilseq));
    }

    return result;
#endif
}

/**
 * @brief Converts a native path string view to a XER path.
 *
 * This function performs only encoding and separator conversion.
 * It does not resolve relative paths, symbolic links, or the actual file system.
 *
 * @param value Source native path string.
 * @return Converted XER path on success.
 */
[[nodiscard]] inline std::expected<path, error<void>> from_native_path(
    native_path_view value)
{
#ifdef _WIN32
    std::wstring normalized(value);
    detail::normalize_native_to_internal(normalized);

    const auto converted = detail::wstring_to_utf8(normalized);
    if (!converted.has_value()) {
        return std::unexpected(converted.error());
    }

    return path(*converted);
#else
    if (!detail::is_valid_utf8(value)) {
        return std::unexpected(make_error(error_t::ilseq));
    }

    std::string normalized(value);
    detail::normalize_native_to_internal(normalized);

    return path(detail::to_u8string(normalized));
#endif
}

/**
 * @brief Converts a native null-terminated path string to a XER path.
 *
 * A null pointer is treated as invalid input.
 *
 * @param value Source native path string.
 * @return Converted XER path on success.
 */
[[nodiscard]] inline std::expected<path, error<void>> from_native_path(
    const native_path_char_t* value)
{
    if (value == nullptr) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return from_native_path(native_path_view(value));
}

} // namespace xer

#endif /* XER_PATH_H_INCLUDED_ */
