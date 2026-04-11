/**
 * @file xer/bits/standard_streams.h
 * @brief Standard text stream objects.
 */

#pragma once

#ifndef XER_BITS_STANDARD_STREAMS_H_INCLUDED_
#define XER_BITS_STANDARD_STREAMS_H_INCLUDED_

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <new>

#include <xer/bits/advanced_encoding.h>
#include <xer/bits/fopen.h>
#include <xer/bits/text_stream.h>

namespace xer::detail {

/**
 * @brief Closes and destroys a non-owning standard text stream state.
 *
 * The underlying FILE object is not closed.
 *
 * @param handle Opaque stream handle.
 * @return Non-negative on success, negative on failure.
 */
inline int standard_text_state_close(text_stream_handle_t handle) noexcept {
    text_stream_state* const state = text_handle_to_state(handle);
    if (state == nullptr) {
        return -1;
    }

    delete state;
    return 0;
}

/**
 * @brief Reads one byte from a file-backed text stream state.
 *
 * @param state Target text stream state.
 * @param out Destination byte.
 * @return 1 if one byte was read, 0 on EOF, negative on failure.
 */
[[nodiscard]] inline int standard_text_state_read_byte(
    text_stream_state& state,
    unsigned char& out) noexcept {
    if (!std::holds_alternative<text_stream_file_source>(state.source)) {
        return -1;
    }

    if (state.buffer_pos < state.buffer_size) {
        out = state.buffer[static_cast<std::size_t>(state.buffer_pos)];
        ++state.buffer_pos;
        return 1;
    }

    std::FILE* const file = std::get<text_stream_file_source>(state.source).file;
    if (file == nullptr) {
        return -1;
    }

    const std::size_t read_size =
        std::fread(state.buffer.data(), 1, state.buffer.size(), file);

    if (read_size == 0) {
        if (std::ferror(file) != 0) {
            return -1;
        }

        return 0;
    }

    state.buffer_size = static_cast<int>(read_size);
    state.buffer_pos = 1;
    out = state.buffer[0];
    return 1;
}

/**
 * @brief Reads one UTF-8 code point from a file-backed text stream state.
 *
 * @param state Target text stream state.
 * @param out Destination code point.
 * @return 1 if one code point was read, 0 on EOF, negative on failure.
 */
[[nodiscard]] inline int standard_text_state_read_utf8_char(
    text_stream_state& state,
    char32_t& out) noexcept {
    unsigned char b1 = 0;
    const int r1 = standard_text_state_read_byte(state, b1);
    if (r1 <= 0) {
        return r1;
    }

    if (b1 <= 0x7fu) {
        out = static_cast<char32_t>(b1);
        return 1;
    }

    std::uint32_t packed = static_cast<std::uint32_t>(b1);

    if (b1 >= 0xc2u && b1 <= 0xdfu) {
        unsigned char b2 = 0;
        const int r2 = standard_text_state_read_byte(state, b2);
        if (r2 != 1) {
            return -1;
        }

        packed |= static_cast<std::uint32_t>(b2) << 8;
        out = advanced::packed_utf8_to_utf32(packed);
        return out == advanced::detail::invalid_utf32 ? -1 : 1;
    }

    if (b1 >= 0xe0u && b1 <= 0xefu) {
        unsigned char b2 = 0;
        unsigned char b3 = 0;

        const int r2 = standard_text_state_read_byte(state, b2);
        const int r3 = standard_text_state_read_byte(state, b3);
        if (r2 != 1 || r3 != 1) {
            return -1;
        }

        packed |= static_cast<std::uint32_t>(b2) << 8;
        packed |= static_cast<std::uint32_t>(b3) << 16;
        out = advanced::packed_utf8_to_utf32(packed);
        return out == advanced::detail::invalid_utf32 ? -1 : 1;
    }

    if (b1 >= 0xf0u && b1 <= 0xf4u) {
        unsigned char b2 = 0;
        unsigned char b3 = 0;
        unsigned char b4 = 0;

        const int r2 = standard_text_state_read_byte(state, b2);
        const int r3 = standard_text_state_read_byte(state, b3);
        const int r4 = standard_text_state_read_byte(state, b4);
        if (r2 != 1 || r3 != 1 || r4 != 1) {
            return -1;
        }

        packed |= static_cast<std::uint32_t>(b2) << 8;
        packed |= static_cast<std::uint32_t>(b3) << 16;
        packed |= static_cast<std::uint32_t>(b4) << 24;
        out = advanced::packed_utf8_to_utf32(packed);
        return out == advanced::detail::invalid_utf32 ? -1 : 1;
    }

    return -1;
}

/**
 * @brief Reads one CP932 code point from a file-backed text stream state.
 *
 * @param state Target text stream state.
 * @param out Destination code point.
 * @return 1 if one code point was read, 0 on EOF, negative on failure.
 */
[[nodiscard]] inline int standard_text_state_read_cp932_char(
    text_stream_state& state,
    char32_t& out) noexcept {
    unsigned char b1 = 0;
    const int r1 = standard_text_state_read_byte(state, b1);
    if (r1 <= 0) {
        return r1;
    }

    std::uint16_t packed = static_cast<std::uint16_t>(b1);

    if (advanced::detail::is_cp932_single_byte(packed)) {
        out = advanced::packed_cp932_to_utf32(packed);
        return out == advanced::detail::invalid_utf32 ? -1 : 1;
    }

    if (!advanced::detail::is_cp932_lead_byte(b1)) {
        return -1;
    }

    unsigned char b2 = 0;
    const int r2 = standard_text_state_read_byte(state, b2);
    if (r2 != 1) {
        return -1;
    }

    packed |= static_cast<std::uint16_t>(b2) << 8;
    out = advanced::packed_cp932_to_utf32(packed);
    return out == advanced::detail::invalid_utf32 ? -1 : 1;
}

/**
 * @brief Reads characters from a standard text stream state.
 *
 * @param handle Opaque stream handle.
 * @param s Destination buffer.
 * @param n Maximum number of characters to read.
 * @return Number of characters read on success, negative on failure.
 */
[[nodiscard]] inline int standard_text_state_read(
    text_stream_handle_t handle,
    char32_t* s,
    int n) noexcept {
    if (s == nullptr || n < 0) {
        return -1;
    }

    text_stream_state* const state = text_handle_to_state(handle);
    if (state == nullptr) {
        return -1;
    }

    int count = 0;
    while (count < n) {
        char32_t ch = U'\0';
        int result = -1;

        switch (state->encoding) {
        case text_stream_encoding_t::utf8:
            result = standard_text_state_read_utf8_char(*state, ch);
            break;

        case text_stream_encoding_t::cp932:
            result = standard_text_state_read_cp932_char(*state, ch);
            break;

        case text_stream_encoding_t::undecided:
        default:
            result = standard_text_state_read_utf8_char(*state, ch);
            break;
        }

        if (result < 0) {
            return count == 0 ? -1 : count;
        }

        if (result == 0) {
            return count;
        }

        s[count] = ch;
        ++count;
    }

    return count;
}

/**
 * @brief Writes one UTF-32 code point as UTF-8.
 *
 * @param file Destination file.
 * @param ch Source code point.
 * @return Number of bytes written on success, negative on failure.
 */
[[nodiscard]] inline int standard_text_state_write_utf8_char(
    std::FILE* file,
    char32_t ch) noexcept {
    if (file == nullptr) {
        return -1;
    }

    const std::uint32_t packed = advanced::utf32_to_packed_utf8(ch);
    if (packed == advanced::detail::invalid_packed_utf8) {
        return -1;
    }

    unsigned char bytes[4] = {};
    int size = 1;
    bytes[0] = static_cast<unsigned char>(packed & 0xffu);

    if ((packed & 0xff00u) != 0) {
        bytes[1] = static_cast<unsigned char>((packed >> 8) & 0xffu);
        size = 2;
    }

    if ((packed & 0xff0000u) != 0) {
        bytes[2] = static_cast<unsigned char>((packed >> 16) & 0xffu);
        size = 3;
    }

    if ((packed & 0xff000000u) != 0) {
        bytes[3] = static_cast<unsigned char>((packed >> 24) & 0xffu);
        size = 4;
    }

    const std::size_t written = std::fwrite(bytes, 1, static_cast<std::size_t>(size), file);
    return written == static_cast<std::size_t>(size) ? size : -1;
}

/**
 * @brief Writes one UTF-32 code point as CP932.
 *
 * @param file Destination file.
 * @param ch Source code point.
 * @return Number of bytes written on success, negative on failure.
 */
[[nodiscard]] inline int standard_text_state_write_cp932_char(
    std::FILE* file,
    char32_t ch) noexcept {
    if (file == nullptr) {
        return -1;
    }

    const std::int32_t packed_signed = advanced::utf32_to_packed_cp932(ch);
    if (packed_signed == advanced::detail::invalid_packed_cp932) {
        return -1;
    }

    const std::uint16_t packed = static_cast<std::uint16_t>(packed_signed);
    unsigned char bytes[2] = {};
    int size = 1;
    bytes[0] = static_cast<unsigned char>(packed & 0xffu);

    if ((packed & 0xff00u) != 0) {
        bytes[1] = static_cast<unsigned char>((packed >> 8) & 0xffu);
        size = 2;
    }

    const std::size_t written = std::fwrite(bytes, 1, static_cast<std::size_t>(size), file);
    return written == static_cast<std::size_t>(size) ? size : -1;
}

/**
 * @brief Writes characters to a standard text stream state.
 *
 * @param handle Opaque stream handle.
 * @param s Source buffer.
 * @param n Number of characters to write.
 * @return Number of characters written on success, negative on failure.
 */
[[nodiscard]] inline int standard_text_state_write(
    text_stream_handle_t handle,
    const char32_t* s,
    int n) noexcept {
    if (s == nullptr || n < 0) {
        return -1;
    }

    text_stream_state* const state = text_handle_to_state(handle);
    if (state == nullptr) {
        return -1;
    }

    if (!std::holds_alternative<text_stream_file_source>(state->source)) {
        return -1;
    }

    std::FILE* const file = std::get<text_stream_file_source>(state->source).file;
    if (file == nullptr) {
        return -1;
    }

    int count = 0;
    while (count < n) {
        int result = -1;

        switch (state->encoding) {
        case text_stream_encoding_t::utf8:
            result = standard_text_state_write_utf8_char(file, s[count]);
            break;

        case text_stream_encoding_t::cp932:
            result = standard_text_state_write_cp932_char(file, s[count]);
            break;

        case text_stream_encoding_t::undecided:
        default:
            result = standard_text_state_write_utf8_char(file, s[count]);
            break;
        }

        if (result < 0) {
            return count == 0 ? -1 : count;
        }

        ++count;
    }

    return count;
}

/**
 * @brief Creates a standard text stream object.
 *
 * The resulting stream does not own the underlying FILE object.
 *
 * @param file Underlying FILE object.
 * @param encoding Stream encoding.
 * @return Created standard text stream, or an empty stream on allocation failure.
 */
[[nodiscard]] inline text_stream make_standard_text_stream(
    std::FILE* file,
    text_stream_encoding_t encoding) noexcept {
    auto* const state = new (std::nothrow) text_stream_state();
    if (state == nullptr) {
        return {};
    }

    state->source = text_stream_file_source{file};
    state->encoding = encoding;
    reset_text_stream_runtime_state(*state);

    return text_stream(
        reinterpret_cast<text_stream_handle_t>(state),
        standard_text_state_close,
        standard_text_state_read,
        standard_text_state_write,
        text_state_getpos,
        text_state_setpos,
        text_state_seek_end);
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Standard input text stream.
 */
inline text_stream standard_input =
#ifdef _WIN32
    detail::make_standard_text_stream(stdin, detail::text_stream_encoding_t::utf8);
#else
    detail::make_standard_text_stream(stdin, detail::text_stream_encoding_t::utf8);
#endif

/**
 * @brief Standard output text stream.
 */
inline text_stream standard_output =
#ifdef _WIN32
    detail::make_standard_text_stream(stdout, detail::text_stream_encoding_t::utf8);
#else
    detail::make_standard_text_stream(stdout, detail::text_stream_encoding_t::utf8);
#endif

/**
 * @brief Standard error text stream.
 */
inline text_stream standard_error =
#ifdef _WIN32
    detail::make_standard_text_stream(stderr, detail::text_stream_encoding_t::utf8);
#else
    detail::make_standard_text_stream(stderr, detail::text_stream_encoding_t::utf8);
#endif

} // namespace xer

#ifndef xer_stdin
#    define xer_stdin (::xer::standard_input)
#endif

#ifndef xer_stdout
#    define xer_stdout (::xer::standard_output)
#endif

#ifndef xer_stderr
#    define xer_stderr (::xer::standard_error)
#endif

#endif /* XER_BITS_STANDARD_STREAMS_H_INCLUDED_ */
