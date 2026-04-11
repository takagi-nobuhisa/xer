/**
 * @file xer/bits/multibyte_string.h
 * @brief Multibyte string conversion functions.
 */

#pragma once

#ifndef XER_BITS_MULTIBYTE_STRING_H_INCLUDED_
#define XER_BITS_MULTIBYTE_STRING_H_INCLUDED_

#include <cstddef>
#include <cstdint>
#include <cwchar>
#include <expected>
#include <type_traits>

#include <xer/bits/advanced_encoding.h>
#include <xer/bits/common.h>
#include <xer/bits/mbstate.h>
#include <xer/error.h>

#if defined(XER_MB_CHAR_UTF8) && defined(XER_MB_CHAR_CP932)
#error "XER_MB_CHAR_UTF8 and XER_MB_CHAR_CP932 cannot both be defined."
#endif

namespace xer::detail {

/**
 * @brief Result of single-character decoding.
 */
struct decoded_char {
    /**
     * @brief Decoded code point.
     */
    char32_t value;

    /**
     * @brief Number of bytes consumed from the current input buffer.
     *
     * When a state object already holds incomplete bytes, this value only counts
     * bytes newly consumed from the current function argument.
     */
    std::size_t bytes;

    /**
     * @brief Whether the decoded character is the null character.
     */
    bool is_null;

    /**
     * @brief Whether the input ended with an incomplete sequence and the state
     * object accepted it.
     */
    bool incomplete;
};

/**
 * @brief Result of single-character encoding.
 */
struct encoded_char {
    /**
     * @brief Encoded bytes.
     */
    std::uint8_t bytes[4];

    /**
     * @brief Number of valid encoded bytes.
     */
    std::size_t size;

    /**
     * @brief Whether the encoded character is the null character.
     */
    bool is_null;
};

/**
 * @brief Creates an unexpected error result.
 *
 * @tparam T Value type of expected.
 * @param code Error code.
 * @return Unexpected error result.
 */
template<typename T>
[[nodiscard]] inline std::expected<T, error<void>> unexpected_error(error_t code) {
    return std::unexpected(make_error(code));
}

/**
 * @brief Returns whether the specified code point is the invalid UTF-32 value.
 *
 * @param value Code point.
 * @return `true` if invalid, otherwise `false`.
 */
[[nodiscard]] constexpr bool is_invalid_utf32(char32_t value) noexcept {
    return value == xer::advanced::detail::invalid_utf32;
}

/**
 * @brief Returns the default encoding for plain `char`.
 *
 * @return Default encoding.
 */
[[nodiscard]] constexpr multibyte_encoding default_char_encoding() noexcept {
#if defined(XER_MB_CHAR_UTF8)
    return multibyte_encoding::utf8;
#elif defined(XER_MB_CHAR_CP932)
    return multibyte_encoding::cp932;
#elif defined(_WIN32)
    return multibyte_encoding::cp932;
#else
    return multibyte_encoding::utf8;
#endif
}

/**
 * @brief Resolves an encoding stored in state or default configuration.
 *
 * @param ps State object.
 * @param fallback Fallback encoding.
 * @return Effective encoding.
 */
[[nodiscard]] inline multibyte_encoding resolve_encoding(
    const xer::mbstate_t* ps,
    multibyte_encoding fallback) noexcept {
    if (ps != nullptr && ps->encoding != multibyte_encoding::unspecified) {
        return ps->encoding;
    }
    return fallback;
}

/**
 * @brief Clears buffered bytes in state.
 *
 * @param ps State object.
 */
inline void clear_state(xer::mbstate_t* ps) noexcept {
    if (ps == nullptr) {
        return;
    }

    ps->size = 0;
    ps->bytes[0] = 0;
    ps->bytes[1] = 0;
    ps->bytes[2] = 0;
    ps->bytes[3] = 0;
}

/**
 * @brief Stores incomplete bytes into state.
 *
 * @param ps State object.
 * @param encoding Encoding kind.
 * @param bytes Source bytes.
 * @param size Number of bytes.
 * @return `true` on success, otherwise `false`.
 */
[[nodiscard]] inline bool store_state(
    xer::mbstate_t* ps,
    multibyte_encoding encoding,
    const std::uint8_t* bytes,
    std::size_t size) noexcept {
    if (ps == nullptr || size > multibyte_state_buffer_size) {
        return false;
    }

    ps->encoding = encoding;
    ps->size = static_cast<std::uint8_t>(size);

    for (std::size_t i = 0; i < size; ++i) {
        ps->bytes[i] = bytes[i];
    }
    for (std::size_t i = size; i < multibyte_state_buffer_size; ++i) {
        ps->bytes[i] = 0;
    }

    return true;
}

/**
 * @brief Checks whether a code point is a Unicode surrogate.
 *
 * @param value Code point.
 * @return `true` if surrogate, otherwise `false`.
 */
[[nodiscard]] constexpr bool is_surrogate(char32_t value) noexcept {
    return value >= static_cast<char32_t>(0xD800) &&
           value <= static_cast<char32_t>(0xDFFF);
}

/**
 * @brief Converts a code point to wchar_t.
 *
 * @param out Output pointer.
 * @param value Code point.
 * @return Success or error.
 */
[[nodiscard]] inline std::expected<void, error<void>> write_tc(wchar_t* out, char32_t value) {
    if (out == nullptr) {
        return {};
    }

    if constexpr (sizeof(wchar_t) == 2) {
        if (value > 0xFFFF || is_surrogate(value) || is_invalid_utf32(value)) {
            return std::unexpected(make_error(error_t::ilseq));
        }
        *out = static_cast<wchar_t>(value);
    } else if constexpr (sizeof(wchar_t) == 4) {
        if (is_invalid_utf32(value)) {
            return std::unexpected(make_error(error_t::ilseq));
        }
        *out = static_cast<wchar_t>(value);
    } else {
        static_assert(sizeof(wchar_t) == 2 || sizeof(wchar_t) == 4, "unsupported wchar_t size");
    }

    return {};
}

/**
 * @brief Converts a code point to char16_t.
 *
 * @param out Output pointer.
 * @param value Code point.
 * @return Success or error.
 */
[[nodiscard]] inline std::expected<void, error<void>> write_tc(char16_t* out, char32_t value) {
    if (out == nullptr) {
        return {};
    }

    if (value > 0xFFFF || is_surrogate(value) || is_invalid_utf32(value)) {
        return std::unexpected(make_error(error_t::ilseq));
    }

    *out = static_cast<char16_t>(value);
    return {};
}

/**
 * @brief Converts a code point to char32_t.
 *
 * @param out Output pointer.
 * @param value Code point.
 * @return Success or error.
 */
[[nodiscard]] inline std::expected<void, error<void>> write_tc(char32_t* out, char32_t value) {
    if (is_invalid_utf32(value)) {
        return std::unexpected(make_error(error_t::ilseq));
    }

    if (out != nullptr) {
        *out = value;
    }
    return {};
}

/**
 * @brief Converts wchar_t to code point.
 *
 * @param value Source value.
 * @return Code point or error.
 */
[[nodiscard]] inline std::expected<char32_t, error<void>> read_tc(wchar_t value) {
    if constexpr (sizeof(wchar_t) == 2) {
        const char32_t cp = static_cast<char32_t>(static_cast<char16_t>(value));
        if (is_surrogate(cp)) {
            return std::unexpected(make_error(error_t::ilseq));
        }
        return cp;
    } else if constexpr (sizeof(wchar_t) == 4) {
        const char32_t cp = static_cast<char32_t>(value);
        if (is_invalid_utf32(cp)) {
            return std::unexpected(make_error(error_t::ilseq));
        }
        return cp;
    } else {
        static_assert(sizeof(wchar_t) == 2 || sizeof(wchar_t) == 4, "unsupported wchar_t size");
    }
}

/**
 * @brief Converts char16_t to code point.
 *
 * @param value Source value.
 * @return Code point or error.
 */
[[nodiscard]] inline std::expected<char32_t, error<void>> read_tc(char16_t value) {
    const char32_t cp = static_cast<char32_t>(value);
    if (is_surrogate(cp)) {
        return std::unexpected(make_error(error_t::ilseq));
    }
    return cp;
}

/**
 * @brief Converts char32_t to validated code point.
 *
 * @param value Source value.
 * @return Code point or error.
 */
[[nodiscard]] inline std::expected<char32_t, error<void>> read_tc(char32_t value) {
    if (is_invalid_utf32(value) || is_surrogate(value)) {
        return std::unexpected(make_error(error_t::ilseq));
    }
    return value;
}

/**
 * @brief Decodes one UTF-8 character.
 *
 * @param bytes Input bytes.
 * @param size Available byte count.
 * @param allow_incomplete Whether incomplete input may be accepted into state.
 * @return Decoded result or error.
 */
[[nodiscard]] inline std::expected<decoded_char, error<void>> decode_utf8_char(
    const std::uint8_t* bytes,
    std::size_t size,
    bool allow_incomplete) {
    if (size == 0) {
        return unexpected_error<decoded_char>(error_t::ilseq);
    }

    const std::uint8_t b0 = bytes[0];

    if (b0 <= 0x7Fu) {
        return decoded_char{
            .value = static_cast<char32_t>(b0),
            .bytes = 1,
            .is_null = (b0 == 0),
            .incomplete = false,
        };
    }

    std::size_t expected_size = 0;
    if (b0 >= 0xC2u && b0 <= 0xDFu) {
        expected_size = 2;
    } else if (b0 >= 0xE0u && b0 <= 0xEFu) {
        expected_size = 3;
    } else if (b0 >= 0xF0u && b0 <= 0xF4u) {
        expected_size = 4;
    } else {
        return unexpected_error<decoded_char>(error_t::ilseq);
    }

    if (size < expected_size) {
        if (allow_incomplete) {
            return decoded_char{
                .value = 0,
                .bytes = 0,
                .is_null = false,
                .incomplete = true,
            };
        }
        return unexpected_error<decoded_char>(error_t::ilseq);
    }

    std::uint32_t packed = 0;
    for (std::size_t i = 0; i < expected_size; ++i) {
        packed |= static_cast<std::uint32_t>(bytes[i]) << (i * 8u);
    }

    const char32_t value = xer::advanced::packed_utf8_to_utf32(packed);
    if (value == xer::advanced::detail::invalid_utf32 || is_surrogate(value)) {
        return unexpected_error<decoded_char>(error_t::ilseq);
    }

    return decoded_char{
        .value = value,
        .bytes = expected_size,
        .is_null = (value == U'\0'),
        .incomplete = false,
    };
}

/**
 * @brief Decodes one CP932 character.
 *
 * @param bytes Input bytes.
 * @param size Available byte count.
 * @param allow_incomplete Whether incomplete input may be accepted into state.
 * @return Decoded result or error.
 */
[[nodiscard]] inline std::expected<decoded_char, error<void>> decode_cp932_char(
    const std::uint8_t* bytes,
    std::size_t size,
    bool allow_incomplete) {
    if (size == 0) {
        return unexpected_error<decoded_char>(error_t::ilseq);
    }

    const std::uint8_t b0 = bytes[0];

    if (b0 <= 0x7Fu) {
        return decoded_char{
            .value = static_cast<char32_t>(b0),
            .bytes = 1,
            .is_null = (b0 == 0),
            .incomplete = false,
        };
    }

    if (b0 >= 0xA1u && b0 <= 0xDFu) {
        const char32_t value = static_cast<char32_t>(0xFF61u + (b0 - 0xA1u));
        return decoded_char{
            .value = value,
            .bytes = 1,
            .is_null = false,
            .incomplete = false,
        };
    }

    const bool is_lead =
        (b0 >= 0x81u && b0 <= 0x9Fu) ||
        (b0 >= 0xE0u && b0 <= 0xFCu);

    if (!is_lead) {
        return unexpected_error<decoded_char>(error_t::ilseq);
    }

    if (size < 2) {
        if (allow_incomplete) {
            return decoded_char{
                .value = 0,
                .bytes = 0,
                .is_null = false,
                .incomplete = true,
            };
        }
        return unexpected_error<decoded_char>(error_t::ilseq);
    }

    const std::uint8_t b1 = bytes[1];
    const bool is_trail =
        (b1 >= 0x40u && b1 <= 0x7Eu) ||
        (b1 >= 0x80u && b1 <= 0xFCu);

    if (!is_trail) {
        return unexpected_error<decoded_char>(error_t::ilseq);
    }

    const std::uint16_t packed =
        static_cast<std::uint16_t>(b0) |
        (static_cast<std::uint16_t>(b1) << 8u);

    const char32_t value = xer::advanced::packed_cp932_to_utf32(packed);
    if (value == xer::advanced::detail::invalid_utf32 || is_surrogate(value)) {
        return unexpected_error<decoded_char>(error_t::ilseq);
    }

    return decoded_char{
        .value = value,
        .bytes = 2,
        .is_null = false,
        .incomplete = false,
    };
}

/**
 * @brief Decodes one character according to the specified encoding.
 *
 * @param bytes Input bytes.
 * @param size Available byte count.
 * @param encoding Source encoding.
 * @param allow_incomplete Whether incomplete input may be accepted into state.
 * @return Decoded result or error.
 */
[[nodiscard]] inline std::expected<decoded_char, error<void>> decode_mb_char(
    const std::uint8_t* bytes,
    std::size_t size,
    multibyte_encoding encoding,
    bool allow_incomplete) {
    switch (encoding) {
    case multibyte_encoding::cp932:
        return decode_cp932_char(bytes, size, allow_incomplete);
    case multibyte_encoding::utf8:
        return decode_utf8_char(bytes, size, allow_incomplete);
    case multibyte_encoding::unspecified:
        return unexpected_error<decoded_char>(error_t::invalid_argument);
    }

    return unexpected_error<decoded_char>(error_t::invalid_argument);
}

/**
 * @brief Encodes one code point to UTF-8.
 *
 * @param value Source code point.
 * @return Encoded result or error.
 */
[[nodiscard]] inline std::expected<encoded_char, error<void>> encode_utf8_mb_char(char32_t value) {
    if (is_invalid_utf32(value) || is_surrogate(value)) {
        return unexpected_error<encoded_char>(error_t::ilseq);
    }

    const std::uint32_t packed = xer::advanced::utf32_to_packed_utf8(value);
    if (packed == xer::advanced::detail::invalid_packed_utf8) {
        return unexpected_error<encoded_char>(error_t::ilseq);
    }

    encoded_char result{};
    result.bytes[0] = static_cast<std::uint8_t>(packed & 0xFFu);
    result.bytes[1] = static_cast<std::uint8_t>((packed >> 8u) & 0xFFu);
    result.bytes[2] = static_cast<std::uint8_t>((packed >> 16u) & 0xFFu);
    result.bytes[3] = static_cast<std::uint8_t>((packed >> 24u) & 0xFFu);

    result.size = 1;
    if (result.bytes[1] != 0) {
        result.size = 2;
    }
    if (result.bytes[2] != 0) {
        result.size = 3;
    }
    if (result.bytes[3] != 0) {
        result.size = 4;
    }

    result.is_null = (value == U'\0');
    return result;
}

/**
 * @brief Encodes one code point to CP932.
 *
 * @param value Source code point.
 * @return Encoded result or error.
 */
[[nodiscard]] inline std::expected<encoded_char, error<void>> encode_cp932_char(char32_t value) {
    if (is_invalid_utf32(value) || is_surrogate(value)) {
        return unexpected_error<encoded_char>(error_t::ilseq);
    }

    encoded_char result{};

    if (value <= 0x7Fu) {
        result.bytes[0] = static_cast<std::uint8_t>(value);
        result.size = 1;
        result.is_null = (value == U'\0');
        return result;
    }

    if (value >= 0xFF61u && value <= 0xFF9Fu) {
        result.bytes[0] = static_cast<std::uint8_t>(0xA1u + (value - 0xFF61u));
        result.size = 1;
        result.is_null = false;
        return result;
    }

    const std::int32_t packed = xer::advanced::utf32_to_packed_cp932(value);
    if (packed < 0) {
        return unexpected_error<encoded_char>(error_t::ilseq);
    }

    result.bytes[0] = static_cast<std::uint8_t>(packed & 0xFFu);
    result.bytes[1] = static_cast<std::uint8_t>((packed >> 8u) & 0xFFu);
    result.size = 2;
    result.is_null = false;
    return result;
}

/**
 * @brief Encodes one code point according to the specified encoding.
 *
 * @param value Source code point.
 * @param encoding Target encoding.
 * @return Encoded result or error.
 */
[[nodiscard]] inline std::expected<encoded_char, error<void>> encode_mb_char(
    char32_t value,
    multibyte_encoding encoding) {
    switch (encoding) {
    case multibyte_encoding::cp932:
        return encode_cp932_char(value);
    case multibyte_encoding::utf8:
        return encode_utf8_mb_char(value);
    case multibyte_encoding::unspecified:
        return unexpected_error<encoded_char>(error_t::invalid_argument);
    }

    return unexpected_error<encoded_char>(error_t::invalid_argument);
}

/**
 * @brief Converts any supported multibyte input pointer to byte pointer.
 *
 * @tparam CharType Source character type.
 * @param s Source pointer.
 * @return Byte pointer.
 */
template<typename CharType>
[[nodiscard]] inline const std::uint8_t* as_bytes(const CharType* s) noexcept {
    return reinterpret_cast<const std::uint8_t*>(s);
}

/**
 * @brief Converts any supported output pointer to byte pointer.
 *
 * @tparam CharType Destination character type.
 * @param s Destination pointer.
 * @return Byte pointer.
 */
template<typename CharType>
[[nodiscard]] inline std::uint8_t* as_bytes(CharType* s) noexcept {
    return reinterpret_cast<std::uint8_t*>(s);
}

/**
 * @brief Shared implementation of mbtotc.
 *
 * @tparam TcType Output text character type.
 * @tparam MbType Input multibyte element type.
 * @param out Output pointer.
 * @param s Input string.
 * @param n Maximum number of bytes.
 * @param encoding Source encoding.
 * @param ps State object.
 * @return Consumed byte count or error.
 */
template<typename TcType, typename MbType>
[[nodiscard]] inline std::expected<std::size_t, error<void>> mbtotc_impl(
    TcType* out,
    const MbType* s,
    std::size_t n,
    multibyte_encoding encoding,
    xer::mbstate_t* ps) {
    if (s == nullptr) {
        return unexpected_error<std::size_t>(error_t::invalid_argument);
    }

    const multibyte_encoding effective_encoding = resolve_encoding(ps, encoding);

    std::uint8_t buffer[multibyte_state_buffer_size] = {0, 0, 0, 0};
    std::size_t buffered = 0;

    if (ps != nullptr && !ps->empty()) {
        buffered = ps->size;
        for (std::size_t i = 0; i < buffered; ++i) {
            buffer[i] = ps->bytes[i];
        }
    }

    const std::size_t copy_bytes =
        (n + buffered > multibyte_state_buffer_size)
            ? (multibyte_state_buffer_size - buffered)
            : n;

    const std::uint8_t* input = as_bytes(s);
    for (std::size_t i = 0; i < copy_bytes; ++i) {
        buffer[buffered + i] = input[i];
    }

    const std::size_t available = buffered + copy_bytes;
    auto decoded = decode_mb_char(buffer, available, effective_encoding, ps != nullptr);
    if (!decoded.has_value()) {
        clear_state(ps);
        return std::unexpected(decoded.error());
    }

    if (decoded->incomplete) {
        if (!store_state(ps, effective_encoding, buffer, available)) {
            clear_state(ps);
            return unexpected_error<std::size_t>(error_t::ilseq);
        }
        return std::size_t{0};
    }

    clear_state(ps);

    auto written = write_tc(out, decoded->value);
    if (!written.has_value()) {
        return std::unexpected(written.error());
    }

    const std::size_t consumed_from_current =
        decoded->bytes > buffered ? (decoded->bytes - buffered) : 0;

    return consumed_from_current;
}

/**
 * @brief Shared implementation of tctomb.
 *
 * @tparam MbType Output multibyte element type.
 * @tparam TcType Input text character type.
 * @param out Output buffer.
 * @param n Output buffer size in bytes.
 * @param value Source text character.
 * @param encoding Target encoding.
 * @param ps State object.
 * @return Written byte count or error.
 */
template<typename MbType, typename TcType>
[[nodiscard]] inline std::expected<std::size_t, error<void>> tctomb_impl(
    MbType* out,
    std::size_t n,
    TcType value,
    multibyte_encoding encoding,
    xer::mbstate_t* ps) {
    const auto cp = read_tc(value);
    if (!cp.has_value()) {
        return std::unexpected(cp.error());
    }

    const multibyte_encoding effective_encoding = resolve_encoding(ps, encoding);
    auto encoded = encode_mb_char(*cp, effective_encoding);
    if (!encoded.has_value()) {
        clear_state(ps);
        return std::unexpected(encoded.error());
    }

    clear_state(ps);

    if (out == nullptr) {
        return encoded->size;
    }

    if (n < encoded->size) {
        return unexpected_error<std::size_t>(error_t::overflow_error);
    }

    std::uint8_t* bytes = as_bytes(out);
    for (std::size_t i = 0; i < encoded->size; ++i) {
        bytes[i] = encoded->bytes[i];
    }

    return encoded->size;
}

/**
 * @brief Shared implementation of mbstotcs.
 *
 * @tparam TcType Output text character type.
 * @tparam MbType Input multibyte element type.
 * @param out Output buffer.
 * @param s Input string.
 * @param n Output capacity in characters.
 * @param encoding Source encoding.
 * @param ps State object.
 * @return Consumed byte count or error.
 */
template<typename TcType, typename MbType>
[[nodiscard]] inline std::expected<std::size_t, error<void>> mbstotcs_impl(
    TcType* out,
    const MbType* s,
    std::size_t n,
    multibyte_encoding encoding,
    xer::mbstate_t* ps) {
    if (s == nullptr) {
        return unexpected_error<std::size_t>(error_t::invalid_argument);
    }

    std::size_t total_bytes = 0;
    std::size_t out_index = 0;
    const MbType* current = s;

    while (true) {
        char32_t cp = U'\0';
        auto decoded = mbtotc_impl(&cp, current, multibyte_state_buffer_size, encoding, ps);
        if (!decoded.has_value()) {
            return std::unexpected(decoded.error());
        }

        if (*decoded == 0) {
            return total_bytes;
        }

        if (cp == U'\0') {
            return total_bytes;
        }

        if (out == nullptr) {
            total_bytes += *decoded;
            current += *decoded;
            continue;
        }

        if (out_index >= n) {
            return unexpected_error<std::size_t>(error_t::overflow_error);
        }

        auto written = write_tc(&out[out_index], cp);
        if (!written.has_value()) {
            return std::unexpected(written.error());
        }

        total_bytes += *decoded;
        current += *decoded;
        ++out_index;
    }
}

/**
 * @brief Shared implementation of tcstombs.
 *
 * @tparam MbType Output multibyte element type.
 * @tparam TcType Input text character type.
 * @param out Output buffer.
 * @param n Output capacity in bytes.
 * @param s Input text-character string.
 * @param encoding Target encoding.
 * @param ps State object.
 * @return Written byte count or error.
 */
template<typename MbType, typename TcType>
[[nodiscard]] inline std::expected<std::size_t, error<void>> tcstombs_impl(
    MbType* out,
    std::size_t n,
    const TcType* s,
    multibyte_encoding encoding,
    xer::mbstate_t* ps) {
    if (s == nullptr) {
        return unexpected_error<std::size_t>(error_t::invalid_argument);
    }

    std::size_t total_bytes = 0;
    std::size_t index = 0;

    while (true) {
        const auto cp = read_tc(s[index]);
        if (!cp.has_value()) {
            return std::unexpected(cp.error());
        }

        if (*cp == U'\0') {
            clear_state(ps);

            if (out != nullptr) {
                if (total_bytes >= n) {
                    return unexpected_error<std::size_t>(error_t::overflow_error);
                }
                as_bytes(out)[total_bytes] = 0;
            }

            return total_bytes;
        }

        auto encoded = encode_mb_char(*cp, resolve_encoding(ps, encoding));
        if (!encoded.has_value()) {
            clear_state(ps);
            return std::unexpected(encoded.error());
        }

        clear_state(ps);

        if (out == nullptr) {
            total_bytes += encoded->size;
            ++index;
            continue;
        }

        if (total_bytes + encoded->size >= n) {
            return unexpected_error<std::size_t>(error_t::overflow_error);
        }

        std::uint8_t* bytes = as_bytes(out);
        for (std::size_t i = 0; i < encoded->size; ++i) {
            bytes[total_bytes + i] = encoded->bytes[i];
        }

        total_bytes += encoded->size;
        ++index;
    }
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Returns the length in bytes of the first multibyte character.
 *
 * @param s Input string.
 * @param n Maximum number of input bytes.
 * @return Consumed byte count or error.
 */
[[nodiscard]] inline std::expected<std::size_t, error<void>> mblen(
    const char* s,
    std::size_t n) {
    return detail::mbtotc_impl(
        static_cast<char32_t*>(nullptr),
        s,
        n,
        detail::default_char_encoding(),
        nullptr);
}

/**
 * @brief Returns the length in bytes of the first multibyte character.
 *
 * @param s Input string.
 * @param n Maximum number of input bytes.
 * @return Consumed byte count or error.
 */
[[nodiscard]] inline std::expected<std::size_t, error<void>> mblen(
    const unsigned char* s,
    std::size_t n) {
    return detail::mbtotc_impl(
        static_cast<char32_t*>(nullptr),
        s,
        n,
        detail::multibyte_encoding::cp932,
        nullptr);
}

/**
 * @brief Returns the length in bytes of the first multibyte character.
 *
 * @param s Input string.
 * @param n Maximum number of input bytes.
 * @return Consumed byte count or error.
 */
[[nodiscard]] inline std::expected<std::size_t, error<void>> mblen(
    const char8_t* s,
    std::size_t n) {
    return detail::mbtotc_impl(
        static_cast<char32_t*>(nullptr),
        s,
        n,
        detail::multibyte_encoding::utf8,
        nullptr);
}

/**
 * @brief Returns the length in bytes of the first multibyte character.
 *
 * @param s Input string.
 * @param n Maximum number of input bytes.
 * @param ps State object.
 * @return Consumed byte count or error.
 */
[[nodiscard]] inline std::expected<std::size_t, error<void>> mblen(
    const char* s,
    std::size_t n,
    mbstate_t* ps) {
    return detail::mbtotc_impl(
        static_cast<char32_t*>(nullptr),
        s,
        n,
        detail::default_char_encoding(),
        ps);
}

/**
 * @brief Returns the length in bytes of the first multibyte character.
 *
 * @param s Input string.
 * @param n Maximum number of input bytes.
 * @param ps State object.
 * @return Consumed byte count or error.
 */
[[nodiscard]] inline std::expected<std::size_t, error<void>> mblen(
    const unsigned char* s,
    std::size_t n,
    mbstate_t* ps) {
    return detail::mbtotc_impl(
        static_cast<char32_t*>(nullptr),
        s,
        n,
        detail::multibyte_encoding::cp932,
        ps);
}

/**
 * @brief Returns the length in bytes of the first multibyte character.
 *
 * @param s Input string.
 * @param n Maximum number of input bytes.
 * @param ps State object.
 * @return Consumed byte count or error.
 */
[[nodiscard]] inline std::expected<std::size_t, error<void>> mblen(
    const char8_t* s,
    std::size_t n,
    mbstate_t* ps) {
    return detail::mbtotc_impl(
        static_cast<char32_t*>(nullptr),
        s,
        n,
        detail::multibyte_encoding::utf8,
        ps);
}

#define XER_DEFINE_MBTOTC_FOR_TC(tc_type) \
    [[nodiscard]] inline std::expected<std::size_t, error<void>> mbtotc( \
        tc_type* out, \
        const char* s, \
        std::size_t n) { \
        return detail::mbtotc_impl(out, s, n, detail::default_char_encoding(), nullptr); \
    } \
    [[nodiscard]] inline std::expected<std::size_t, error<void>> mbtotc( \
        tc_type* out, \
        const unsigned char* s, \
        std::size_t n) { \
        return detail::mbtotc_impl(out, s, n, detail::multibyte_encoding::cp932, nullptr); \
    } \
    [[nodiscard]] inline std::expected<std::size_t, error<void>> mbtotc( \
        tc_type* out, \
        const char8_t* s, \
        std::size_t n) { \
        return detail::mbtotc_impl(out, s, n, detail::multibyte_encoding::utf8, nullptr); \
    } \
    [[nodiscard]] inline std::expected<std::size_t, error<void>> mbtotc( \
        tc_type* out, \
        const char* s, \
        std::size_t n, \
        mbstate_t* ps) { \
        return detail::mbtotc_impl(out, s, n, detail::default_char_encoding(), ps); \
    } \
    [[nodiscard]] inline std::expected<std::size_t, error<void>> mbtotc( \
        tc_type* out, \
        const unsigned char* s, \
        std::size_t n, \
        mbstate_t* ps) { \
        return detail::mbtotc_impl(out, s, n, detail::multibyte_encoding::cp932, ps); \
    } \
    [[nodiscard]] inline std::expected<std::size_t, error<void>> mbtotc( \
        tc_type* out, \
        const char8_t* s, \
        std::size_t n, \
        mbstate_t* ps) { \
        return detail::mbtotc_impl(out, s, n, detail::multibyte_encoding::utf8, ps); \
    }

XER_DEFINE_MBTOTC_FOR_TC(wchar_t)
XER_DEFINE_MBTOTC_FOR_TC(char16_t)
XER_DEFINE_MBTOTC_FOR_TC(char32_t)

#undef XER_DEFINE_MBTOTC_FOR_TC

#define XER_DEFINE_TCTOMB_FOR_TC(tc_type) \
    [[nodiscard]] inline std::expected<std::size_t, error<void>> tctomb( \
        char* out, \
        std::size_t n, \
        tc_type value) { \
        return detail::tctomb_impl(out, n, value, detail::default_char_encoding(), nullptr); \
    } \
    [[nodiscard]] inline std::expected<std::size_t, error<void>> tctomb( \
        unsigned char* out, \
        std::size_t n, \
        tc_type value) { \
        return detail::tctomb_impl(out, n, value, detail::multibyte_encoding::cp932, nullptr); \
    } \
    [[nodiscard]] inline std::expected<std::size_t, error<void>> tctomb( \
        char8_t* out, \
        std::size_t n, \
        tc_type value) { \
        return detail::tctomb_impl(out, n, value, detail::multibyte_encoding::utf8, nullptr); \
    } \
    [[nodiscard]] inline std::expected<std::size_t, error<void>> tctomb( \
        char* out, \
        std::size_t n, \
        tc_type value, \
        mbstate_t* ps) { \
        return detail::tctomb_impl(out, n, value, detail::default_char_encoding(), ps); \
    } \
    [[nodiscard]] inline std::expected<std::size_t, error<void>> tctomb( \
        unsigned char* out, \
        std::size_t n, \
        tc_type value, \
        mbstate_t* ps) { \
        return detail::tctomb_impl(out, n, value, detail::multibyte_encoding::cp932, ps); \
    } \
    [[nodiscard]] inline std::expected<std::size_t, error<void>> tctomb( \
        char8_t* out, \
        std::size_t n, \
        tc_type value, \
        mbstate_t* ps) { \
        return detail::tctomb_impl(out, n, value, detail::multibyte_encoding::utf8, ps); \
    }

XER_DEFINE_TCTOMB_FOR_TC(wchar_t)
XER_DEFINE_TCTOMB_FOR_TC(char16_t)
XER_DEFINE_TCTOMB_FOR_TC(char32_t)

#undef XER_DEFINE_TCTOMB_FOR_TC

#define XER_DEFINE_MBSTOTCS_FOR_TC(tc_type) \
    [[nodiscard]] inline std::expected<std::size_t, error<void>> mbstotcs( \
        tc_type* out, \
        const char* s, \
        std::size_t n) { \
        return detail::mbstotcs_impl(out, s, n, detail::default_char_encoding(), nullptr); \
    } \
    [[nodiscard]] inline std::expected<std::size_t, error<void>> mbstotcs( \
        tc_type* out, \
        const unsigned char* s, \
        std::size_t n) { \
        return detail::mbstotcs_impl(out, s, n, detail::multibyte_encoding::cp932, nullptr); \
    } \
    [[nodiscard]] inline std::expected<std::size_t, error<void>> mbstotcs( \
        tc_type* out, \
        const char8_t* s, \
        std::size_t n) { \
        return detail::mbstotcs_impl(out, s, n, detail::multibyte_encoding::utf8, nullptr); \
    } \
    [[nodiscard]] inline std::expected<std::size_t, error<void>> mbstotcs( \
        tc_type* out, \
        const char* s, \
        std::size_t n, \
        mbstate_t* ps) { \
        return detail::mbstotcs_impl(out, s, n, detail::default_char_encoding(), ps); \
    } \
    [[nodiscard]] inline std::expected<std::size_t, error<void>> mbstotcs( \
        tc_type* out, \
        const unsigned char* s, \
        std::size_t n, \
        mbstate_t* ps) { \
        return detail::mbstotcs_impl(out, s, n, detail::multibyte_encoding::cp932, ps); \
    } \
    [[nodiscard]] inline std::expected<std::size_t, error<void>> mbstotcs( \
        tc_type* out, \
        const char8_t* s, \
        std::size_t n, \
        mbstate_t* ps) { \
        return detail::mbstotcs_impl(out, s, n, detail::multibyte_encoding::utf8, ps); \
    }

XER_DEFINE_MBSTOTCS_FOR_TC(wchar_t)
XER_DEFINE_MBSTOTCS_FOR_TC(char16_t)
XER_DEFINE_MBSTOTCS_FOR_TC(char32_t)

#undef XER_DEFINE_MBSTOTCS_FOR_TC

#define XER_DEFINE_TCSTOMBS_FOR_TC(tc_type) \
    [[nodiscard]] inline std::expected<std::size_t, error<void>> tcstombs( \
        char* out, \
        std::size_t n, \
        const tc_type* s) { \
        return detail::tcstombs_impl(out, n, s, detail::default_char_encoding(), nullptr); \
    } \
    [[nodiscard]] inline std::expected<std::size_t, error<void>> tcstombs( \
        unsigned char* out, \
        std::size_t n, \
        const tc_type* s) { \
        return detail::tcstombs_impl(out, n, s, detail::multibyte_encoding::cp932, nullptr); \
    } \
    [[nodiscard]] inline std::expected<std::size_t, error<void>> tcstombs( \
        char8_t* out, \
        std::size_t n, \
        const tc_type* s) { \
        return detail::tcstombs_impl(out, n, s, detail::multibyte_encoding::utf8, nullptr); \
    } \
    [[nodiscard]] inline std::expected<std::size_t, error<void>> tcstombs( \
        char* out, \
        std::size_t n, \
        const tc_type* s, \
        mbstate_t* ps) { \
        return detail::tcstombs_impl(out, n, s, detail::default_char_encoding(), ps); \
    } \
    [[nodiscard]] inline std::expected<std::size_t, error<void>> tcstombs( \
        unsigned char* out, \
        std::size_t n, \
        const tc_type* s, \
        mbstate_t* ps) { \
        return detail::tcstombs_impl(out, n, s, detail::multibyte_encoding::cp932, ps); \
    } \
    [[nodiscard]] inline std::expected<std::size_t, error<void>> tcstombs( \
        char8_t* out, \
        std::size_t n, \
        const tc_type* s, \
        mbstate_t* ps) { \
        return detail::tcstombs_impl(out, n, s, detail::multibyte_encoding::utf8, ps); \
    }

XER_DEFINE_TCSTOMBS_FOR_TC(wchar_t)
XER_DEFINE_TCSTOMBS_FOR_TC(char16_t)
XER_DEFINE_TCSTOMBS_FOR_TC(char32_t)

#undef XER_DEFINE_TCSTOMBS_FOR_TC

} // namespace xer

#endif // XER_BITS_MULTIBYTE_STRING_H_INCLUDED_
