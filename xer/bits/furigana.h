/**
 * @file xer/bits/furigana.h
 * @brief Furigana formatting facilities.
 */

#pragma once

#ifndef XER_BITS_FURIGANA_H_INCLUDED_
#define XER_BITS_FURIGANA_H_INCLUDED_

#include <cstdint>
#include <string>
#include <string_view>

#include <xer/bits/common.h>
#include <xer/bits/escape_html.h>

namespace xer::ja {

/**
 * @brief Output style for @ref to_furigana.
 */
enum class furigana_style : std::uint8_t {
    /** HTML ruby markup, such as `<ruby>学校<rt>がっこう</rt></ruby>`. */
    html,
    /** Parenthesized text, such as `学校(がっこう)`. */
    paren,
};

/** Style selector for HTML ruby markup. */
inline constexpr furigana_style ruby_html = furigana_style::html;
/** Style selector for parenthesized furigana text. */
inline constexpr furigana_style ruby_paren = furigana_style::paren;

namespace detail {

inline auto append_html_escaped(
    std::u8string& out,
    const std::u8string_view text) -> void
{
    for (const char8_t ch : text) {
        if (const auto escaped =
                ::xer::detail::escape_html(static_cast<char32_t>(ch));
            escaped != nullptr) {
            out.append(escaped);
        } else {
            out.push_back(ch);
        }
    }
}

[[nodiscard]] inline auto estimate_html_escaped_size(
    const std::u8string_view text) -> std::size_t
{
    std::size_t size = 0;

    for (const char8_t ch : text) {
        if (const auto escaped =
                ::xer::detail::escape_html(static_cast<char32_t>(ch));
            escaped != nullptr) {
            size += std::u8string_view(escaped).size();
        } else {
            ++size;
        }
    }

    return size;
}

[[nodiscard]] inline auto make_html_furigana(
    const std::u8string_view text,
    const std::u8string_view reading) -> std::u8string
{
    constexpr std::u8string_view prefix = u8"<ruby>";
    constexpr std::u8string_view middle = u8"<rt>";
    constexpr std::u8string_view suffix = u8"</rt></ruby>";

    std::u8string out;
    out.reserve(
        prefix.size()
        + estimate_html_escaped_size(text)
        + middle.size()
        + estimate_html_escaped_size(reading)
        + suffix.size());

    out.append(prefix);
    append_html_escaped(out, text);
    out.append(middle);
    append_html_escaped(out, reading);
    out.append(suffix);

    return out;
}

[[nodiscard]] inline auto make_parenthesized_furigana(
    const std::u8string_view text,
    const std::u8string_view reading) -> std::u8string
{
    std::u8string out;
    out.reserve(text.size() + reading.size() + 2);

    out.append(text);
    out.push_back(u8'(');
    out.append(reading);
    out.push_back(u8')');

    return out;
}

} // namespace detail

/**
 * @brief Formats a word and its reading as a furigana-bearing string.
 *
 * @param text Base text.
 * @param reading Furigana reading.
 * @param style Output style.
 * @return Formatted furigana string.
 *
 * @note `ruby_html` HTML-escapes both `text` and `reading`.
 * @note `ruby_paren` concatenates the arguments as `text(reading)`.
 */
[[nodiscard]] inline auto to_furigana(
    const std::u8string_view text,
    const std::u8string_view reading,
    const furigana_style style) -> std::u8string
{
    switch (style) {
    case furigana_style::html:
        return detail::make_html_furigana(text, reading);
    case furigana_style::paren:
        return detail::make_parenthesized_furigana(text, reading);
    }

    return {};
}

} // namespace xer::ja

#endif /* XER_BITS_FURIGANA_H_INCLUDED_ */
