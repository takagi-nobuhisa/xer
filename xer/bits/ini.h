/**
 * @file xer/bits/ini.h
 * @brief Internal INI decode and encode implementation.
 */

#pragma once

#ifndef XER_BITS_INI_H_INCLUDED_
#define XER_BITS_INI_H_INCLUDED_

#include <cstddef>
#include <expected>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <xer/bits/common.h>
#include <xer/error.h>

namespace xer {

/**
 * @brief Represents one INI key-value entry.
 *
 * INI has no standard value type system, so both the key and the value are
 * stored as UTF-8 strings. Duplicate keys are intentionally preserved.
 */
struct ini_entry {
    std::u8string key;
    std::u8string value;
};

/**
 * @brief Represents one INI section.
 *
 * Sections are stored in source order. Duplicate section names are intentionally
 * preserved instead of being merged, because merging could silently change the
 * meaning of an implementation-defined INI dialect.
 */
struct ini_section {
    std::u8string name;
    std::vector<ini_entry> entries;
};

/**
 * @brief Represents a decoded INI file.
 *
 * Entries before the first section are stored in @ref entries. Section entries
 * are stored in @ref sections, preserving the original order as far as the
 * supported subset can represent it.
 */
struct ini_file {
    std::vector<ini_entry> entries;
    std::vector<ini_section> sections;
};

} // namespace xer

namespace xer::detail {

[[nodiscard]] constexpr auto ini_is_ascii_space(char8_t ch) noexcept -> bool
{
    return ch == u8' ' || ch == u8'\t' || ch == u8'\n' ||
           ch == u8'\r' || ch == u8'\f' || ch == u8'\v';
}

[[nodiscard]] constexpr auto ini_trim_ascii_space(
    std::u8string_view value) noexcept -> std::u8string_view
{
    std::size_t first = 0;
    while (first < value.size() && ini_is_ascii_space(value[first])) {
        ++first;
    }

    std::size_t last = value.size();
    while (last > first && ini_is_ascii_space(value[last - 1])) {
        --last;
    }

    return value.substr(first, last - first);
}

[[nodiscard]] inline auto ini_contains_char(
    std::u8string_view value,
    char8_t ch) noexcept -> bool
{
    return value.find(ch) != std::u8string_view::npos;
}

[[nodiscard]] inline auto ini_contains_line_break(
    std::u8string_view value) noexcept -> bool
{
    return ini_contains_char(value, u8'\n') || ini_contains_char(value, u8'\r');
}

[[nodiscard]] inline auto ini_is_valid_utf8(std::u8string_view value) noexcept
    -> bool
{
    std::size_t i = 0;

    while (i < value.size()) {
        const unsigned char c0 = static_cast<unsigned char>(value[i]);

        if (c0 <= 0x7f) {
            ++i;
            continue;
        }

        if (c0 >= 0xc2 && c0 <= 0xdf) {
            if (i + 1 >= value.size()) {
                return false;
            }

            const unsigned char c1 = static_cast<unsigned char>(value[i + 1]);
            if ((c1 & 0xc0u) != 0x80u) {
                return false;
            }

            i += 2;
            continue;
        }

        if (c0 == 0xe0) {
            if (i + 2 >= value.size()) {
                return false;
            }

            const unsigned char c1 = static_cast<unsigned char>(value[i + 1]);
            const unsigned char c2 = static_cast<unsigned char>(value[i + 2]);
            if (c1 < 0xa0 || c1 > 0xbf || (c2 & 0xc0u) != 0x80u) {
                return false;
            }

            i += 3;
            continue;
        }

        if (c0 >= 0xe1 && c0 <= 0xec) {
            if (i + 2 >= value.size()) {
                return false;
            }

            const unsigned char c1 = static_cast<unsigned char>(value[i + 1]);
            const unsigned char c2 = static_cast<unsigned char>(value[i + 2]);
            if ((c1 & 0xc0u) != 0x80u || (c2 & 0xc0u) != 0x80u) {
                return false;
            }

            i += 3;
            continue;
        }

        if (c0 == 0xed) {
            if (i + 2 >= value.size()) {
                return false;
            }

            const unsigned char c1 = static_cast<unsigned char>(value[i + 1]);
            const unsigned char c2 = static_cast<unsigned char>(value[i + 2]);
            if (c1 < 0x80 || c1 > 0x9f || (c2 & 0xc0u) != 0x80u) {
                return false;
            }

            i += 3;
            continue;
        }

        if (c0 >= 0xee && c0 <= 0xef) {
            if (i + 2 >= value.size()) {
                return false;
            }

            const unsigned char c1 = static_cast<unsigned char>(value[i + 1]);
            const unsigned char c2 = static_cast<unsigned char>(value[i + 2]);
            if ((c1 & 0xc0u) != 0x80u || (c2 & 0xc0u) != 0x80u) {
                return false;
            }

            i += 3;
            continue;
        }

        if (c0 == 0xf0) {
            if (i + 3 >= value.size()) {
                return false;
            }

            const unsigned char c1 = static_cast<unsigned char>(value[i + 1]);
            const unsigned char c2 = static_cast<unsigned char>(value[i + 2]);
            const unsigned char c3 = static_cast<unsigned char>(value[i + 3]);
            if (c1 < 0x90 || c1 > 0xbf || (c2 & 0xc0u) != 0x80u ||
                (c3 & 0xc0u) != 0x80u) {
                return false;
            }

            i += 4;
            continue;
        }

        if (c0 >= 0xf1 && c0 <= 0xf3) {
            if (i + 3 >= value.size()) {
                return false;
            }

            const unsigned char c1 = static_cast<unsigned char>(value[i + 1]);
            const unsigned char c2 = static_cast<unsigned char>(value[i + 2]);
            const unsigned char c3 = static_cast<unsigned char>(value[i + 3]);
            if ((c1 & 0xc0u) != 0x80u || (c2 & 0xc0u) != 0x80u ||
                (c3 & 0xc0u) != 0x80u) {
                return false;
            }

            i += 4;
            continue;
        }

        if (c0 == 0xf4) {
            if (i + 3 >= value.size()) {
                return false;
            }

            const unsigned char c1 = static_cast<unsigned char>(value[i + 1]);
            const unsigned char c2 = static_cast<unsigned char>(value[i + 2]);
            const unsigned char c3 = static_cast<unsigned char>(value[i + 3]);
            if (c1 < 0x80 || c1 > 0x8f || (c2 & 0xc0u) != 0x80u ||
                (c3 & 0xc0u) != 0x80u) {
                return false;
            }

            i += 4;
            continue;
        }

        return false;
    }

    return true;
}

[[nodiscard]] inline auto ini_can_write_trimmed_token(
    std::u8string_view value) noexcept -> bool
{
    return ini_trim_ascii_space(value) == value && !ini_contains_line_break(value);
}

[[nodiscard]] inline auto ini_can_write_key(std::u8string_view value) noexcept
    -> bool
{
    if (value.empty() || !ini_can_write_trimmed_token(value)) {
        return false;
    }

    if (value.front() == u8';' || value.front() == u8'#' ||
        value.front() == u8'[') {
        return false;
    }

    return !ini_contains_char(value, u8'=');
}

[[nodiscard]] inline auto ini_can_write_value(std::u8string_view value) noexcept
    -> bool
{
    return ini_can_write_trimmed_token(value);
}

[[nodiscard]] inline auto ini_can_write_section_name(
    std::u8string_view value) noexcept -> bool
{
    if (value.empty() || !ini_can_write_trimmed_token(value)) {
        return false;
    }

    return !ini_contains_line_break(value) && !ini_contains_char(value, u8']');
}

class ini_decoder {
public:
    explicit ini_decoder(std::u8string_view text_) noexcept
        : text(text_), pos(0), current_section(nullptr)
    {
    }

    [[nodiscard]] auto parse() -> result<ini_file>
    {
        if (!ini_is_valid_utf8(text)) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        ini_file out;

        while (pos < text.size()) {
            const std::u8string_view line = read_line();
            auto parsed = parse_line(out, line);
            if (!parsed.has_value()) {
                return std::unexpected(parsed.error());
            }
        }

        return out;
    }

private:
    std::u8string_view text;
    std::size_t pos;
    ini_section* current_section;

    [[nodiscard]] auto read_line() noexcept -> std::u8string_view
    {
        const std::size_t begin = pos;

        while (pos < text.size() && text[pos] != u8'\n' && text[pos] != u8'\r') {
            ++pos;
        }

        const std::size_t end = pos;

        if (pos < text.size()) {
            if (text[pos] == u8'\r' && (pos + 1) < text.size() &&
                text[pos + 1] == u8'\n') {
                pos += 2;
            } else {
                ++pos;
            }
        }

        return text.substr(begin, end - begin);
    }

    [[nodiscard]] auto parse_line(
        ini_file& out,
        std::u8string_view line) -> result<void>
    {
        line = ini_trim_ascii_space(line);

        if (line.empty()) {
            return {};
        }

        if (line.front() == u8';' || line.front() == u8'#') {
            return {};
        }

        if (line.front() == u8'[') {
            return parse_section(out, line);
        }

        return parse_entry(out, line);
    }

    [[nodiscard]] auto parse_section(
        ini_file& out,
        std::u8string_view line) -> result<void>
    {
        if (line.size() < 2 || line.back() != u8']') {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        const std::u8string_view name = ini_trim_ascii_space(
            line.substr(1, line.size() - 2));
        if (name.empty()) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        out.sections.push_back(ini_section{std::u8string(name), {}});
        current_section = &out.sections.back();
        return {};
    }

    [[nodiscard]] auto parse_entry(
        ini_file& out,
        std::u8string_view line) -> result<void>
    {
        const std::size_t separator = line.find(u8'=');
        if (separator == std::u8string_view::npos) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        const std::u8string_view key = ini_trim_ascii_space(
            line.substr(0, separator));
        const std::u8string_view value = ini_trim_ascii_space(
            line.substr(separator + 1));

        if (key.empty()) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        ini_entry entry{std::u8string(key), std::u8string(value)};

        if (current_section == nullptr) {
            out.entries.push_back(std::move(entry));
        } else {
            current_section->entries.push_back(std::move(entry));
        }

        return {};
    }
};

[[nodiscard]] inline auto ini_validate_entry(
    const ini_entry& entry) -> result<void>
{
    if (!ini_is_valid_utf8(entry.key) || !ini_is_valid_utf8(entry.value)) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    if (!ini_can_write_key(entry.key) || !ini_can_write_value(entry.value)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return {};
}

[[nodiscard]] inline auto ini_validate_section(
    const ini_section& section) -> result<void>
{
    if (!ini_is_valid_utf8(section.name)) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    if (!ini_can_write_section_name(section.name)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    for (const auto& entry : section.entries) {
        auto validated = ini_validate_entry(entry);
        if (!validated.has_value()) {
            return validated;
        }
    }

    return {};
}

inline void ini_encode_entry(std::u8string& out, const ini_entry& entry)
{
    out.append(entry.key);
    out.push_back(u8'=');
    out.append(entry.value);
    out.push_back(u8'\n');
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Decodes an INI text into an ordered INI representation.
 *
 * The supported subset accepts blank lines, full-line comments beginning with
 * `;` or `#`, global `key=value` entries, and `[section]` headings. Leading and
 * trailing ASCII whitespace around keys, values, and section names is removed.
 * Duplicate keys and duplicate sections are preserved.
 *
 * @param text UTF-8 INI text.
 * @return Parsed INI representation on success.
 */
[[nodiscard]] inline auto ini_decode(std::u8string_view text) -> result<ini_file>
{
    detail::ini_decoder decoder(text);
    return decoder.parse();
}

/**
 * @brief Encodes an INI representation into UTF-8 INI text.
 *
 * This function emits a compact, deterministic representation using `key=value`
 * lines and `[section]` headings. Because the initial INI subset has no quoting
 * or escaping rules, strings that cannot be represented without changing how
 * they would be decoded are rejected.
 *
 * @param value INI representation to encode.
 * @return UTF-8 INI text on success.
 */
[[nodiscard]] inline auto ini_encode(const ini_file& value) -> result<std::u8string>
{
    std::u8string out;

    for (const auto& entry : value.entries) {
        auto validated = detail::ini_validate_entry(entry);
        if (!validated.has_value()) {
            return std::unexpected(validated.error());
        }

        detail::ini_encode_entry(out, entry);
    }

    for (const auto& section : value.sections) {
        auto validated = detail::ini_validate_section(section);
        if (!validated.has_value()) {
            return std::unexpected(validated.error());
        }

        if (!out.empty()) {
            out.push_back(u8'\n');
        }

        out.push_back(u8'[');
        out.append(section.name);
        out.push_back(u8']');
        out.push_back(u8'\n');

        for (const auto& entry : section.entries) {
            detail::ini_encode_entry(out, entry);
        }
    }

    return out;
}

} // namespace xer

#endif /* XER_BITS_INI_H_INCLUDED_ */
