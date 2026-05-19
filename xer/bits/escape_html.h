/**
 * @file xer/bits/escape_html.h
 * @brief Lightweight HTML escape/unescape helpers for internal use.
 */

#pragma once

#ifndef XER_BITS_ESCAPE_HTML_H_INCLUDED_
#define XER_BITS_ESCAPE_HTML_H_INCLUDED_

#include <string_view>

#include <xer/bits/common.h>

namespace xer::detail {

/**
 * @brief Returns the HTML entity for a special character.
 *
 * @param ch Character to escape.
 * @return A UTF-8 entity string, or `nullptr` when no escaping is needed.
 */
[[nodiscard]] constexpr auto escape_html(const char32_t ch) noexcept
    -> const char8_t*
{
    switch (ch) {
    case U'&':
        return u8"&amp;";
    case U'<':
        return u8"&lt;";
    case U'>':
        return u8"&gt;";
    case U'"':
        return u8"&quot;";
    case U'\'':
        return u8"&#39;";
    default:
        return nullptr;
    }
}

/**
 * @brief Returns the character represented by a supported HTML entity.
 *
 * @param text HTML entity text.
 * @return The unescaped character, or `U'\0'` when `text` is unsupported.
 */
[[nodiscard]] constexpr auto unescape_html(
    const std::u8string_view text) noexcept -> char32_t
{
    if (text == u8"&amp;") {
        return U'&';
    }
    if (text == u8"&lt;") {
        return U'<';
    }
    if (text == u8"&gt;") {
        return U'>';
    }
    if (text == u8"&quot;") {
        return U'"';
    }
    if (text == u8"&#39;") {
        return U'\'';
    }

    return U'\0';
}

} // namespace xer::detail

#endif /* XER_BITS_ESCAPE_HTML_H_INCLUDED_ */
