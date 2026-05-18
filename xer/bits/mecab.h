/**
 * @file xer/bits/mecab.h
 * @brief Internal MeCab-based Japanese text analysis facilities.
 */

#pragma once

#ifndef XER_BITS_MECAB_H_INCLUDED_
#define XER_BITS_MECAB_H_INCLUDED_

#include <cstddef>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <xer/bits/fclose.h>
#include <xer/bits/file_entry.h>
#include <xer/bits/getenv.h>
#include <xer/bits/stream_contents.h>
#include <xer/bits/text_encoding_common.h>
#include <xer/error.h>
#include <xer/path.h>
#include <xer/process.h>

namespace xer {

/**
 * @brief Options used to invoke MeCab.
 */
struct mecab_options {
    /**
     * @brief MeCab executable path.
     *
     * An empty path means that XER searches the `PATH` environment variable
     * for the platform's ordinary MeCab executable name.
     */
    path program;
};

/**
 * @brief One raw MeCab token result.
 */
struct mecab_token {
    /**
     * @brief Surface text of the token.
     */
    std::u8string surface;

    /**
     * @brief Raw MeCab feature text.
     *
     * The contents are dictionary-dependent and are preserved as emitted by
     * MeCab's `%H` formatter.
     */
    std::u8string feature;
};

namespace detail {

#if defined(_WIN32)
inline constexpr std::u8string_view mecab_default_program_name = u8"mecab.exe";
inline constexpr char8_t mecab_path_separator = u8';';
#else
inline constexpr std::u8string_view mecab_default_program_name = u8"mecab";
inline constexpr char8_t mecab_path_separator = u8':';
#endif

[[nodiscard]] inline auto mecab_trim_outer_quotes(
    std::u8string_view value) noexcept -> std::u8string_view
{
    if (value.size() >= 2 && value.front() == u8'"' && value.back() == u8'"') {
        return value.substr(1, value.size() - 2);
    }

    return value;
}

[[nodiscard]] inline auto mecab_find_program_in_path(
    std::u8string_view path_value) -> result<path>
{
    std::size_t start = 0;

    while (start <= path_value.size()) {
        const std::size_t position = path_value.find(mecab_path_separator, start);
        const std::size_t length = position == std::u8string_view::npos
            ? path_value.size() - start
            : position - start;

        const std::u8string_view raw_directory = path_value.substr(start, length);
        const std::u8string_view directory = mecab_trim_outer_quotes(raw_directory);

        if (!directory.empty()) {
            const auto candidate = path(directory) / path(mecab_default_program_name);
            if (candidate.has_value() && is_file(*candidate)) {
                return *candidate;
            }
        }

        if (position == std::u8string_view::npos) {
            break;
        }

        start = position + 1;
    }

    return std::unexpected(make_error(error_t::not_found));
}

[[nodiscard]] inline auto mecab_resolve_program(
    const mecab_options& options) -> result<path>
{
    if (!options.program.str().empty()) {
        return options.program;
    }

    const auto path_value = xer::getenv(u8"PATH");
    if (!path_value.has_value()) {
        return std::unexpected(path_value.error());
    }

    return mecab_find_program_in_path(*path_value);
}

[[nodiscard]] inline auto mecab_text_to_bytes(
    std::u8string_view text) -> result<std::vector<std::byte>>
{
    const std::string narrow = to_byte_string(text);
    if (!is_valid_utf8(narrow)) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    std::vector<std::byte> bytes;
    bytes.reserve(text.size() + 1);

    for (const char8_t ch : text) {
        bytes.push_back(static_cast<std::byte>(ch));
    }

    if (text.empty() || text.back() != u8'\n') {
        bytes.push_back(static_cast<std::byte>(u8'\n'));
    }

    return bytes;
}

[[nodiscard]] inline auto mecab_bytes_to_text(
    const std::vector<std::byte>& bytes) -> result<std::u8string>
{
    std::string narrow;
    narrow.reserve(bytes.size());

    for (const std::byte value : bytes) {
        narrow.push_back(static_cast<char>(std::to_integer<unsigned char>(value)));
    }

    if (!is_valid_utf8(narrow)) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    return to_u8string(narrow);
}

[[nodiscard]] inline auto mecab_make_process_options(
    const path& program) -> process_options
{
    return process_options {
        program,
        {
            u8"--node-format=%m\\t%H\\n",
            u8"--unk-format=%m\\t%H\\n",
            u8"--bos-format=",
            u8"--eos-format=EOS\\n",
        },
        process_stdio::pipe,
        process_stdio::pipe,
        process_stdio::null};
}

[[nodiscard]] inline auto mecab_strip_trailing_carriage_return(
    std::u8string_view value) noexcept -> std::u8string_view
{
    if (!value.empty() && value.back() == u8'\r') {
        return value.substr(0, value.size() - 1);
    }

    return value;
}

[[nodiscard]] inline auto mecab_parse_output(
    std::u8string_view output) -> result<std::vector<mecab_token>>
{
    std::vector<mecab_token> tokens;
    std::size_t start = 0;

    while (start <= output.size()) {
        const std::size_t position = output.find(u8'\n', start);
        const std::size_t length = position == std::u8string_view::npos
            ? output.size() - start
            : position - start;

        const std::u8string_view raw_line = output.substr(start, length);
        const std::u8string_view line = mecab_strip_trailing_carriage_return(raw_line);

        if (!line.empty()) {
            if (line != u8"EOS") {
                const std::size_t tab = line.find(u8'\t');
                if (tab == std::u8string_view::npos) {
                    return std::unexpected(make_error(error_t::process_error));
                }

                tokens.push_back(mecab_token {
                    std::u8string(line.substr(0, tab)),
                    std::u8string(line.substr(tab + 1)),
                });
            }
        }

        if (position == std::u8string_view::npos) {
            break;
        }

        start = position + 1;
    }

    return tokens;
}

} // namespace detail

/**
 * @brief Parses UTF-8 Japanese text with MeCab and returns raw token results.
 *
 * XER invokes MeCab as a child process and uses an explicit output format that
 * yields one token per line as `surface<TAB>feature`. The returned feature text
 * is MeCab's raw `%H` field and remains dictionary-dependent.
 *
 * If @p options does not specify a program path, XER searches the `PATH`
 * environment variable for the ordinary MeCab executable name.
 *
 * @param text UTF-8 source text.
 * @param options MeCab invocation options.
 * @return Raw MeCab token results on success.
 * @return `error_t::encoding_error` when @p text or MeCab output is not valid UTF-8.
 * @return `error_t::not_found` when no MeCab executable can be found automatically.
 * @return `error_t::process_error` when MeCab cannot be executed, exits unsuccessfully,
 *         or emits output that does not match XER's requested result format.
 */
[[nodiscard]] inline auto mecab_parse(
    std::u8string_view text,
    const mecab_options& options = {}) -> result<std::vector<mecab_token>>
{
    const auto program = detail::mecab_resolve_program(options);
    if (!program.has_value()) {
        return std::unexpected(program.error());
    }

    const auto input = detail::mecab_text_to_bytes(text);
    if (!input.has_value()) {
        return std::unexpected(input.error());
    }

    auto spawned = process_spawn(detail::mecab_make_process_options(*program));
    if (!spawned.has_value()) {
        return std::unexpected(spawned.error());
    }

    if (!spawned->stdin_stream.has_value() || !spawned->stdout_stream.has_value()) {
        return std::unexpected(make_error(error_t::process_error));
    }

    const auto written = stream_put_contents(
        *spawned->stdin_stream,
        std::span<const std::byte>(*input));
    if (!written.has_value()) {
        return std::unexpected(written.error());
    }

    const auto closed = fclose(*spawned->stdin_stream);
    if (!closed.has_value()) {
        return std::unexpected(closed.error());
    }

    const auto output_bytes = stream_get_contents(*spawned->stdout_stream);
    if (!output_bytes.has_value()) {
        return std::unexpected(output_bytes.error());
    }

    const auto waited = process_wait(spawned->proc);
    if (!waited.has_value() || waited->exit_code != 0) {
        return std::unexpected(make_error(error_t::process_error));
    }

    const auto output = detail::mecab_bytes_to_text(*output_bytes);
    if (!output.has_value()) {
        return std::unexpected(output.error());
    }

    return detail::mecab_parse_output(*output);
}

} // namespace xer

#endif /* XER_BITS_MECAB_H_INCLUDED_ */
