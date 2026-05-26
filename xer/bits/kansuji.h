/**
 * @file xer/bits/kansuji.h
 * @brief Kansuji integer conversion facilities.
 */

#pragma once

#ifndef XER_BITS_KANSUJI_H_INCLUDED_
#define XER_BITS_KANSUJI_H_INCLUDED_

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

#include <xer/bits/common.h>
#include <xer/error.h>

namespace xer::ja {

/**
 * @brief Output style for @ref to_kansuji.
 */
enum class kansuji_style : std::uint8_t {
    /** Arabic digits with Japanese large units, such as `12億34万5`. */
    k10,
    /** Ordinary positional Kansuji, such as `十二億三十四万五`. */
    k十,
    /** Per-digit Kansuji, such as `一二億三四万五`. */
    k一〇,
    /** Practical Daiji positional Kansuji, such as `壱拾`. */
    k拾,
};

/** Style selector for Arabic digits with Japanese large units. */
inline constexpr kansuji_style k10 = kansuji_style::k10;
/** Style selector for ordinary positional Kansuji. */
inline constexpr kansuji_style k十 = kansuji_style::k十;
/** Style selector for per-digit Kansuji. */
inline constexpr kansuji_style k一〇 = kansuji_style::k一〇;
/** Style selector for practical Daiji positional Kansuji. */
inline constexpr kansuji_style k拾 = kansuji_style::k拾;

namespace detail {

inline constexpr std::array<std::u8string_view, 10> kansuji_digits = {
    u8"〇",
    u8"一",
    u8"二",
    u8"三",
    u8"四",
    u8"五",
    u8"六",
    u8"七",
    u8"八",
    u8"九",
};

inline constexpr std::array<std::u8string_view, 10> daiji_digits = {
    u8"零",
    u8"壱",
    u8"弐",
    u8"参",
    u8"四",
    u8"五",
    u8"六",
    u8"七",
    u8"八",
    u8"九",
};

inline constexpr std::array<std::u8string_view, 5> kansuji_large_units = {
    u8"",
    u8"万",
    u8"億",
    u8"兆",
    u8"京",
};

inline auto append_view(
    std::u8string& out,
    const std::u8string_view view) -> void
{
    out.append(view.data(), view.size());
}

inline auto append_ascii_group(
    std::u8string& out,
    std::uint16_t value) -> void
{
    std::array<char8_t, 4> buffer{};
    std::size_t size = 0;

    do {
        buffer[size++] = static_cast<char8_t>(u8'0' + (value % 10u));
        value = static_cast<std::uint16_t>(value / 10u);
    } while (value != 0u);

    while (size > 0u) {
        out.push_back(buffer[--size]);
    }
}

inline auto append_digitwise_group(
    std::u8string& out,
    std::uint16_t value) -> void
{
    std::array<std::uint8_t, 4> buffer{};
    std::size_t size = 0;

    do {
        buffer[size++] = static_cast<std::uint8_t>(value % 10u);
        value = static_cast<std::uint16_t>(value / 10u);
    } while (value != 0u);

    while (size > 0u) {
        append_view(out, kansuji_digits[buffer[--size]]);
    }
}

inline auto append_positional_group(
    std::u8string& out,
    const std::uint16_t value,
    const bool daiji) -> void
{
    const std::array<std::uint16_t, 4> divisors = {1000u, 100u, 10u, 1u};
    const std::array<std::u8string_view, 4> ordinary_units = {
        u8"千",
        u8"百",
        u8"十",
        u8"",
    };
    const std::array<std::u8string_view, 4> daiji_units = {
        u8"千",
        u8"百",
        u8"拾",
        u8"",
    };

    std::uint16_t remaining = value;
    for (std::size_t index = 0; index < divisors.size(); ++index) {
        const auto divisor = divisors[index];
        const auto digit = static_cast<std::uint8_t>(remaining / divisor);
        remaining = static_cast<std::uint16_t>(remaining % divisor);

        if (digit == 0u) {
            continue;
        }

        const bool has_small_unit = index + 1u < divisors.size();
        if (daiji) {
            append_view(out, daiji_digits[digit]);
        } else if (!(has_small_unit && digit == 1u)) {
            append_view(out, kansuji_digits[digit]);
        }

        append_view(out, daiji ? daiji_units[index] : ordinary_units[index]);
    }
}

inline auto append_kansuji_group(
    std::u8string& out,
    const std::uint16_t group,
    const kansuji_style style) -> void
{
    switch (style) {
    case kansuji_style::k10:
        append_ascii_group(out, group);
        return;
    case kansuji_style::k十:
        append_positional_group(out, group, false);
        return;
    case kansuji_style::k一〇:
        append_digitwise_group(out, group);
        return;
    case kansuji_style::k拾:
        append_positional_group(out, group, true);
        return;
    }
}

enum class kansuji_token_kind : std::uint8_t {
    ascii_digit,
    normal_digit,
    daiji_digit,
    circle_zero,
    word_zero,
    small_unit,
    large_unit,
};

struct kansuji_token {
    kansuji_token_kind kind;
    std::uint64_t value;
};

[[nodiscard]] inline auto kansuji_starts_with(
    const std::u8string_view text,
    const std::size_t offset,
    const std::u8string_view needle) noexcept -> bool
{
    return offset <= text.size() && text.substr(offset).starts_with(needle);
}

[[nodiscard]] inline auto push_kansuji_token_if_match(
    std::vector<kansuji_token>& out,
    const std::u8string_view text,
    std::size_t& offset,
    const std::u8string_view needle,
    const kansuji_token_kind kind,
    const std::uint64_t value) -> bool
{
    if (!kansuji_starts_with(text, offset, needle)) {
        return false;
    }

    out.push_back(kansuji_token{kind, value});
    offset += needle.size();
    return true;
}

[[nodiscard]] inline auto tokenize_kansuji(
    const std::u8string_view text) -> result<std::vector<kansuji_token>>
{
    if (text.empty()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::vector<kansuji_token> tokens;
    tokens.reserve(text.size());

    std::size_t offset = 0;
    while (offset < text.size()) {
        const char8_t ch = text[offset];
        if (ch >= u8'0' && ch <= u8'9') {
            tokens.push_back(kansuji_token{
                kansuji_token_kind::ascii_digit,
                static_cast<std::uint64_t>(ch - u8'0'),
            });
            ++offset;
            continue;
        }

        if (push_kansuji_token_if_match(
                tokens, text, offset, u8"〇", kansuji_token_kind::circle_zero, 0u) ||
            push_kansuji_token_if_match(
                tokens, text, offset, u8"零", kansuji_token_kind::word_zero, 0u) ||
            push_kansuji_token_if_match(
                tokens, text, offset, u8"一", kansuji_token_kind::normal_digit, 1u) ||
            push_kansuji_token_if_match(
                tokens, text, offset, u8"二", kansuji_token_kind::normal_digit, 2u) ||
            push_kansuji_token_if_match(
                tokens, text, offset, u8"三", kansuji_token_kind::normal_digit, 3u) ||
            push_kansuji_token_if_match(
                tokens, text, offset, u8"四", kansuji_token_kind::normal_digit, 4u) ||
            push_kansuji_token_if_match(
                tokens, text, offset, u8"五", kansuji_token_kind::normal_digit, 5u) ||
            push_kansuji_token_if_match(
                tokens, text, offset, u8"六", kansuji_token_kind::normal_digit, 6u) ||
            push_kansuji_token_if_match(
                tokens, text, offset, u8"七", kansuji_token_kind::normal_digit, 7u) ||
            push_kansuji_token_if_match(
                tokens, text, offset, u8"八", kansuji_token_kind::normal_digit, 8u) ||
            push_kansuji_token_if_match(
                tokens, text, offset, u8"九", kansuji_token_kind::normal_digit, 9u) ||
            push_kansuji_token_if_match(
                tokens, text, offset, u8"壱", kansuji_token_kind::daiji_digit, 1u) ||
            push_kansuji_token_if_match(
                tokens, text, offset, u8"弐", kansuji_token_kind::daiji_digit, 2u) ||
            push_kansuji_token_if_match(
                tokens, text, offset, u8"参", kansuji_token_kind::daiji_digit, 3u) ||
            push_kansuji_token_if_match(
                tokens, text, offset, u8"千", kansuji_token_kind::small_unit, 1000u) ||
            push_kansuji_token_if_match(
                tokens, text, offset, u8"阡", kansuji_token_kind::small_unit, 1000u) ||
            push_kansuji_token_if_match(
                tokens, text, offset, u8"百", kansuji_token_kind::small_unit, 100u) ||
            push_kansuji_token_if_match(
                tokens, text, offset, u8"佰", kansuji_token_kind::small_unit, 100u) ||
            push_kansuji_token_if_match(
                tokens, text, offset, u8"十", kansuji_token_kind::small_unit, 10u) ||
            push_kansuji_token_if_match(
                tokens, text, offset, u8"拾", kansuji_token_kind::small_unit, 10u) ||
            push_kansuji_token_if_match(
                tokens, text, offset, u8"京", kansuji_token_kind::large_unit,
                UINT64_C(10000000000000000)) ||
            push_kansuji_token_if_match(
                tokens, text, offset, u8"兆", kansuji_token_kind::large_unit,
                UINT64_C(1000000000000)) ||
            push_kansuji_token_if_match(
                tokens, text, offset, u8"億", kansuji_token_kind::large_unit,
                UINT64_C(100000000)) ||
            push_kansuji_token_if_match(
                tokens, text, offset, u8"万", kansuji_token_kind::large_unit,
                UINT64_C(10000)) ||
            push_kansuji_token_if_match(
                tokens, text, offset, u8"萬", kansuji_token_kind::large_unit,
                UINT64_C(10000))) {
            continue;
        }

        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return tokens;
}

[[nodiscard]] constexpr auto kansuji_is_whole_zero_token(
    const kansuji_token& token) noexcept -> bool
{
    return (token.kind == kansuji_token_kind::ascii_digit && token.value == 0u) ||
           token.kind == kansuji_token_kind::circle_zero ||
           token.kind == kansuji_token_kind::word_zero;
}

[[nodiscard]] inline auto parse_ascii_kansuji_group(
    const std::vector<kansuji_token>& tokens,
    const std::size_t first,
    const std::size_t last) -> result<std::uint16_t>
{
    const std::size_t count = last - first;
    if (count == 0u || count > 4u) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (count > 1u && tokens[first].value == 0u) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::uint16_t value = 0;
    for (std::size_t index = first; index < last; ++index) {
        value = static_cast<std::uint16_t>(value * 10u + tokens[index].value);
    }

    return value;
}

[[nodiscard]] inline auto parse_digitwise_kansuji_group(
    const std::vector<kansuji_token>& tokens,
    const std::size_t first,
    const std::size_t last) -> result<std::uint16_t>
{
    const std::size_t count = last - first;
    if (count == 0u || count > 4u) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (count > 1u && tokens[first].kind == kansuji_token_kind::circle_zero) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::uint16_t value = 0;
    for (std::size_t index = first; index < last; ++index) {
        value = static_cast<std::uint16_t>(value * 10u + tokens[index].value);
    }

    return value;
}

[[nodiscard]] inline auto parse_positional_kansuji_group(
    const std::vector<kansuji_token>& tokens,
    const std::size_t first,
    const std::size_t last) -> result<std::uint16_t>
{
    if (first == last) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::uint16_t total = 0;
    std::uint16_t pending_digit = 0;
    bool has_pending_digit = false;
    std::uint64_t previous_unit = UINT64_C(10000);
    bool consumed = false;

    for (std::size_t index = first; index < last; ++index) {
        const kansuji_token token = tokens[index];
        switch (token.kind) {
        case kansuji_token_kind::normal_digit:
        case kansuji_token_kind::daiji_digit:
            if (has_pending_digit) {
                return std::unexpected(make_error(error_t::invalid_argument));
            }
            pending_digit = static_cast<std::uint16_t>(token.value);
            has_pending_digit = true;
            break;
        case kansuji_token_kind::small_unit: {
            if (token.value >= previous_unit) {
                return std::unexpected(make_error(error_t::invalid_argument));
            }

            const std::uint16_t coefficient = has_pending_digit ? pending_digit : 1u;
            total = static_cast<std::uint16_t>(total + coefficient * token.value);
            has_pending_digit = false;
            pending_digit = 0;
            previous_unit = token.value;
            consumed = true;
            break;
        }
        case kansuji_token_kind::ascii_digit:
        case kansuji_token_kind::circle_zero:
        case kansuji_token_kind::word_zero:
        case kansuji_token_kind::large_unit:
            return std::unexpected(make_error(error_t::invalid_argument));
        }
    }

    if (has_pending_digit) {
        total = static_cast<std::uint16_t>(total + pending_digit);
        consumed = true;
    }

    if (!consumed || total == 0u || total > 9999u) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return total;
}

[[nodiscard]] inline auto parse_kansuji_group(
    const std::vector<kansuji_token>& tokens,
    const std::size_t first,
    const std::size_t last) -> result<std::uint16_t>
{
    if (first == last) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    bool ascii_only = true;
    bool digitwise_only = true;

    for (std::size_t index = first; index < last; ++index) {
        const auto kind = tokens[index].kind;
        ascii_only = ascii_only && kind == kansuji_token_kind::ascii_digit;
        digitwise_only = digitwise_only &&
            (kind == kansuji_token_kind::normal_digit ||
             kind == kansuji_token_kind::circle_zero);
    }

    if (ascii_only) {
        return parse_ascii_kansuji_group(tokens, first, last);
    }

    if (digitwise_only) {
        return parse_digitwise_kansuji_group(tokens, first, last);
    }

    return parse_positional_kansuji_group(tokens, first, last);
}

[[nodiscard]] inline auto checked_add_kansuji_group(
    std::uint64_t& total,
    const std::uint16_t group,
    const std::uint64_t multiplier) -> result<void>
{
    const auto max = std::numeric_limits<std::uint64_t>::max();
    if (group > max / multiplier) {
        return std::unexpected(make_error(error_t::overflow_error));
    }

    const std::uint64_t scaled = static_cast<std::uint64_t>(group) * multiplier;
    if (total > max - scaled) {
        return std::unexpected(make_error(error_t::overflow_error));
    }

    total += scaled;
    return {};
}

} // namespace detail

/**
 * @brief Converts a non-negative integer to Kansuji text.
 *
 * @param value Value to convert.
 * @param style Output style.
 * @return Kansuji text.
 */
[[nodiscard]] inline auto to_kansuji(
    const std::uint64_t value,
    const kansuji_style style) -> std::u8string
{
    if (value == 0u) {
        switch (style) {
        case kansuji_style::k10:
            return std::u8string(u8"0");
        case kansuji_style::k十:
            return std::u8string(u8"零");
        case kansuji_style::k一〇:
            return std::u8string(u8"〇");
        case kansuji_style::k拾:
            return std::u8string(u8"零");
        }
    }

    std::array<std::uint16_t, 5> groups{};
    std::uint64_t remaining = value;
    for (std::size_t index = 0; index < groups.size(); ++index) {
        groups[index] = static_cast<std::uint16_t>(remaining % UINT64_C(10000));
        remaining /= UINT64_C(10000);
    }

    std::u8string out;
    for (std::size_t index = groups.size(); index > 0u; --index) {
        const std::size_t group_index = index - 1u;
        const std::uint16_t group = groups[group_index];
        if (group == 0u) {
            continue;
        }

        detail::append_kansuji_group(out, group, style);
        detail::append_view(out, detail::kansuji_large_units[group_index]);
    }

    return out;
}

/**
 * @brief Parses practical Kansuji notation into an unsigned 64-bit integer.
 *
 * The parser accepts the notation families generated by @ref to_kansuji and a
 * practical Daiji subset normalized from `壱`, `弐`, `参`, `拾`, `佰`, `阡`, and
 * `萬`. Large units must appear in descending order and must be preceded by a
 * non-empty numeric group.
 *
 * @param text Kansuji text to parse.
 * @return Parsed value on success.
 * @return `error_t::invalid_argument` if @p text is syntactically invalid.
 * @return `error_t::overflow_error` if @p text exceeds `std::uint64_t`.
 */
[[nodiscard]] inline auto from_kansuji(
    const std::u8string_view text) -> result<std::uint64_t>
{
    const auto tokenized = detail::tokenize_kansuji(text);
    if (!tokenized.has_value()) {
        return std::unexpected(tokenized.error());
    }

    const auto& tokens = *tokenized;
    if (tokens.size() == 1u && detail::kansuji_is_whole_zero_token(tokens.front())) {
        return UINT64_C(0);
    }

    std::uint64_t total = 0;
    std::size_t group_first = 0;
    std::uint64_t previous_large_unit = std::numeric_limits<std::uint64_t>::max();
    bool has_large_unit = false;

    for (std::size_t index = 0; index < tokens.size(); ++index) {
        const auto token = tokens[index];
        if (token.kind != detail::kansuji_token_kind::large_unit) {
            continue;
        }

        if (token.value >= previous_large_unit || group_first == index) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        const auto group = detail::parse_kansuji_group(tokens, group_first, index);
        if (!group.has_value() || *group == 0u) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        const auto added = detail::checked_add_kansuji_group(total, *group, token.value);
        if (!added.has_value()) {
            return std::unexpected(added.error());
        }

        has_large_unit = true;
        previous_large_unit = token.value;
        group_first = index + 1u;
    }

    if (group_first < tokens.size()) {
        const auto group = detail::parse_kansuji_group(tokens, group_first, tokens.size());
        if (!group.has_value()) {
            return std::unexpected(group.error());
        }
        if (has_large_unit && *group == 0u) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        const auto added = detail::checked_add_kansuji_group(total, *group, UINT64_C(1));
        if (!added.has_value()) {
            return std::unexpected(added.error());
        }
    } else if (!has_large_unit) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return total;
}

} // namespace xer::ja

#endif /* XER_BITS_KANSUJI_H_INCLUDED_ */
