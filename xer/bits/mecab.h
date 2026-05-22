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
 * @brief Split MeCab feature fields.
 *
 * MeCab feature fields are dictionary-dependent. XER stores all comma-separated
 * fields in @ref 項目 and also fills the common IPADIC-style fields when those
 * positions are present.
 */
struct mecab_features {
    /**
     * @brief Part of speech.
     */
    std::u8string 品詞;

    /**
     * @brief Part-of-speech subclass 1.
     */
    std::u8string 品詞細分類1;

    /**
     * @brief Part-of-speech subclass 2.
     */
    std::u8string 品詞細分類2;

    /**
     * @brief Part-of-speech subclass 3.
     */
    std::u8string 品詞細分類3;

    /**
     * @brief Conjugation type.
     */
    std::u8string 活用型;

    /**
     * @brief Conjugation form.
     */
    std::u8string 活用形;

    /**
     * @brief Base form.
     */
    std::u8string 原形;

    /**
     * @brief Reading.
     */
    std::u8string 読み;

    /**
     * @brief Pronunciation.
     */
    std::u8string 発音;

    /**
     * @brief All comma-separated feature fields, preserving dictionary-specific data.
     */
    std::vector<std::u8string> 項目;
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

    /**
     * @brief Split feature fields.
     */
    mecab_features features;
};

/**
 * @brief Kind of a MeCab-derived phrase range.
 */
enum class mecab_phrase_kind {
    /**
     * @brief A bunsetsu-like phrase.
     */
    bunsetsu,

    /**
     * @brief A symbol or a consecutive symbol range.
     */
    symbol,
};

/**
 * @brief A subrange of a MeCab token sequence.
 */
struct mecab_phrase {
    /**
     * @brief Range kind.
     */
    mecab_phrase_kind kind = mecab_phrase_kind::bunsetsu;

    /**
     * @brief First token index in the original token sequence.
     */
    std::size_t index = 0;

    /**
     * @brief Number of tokens in the range.
     */
    std::size_t count = 0;
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

[[nodiscard]] inline auto mecab_unescape_feature_field(
    std::u8string_view value) -> std::u8string
{
    const bool quoted = value.size() >= 2
        && value.front() == u8'"'
        && value.back() == u8'"';

    const std::size_t begin = quoted ? 1 : 0;
    const std::size_t end = quoted ? value.size() - 1 : value.size();

    std::u8string result;
    result.reserve(end - begin);

    for (std::size_t i = begin; i < end; ++i) {
        const char8_t ch = value[i];
        if (quoted && ch == u8'"' && i + 1 < end && value[i + 1] == u8'"') {
            result.push_back(u8'"');
            ++i;
        } else {
            result.push_back(ch);
        }
    }

    return result;
}

[[nodiscard]] inline auto mecab_split_feature_fields(
    std::u8string_view feature) -> std::vector<std::u8string>
{
    std::vector<std::u8string> fields;
    std::size_t start = 0;
    bool quoted = false;

    for (std::size_t i = 0; i < feature.size(); ++i) {
        const char8_t ch = feature[i];

        if (ch == u8'"') {
            if (quoted && i + 1 < feature.size() && feature[i + 1] == u8'"') {
                ++i;
            } else {
                quoted = !quoted;
            }
        } else if (ch == u8',' && !quoted) {
            fields.push_back(mecab_unescape_feature_field(feature.substr(start, i - start)));
            start = i + 1;
        }
    }

    fields.push_back(mecab_unescape_feature_field(feature.substr(start)));
    return fields;
}

[[nodiscard]] inline auto mecab_field_or_empty(
    const std::vector<std::u8string>& fields,
    std::size_t index) -> std::u8string
{
    if (index < fields.size()) {
        return fields[index];
    }

    return {};
}

[[nodiscard]] inline auto mecab_parse_features(
    std::u8string_view feature) -> mecab_features
{
    auto fields = mecab_split_feature_fields(feature);

    mecab_features features {
        mecab_field_or_empty(fields, 0),
        mecab_field_or_empty(fields, 1),
        mecab_field_or_empty(fields, 2),
        mecab_field_or_empty(fields, 3),
        mecab_field_or_empty(fields, 4),
        mecab_field_or_empty(fields, 5),
        mecab_field_or_empty(fields, 6),
        mecab_field_or_empty(fields, 7),
        mecab_field_or_empty(fields, 8),
        std::move(fields),
    };

    return features;
}

[[nodiscard]] inline auto mecab_feature_is(
    std::u8string_view value,
    std::u8string_view expected) noexcept -> bool
{
    return value == expected;
}

[[nodiscard]] inline auto mecab_feature_starts_with(
    std::u8string_view value,
    std::u8string_view prefix) noexcept -> bool
{
    return value.starts_with(prefix);
}

[[nodiscard]] inline auto mecab_is_symbol(
    const mecab_token& token) noexcept -> bool
{
    return mecab_feature_is(token.features.品詞, u8"記号");
}

[[nodiscard]] inline auto mecab_is_particle_or_auxiliary(
    const mecab_token& token) noexcept -> bool
{
    return mecab_feature_is(token.features.品詞, u8"助詞")
        || mecab_feature_is(token.features.品詞, u8"助動詞");
}

[[nodiscard]] inline auto mecab_is_prefix(
    const mecab_token& token) noexcept -> bool
{
    return mecab_feature_is(token.features.品詞, u8"接頭詞");
}

[[nodiscard]] inline auto mecab_is_suffix(
    const mecab_token& token) noexcept -> bool
{
    return mecab_feature_is(token.features.品詞, u8"接尾詞")
        || mecab_feature_is(token.features.品詞細分類1, u8"接尾")
        || mecab_feature_is(token.features.品詞細分類2, u8"接尾")
        || mecab_feature_is(token.features.品詞細分類3, u8"接尾");
}

[[nodiscard]] inline auto mecab_is_non_independent(
    const mecab_token& token) noexcept -> bool
{
    return mecab_feature_is(token.features.品詞細分類1, u8"非自立")
        || mecab_feature_is(token.features.品詞細分類2, u8"非自立")
        || mecab_feature_is(token.features.品詞細分類3, u8"非自立");
}

[[nodiscard]] inline auto mecab_is_dependent_word(
    const mecab_token& token) noexcept -> bool
{
    return mecab_is_particle_or_auxiliary(token)
        || mecab_is_suffix(token)
        || mecab_is_non_independent(token);
}

[[nodiscard]] inline auto mecab_is_noun(
    const mecab_token& token) noexcept -> bool
{
    return mecab_feature_is(token.features.品詞, u8"名詞");
}

[[nodiscard]] inline auto mecab_is_predicate(
    const mecab_token& token) noexcept -> bool
{
    return mecab_feature_is(token.features.品詞, u8"動詞")
        || mecab_feature_is(token.features.品詞, u8"形容詞");
}

[[nodiscard]] inline auto mecab_is_renyou_form(
    const mecab_token& token) noexcept -> bool
{
    return mecab_feature_starts_with(token.features.活用形, u8"連用");
}

[[nodiscard]] inline auto mecab_is_independent_word(
    const mecab_token& token) noexcept -> bool
{
    return !token.features.品詞.empty()
        && !mecab_is_symbol(token)
        && !mecab_is_dependent_word(token);
}

[[nodiscard]] inline auto mecab_phrase_should_continue(
    const mecab_token& previous,
    const mecab_token& current) noexcept -> bool
{
    if (mecab_is_dependent_word(current)) {
        return true;
    }

    if (mecab_is_prefix(previous)) {
        return true;
    }

    if (mecab_is_noun(previous) && mecab_is_noun(current)) {
        return true;
    }

    if (mecab_is_predicate(previous)
        && mecab_is_renyou_form(previous)
        && mecab_is_independent_word(current)) {
        return true;
    }

    return false;
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

                std::u8string feature(line.substr(tab + 1));
                auto features = mecab_parse_features(feature);

                tokens.push_back(mecab_token {
                    std::u8string(line.substr(0, tab)),
                    std::move(feature),
                    std::move(features),
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
 * @brief Splits MeCab tokens into bunsetsu-like phrase ranges and symbol ranges.
 *
 * This function is a practical bunsetsu approximation built on top of MeCab
 * tokens. MeCab itself does not report bunsetsu boundaries. XER therefore uses
 * simple rules based on the split feature fields: dependent words such as
 * particles and auxiliary verbs attach to the preceding bunsetsu, consecutive
 * nouns stay in the same bunsetsu, a continuative predicate followed by an
 * independent word stays in the same bunsetsu, and symbols are emitted as
 * separate symbol ranges.
 *
 * The returned ranges refer to @p tokens by index and count. They do not own or
 * copy token text.
 *
 * @param tokens MeCab token sequence.
 * @return Bunsetsu-like phrase ranges and symbol ranges.
 */
[[nodiscard]] inline auto mecab_split_phrases(
    std::span<const mecab_token> tokens) -> std::vector<mecab_phrase>
{
    std::vector<mecab_phrase> phrases;

    std::size_t bunsetsu_index = 0;
    std::size_t bunsetsu_count = 0;

    const auto push_bunsetsu = [&]() {
        if (bunsetsu_count != 0) {
            phrases.push_back(mecab_phrase {
                mecab_phrase_kind::bunsetsu,
                bunsetsu_index,
                bunsetsu_count,
            });
            bunsetsu_count = 0;
        }
    };

    std::size_t i = 0;
    while (i < tokens.size()) {
        if (detail::mecab_is_symbol(tokens[i])) {
            push_bunsetsu();

            const std::size_t symbol_index = i;
            do {
                ++i;
            } while (i < tokens.size() && detail::mecab_is_symbol(tokens[i]));

            phrases.push_back(mecab_phrase {
                mecab_phrase_kind::symbol,
                symbol_index,
                i - symbol_index,
            });
            continue;
        }

        if (bunsetsu_count == 0) {
            bunsetsu_index = i;
            bunsetsu_count = 1;
        } else {
            const auto& previous = tokens[bunsetsu_index + bunsetsu_count - 1];
            if (detail::mecab_phrase_should_continue(previous, tokens[i])) {
                ++bunsetsu_count;
            } else {
                push_bunsetsu();
                bunsetsu_index = i;
                bunsetsu_count = 1;
            }
        }

        ++i;
    }

    push_bunsetsu();
    return phrases;
}

/**
 * @brief Parses UTF-8 Japanese text with MeCab and returns raw token results.
 *
 * XER invokes MeCab as a child process and uses an explicit output format that
 * yields one token per line as `surface<TAB>feature`. The returned feature text
 * is MeCab's raw `%H` field and remains dictionary-dependent.
 *
 * The split feature fields are also stored in @ref mecab_token::features. The
 * named members of @ref mecab_features follow common IPADIC-style positions;
 * all fields are preserved in @ref mecab_features::項目 so dictionary-specific
 * feature layouts can still be handled by the caller.
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
