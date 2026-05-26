/**
 * @file xer/bits/unicode_normalize.h
 * @brief ICU-based Unicode normalization implementation.
 */

#pragma once

#ifndef XER_BITS_UNICODE_NORMALIZE_H_INCLUDED_
#define XER_BITS_UNICODE_NORMALIZE_H_INCLUDED_

#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#if defined(__has_include)
#    if __has_include(<unicode/utypes.h>) && \
        __has_include(<unicode/ustring.h>) && \
        __has_include(<unicode/unorm2.h>)
#        include <unicode/utypes.h>
#        include <unicode/ustring.h>
#        include <unicode/unorm2.h>
#    else
#        error "xer/unicode.h requires ICU C API headers: <unicode/utypes.h>, <unicode/ustring.h>, and <unicode/unorm2.h>."
#    endif
#else
#    include <unicode/utypes.h>
#    include <unicode/ustring.h>
#    include <unicode/unorm2.h>
#endif

#include <xer/error.h>

namespace xer {

namespace detail {

[[nodiscard]] inline auto size_to_icu_length(std::size_t size) noexcept
    -> result<std::int32_t>
{
    if (size > static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max())) {
        return std::unexpected(make_error(error_t::length_error));
    }

    return static_cast<std::int32_t>(size);
}

[[nodiscard]] inline auto icu_status_to_error(UErrorCode status) noexcept -> error_t
{
    if (status == U_INVALID_CHAR_FOUND || status == U_TRUNCATED_CHAR_FOUND ||
        status == U_ILLEGAL_CHAR_FOUND) {
        return error_t::encoding_error;
    }

    if (status == U_BUFFER_OVERFLOW_ERROR || status == U_STRING_NOT_TERMINATED_WARNING) {
        return error_t::length_error;
    }

    return error_t::runtime_error;
}

[[nodiscard]] inline auto utf8_to_icu_utf16(std::u8string_view text)
    -> result<std::vector<UChar>>
{
    const auto input_length_result = size_to_icu_length(text.size());
    if (!input_length_result.has_value()) {
        return std::unexpected(input_length_result.error());
    }

    const char* const input = reinterpret_cast<const char*>(text.data());
    const std::int32_t input_length = *input_length_result;

    UErrorCode status = U_ZERO_ERROR;
    std::int32_t required_length = 0;
    u_strFromUTF8(
        nullptr,
        0,
        &required_length,
        input,
        input_length,
        &status);

    if (status != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(status)) {
        return std::unexpected(make_error(icu_status_to_error(status)));
    }

    std::vector<UChar> result(static_cast<std::size_t>(required_length));
    status = U_ZERO_ERROR;
    std::int32_t actual_length = 0;
    u_strFromUTF8(
        result.data(),
        required_length,
        &actual_length,
        input,
        input_length,
        &status);

    if (U_FAILURE(status)) {
        return std::unexpected(make_error(icu_status_to_error(status)));
    }

    result.resize(static_cast<std::size_t>(actual_length));
    return result;
}

[[nodiscard]] inline auto icu_utf16_to_utf8(std::span<const UChar> text)
    -> result<std::u8string>
{
    const auto input_length_result = size_to_icu_length(text.size());
    if (!input_length_result.has_value()) {
        return std::unexpected(input_length_result.error());
    }

    const std::int32_t input_length = *input_length_result;

    UErrorCode status = U_ZERO_ERROR;
    std::int32_t required_length = 0;
    u_strToUTF8(
        nullptr,
        0,
        &required_length,
        text.data(),
        input_length,
        &status);

    if (status != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(status)) {
        return std::unexpected(make_error(icu_status_to_error(status)));
    }

    std::string buffer(static_cast<std::size_t>(required_length), '\0');
    status = U_ZERO_ERROR;
    std::int32_t actual_length = 0;
    u_strToUTF8(
        buffer.data(),
        required_length,
        &actual_length,
        text.data(),
        input_length,
        &status);

    if (U_FAILURE(status)) {
        return std::unexpected(make_error(icu_status_to_error(status)));
    }

    std::u8string result;
    result.resize(static_cast<std::size_t>(actual_length));
    for (std::size_t i = 0; i < result.size(); ++i) {
        result[i] = static_cast<char8_t>(static_cast<unsigned char>(buffer[i]));
    }

    return result;
}

[[nodiscard]] inline auto get_icu_nfc_instance() noexcept -> result<const UNormalizer2*>
{
    UErrorCode status = U_ZERO_ERROR;
    const UNormalizer2* const normalizer = unorm2_getNFCInstance(&status);
    if (U_FAILURE(status) || normalizer == nullptr) {
        return std::unexpected(make_error(icu_status_to_error(status)));
    }

    return normalizer;
}

} // namespace detail

/**
 * @brief Normalizes UTF-8 text to Unicode Normalization Form C.
 * @param text Source UTF-8 text.
 * @return NFC-normalized UTF-8 text, or an error if the input is invalid UTF-8
 *         or ICU reports a failure.
 */
[[nodiscard]] inline auto normalize_nfc(std::u8string_view text)
    -> result<std::u8string>
{
    const auto normalizer_result = detail::get_icu_nfc_instance();
    if (!normalizer_result.has_value()) {
        return std::unexpected(normalizer_result.error());
    }

    const auto source_result = detail::utf8_to_icu_utf16(text);
    if (!source_result.has_value()) {
        return std::unexpected(source_result.error());
    }

    const std::vector<UChar>& source = *source_result;
    const auto source_length_result = detail::size_to_icu_length(source.size());
    if (!source_length_result.has_value()) {
        return std::unexpected(source_length_result.error());
    }

    const UNormalizer2* const normalizer = *normalizer_result;
    const std::int32_t source_length = *source_length_result;

    UErrorCode status = U_ZERO_ERROR;
    const std::int32_t required_length = unorm2_normalize(
        normalizer,
        source.data(),
        source_length,
        nullptr,
        0,
        &status);

    if (status != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(status)) {
        return std::unexpected(make_error(detail::icu_status_to_error(status)));
    }

    std::vector<UChar> normalized(static_cast<std::size_t>(required_length));
    status = U_ZERO_ERROR;
    const std::int32_t actual_length = unorm2_normalize(
        normalizer,
        source.data(),
        source_length,
        normalized.data(),
        required_length,
        &status);

    if (U_FAILURE(status)) {
        return std::unexpected(make_error(detail::icu_status_to_error(status)));
    }

    normalized.resize(static_cast<std::size_t>(actual_length));
    return detail::icu_utf16_to_utf8(normalized);
}

/**
 * @brief Tests whether UTF-8 text is already normalized to Unicode NFC.
 * @param text Source UTF-8 text.
 * @return `true` if the text is already NFC, `false` if not, or an error if
 *         the input is invalid UTF-8 or ICU reports a failure.
 */
[[nodiscard]] inline auto is_normalized_nfc(std::u8string_view text)
    -> result<bool>
{
    const auto normalizer_result = detail::get_icu_nfc_instance();
    if (!normalizer_result.has_value()) {
        return std::unexpected(normalizer_result.error());
    }

    const auto source_result = detail::utf8_to_icu_utf16(text);
    if (!source_result.has_value()) {
        return std::unexpected(source_result.error());
    }

    const std::vector<UChar>& source = *source_result;
    const auto source_length_result = detail::size_to_icu_length(source.size());
    if (!source_length_result.has_value()) {
        return std::unexpected(source_length_result.error());
    }

    UErrorCode status = U_ZERO_ERROR;
    const UBool normalized = unorm2_isNormalized(
        *normalizer_result,
        source.data(),
        *source_length_result,
        &status);

    if (U_FAILURE(status)) {
        return std::unexpected(make_error(detail::icu_status_to_error(status)));
    }

    return normalized != 0;
}

} // namespace xer

#endif /* XER_BITS_UNICODE_NORMALIZE_H_INCLUDED_ */
