/**
 * @file xer/bits/character_width.h
 * @brief Internal fullwidth and halfwidth character helpers.
 */

#pragma once

#ifndef XER_BITS_CHARACTER_WIDTH_H_INCLUDED_
#define XER_BITS_CHARACTER_WIDTH_H_INCLUDED_

#include <expected>

#include <xer/bits/common.h>
#include <xer/bits/kana_width.h>
#include <xer/error.h>

namespace xer::detail {

[[nodiscard]] constexpr auto is_halfwidth_digit(char32_t c) noexcept -> bool
{
    return c >= U'0' && c <= U'9';
}

[[nodiscard]] constexpr auto is_fullwidth_digit(char32_t c) noexcept -> bool
{
    return c >= U'\uff10' && c <= U'\uff19';
}

[[nodiscard]] constexpr auto is_halfwidth_alpha(char32_t c) noexcept -> bool
{
    return (c >= U'A' && c <= U'Z') || (c >= U'a' && c <= U'z');
}

[[nodiscard]] constexpr auto is_fullwidth_alpha(char32_t c) noexcept -> bool
{
    return (c >= U'\uff21' && c <= U'\uff3a') ||
           (c >= U'\uff41' && c <= U'\uff5a');
}

[[nodiscard]] constexpr auto is_halfwidth_space(char32_t c) noexcept -> bool
{
    return c == U' ';
}

[[nodiscard]] constexpr auto is_fullwidth_space(char32_t c) noexcept -> bool
{
    return c == U'\u3000';
}

[[nodiscard]] constexpr auto is_halfwidth_ascii_graph(char32_t c) noexcept -> bool
{
    return c >= U'\x21' && c <= U'\x7e';
}

[[nodiscard]] constexpr auto is_fullwidth_ascii_graph(char32_t c) noexcept -> bool
{
    return c >= U'\uff01' && c <= U'\uff5e';
}

[[nodiscard]] constexpr auto is_halfwidth_punct(char32_t c) noexcept -> bool
{
    return is_halfwidth_ascii_graph(c) && !is_halfwidth_alpha(c) &&
           !is_halfwidth_digit(c);
}

[[nodiscard]] constexpr auto is_fullwidth_punct(char32_t c) noexcept -> bool
{
    return is_fullwidth_ascii_graph(c) && !is_fullwidth_alpha(c) &&
           !is_fullwidth_digit(c);
}

[[nodiscard]] constexpr auto is_halfwidth_graph(char32_t c) noexcept -> bool
{
    return is_halfwidth_alpha(c) || is_halfwidth_digit(c) ||
           is_halfwidth_punct(c) || is_halfwidth_kana(c);
}

[[nodiscard]] constexpr auto is_fullwidth_graph(char32_t c) noexcept -> bool
{
    return is_fullwidth_alpha(c) || is_fullwidth_digit(c) ||
           is_fullwidth_punct(c) || is_fullwidth_kana(c);
}

[[nodiscard]] constexpr auto is_halfwidth_print(char32_t c) noexcept -> bool
{
    return is_halfwidth_space(c) || is_halfwidth_graph(c);
}

[[nodiscard]] constexpr auto is_fullwidth_print(char32_t c) noexcept -> bool
{
    return is_fullwidth_space(c) || is_fullwidth_graph(c);
}

[[nodiscard]] constexpr auto is_halfwidth(char32_t c) noexcept -> bool
{
    return is_halfwidth_print(c);
}

[[nodiscard]] constexpr auto is_fullwidth(char32_t c) noexcept -> bool
{
    return is_fullwidth_print(c);
}

[[nodiscard]] constexpr auto to_fullwidth_digit(char32_t c) -> result<char32_t>
{
    if (is_halfwidth_digit(c)) {
        return static_cast<char32_t>(c - U'0' + U'\uff10');
    }

    if (is_fullwidth_digit(c)) {
        return c;
    }

    return std::unexpected(make_error(error_t::invalid_argument));
}

[[nodiscard]] constexpr auto to_halfwidth_digit(char32_t c) -> result<char32_t>
{
    if (is_fullwidth_digit(c)) {
        return static_cast<char32_t>(c - U'\uff10' + U'0');
    }

    if (is_halfwidth_digit(c)) {
        return c;
    }

    return std::unexpected(make_error(error_t::invalid_argument));
}

[[nodiscard]] constexpr auto to_fullwidth_alpha(char32_t c) -> result<char32_t>
{
    if (is_halfwidth_alpha(c)) {
        return static_cast<char32_t>(c + 0xfee0);
    }

    if (is_fullwidth_alpha(c)) {
        return c;
    }

    return std::unexpected(make_error(error_t::invalid_argument));
}

[[nodiscard]] constexpr auto to_halfwidth_alpha(char32_t c) -> result<char32_t>
{
    if (is_fullwidth_alpha(c)) {
        return static_cast<char32_t>(c - 0xfee0);
    }

    if (is_halfwidth_alpha(c)) {
        return c;
    }

    return std::unexpected(make_error(error_t::invalid_argument));
}

[[nodiscard]] constexpr auto to_fullwidth_punct(char32_t c) -> result<char32_t>
{
    if (is_halfwidth_punct(c)) {
        return static_cast<char32_t>(c + 0xfee0);
    }

    if (is_fullwidth_punct(c)) {
        return c;
    }

    return std::unexpected(make_error(error_t::invalid_argument));
}

[[nodiscard]] constexpr auto to_halfwidth_punct(char32_t c) -> result<char32_t>
{
    if (is_fullwidth_punct(c)) {
        return static_cast<char32_t>(c - 0xfee0);
    }

    if (is_halfwidth_punct(c)) {
        return c;
    }

    return std::unexpected(make_error(error_t::invalid_argument));
}

[[nodiscard]] constexpr auto to_fullwidth_space(char32_t c) -> result<char32_t>
{
    if (is_halfwidth_space(c) || is_fullwidth_space(c)) {
        return U'\u3000';
    }

    return std::unexpected(make_error(error_t::invalid_argument));
}

[[nodiscard]] constexpr auto to_halfwidth_space(char32_t c) -> result<char32_t>
{
    if (is_fullwidth_space(c) || is_halfwidth_space(c)) {
        return U' ';
    }

    return std::unexpected(make_error(error_t::invalid_argument));
}

[[nodiscard]] constexpr auto to_fullwidth_graph(char32_t c) -> result<char32_t>
{
    if (is_halfwidth_alpha(c)) {
        return to_fullwidth_alpha(c);
    }

    if (is_halfwidth_digit(c)) {
        return to_fullwidth_digit(c);
    }

    if (is_halfwidth_punct(c)) {
        return to_fullwidth_punct(c);
    }

    if (is_halfwidth_kana(c)) {
        return to_fullwidth_kana(c);
    }

    if (is_fullwidth_graph(c)) {
        return c;
    }

    return std::unexpected(make_error(error_t::invalid_argument));
}

[[nodiscard]] constexpr auto to_halfwidth_graph(char32_t c) -> result<char32_t>
{
    if (is_fullwidth_alpha(c)) {
        return to_halfwidth_alpha(c);
    }

    if (is_fullwidth_digit(c)) {
        return to_halfwidth_digit(c);
    }

    if (is_fullwidth_punct(c)) {
        return to_halfwidth_punct(c);
    }

    if (is_fullwidth_kana(c)) {
        return to_halfwidth_kana(c);
    }

    if (is_halfwidth_graph(c)) {
        return c;
    }

    return std::unexpected(make_error(error_t::invalid_argument));
}

[[nodiscard]] constexpr auto to_fullwidth_print(char32_t c) -> result<char32_t>
{
    if (is_halfwidth_space(c) || is_fullwidth_space(c)) {
        return to_fullwidth_space(c);
    }

    return to_fullwidth_graph(c);
}

[[nodiscard]] constexpr auto to_halfwidth_print(char32_t c) -> result<char32_t>
{
    if (is_fullwidth_space(c) || is_halfwidth_space(c)) {
        return to_halfwidth_space(c);
    }

    return to_halfwidth_graph(c);
}

[[nodiscard]] constexpr auto to_fullwidth(char32_t c) -> result<char32_t>
{
    return to_fullwidth_print(c);
}

[[nodiscard]] constexpr auto to_halfwidth(char32_t c) -> result<char32_t>
{
    return to_halfwidth_print(c);
}

} // namespace xer::detail

#endif /* XER_BITS_CHARACTER_WIDTH_H_INCLUDED_ */
