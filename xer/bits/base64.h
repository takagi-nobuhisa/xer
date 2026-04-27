/**
 * @file xer/bits/base64.h
 * @brief Internal Base64 encode/decode implementation.
 */

#pragma once

#ifndef XER_BITS_BASE64_H_INCLUDED_
#define XER_BITS_BASE64_H_INCLUDED_

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <xer/error.h>

namespace xer::detail {

inline constexpr std::u8string_view base64_alphabet =
    u8"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

[[nodiscard]] constexpr auto base64_is_ascii_space(char8_t c) noexcept -> bool
{
    return c == u8' ' || c == u8'\t' || c == u8'\n' || c == u8'\r' ||
           c == u8'\f' || c == u8'\v';
}

[[nodiscard]] constexpr auto base64_decode_value(char8_t c) noexcept -> int
{
    if (c >= u8'A' && c <= u8'Z') {
        return static_cast<int>(c - u8'A');
    }

    if (c >= u8'a' && c <= u8'z') {
        return static_cast<int>(c - u8'a') + 26;
    }

    if (c >= u8'0' && c <= u8'9') {
        return static_cast<int>(c - u8'0') + 52;
    }

    if (c == u8'+') {
        return 62;
    }

    if (c == u8'/') {
        return 63;
    }

    return -1;
}

[[nodiscard]] inline auto base64_strip_ascii_space(std::u8string_view text)
    -> result<std::u8string>
{
    std::u8string stripped;
    stripped.reserve(text.size());

    for (const char8_t c : text) {
        if (base64_is_ascii_space(c)) {
            continue;
        }

        stripped.push_back(c);
    }

    return stripped;
}

[[nodiscard]] inline auto base64_decode_quartet(
    const std::array<char8_t, 4>& quartet,
    std::vector<std::byte>& out,
    bool is_final) -> result<void>
{
    std::size_t padding = 0;
    if (quartet[3] == u8'=') {
        ++padding;
    }
    if (quartet[2] == u8'=') {
        ++padding;
    }

    if (padding != 0 && !is_final) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (quartet[0] == u8'=' || quartet[1] == u8'=') {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (quartet[2] == u8'=' && quartet[3] != u8'=') {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const int v0 = base64_decode_value(quartet[0]);
    const int v1 = base64_decode_value(quartet[1]);
    const int v2 = quartet[2] == u8'=' ? 0 : base64_decode_value(quartet[2]);
    const int v3 = quartet[3] == u8'=' ? 0 : base64_decode_value(quartet[3]);

    if (v0 < 0 || v1 < 0 || v2 < 0 || v3 < 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (padding == 2 && (v1 & 0x0F) != 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (padding == 1 && (v2 & 0x03) != 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const auto b0 = static_cast<std::uint8_t>((v0 << 2) | (v1 >> 4));
    out.push_back(static_cast<std::byte>(b0));

    if (padding < 2) {
        const auto b1 =
            static_cast<std::uint8_t>(((v1 & 0x0F) << 4) | (v2 >> 2));
        out.push_back(static_cast<std::byte>(b1));
    }

    if (padding < 1) {
        const auto b2 = static_cast<std::uint8_t>(((v2 & 0x03) << 6) | v3);
        out.push_back(static_cast<std::byte>(b2));
    }

    return {};
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Encodes binary data into standard Base64 text.
 *
 * The current implementation uses the standard Base64 alphabet
 * `A-Z`, `a-z`, `0-9`, `+`, and `/`. Padding with `=` is always emitted.
 * Line breaks are not inserted.
 *
 * The function returns `xer::result` so that future variants and output
 * policies can report ordinary failures without changing the public shape of
 * the API.
 *
 * @param data Binary data to encode.
 * @return Base64 text on success.
 */
[[nodiscard]] inline auto base64_encode(std::span<const std::byte> data)
    -> result<std::u8string>
{
    std::u8string out;
    const std::size_t max_size = out.max_size();
    if (data.size() > (max_size / 4u) * 3u) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    out.reserve(((data.size() + 2u) / 3u) * 4u);

    for (std::size_t i = 0; i < data.size(); i += 3u) {
        const auto b0 = static_cast<std::uint8_t>(data[i]);
        const bool has_b1 = i + 1u < data.size();
        const bool has_b2 = i + 2u < data.size();
        const auto b1 =
            has_b1 ? static_cast<std::uint8_t>(data[i + 1u]) : 0u;
        const auto b2 =
            has_b2 ? static_cast<std::uint8_t>(data[i + 2u]) : 0u;

        out.push_back(detail::base64_alphabet[b0 >> 2]);
        out.push_back(
            detail::base64_alphabet[((b0 & 0x03u) << 4) | (b1 >> 4)]);
        out.push_back(
            has_b1
                ? detail::base64_alphabet[((b1 & 0x0Fu) << 2) | (b2 >> 6)]
                : u8'=');
        out.push_back(has_b2 ? detail::base64_alphabet[b2 & 0x3Fu] : u8'=');
    }

    return out;
}

/**
 * @brief Decodes standard Base64 text into binary data.
 *
 * ASCII whitespace in the input is ignored. All other characters must belong
 * to the standard Base64 alphabet or be valid `=` padding. Padding must appear
 * only at the end, and the number of effective input characters must be a
 * multiple of four.
 *
 * @param text Base64 text to decode.
 * @return Decoded binary data on success.
 */
[[nodiscard]] inline auto base64_decode(std::u8string_view text)
    -> result<std::vector<std::byte>>
{
    const auto stripped_result = detail::base64_strip_ascii_space(text);
    if (!stripped_result.has_value()) {
        return std::unexpected(stripped_result.error());
    }

    const std::u8string& stripped = *stripped_result;
    if (stripped.empty()) {
        return std::vector<std::byte>{};
    }

    if (stripped.size() % 4u != 0u) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::vector<std::byte> out;
    const std::size_t max_size = out.max_size();
    if (stripped.size() / 4u > max_size / 3u) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    out.reserve((stripped.size() / 4u) * 3u);

    for (std::size_t i = 0; i < stripped.size(); i += 4u) {
        const std::array<char8_t, 4> quartet = {
            stripped[i],
            stripped[i + 1u],
            stripped[i + 2u],
            stripped[i + 3u],
        };
        const bool is_final = i + 4u == stripped.size();

        auto decoded = detail::base64_decode_quartet(quartet, out, is_final);
        if (!decoded.has_value()) {
            return std::unexpected(decoded.error());
        }
    }

    return out;
}

} // namespace xer

#endif /* XER_BITS_BASE64_H_INCLUDED_ */
