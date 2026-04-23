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
#include <type_traits>

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

namespace detail {

/**
 * @brief Returns whether the specified UTF-8 code unit is an ASCII alphabet.
 *
 * @param ch UTF-8 code unit to test.
 * @return true if alphabetic, otherwise false.
 */
[[nodiscard]] constexpr auto is_ascii_alpha(const char8_t ch) noexcept -> bool
{
    return (ch >= u8'a' && ch <= u8'z') || (ch >= u8'A' && ch <= u8'Z');
}

/**
 * @brief Returns whether the path starts with a drive prefix.
 *
 * Examples:
 * - C:
 * - C:/foo
 * - C:foo
 *
 * @param value Path string.
 * @return true if the path starts with a drive prefix, otherwise false.
 */
[[nodiscard]] constexpr auto has_drive_prefix(
    const std::u8string_view value) noexcept -> bool
{
    return value.size() >= 2 && is_ascii_alpha(value[0]) && value[1] == u8':';
}

/**
 * @brief Returns whether the path is a UNC path.
 *
 * @param value Path string.
 * @return true if the path starts with "//", otherwise false.
 */
[[nodiscard]] constexpr auto is_unc_path(
    const std::u8string_view value) noexcept -> bool
{
    return value.size() >= 2 && value[0] == u8'/' && value[1] == u8'/';
}

/**
 * @brief Returns whether the path is an absolute path.
 *
 * @param value Path string.
 * @return true if absolute, otherwise false.
 */
[[nodiscard]] constexpr auto is_absolute_path(
    const std::u8string_view value) noexcept -> bool
{
    if (value.empty()) {
        return false;
    }

    if (is_unc_path(value)) {
        return true;
    }

    if (has_drive_prefix(value)) {
        return value.size() >= 3 && value[2] == u8'/';
    }

    return value[0] == u8'/';
}

/**
 * @brief Returns whether the path is a drive-relative path.
 *
 * Example:
 * - C:foo
 *
 * @param value Path string.
 * @return true if drive-relative, otherwise false.
 */
[[nodiscard]] constexpr auto is_drive_relative_path(
    const std::u8string_view value) noexcept -> bool
{
    return has_drive_prefix(value) &&
           !(value.size() >= 3 && value[2] == u8'/');
}

/**
 * @brief Returns the drive letter code unit.
 *
 * The caller must ensure that the path has a drive prefix.
 *
 * @param value Path string.
 * @return Drive letter code unit.
 */
[[nodiscard]] constexpr auto drive_letter(
    const std::u8string_view value) noexcept -> char8_t
{
    return value[0];
}

/**
 * @brief Removes redundant trailing separators for lexical analysis.
 *
 * This keeps:
 * - "/"
 * - "//"
 * - "C:/"
 *
 * @param value Path string.
 * @return Trimmed path view.
 */
[[nodiscard]] constexpr auto trim_trailing_separators(
    std::u8string_view value) noexcept -> std::u8string_view
{
    while (!value.empty()) {
        if (value.size() == 1 && value[0] == u8'/') {
            break;
        }

        if (value.size() == 2 && value[0] == u8'/' && value[1] == u8'/') {
            break;
        }

        if (has_drive_prefix(value) &&
            value.size() == 3 &&
            value[2] == u8'/') {
            break;
        }

        if (value.back() != u8'/') {
            break;
        }

        value.remove_suffix(1);
    }

    return value;
}

/**
 * @brief Returns the basename view of a normalized UTF-8 path.
 *
 * @param value Normalized path string.
 * @return Basename view.
 */
[[nodiscard]] constexpr auto basename_view(
    const std::u8string_view value) noexcept -> std::u8string_view
{
    const auto trimmed = trim_trailing_separators(value);
    if (trimmed.empty()) {
        return std::u8string_view();
    }

    const auto pos = trimmed.find_last_of(u8'/');
    if (pos == std::u8string_view::npos) {
        return trimmed;
    }

    return trimmed.substr(pos + 1);
}

/**
 * @brief Normalizes path separators in place.
 *
 * @param value Target UTF-8 string.
 */
inline void normalize_separators(std::u8string& value) noexcept
{
    for (auto& ch : value) {
        if (ch == u8'\\') {
            ch = u8'/';
        }
    }
}

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
        detail::normalize_separators(value_);
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
        detail::normalize_separators(value_);
    }

    /**
     * @brief Returns the normalized internal UTF-8 representation.
     *
     * @return Normalized UTF-8 path string.
     */
    [[nodiscard]] auto str() const noexcept -> view_type
    {
        return value_;
    }

    /**
     * @brief Lexically appends another path.
     *
     * @param rhs Right-hand path.
     * @return Success on completion.
     */
    [[nodiscard]] auto operator/=(const path& rhs) -> result<void>
    {
        const auto left = str();
        const auto right = rhs.str();

        if (right.empty()) {
            return {};
        }

        if (detail::is_absolute_path(right)) {
            if (left == u8"/" && !detail::is_unc_path(right)) {
                value_.assign(right.begin(), right.end());
                return {};
            }

            if (detail::has_drive_prefix(left) &&
                left.size() == 2 &&
                !detail::is_unc_path(right) &&
                right[0] == u8'/') {
                value_.assign(left.begin(), left.end());
                value_.append(right.begin(), right.end());
                return {};
            }

            return std::unexpected(make_error(error_t::invalid_argument));
        }

        if (detail::is_drive_relative_path(right)) {
            if (!detail::has_drive_prefix(left)) {
                return std::unexpected(make_error(error_t::invalid_argument));
            }

            if (detail::drive_letter(left) != detail::drive_letter(right)) {
                return std::unexpected(make_error(error_t::invalid_argument));
            }

            const auto suffix = right.substr(2);

            if (!value_.empty() && value_.back() != u8'/') {
                value_.push_back(u8'/');
            }

            value_.append(suffix.begin(), suffix.end());
            return {};
        }

        if (value_.empty()) {
            value_.assign(right.begin(), right.end());
            return {};
        }

        if (value_.back() != u8'/') {
            value_.push_back(u8'/');
        }

        value_.append(right.begin(), right.end());
        return {};
    }

private:
    string_type value_;
};

/**
 * @brief Lexically appends two paths.
 *
 * @param lhs Left-hand path.
 * @param rhs Right-hand path.
 * @return Joined path on success.
 */
[[nodiscard]] inline auto operator/(
    path lhs,
    const path& rhs) -> result<path>
{
    const auto result = (lhs /= rhs);
    if (!result.has_value()) {
        return std::unexpected(result.error());
    }

    return lhs;
}

/**
 * @brief Returns the final path component.
 *
 * @param value Source path.
 * @return Basename view.
 */
[[nodiscard]] inline auto basename(const path& value) noexcept -> std::u8string_view
{
    return detail::basename_view(value.str());
}

/**
 * @brief Returns the extension starting from the first '.' in basename.
 *
 * @param value Source path.
 * @return Extension view, or empty view if absent.
 */
[[nodiscard]] inline auto extension(const path& value) noexcept -> std::u8string_view
{
    const auto name = basename(value);
    const auto pos = name.find(u8'.');
    if (pos == std::u8string_view::npos) {
        return std::u8string_view();
    }

    return name.substr(pos);
}

/**
 * @brief Returns the stem, that is basename without extension().
 *
 * @param value Source path.
 * @return Stem view.
 */
[[nodiscard]] inline auto stem(const path& value) noexcept -> std::u8string_view
{
    const auto name = basename(value);
    const auto ext = extension(value);
    if (ext.empty()) {
        return name;
    }

    return name.substr(0, name.size() - ext.size());
}

/**
 * @brief Returns the lexical parent path.
 *
 * @param value Source path.
 * @return Parent path on success.
 */
[[nodiscard]] inline auto parent_path(const path& value) -> result<path>
{
    const auto s = detail::trim_trailing_separators(value.str());
    if (s.empty()) {
        return std::unexpected(make_error(error_t::not_found));
    }

    if (s == u8"/" || s == u8"//") {
        return std::unexpected(make_error(error_t::not_found));
    }

    if (detail::has_drive_prefix(s) && s.size() == 2) {
        return std::unexpected(make_error(error_t::not_found));
    }

    if (detail::has_drive_prefix(s) && s.size() == 3 && s[2] == u8'/') {
        return std::unexpected(make_error(error_t::not_found));
    }

    const auto pos = s.find_last_of(u8'/');
    if (pos == std::u8string_view::npos) {
        if (detail::has_drive_prefix(s)) {
            if (s.size() == 2) {
                return std::unexpected(make_error(error_t::not_found));
            }

            return path(s.substr(0, 2));
        }

        return std::unexpected(make_error(error_t::not_found));
    }

    if (pos == 0) {
        return path(u8"/");
    }

    if (detail::has_drive_prefix(s) && pos == 2) {
        return path(s.substr(0, 3));
    }

    return path(s.substr(0, pos));
}

/**
 * @brief Returns whether the path is absolute.
 *
 * @param value Source path.
 * @return true if absolute, otherwise false.
 */
[[nodiscard]] inline auto is_absolute(const path& value) noexcept -> bool
{
    return detail::is_absolute_path(value.str());
}

/**
 * @brief Returns whether the path is relative.
 *
 * @param value Source path.
 * @return true if relative, otherwise false.
 */
[[nodiscard]] inline auto is_relative(const path& value) noexcept -> bool
{
    return !is_absolute(value);
}

/**
 * @brief Converts a XER path to the native path string type.
 *
 * This function performs only encoding and separator conversion.
 * It does not resolve relative paths, symbolic links, or the actual file system.
 *
 * @param value Source path.
 * @return Native path string on success.
 */
[[nodiscard]] inline auto to_native_path(
    const path& value) -> std::expected<native_path_string, error<void>>
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
        return std::unexpected(make_error(error_t::encoding_error));
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
[[nodiscard]] inline auto from_native_path(
    native_path_view value) -> std::expected<path, error<void>>
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
        return std::unexpected(make_error(error_t::encoding_error));
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
[[nodiscard]] inline auto from_native_path(
    const native_path_char_t* value) -> std::expected<path, error<void>>
{
    if (value == nullptr) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return from_native_path(native_path_view(value));
}

} // namespace xer

#endif /* XER_PATH_H_INCLUDED_ */
