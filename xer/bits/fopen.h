/**
 * @file xer/bits/fopen.h
 * @brief Internal fopen, memopen, and stropen function implementations.
 */

#pragma once

#ifndef XER_BITS_FOPEN_H_INCLUDED_
#define XER_BITS_FOPEN_H_INCLUDED_

#include <array>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <expected>
#include <new>
#include <span>
#include <string>
#include <string_view>
#include <variant>

#include <xer/bits/advanced_encoding.h>
#include <xer/bits/binary_stream.h>
#include <xer/bits/text_stream.h>
#include <xer/bits/utf8_char_encode.h>
#include <xer/error.h>
#include <xer/path.h>

namespace xer {

/**
 * @brief Text encoding selector.
 */
enum class encoding_t {
    utf8,
    cp932,
    auto_detect
};

namespace detail {

/**
 * @brief Internal normalized open mode.
 */
enum class open_mode {
    r,
    rp,
    w,
    wp,
    a,
    ap,
    error
};

/**
 * @brief Returns whether the specified mode is a readable mode.
 *
 * @param mode Parsed open mode.
 * @return true if the mode reads from the stream.
 * @return false otherwise.
 */
[[nodiscard]] constexpr auto is_read_mode(open_mode mode) noexcept -> bool {
    return mode == open_mode::r || mode == open_mode::rp || mode == open_mode::wp ||
           mode == open_mode::ap;
}

/**
 * @brief Returns whether the specified mode is a writable mode.
 *
 * @param mode Parsed open mode.
 * @return true if the mode writes to the stream.
 * @return false otherwise.
 */
[[nodiscard]] constexpr auto is_write_mode(open_mode mode) noexcept -> bool {
    return mode == open_mode::rp || mode == open_mode::w || mode == open_mode::wp ||
           mode == open_mode::a || mode == open_mode::ap;
}

/**
 * @brief Returns whether the specified mode is an append mode.
 *
 * @param mode Parsed open mode.
 * @return true if the mode appends to the stream.
 * @return false otherwise.
 */
[[nodiscard]] constexpr auto is_append_mode(open_mode mode) noexcept -> bool {
    return mode == open_mode::a || mode == open_mode::ap;
}

/**
 * @brief Returns whether the specified mode is an update mode.
 *
 * @param mode Parsed open mode.
 * @return true if the mode enables both reading and writing.
 * @return false otherwise.
 */
[[nodiscard]] constexpr auto is_update_mode(open_mode mode) noexcept -> bool {
    return mode == open_mode::rp || mode == open_mode::wp || mode == open_mode::ap;
}

/**
 * @brief Parses a binary fopen mode string.
 *
 * Supported base mode characters are 'r', 'w', and 'a'.
 * '+' is optional.
 * 'b' is optional.
 * 't' is rejected.
 * The positions of '+', 'b', and 't' are not fixed.
 *
 * @param mode Source mode string.
 * @return Parsed open mode on success, open_mode::error on failure.
 */
[[nodiscard]] inline auto parse_binary_open_mode(const char* mode) noexcept -> open_mode {
    if (mode == nullptr || *mode == '\0') {
        return open_mode::error;
    }

    bool seen_r = false;
    bool seen_w = false;
    bool seen_a = false;
    bool seen_p = false;
    bool seen_b = false;

    for (const char* p = mode; *p != '\0'; ++p) {
        switch (*p) {
        case 'r':
            if (seen_r || seen_w || seen_a) {
                return open_mode::error;
            }
            seen_r = true;
            break;

        case 'w':
            if (seen_r || seen_w || seen_a) {
                return open_mode::error;
            }
            seen_w = true;
            break;

        case 'a':
            if (seen_r || seen_w || seen_a) {
                return open_mode::error;
            }
            seen_a = true;
            break;

        case '+':
            if (seen_p) {
                return open_mode::error;
            }
            seen_p = true;
            break;

        case 'b':
            if (seen_b) {
                return open_mode::error;
            }
            seen_b = true;
            break;

        case 't':
            return open_mode::error;

        default:
            return open_mode::error;
        }
    }

    if (seen_r) {
        return seen_p ? open_mode::rp : open_mode::r;
    }

    if (seen_w) {
        return seen_p ? open_mode::wp : open_mode::w;
    }

    if (seen_a) {
        return seen_p ? open_mode::ap : open_mode::a;
    }

    return open_mode::error;
}

/**
 * @brief Parses a text fopen mode string.
 *
 * Supported base mode characters are 'r', 'w', and 'a'.
 * '+' is optional.
 * 't' is optional.
 * 'b' is rejected.
 * The positions of '+', 'b', and 't' are not fixed.
 *
 * @param mode Source mode string.
 * @return Parsed open mode on success, open_mode::error on failure.
 */
[[nodiscard]] inline auto parse_text_open_mode(const char* mode) noexcept -> open_mode {
    if (mode == nullptr || *mode == '\0') {
        return open_mode::error;
    }

    bool seen_r = false;
    bool seen_w = false;
    bool seen_a = false;
    bool seen_p = false;
    bool seen_t = false;

    for (const char* p = mode; *p != '\0'; ++p) {
        switch (*p) {
        case 'r':
            if (seen_r || seen_w || seen_a) {
                return open_mode::error;
            }
            seen_r = true;
            break;

        case 'w':
            if (seen_r || seen_w || seen_a) {
                return open_mode::error;
            }
            seen_w = true;
            break;

        case 'a':
            if (seen_r || seen_w || seen_a) {
                return open_mode::error;
            }
            seen_a = true;
            break;

        case '+':
            if (seen_p) {
                return open_mode::error;
            }
            seen_p = true;
            break;

        case 't':
            if (seen_t) {
                return open_mode::error;
            }
            seen_t = true;
            break;

        case 'b':
            return open_mode::error;

        default:
            return open_mode::error;
        }
    }

    if (seen_r) {
        return seen_p ? open_mode::rp : open_mode::r;
    }

    if (seen_w) {
        return seen_p ? open_mode::wp : open_mode::w;
    }

    if (seen_a) {
        return seen_p ? open_mode::ap : open_mode::a;
    }

    return open_mode::error;
}

/**
 * @brief Converts an internal open mode to a native fopen mode string.
 *
 * @param mode Parsed open mode.
 * @param text_mode Whether to generate a text mode string.
 * @return Native mode string.
 */
[[nodiscard]] inline auto to_native_mode_string(
    open_mode mode,
    bool text_mode) noexcept {
    switch (mode) {
    case open_mode::r:
        return text_mode ? "r" : "rb";

    case open_mode::rp:
        return text_mode ? "r+" : "r+b";

    case open_mode::w:
        return text_mode ? "w" : "wb";

    case open_mode::wp:
        return text_mode ? "w+" : "w+b";

    case open_mode::a:
        return text_mode ? "a" : "ab";

    case open_mode::ap:
        return text_mode ? "a+" : "a+b";

    case open_mode::error:
    default:
        return "";
    }
}

/**
 * @brief Opens a native FILE stream.
 *
 * @param filename Source path.
 * @param mode Native fopen mode string.
 * @return Opened FILE pointer on success, nullptr on failure.
 */
[[nodiscard]] inline auto open_native_file(
    const path& filename,
    const char* mode) noexcept -> std::FILE* {
    const auto native_path = to_native_path(filename);
    if (!native_path.has_value()) {
        return nullptr;
    }

#ifdef _WIN32
    std::wstring wmode;
    for (const char* p = mode; *p != '\0'; ++p) {
        wmode.push_back(static_cast<wchar_t>(*p));
    }

    return _wfopen(native_path->c_str(), wmode.c_str());
#else
    return std::fopen(native_path->c_str(), mode);
#endif
}

/**
 * @brief File-backed binary stream source.
 */
struct binary_stream_file_source {
    std::FILE* file = nullptr;
};

/**
 * @brief Memory-backed binary stream source.
 */
struct binary_stream_memory_source {
    std::byte* data = nullptr;
    int size = 0;
    int pos = 0;
};

/**
 * @brief Internal binary stream state.
 */
struct binary_stream_state {
    std::variant<binary_stream_file_source, binary_stream_memory_source> source;
    bool readable = false;
    bool writable = false;
    bool append = false;
};

/**
 * @brief Casts a binary stream handle to binary_stream_state*.
 *
 * @param handle Opaque stream handle.
 * @return Underlying state pointer.
 */
[[nodiscard]] inline auto binary_handle_to_state(
    binary_stream_handle_t handle) noexcept -> binary_stream_state* {
    return reinterpret_cast<binary_stream_state*>(handle);
}

/**
 * @brief Closes and destroys a binary stream state.
 *
 * @param handle Opaque stream handle.
 * @return Non-negative on success, negative on failure.
 */
inline auto binary_state_close(binary_stream_handle_t handle) noexcept -> int {
    binary_stream_state* const state = binary_handle_to_state(handle);
    if (state == nullptr) {
        return -1;
    }

    int result = 0;

    if (std::holds_alternative<binary_stream_file_source>(state->source)) {
        std::FILE* const file = std::get<binary_stream_file_source>(state->source).file;
        if (file == nullptr) {
            result = -1;
        } else {
            result = std::fclose(file);
        }
    }

    delete state;
    return result;
}

/**
 * @brief Reads from a binary stream state.
 *
 * @param handle Opaque stream handle.
 * @param s Destination buffer.
 * @param n Maximum number of bytes to read.
 * @return Number of bytes read on success, negative on failure.
 */
inline auto binary_state_read(binary_stream_handle_t handle, void* s, int n) noexcept -> int {
    if (s == nullptr || n < 0) {
        return -1;
    }

    binary_stream_state* const state = binary_handle_to_state(handle);
    if (state == nullptr || !state->readable) {
        return -1;
    }

    if (std::holds_alternative<binary_stream_file_source>(state->source)) {
        std::FILE* const file = std::get<binary_stream_file_source>(state->source).file;
        if (file == nullptr) {
            return -1;
        }

        const std::size_t result = std::fread(s, 1, static_cast<std::size_t>(n), file);

        if (result == 0 && std::ferror(file) != 0) {
            return -1;
        }

        return static_cast<int>(result);
    }

    binary_stream_memory_source& source =
        std::get<binary_stream_memory_source>(state->source);

    if (source.pos < 0 || source.pos > source.size) {
        return -1;
    }

    const int remaining = source.size - source.pos;
    const int count = remaining < n ? remaining : n;

    if (count > 0) {
        std::memcpy(s, source.data + source.pos, static_cast<std::size_t>(count));
        source.pos += count;
    }

    return count;
}

/**
 * @brief Writes to a binary stream state.
 *
 * @param handle Opaque stream handle.
 * @param s Source buffer.
 * @param n Number of bytes to write.
 * @return Number of bytes written on success, negative on failure.
 */
inline auto binary_state_write(binary_stream_handle_t handle, const void* s, int n) noexcept -> int {
    if (s == nullptr || n < 0) {
        return -1;
    }

    binary_stream_state* const state = binary_handle_to_state(handle);
    if (state == nullptr || !state->writable) {
        return -1;
    }

    if (std::holds_alternative<binary_stream_file_source>(state->source)) {
        std::FILE* const file = std::get<binary_stream_file_source>(state->source).file;
        if (file == nullptr) {
            return -1;
        }

        if (state->append) {
#ifdef _WIN32
            if (_fseeki64(file, 0, SEEK_END) < 0) {
                return -1;
            }
#else
            if (fseeko(file, 0, SEEK_END) < 0) {
                return -1;
            }
#endif
        }

        const std::size_t result = std::fwrite(s, 1, static_cast<std::size_t>(n), file);

        if (result == 0 && std::ferror(file) != 0) {
            return -1;
        }

        return static_cast<int>(result);
    }

    binary_stream_memory_source& source =
        std::get<binary_stream_memory_source>(state->source);

    if (state->append) {
        source.pos = source.size;
    }

    if (source.pos < 0 || source.pos > source.size) {
        return -1;
    }

    const int remaining = source.size - source.pos;
    const int count = remaining < n ? remaining : n;

    if (count > 0) {
        std::memcpy(source.data + source.pos, s, static_cast<std::size_t>(count));
        source.pos += count;
    }

    return count;
}

/**
 * @brief Seeks within a binary stream state.
 *
 * @param handle Opaque stream handle.
 * @param pos Seek offset.
 * @param whence Seek origin.
 * @return Non-negative on success, negative on failure.
 */
inline auto binary_state_seek(
    binary_stream_handle_t handle,
    std::int64_t pos,
    int whence) noexcept {
    binary_stream_state* const state = binary_handle_to_state(handle);
    if (state == nullptr) {
        return -1;
    }

    if (std::holds_alternative<binary_stream_file_source>(state->source)) {
        std::FILE* const file = std::get<binary_stream_file_source>(state->source).file;
        if (file == nullptr) {
            return -1;
        }

#ifdef _WIN32
        return _fseeki64(file, pos, whence);
#else
        return fseeko(file, static_cast<off_t>(pos), whence);
#endif
    }

    binary_stream_memory_source& source =
        std::get<binary_stream_memory_source>(state->source);

    std::int64_t base = 0;

    switch (whence) {
    case SEEK_SET:
        base = 0;
        break;

    case SEEK_CUR:
        base = source.pos;
        break;

    case SEEK_END:
        base = source.size;
        break;

    default:
        return -1;
    }

    const std::int64_t next = base + pos;
    if (next < 0 || next > source.size) {
        return -1;
    }

    source.pos = static_cast<int>(next);
    return 0;
}

/**
 * @brief Returns the current position of a binary stream state.
 *
 * @param handle Opaque stream handle.
 * @return Current position on success, negative on failure.
 */
inline auto binary_state_tell(binary_stream_handle_t handle) noexcept -> std::int64_t {
    binary_stream_state* const state = binary_handle_to_state(handle);
    if (state == nullptr) {
        return -1;
    }

    if (std::holds_alternative<binary_stream_file_source>(state->source)) {
        std::FILE* const file = std::get<binary_stream_file_source>(state->source).file;
        if (file == nullptr) {
            return -1;
        }

#ifdef _WIN32
        return _ftelli64(file);
#else
        return static_cast<std::int64_t>(ftello(file));
#endif
    }

    const binary_stream_memory_source& source =
        std::get<binary_stream_memory_source>(state->source);

    return source.pos;
}

/**
 * @brief File-backed text stream source.
 */
struct text_stream_file_source {
    std::FILE* file = nullptr;
};

/**
 * @brief Read-only string-backed text stream source.
 */
struct text_stream_string_view_source {
    std::u8string_view str{};
    int pos = 0;
};

/**
 * @brief Writable string-backed text stream source.
 */
struct text_stream_string_source {
    std::u8string* str = nullptr;
    int pos = 0;
};

/**
 * @brief Internal concrete encoding state for text streams.
 */
enum class text_stream_encoding_t {
    undecided,
    utf8,
    cp932
};

/**
 * @brief Internal multibyte state for text streams.
 */
struct text_stream_mbstate_t {
    unsigned char pending[3] = {0, 0, 0};
    int pending_size = 0;
    int pending_used = 0;
};

/**
 * @brief Internal text stream state.
 */
struct text_stream_state {
    std::variant<
        text_stream_file_source,
        text_stream_string_view_source,
        text_stream_string_source>
        source;
    text_stream_encoding_t encoding = text_stream_encoding_t::undecided;
    std::array<unsigned char, 1024> buffer{};
    int buffer_size = 0;
    int buffer_pos = 0;
    text_stream_mbstate_t mbstate{};
    bool readable = false;
    bool writable = false;
    bool append = false;
};

/**
 * @brief Casts a text stream handle to text_stream_state*.
 *
 * @param handle Opaque stream handle.
 * @return Underlying state pointer.
 */
[[nodiscard]] inline auto text_handle_to_state(text_stream_handle_t handle) noexcept -> text_stream_state* {
    return reinterpret_cast<text_stream_state*>(handle);
}

/**
 * @brief Resets transient text stream decode state.
 *
 * @param state Target text stream state.
 */
inline auto reset_text_stream_runtime_state(text_stream_state& state) noexcept -> void {
    state.buffer_size = 0;
    state.buffer_pos = 0;
    state.mbstate.pending[0] = 0;
    state.mbstate.pending[1] = 0;
    state.mbstate.pending[2] = 0;
    state.mbstate.pending_size = 0;
    state.mbstate.pending_used = 0;
}

/**
 * @brief Reads one byte from a file-backed text stream state.
 *
 * This helper services text decoding for file-backed streams only.
 * String-backed streams store UTF-8 text directly and are decoded through
 * dedicated helpers that work from the borrowed string object.
 *
 * @param state Target text stream state.
 * @param out Destination byte.
 * @return 1 if one byte was read, 0 on EOF, negative on failure.
 */
[[nodiscard]] inline auto text_state_read_file_byte(
    text_stream_state& state,
    unsigned char& out) noexcept -> int {
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
 * @brief Decodes one UTF-8 code point from a byte sequence.
 *
 * The function expects a complete UTF-8 sequence at the specified position.
 * It is used by string-backed text streams whose storage is already UTF-8.
 *
 * @param bytes Source UTF-8 bytes.
 * @param size Available byte count from the current position.
 * @param out Destination code point.
 * @param bytes_used Number of consumed UTF-8 code units.
 * @return 1 on success, 0 on EOF, negative on failure.
 */
[[nodiscard]] inline auto decode_utf8_from_bytes(
    const char8_t* bytes,
    std::size_t size,
    char32_t& out,
    std::size_t& bytes_used) noexcept -> int {
    if (bytes == nullptr) {
        return -1;
    }

    if (size == 0) {
        return 0;
    }

    const unsigned char b1 = static_cast<unsigned char>(bytes[0]);
    if (b1 <= 0x7fu) {
        out = static_cast<char32_t>(b1);
        bytes_used = 1;
        return 1;
    }

    std::uint32_t packed = static_cast<std::uint32_t>(b1);

    if (b1 >= 0xc2u && b1 <= 0xdfu) {
        if (size < 2) {
            return -1;
        }

        packed |= static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[1])) << 8;
        out = advanced::packed_utf8_to_utf32(packed);
        if (out == advanced::detail::invalid_utf32) {
            return -1;
        }

        bytes_used = 2;
        return 1;
    }

    if (b1 >= 0xe0u && b1 <= 0xefu) {
        if (size < 3) {
            return -1;
        }

        packed |= static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[1])) << 8;
        packed |= static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[2])) << 16;
        out = advanced::packed_utf8_to_utf32(packed);
        if (out == advanced::detail::invalid_utf32) {
            return -1;
        }

        bytes_used = 3;
        return 1;
    }

    if (b1 >= 0xf0u && b1 <= 0xf4u) {
        if (size < 4) {
            return -1;
        }

        packed |= static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[1])) << 8;
        packed |= static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[2])) << 16;
        packed |= static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[3])) << 24;
        out = advanced::packed_utf8_to_utf32(packed);
        if (out == advanced::detail::invalid_utf32) {
            return -1;
        }

        bytes_used = 4;
        return 1;
    }

    return -1;
}

/**
 * @brief Reads one UTF-8 code point from a borrowed string view source.
 *
 * The string-view variant is used by read-only stropen streams. The byte
 * position is tracked directly in the source object.
 *
 * @param source Target source object.
 * @param out Destination code point.
 * @return 1 if one code point was read, 0 on EOF, negative on failure.
 */
[[nodiscard]] inline auto text_state_read_string_view_utf8_char(
    text_stream_string_view_source& source,
    char32_t& out) noexcept -> int {
    if (source.pos < 0) {
        return -1;
    }

    const std::size_t offset = static_cast<std::size_t>(source.pos);
    if (offset >= source.str.size()) {
        return 0;
    }

    std::size_t bytes_used = 0;
    const int result = decode_utf8_from_bytes(
        source.str.data() + offset,
        source.str.size() - offset,
        out,
        bytes_used);
    if (result != 1) {
        return result;
    }

    source.pos += static_cast<int>(bytes_used);
    return 1;
}

/**
 * @brief Reads one UTF-8 code point from a writable string source.
 *
 * The writable-string variant is used by stropen overloads taking
 * std::u8string&. Even in writable modes, update modes may still read.
 *
 * @param source Target source object.
 * @param out Destination code point.
 * @return 1 if one code point was read, 0 on EOF, negative on failure.
 */
[[nodiscard]] inline auto text_state_read_string_utf8_char(
    text_stream_string_source& source,
    char32_t& out) noexcept -> int {
    if (source.str == nullptr || source.pos < 0) {
        return -1;
    }

    const std::size_t offset = static_cast<std::size_t>(source.pos);
    if (offset >= source.str->size()) {
        return 0;
    }

    std::size_t bytes_used = 0;
    const int result = decode_utf8_from_bytes(
        source.str->data() + offset,
        source.str->size() - offset,
        out,
        bytes_used);
    if (result != 1) {
        return result;
    }

    source.pos += static_cast<int>(bytes_used);
    return 1;
}

/**
 * @brief Reads one UTF-8 code point from a file-backed text stream state.
 *
 * @param state Target text stream state.
 * @param out Destination code point.
 * @return 1 if one code point was read, 0 on EOF, negative on failure.
 */
[[nodiscard]] inline auto text_state_read_file_utf8_char(
    text_stream_state& state,
    char32_t& out) noexcept -> int {
    unsigned char b1 = 0;
    const int r1 = text_state_read_file_byte(state, b1);
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
        if (text_state_read_file_byte(state, b2) != 1) {
            return -1;
        }

        packed |= static_cast<std::uint32_t>(b2) << 8;
        out = advanced::packed_utf8_to_utf32(packed);
        return out == advanced::detail::invalid_utf32 ? -1 : 1;
    }

    if (b1 >= 0xe0u && b1 <= 0xefu) {
        unsigned char b2 = 0;
        unsigned char b3 = 0;
        if (text_state_read_file_byte(state, b2) != 1 ||
            text_state_read_file_byte(state, b3) != 1) {
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
        if (text_state_read_file_byte(state, b2) != 1 ||
            text_state_read_file_byte(state, b3) != 1 ||
            text_state_read_file_byte(state, b4) != 1) {
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
 * CP932 is supported only for file-backed streams. String-backed streams are
 * defined to store UTF-8 text.
 *
 * @param state Target text stream state.
 * @param out Destination code point.
 * @return 1 if one code point was read, 0 on EOF, negative on failure.
 */
[[nodiscard]] inline auto text_state_read_file_cp932_char(
    text_stream_state& state,
    char32_t& out) noexcept -> int {
    unsigned char b1 = 0;
    const int r1 = text_state_read_file_byte(state, b1);
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
    if (text_state_read_file_byte(state, b2) != 1) {
        return -1;
    }

    packed |= static_cast<std::uint16_t>(b2) << 8;
    out = advanced::packed_cp932_to_utf32(packed);
    return out == advanced::detail::invalid_utf32 ? -1 : 1;
}

/**
 * @brief Reads characters from a text stream state.
 *
 * This is the concrete read function installed into text_stream objects created
 * by fopen and stropen. The implementation dispatches according to both the
 * backing source kind and the configured text encoding.
 *
 * @param handle Opaque stream handle.
 * @param s Destination buffer.
 * @param n Maximum number of characters to read.
 * @return Number of characters read on success, negative on failure.
 */
[[nodiscard]] inline auto text_state_read(
    text_stream_handle_t handle,
    char32_t* s,
    int n) noexcept -> int {
    if (s == nullptr || n < 0) {
        return -1;
    }

    text_stream_state* const state = text_handle_to_state(handle);
    if (state == nullptr || !state->readable) {
        return -1;
    }

    int count = 0;
    while (count < n) {
        char32_t ch = U'\0';
        int result = -1;

        if (std::holds_alternative<text_stream_string_view_source>(state->source)) {
            auto& source = std::get<text_stream_string_view_source>(state->source);
            result = text_state_read_string_view_utf8_char(source, ch);
        } else if (std::holds_alternative<text_stream_string_source>(state->source)) {
            auto& source = std::get<text_stream_string_source>(state->source);
            result = text_state_read_string_utf8_char(source, ch);
        } else {
            switch (state->encoding) {
            case text_stream_encoding_t::cp932:
                result = text_state_read_file_cp932_char(*state, ch);
                break;
            case text_stream_encoding_t::utf8:
            case text_stream_encoding_t::undecided:
            default:
                result = text_state_read_file_utf8_char(*state, ch);
                break;
            }
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
 * @brief Encodes one UTF-32 code point as UTF-8 bytes.
 *
 * @param ch Source code point.
 * @param out Destination byte string.
 * @return true on success, false on failure.
 */
[[nodiscard]] inline auto encode_utf8_char_to_string(
    char32_t ch,
    std::u8string& out) noexcept -> bool {
    const auto appended = append_utf8_char(out, ch);
    return appended.has_value();
}

/**
 * @brief Writes one UTF-32 code point as UTF-8 to a native FILE.
 *
 * @param file Destination file.
 * @param ch Source code point.
 * @return Number of bytes written on success, negative on failure.
 */
[[nodiscard]] inline auto text_state_write_file_utf8_char(
    std::FILE* file,
    char32_t ch) noexcept -> int {
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
 * @brief Writes one UTF-32 code point as CP932 to a native FILE.
 *
 * @param file Destination file.
 * @param ch Source code point.
 * @return Number of bytes written on success, negative on failure.
 */
[[nodiscard]] inline auto text_state_write_file_cp932_char(
    std::FILE* file,
    char32_t ch) noexcept -> int {
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
 * @brief Writes characters to a text stream state.
 *
 * String-backed streams store UTF-8 bytes directly. File-backed streams use
 * the configured external encoding.
 *
 * @param handle Opaque stream handle.
 * @param s Source buffer.
 * @param n Number of characters to write.
 * @return Number of characters written on success, negative on failure.
 */
[[nodiscard]] inline auto text_state_write(
    text_stream_handle_t handle,
    const char32_t* s,
    int n) noexcept -> int {
    if (s == nullptr || n < 0) {
        return -1;
    }

    text_stream_state* const state = text_handle_to_state(handle);
    if (state == nullptr || !state->writable) {
        return -1;
    }

    if (std::holds_alternative<text_stream_string_view_source>(state->source)) {
        return -1;
    }

    int count = 0;
    while (count < n) {
        if (std::holds_alternative<text_stream_string_source>(state->source)) {
            auto& source = std::get<text_stream_string_source>(state->source);
            if (source.str == nullptr || source.pos < 0) {
                return count == 0 ? -1 : count;
            }

            std::u8string encoded;
            if (!encode_utf8_char_to_string(s[count], encoded)) {
                return count == 0 ? -1 : count;
            }

            const std::size_t offset = static_cast<std::size_t>(source.pos);
            if (offset > source.str->size()) {
                return count == 0 ? -1 : count;
            }

            if (state->append) {
                source.pos = static_cast<int>(source.str->size());
            }

            const std::size_t write_offset = static_cast<std::size_t>(source.pos);
            const std::size_t end_offset = write_offset + encoded.size();
            if (end_offset > source.str->size()) {
                source.str->resize(end_offset);
            }

            for (std::size_t i = 0; i < encoded.size(); ++i) {
                (*source.str)[write_offset + i] = encoded[i];
            }

            source.pos += static_cast<int>(encoded.size());
            ++count;
            continue;
        }

        std::FILE* const file = std::get<text_stream_file_source>(state->source).file;
        int result = -1;
        switch (state->encoding) {
        case text_stream_encoding_t::cp932:
            result = text_state_write_file_cp932_char(file, s[count]);
            break;
        case text_stream_encoding_t::utf8:
        case text_stream_encoding_t::undecided:
        default:
            result = text_state_write_file_utf8_char(file, s[count]);
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
 * @brief Closes and destroys a text stream state.
 *
 * @param handle Opaque stream handle.
 * @return Non-negative on success, negative on failure.
 */
inline auto text_state_close(text_stream_handle_t handle) noexcept -> int {
    text_stream_state* const state = text_handle_to_state(handle);
    if (state == nullptr) {
        return -1;
    }

    int result = 0;

    if (std::holds_alternative<text_stream_file_source>(state->source)) {
        std::FILE* const file = std::get<text_stream_file_source>(state->source).file;
        if (file == nullptr) {
            result = -1;
        } else {
            result = std::fclose(file);
        }
    }

    delete state;
    return result;
}

/**
 * @brief Returns the current position of a text stream state.
 *
 * @param handle Opaque stream handle.
 * @return Current position on success, negative on failure.
 */
inline auto text_state_getpos(text_stream_handle_t handle) noexcept -> text_stream_pos_t {
    text_stream_state* const state = text_handle_to_state(handle);
    if (state == nullptr) {
        return -1;
    }

    if (std::holds_alternative<text_stream_file_source>(state->source)) {
        std::FILE* const file = std::get<text_stream_file_source>(state->source).file;
        if (file == nullptr) {
            return -1;
        }

#ifdef _WIN32
        const std::int64_t pos = _ftelli64(file);
#else
        const std::int64_t pos = static_cast<std::int64_t>(ftello(file));
#endif

        if (pos < 0) {
            return -1;
        }

        return pos;
    }

    if (std::holds_alternative<text_stream_string_view_source>(state->source)) {
        return std::get<text_stream_string_view_source>(state->source).pos;
    }

    return std::get<text_stream_string_source>(state->source).pos;
}

/**
 * @brief Sets the current position of a text stream state.
 *
 * @param handle Opaque stream handle.
 * @param pos Position previously obtained by getpos.
 * @return Non-negative on success, negative on failure.
 */
inline auto text_state_setpos(text_stream_handle_t handle, text_stream_pos_t pos) noexcept -> int {
    text_stream_state* const state = text_handle_to_state(handle);
    if (state == nullptr || pos < 0) {
        return -1;
    }

    reset_text_stream_runtime_state(*state);

    if (std::holds_alternative<text_stream_file_source>(state->source)) {
        std::FILE* const file = std::get<text_stream_file_source>(state->source).file;
        if (file == nullptr) {
            return -1;
        }

#ifdef _WIN32
        return _fseeki64(file, pos, SEEK_SET);
#else
        return fseeko(file, static_cast<off_t>(pos), SEEK_SET);
#endif
    }

    if (std::holds_alternative<text_stream_string_view_source>(state->source)) {
        text_stream_string_view_source& source =
            std::get<text_stream_string_view_source>(state->source);

        if (pos > static_cast<text_stream_pos_t>(source.str.size())) {
            return -1;
        }

        source.pos = static_cast<int>(pos);
        return 0;
    }

    text_stream_string_source& source =
        std::get<text_stream_string_source>(state->source);

    if (source.str == nullptr || pos > static_cast<text_stream_pos_t>(source.str->size())) {
        return -1;
    }

    source.pos = static_cast<int>(pos);
    return 0;
}

/**
 * @brief Seeks to the end of a text stream state.
 *
 * @param handle Opaque stream handle.
 * @return Non-negative on success, negative on failure.
 */
inline auto text_state_seek_end(text_stream_handle_t handle) noexcept -> int {
    text_stream_state* const state = text_handle_to_state(handle);
    if (state == nullptr) {
        return -1;
    }

    reset_text_stream_runtime_state(*state);

    if (std::holds_alternative<text_stream_file_source>(state->source)) {
        std::FILE* const file = std::get<text_stream_file_source>(state->source).file;
        if (file == nullptr) {
            return -1;
        }

#ifdef _WIN32
        return _fseeki64(file, 0, SEEK_END);
#else
        return fseeko(file, 0, SEEK_END);
#endif
    }

    if (std::holds_alternative<text_stream_string_view_source>(state->source)) {
        text_stream_string_view_source& source =
            std::get<text_stream_string_view_source>(state->source);
        source.pos = static_cast<int>(source.str.size());
        return 0;
    }

    text_stream_string_source& source =
        std::get<text_stream_string_source>(state->source);

    if (source.str == nullptr) {
        return -1;
    }

    source.pos = static_cast<int>(source.str->size());
    return 0;
}

} // namespace detail

/**
 * @brief Opens a binary file stream.
 *
 * @param filename Source file path.
 * @param mode Open mode string.
 * @return Opened binary stream on success.
 * @return Unexpected error on failure.
 */
[[nodiscard]] inline auto fopen(
    const path& filename,
    const char* mode) noexcept -> result<binary_stream> {
    const detail::open_mode parsed = detail::parse_binary_open_mode(mode);
    if (parsed == detail::open_mode::error) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::FILE* const file =
        detail::open_native_file(filename, detail::to_native_mode_string(parsed, false));
    if (file == nullptr) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    auto* const state = new (std::nothrow) detail::binary_stream_state();
    if (state == nullptr) {
        (void)std::fclose(file);
        return std::unexpected(make_error(error_t::runtime_error));
    }

    state->source = detail::binary_stream_file_source{file};
    state->readable = detail::is_read_mode(parsed);
    state->writable = detail::is_write_mode(parsed);
    state->append = detail::is_append_mode(parsed);

    return binary_stream(
        reinterpret_cast<binary_stream_handle_t>(state),
        detail::binary_state_close,
        detail::binary_state_read,
        detail::binary_state_write,
        detail::binary_state_seek,
        detail::binary_state_tell);
}

/**
 * @brief Opens a text file stream.
 *
 * Character input and output are implemented separately by fgetc/fputc
 * equivalents. This function prepares the stream state only.
 *
 * @param filename Source file path.
 * @param mode Open mode string.
 * @param encoding Text encoding selector.
 * @return Opened text stream on success.
 * @return Unexpected error on failure.
 */
[[nodiscard]] inline auto fopen(
    const path& filename,
    const char* mode,
    encoding_t encoding) noexcept -> result<text_stream> {
    const detail::open_mode parsed = detail::parse_text_open_mode(mode);
    if (parsed == detail::open_mode::error) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (encoding == encoding_t::auto_detect && detail::is_write_mode(parsed)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::FILE* const file =
        detail::open_native_file(filename, detail::to_native_mode_string(parsed, true));
    if (file == nullptr) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    auto* const state = new (std::nothrow) detail::text_stream_state();
    if (state == nullptr) {
        (void)std::fclose(file);
        return std::unexpected(make_error(error_t::runtime_error));
    }

    state->source = detail::text_stream_file_source{file};
    state->readable = detail::is_read_mode(parsed);
    state->writable = detail::is_write_mode(parsed);
    state->append = detail::is_append_mode(parsed);
    detail::reset_text_stream_runtime_state(*state);

    switch (encoding) {
    case encoding_t::utf8:
        state->encoding = detail::text_stream_encoding_t::utf8;
        break;

    case encoding_t::cp932:
        state->encoding = detail::text_stream_encoding_t::cp932;
        break;

    case encoding_t::auto_detect:
        state->encoding = detail::text_stream_encoding_t::undecided;
        break;

    default:
        delete state;
        (void)std::fclose(file);
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return text_stream(
        reinterpret_cast<text_stream_handle_t>(state),
        detail::text_state_close,
        detail::text_state_read,
        detail::text_state_write,
        detail::text_state_getpos,
        detail::text_state_setpos,
        detail::text_state_seek_end);
}

/**
 * @brief Opens a binary memory stream.
 *
 * The specified memory region is borrowed and must outlive the stream.
 *
 * @param mem Target memory region.
 * @param mode Open mode string.
 * @return Opened binary stream on success.
 * @return Unexpected error on failure.
 */
[[nodiscard]] inline auto memopen(
    std::span<std::byte> mem,
    const char* mode) noexcept -> result<binary_stream> {
    const detail::open_mode parsed = detail::parse_binary_open_mode(mode);
    if (parsed == detail::open_mode::error) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    auto* const state = new (std::nothrow) detail::binary_stream_state();
    if (state == nullptr) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    detail::binary_stream_memory_source source;
    source.data = mem.data();
    source.size = static_cast<int>(mem.size());
    source.pos = detail::is_append_mode(parsed) ? source.size : 0;

    state->source = source;
    state->readable = detail::is_read_mode(parsed);
    state->writable = detail::is_write_mode(parsed);
    state->append = detail::is_append_mode(parsed);

    if (parsed == detail::open_mode::w || parsed == detail::open_mode::wp) {
        source.pos = 0;
        state->source = source;
    }

    return binary_stream(
        reinterpret_cast<binary_stream_handle_t>(state),
        detail::binary_state_close,
        detail::binary_state_read,
        detail::binary_state_write,
        detail::binary_state_seek,
        detail::binary_state_tell);
}

/**
 * @brief Opens a read-only text string stream.
 *
 * The specified string view is borrowed and must outlive the stream.
 * Only read modes are accepted.
 *
 * @param str Source string view.
 * @param mode Open mode string.
 * @return Opened text stream on success.
 * @return Unexpected error on failure.
 */
[[nodiscard]] inline auto stropen(
    std::u8string_view str,
    const char* mode) noexcept -> result<text_stream> {
    const detail::open_mode parsed = detail::parse_text_open_mode(mode);
    if (parsed == detail::open_mode::error) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (parsed != detail::open_mode::r) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    auto* const state = new (std::nothrow) detail::text_stream_state();
    if (state == nullptr) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    state->source = detail::text_stream_string_view_source{str, 0};
    state->encoding = detail::text_stream_encoding_t::utf8;
    state->readable = true;
    state->writable = false;
    state->append = false;
    detail::reset_text_stream_runtime_state(*state);

    return text_stream(
        reinterpret_cast<text_stream_handle_t>(state),
        detail::text_state_close,
        detail::text_state_read,
        detail::text_state_write,
        detail::text_state_getpos,
        detail::text_state_setpos,
        detail::text_state_seek_end);
}

/**
 * @brief Opens a writable text string stream.
 *
 * The specified string is borrowed and must outlive the stream.
 *
 * @param str Target string.
 * @param mode Open mode string.
 * @return Opened text stream on success.
 * @return Unexpected error on failure.
 */
[[nodiscard]] inline auto stropen(
    std::u8string& str,
    const char* mode) noexcept -> result<text_stream> {
    const detail::open_mode parsed = detail::parse_text_open_mode(mode);
    if (parsed == detail::open_mode::error) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (parsed == detail::open_mode::w || parsed == detail::open_mode::wp) {
        str.clear();
    }

    auto* const state = new (std::nothrow) detail::text_stream_state();
    if (state == nullptr) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    detail::text_stream_string_source source;
    source.str = &str;
    source.pos = detail::is_append_mode(parsed) ? static_cast<int>(str.size()) : 0;

    state->source = source;
    state->encoding = detail::text_stream_encoding_t::utf8;
    state->readable = detail::is_read_mode(parsed);
    state->writable = detail::is_write_mode(parsed);
    state->append = detail::is_append_mode(parsed);
    detail::reset_text_stream_runtime_state(*state);

    return text_stream(
        reinterpret_cast<text_stream_handle_t>(state),
        detail::text_state_close,
        detail::text_state_read,
        detail::text_state_write,
        detail::text_state_getpos,
        detail::text_state_setpos,
        detail::text_state_seek_end);
}

} // namespace xer

#endif /* XER_BITS_FOPEN_H_INCLUDED_ */
