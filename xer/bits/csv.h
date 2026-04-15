/**
 * @file xer/bits/csv.h
 * @brief CSV input/output functions for text streams.
 */

#pragma once

#ifndef XER_BITS_CSV_H_INCLUDED_
#define XER_BITS_CSV_H_INCLUDED_

#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <xer/bits/text_stream_io.h>
#include <xer/bits/utf8_char_encode.h>
#include <xer/error.h>

namespace xer::detail {

/**
 * @brief Returns whether a character is a valid CSV separator.
 *
 * @param ch Character to validate.
 * @return true if valid.
 * @return false otherwise.
 */
[[nodiscard]] constexpr auto is_valid_csv_separator(char32_t ch) noexcept -> bool {
    return ch != U'\0' && ch != U'\r' && ch != U'\n';
}

/**
 * @brief Returns whether a character is a valid CSV enclosure.
 *
 * @param ch Character to validate.
 * @return true if valid.
 * @return false otherwise.
 */
[[nodiscard]] constexpr auto is_valid_csv_enclosure(char32_t ch) noexcept -> bool {
    return ch != U'\0' && ch != U'\r' && ch != U'\n';
}

/**
 * @brief Returns whether a character should force quoted CSV output.
 *
 * @param ch Character to test.
 * @param separator Field separator.
 * @param enclosure Field enclosure.
 * @return true if the field should be quoted.
 * @return false otherwise.
 */
[[nodiscard]] constexpr auto csv_char_requires_quotes(
    char32_t ch,
    char32_t separator,
    char32_t enclosure) noexcept -> bool {
    return ch == separator || ch == enclosure || ch == U'\r' || ch == U'\n' || ch == U' ' ||
           ch == U'\t';
}

/**
 * @brief Appends one UTF-8 character to a field buffer.
 *
 * @param field Destination field.
 * @param ch Character to append.
 * @return Success or failure.
 */
[[nodiscard]] inline auto append_csv_char(
    std::u8string& field,
    char32_t ch) -> result<void> {
    const auto appended = append_utf8_char(field, ch);
    if (!appended.has_value()) {
        return std::unexpected(appended.error());
    }

    return {};
}

/**
 * @brief Appends a CSV field to an output record.
 *
 * The field is UTF-8 decoded first so that enclosure doubling is performed on
 * Unicode code points rather than raw bytes.
 *
 * @param out Destination CSV record.
 * @param field Source field.
 * @param separator Field separator.
 * @param enclosure Field enclosure.
 * @return Success or failure.
 */
[[nodiscard]] inline auto append_csv_field(
    std::u8string& out,
    std::u8string_view field,
    char32_t separator,
    char32_t enclosure) -> result<void> {
    auto decoded = decode_utf8_string(field);
    if (!decoded.has_value()) {
        return std::unexpected(decoded.error());
    }

    bool needs_quotes = field.empty();
    if (!field.empty()) {
        const char32_t first = decoded->front();
        const char32_t last = decoded->back();

        if (first == U' ' || first == U'\t' || last == U' ' || last == U'\t') {
            needs_quotes = true;
        }
    }

    for (const char32_t ch : *decoded) {
        if (csv_char_requires_quotes(ch, separator, enclosure)) {
            needs_quotes = true;
            break;
        }
    }

    if (!needs_quotes) {
        out += field;
        return {};
    }

    auto appended = append_utf8_char(out, enclosure);
    if (!appended.has_value()) {
        return std::unexpected(appended.error());
    }

    for (const char32_t ch : *decoded) {
        if (ch == enclosure) {
            appended = append_utf8_char(out, enclosure);
            if (!appended.has_value()) {
                return std::unexpected(appended.error());
            }
        }

        appended = append_utf8_char(out, ch);
        if (!appended.has_value()) {
            return std::unexpected(appended.error());
        }
    }

    appended = append_utf8_char(out, enclosure);
    if (!appended.has_value()) {
        return std::unexpected(appended.error());
    }

    return {};
}

/**
 * @brief Consumes an optional LF after a CR record terminator.
 *
 * @param stream Target stream.
 * @return Success or failure.
 */
[[nodiscard]] inline auto consume_optional_lf_after_cr(text_stream& stream) -> result<void> {
    const auto next = fgetc(stream);
    if (!next.has_value()) {
        if (next.error().code == error_t::not_found) {
            return {};
        }

        return std::unexpected(next.error());
    }

    if (*next != U'\n') {
        const auto ungot = ungetc(*next, stream);
        if (!ungot.has_value()) {
            return std::unexpected(ungot.error());
        }
    }

    return {};
}

/**
 * @brief Finalizes the current CSV field and appends it to the output vector.
 *
 * @param fields Destination field vector.
 * @param field Current field buffer.
 */
inline auto finalize_csv_field(
    std::vector<std::u8string>& fields,
    std::u8string& field) -> void {
    fields.push_back(std::move(field));
    field.clear();
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Reads one CSV record from a text stream.
 *
 * This function supports quoted fields, doubled enclosure escaping, CRLF, LF,
 * and CR as record terminators, and embedded newlines inside quoted fields.
 *
 * BOM handling is not performed here. It is the responsibility of text_stream.
 *
 * @param stream Target stream.
 * @param separator Field separator.
 * @param enclosure Field enclosure.
 * @return Parsed field vector on success.
 */
[[nodiscard]] inline auto fgetcsv(
    text_stream& stream,
    char32_t separator = U',',
    char32_t enclosure = U'"') -> result<std::vector<std::u8string>> {
    if (!detail::is_valid_csv_separator(separator) || !detail::is_valid_csv_enclosure(enclosure) ||
        separator == enclosure) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    enum class parse_state {
        field_start,
        unquoted,
        quoted,
        quoted_tail
    };

    std::vector<std::u8string> fields;
    std::u8string field;
    parse_state state = parse_state::field_start;
    bool read_anything = false;

    while (true) {
        const auto ch = fgetc(stream);
        if (!ch.has_value()) {
            if (ch.error().code == error_t::not_found) {
                if (!read_anything && state == parse_state::field_start && field.empty() &&
                    fields.empty()) {
                    return std::unexpected(ch.error());
                }

                if (state == parse_state::quoted) {
                    return std::unexpected(make_error(error_t::invalid_argument));
                }

                detail::finalize_csv_field(fields, field);
                return fields;
            }

            return std::unexpected(ch.error());
        }

        const char32_t value = *ch;
        read_anything = true;

        switch (state) {
        case parse_state::field_start:
            if (value == separator) {
                fields.emplace_back();
                continue;
            }

            if (value == enclosure) {
                state = parse_state::quoted;
                continue;
            }

            if (value == U'\n') {
                fields.emplace_back();
                return fields;
            }

            if (value == U'\r') {
                const auto consumed = detail::consume_optional_lf_after_cr(stream);
                if (!consumed.has_value()) {
                    return std::unexpected(consumed.error());
                }

                fields.emplace_back();
                return fields;
            }

            {
                const auto appended = detail::append_csv_char(field, value);
                if (!appended.has_value()) {
                    return std::unexpected(appended.error());
                }
            }
            state = parse_state::unquoted;
            break;

        case parse_state::unquoted:
            if (value == separator) {
                detail::finalize_csv_field(fields, field);
                state = parse_state::field_start;
                continue;
            }

            if (value == U'\n') {
                detail::finalize_csv_field(fields, field);
                return fields;
            }

            if (value == U'\r') {
                const auto consumed = detail::consume_optional_lf_after_cr(stream);
                if (!consumed.has_value()) {
                    return std::unexpected(consumed.error());
                }

                detail::finalize_csv_field(fields, field);
                return fields;
            }

            {
                const auto appended = detail::append_csv_char(field, value);
                if (!appended.has_value()) {
                    return std::unexpected(appended.error());
                }
            }
            break;

        case parse_state::quoted:
            if (value == enclosure) {
                const auto next = fgetc(stream);
                if (!next.has_value()) {
                    if (next.error().code == error_t::not_found) {
                        detail::finalize_csv_field(fields, field);
                        return fields;
                    }

                    return std::unexpected(next.error());
                }

                if (*next == enclosure) {
                    const auto appended = detail::append_csv_char(field, enclosure);
                    if (!appended.has_value()) {
                        return std::unexpected(appended.error());
                    }
                    continue;
                }

                const auto ungot = ungetc(*next, stream);
                if (!ungot.has_value()) {
                    return std::unexpected(ungot.error());
                }

                state = parse_state::quoted_tail;
                continue;
            }

            {
                const auto appended = detail::append_csv_char(field, value);
                if (!appended.has_value()) {
                    return std::unexpected(appended.error());
                }
            }
            break;

        case parse_state::quoted_tail:
            if (value == separator) {
                detail::finalize_csv_field(fields, field);
                state = parse_state::field_start;
                continue;
            }

            if (value == U'\n') {
                detail::finalize_csv_field(fields, field);
                return fields;
            }

            if (value == U'\r') {
                const auto consumed = detail::consume_optional_lf_after_cr(stream);
                if (!consumed.has_value()) {
                    return std::unexpected(consumed.error());
                }

                detail::finalize_csv_field(fields, field);
                return fields;
            }

            if (value == U' ' || value == U'\t') {
                continue;
            }

            return std::unexpected(make_error(error_t::invalid_argument));
        }
    }
}

/**
 * @brief Writes one CSV record to a text stream.
 *
 * The output uses doubled enclosure escaping and appends one logical record
 * terminator. The physical newline sequence is determined by the target
 * text_stream rather than by this function itself.
 *
 * @param fields Source field range.
 * @param stream Target stream.
 * @param separator Field separator.
 * @param enclosure Field enclosure.
 * @return Number of UTF-8 code units requested for output on success.
 */
[[nodiscard]] inline auto fputcsv(
    std::span<const std::u8string_view> fields,
    text_stream& stream,
    char32_t separator = U',',
    char32_t enclosure = U'"') -> result<std::size_t> {
    if (!detail::is_valid_csv_separator(separator) || !detail::is_valid_csv_enclosure(enclosure) ||
        separator == enclosure) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::u8string record;

    for (std::size_t i = 0; i < fields.size(); ++i) {
        if (i != 0) {
            const auto appended = detail::append_utf8_char(record, separator);
            if (!appended.has_value()) {
                return std::unexpected(appended.error());
            }
        }

        const auto appended = detail::append_csv_field(record, fields[i], separator, enclosure);
        if (!appended.has_value()) {
            return std::unexpected(appended.error());
        }
    }

    return fputs(record, stream, true);
}

/**
 * @brief Writes one CSV record to a text stream.
 *
 * This overload has the same newline behavior as the span-based overload: it
 * appends one logical record terminator, and the physical newline sequence is
 * determined by the target text_stream.
 *
 * @param fields Source field vector.
 * @param stream Target stream.
 * @param separator Field separator.
 * @param enclosure Field enclosure.
 * @return Number of UTF-8 code units requested for output on success.
 */
[[nodiscard]] inline auto fputcsv(
    const std::vector<std::u8string>& fields,
    text_stream& stream,
    char32_t separator = U',',
    char32_t enclosure = U'"') -> result<std::size_t> {
    std::vector<std::u8string_view> views;
    views.reserve(fields.size());

    for (const auto& field : fields) {
        views.emplace_back(field);
    }

    return fputcsv(std::span<const std::u8string_view>(views.data(), views.size()), stream,
                   separator, enclosure);
}

} // namespace xer

#endif /* XER_BITS_CSV_H_INCLUDED_ */
