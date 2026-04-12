/**
 * @file xer/bits/getenv.h
 * @brief Internal getenv implementation.
 */

#pragma once

#ifndef XER_BITS_GETENV_H_INCLUDED_
#define XER_BITS_GETENV_H_INCLUDED_

#include <cstdlib>
#include <expected>
#include <string>
#include <string_view>

#include <xer/bits/common.h>
#include <xer/bits/text_encoding_common.h>
#include <xer/error.h>

#ifdef _WIN32
#    include <windows.h>
#endif

namespace xer {

/**
 * @brief Gets the value of an environment variable.
 *
 * The name is interpreted as a UTF-8 string.
 * On Windows, the environment variable is obtained as a wide string and then
 * converted to UTF-8.
 * On Linux, the obtained byte string is required to be valid UTF-8.
 *
 * @param name Environment variable name.
 * @return Environment variable value in UTF-8 on success.
 * @return error<void> on failure.
 */
[[nodiscard]] inline auto getenv(
    std::u8string_view name) -> std::expected<std::u8string, error<void>> {
    if (name.empty()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

#ifdef _WIN32
    const auto narrow_name = detail::to_byte_string(name);
    const auto wide_name = detail::utf8_to_wstring(narrow_name);

    if (!wide_name.has_value()) {
        return std::unexpected(wide_name.error());
    }

    const DWORD required = GetEnvironmentVariableW(
        wide_name->c_str(),
        nullptr,
        0);

    if (required == 0) {
        return std::unexpected(make_error(error_t::not_found));
    }

    std::wstring wide_value(static_cast<std::size_t>(required), L'\0');

    const DWORD copied = GetEnvironmentVariableW(
        wide_name->c_str(),
        wide_value.data(),
        required);

    if (copied == 0 || copied >= required) {
        return std::unexpected(make_error(error_t::not_found));
    }

    wide_value.resize(static_cast<std::size_t>(copied));

    const auto converted = detail::wstring_to_utf8(wide_value);
    if (!converted.has_value()) {
        return std::unexpected(converted.error());
    }

    return *converted;
#else
    const auto narrow_name = detail::to_byte_string(name);
    const char* value = ::getenv(narrow_name.c_str());

    if (value == nullptr) {
        return std::unexpected(make_error(error_t::not_found));
    }

    const std::string_view value_view(value);

    if (!detail::is_valid_utf8(value_view)) {
        return std::unexpected(make_error(error_t::not_found));
    }

    return detail::to_u8string(value_view);
#endif
}

} // namespace xer

#endif /* XER_BITS_GETENV_H_INCLUDED_ */
