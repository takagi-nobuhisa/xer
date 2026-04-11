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

#include <xer/bits/binary_stream.h>
#include <xer/bits/text_stream.h>
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
[[nodiscard]] constexpr bool is_read_mode(open_mode mode) noexcept {
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
[[nodiscard]] constexpr bool is_write_mode(open_mode mode) noexcept {
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
[[nodiscard]] constexpr bool is_append_mode(open_mode mode) noexcept {
    return mode == open_mode::a || mode == open_mode::ap;
}

/**
 * @brief Returns whether the specified mode is an update mode.
 *
 * @param mode Parsed open mode.
 * @return true if the mode enables both reading and writing.
 * @return false otherwise.
 */
[[nodiscard]] constexpr bool is_update_mode(open_mode mode) noexcept {
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
[[nodiscard]] inline open_mode parse_binary_open_mode(const char* mode) noexcept {
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
[[nodiscard]] inline open_mode parse_text_open_mode(const char* mode) noexcept {
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
[[nodiscard]] inline const char* to_native_mode_string(
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
[[nodiscard]] inline std::FILE* open_native_file(
    const path& filename,
    const char* mode) noexcept {
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
[[nodiscard]] inline binary_stream_state* binary_handle_to_state(
    binary_stream_handle_t handle) noexcept {
    return reinterpret_cast<binary_stream_state*>(handle);
}

/**
 * @brief Closes and destroys a binary stream state.
 *
 * @param handle Opaque stream handle.
 * @return Non-negative on success, negative on failure.
 */
inline int binary_state_close(binary_stream_handle_t handle) noexcept {
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
inline int binary_state_read(binary_stream_handle_t handle, void* s, int n) noexcept {
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
inline int binary_state_write(binary_stream_handle_t handle, const void* s, int n) noexcept {
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
inline int binary_state_seek(
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
inline std::int64_t binary_state_tell(binary_stream_handle_t handle) noexcept {
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
};

/**
 * @brief Casts a text stream handle to text_stream_state*.
 *
 * @param handle Opaque stream handle.
 * @return Underlying state pointer.
 */
[[nodiscard]] inline text_stream_state* text_handle_to_state(text_stream_handle_t handle) noexcept {
    return reinterpret_cast<text_stream_state*>(handle);
}

/**
 * @brief Resets transient text stream decode state.
 *
 * @param state Target text stream state.
 */
inline void reset_text_stream_runtime_state(text_stream_state& state) noexcept {
    state.buffer_size = 0;
    state.buffer_pos = 0;
    state.mbstate.pending[0] = 0;
    state.mbstate.pending[1] = 0;
    state.mbstate.pending[2] = 0;
    state.mbstate.pending_size = 0;
    state.mbstate.pending_used = 0;
}

/**
 * @brief Closes and destroys a text stream state.
 *
 * @param handle Opaque stream handle.
 * @return Non-negative on success, negative on failure.
 */
inline int text_state_close(text_stream_handle_t handle) noexcept {
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
inline text_stream_pos_t text_state_getpos(text_stream_handle_t handle) noexcept {
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
inline int text_state_setpos(text_stream_handle_t handle, text_stream_pos_t pos) noexcept {
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
inline int text_state_seek_end(text_stream_handle_t handle) noexcept {
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
[[nodiscard]] inline std::expected<binary_stream, error<void>> fopen(
    const path& filename,
    const char* mode) noexcept {
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
[[nodiscard]] inline std::expected<text_stream, error<void>> fopen(
    const path& filename,
    const char* mode,
    encoding_t encoding) noexcept {
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
        detail::text_stream_read_error,
        detail::text_stream_write_error,
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
[[nodiscard]] inline std::expected<binary_stream, error<void>> memopen(
    std::span<std::byte> mem,
    const char* mode) noexcept {
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
[[nodiscard]] inline std::expected<text_stream, error<void>> stropen(
    std::u8string_view str,
    const char* mode) noexcept {
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

    return text_stream(
        reinterpret_cast<text_stream_handle_t>(state),
        detail::text_state_close,
        detail::text_stream_read_error,
        detail::text_stream_write_error,
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
[[nodiscard]] inline std::expected<text_stream, error<void>> stropen(
    std::u8string& str,
    const char* mode) noexcept {
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

    return text_stream(
        reinterpret_cast<text_stream_handle_t>(state),
        detail::text_state_close,
        detail::text_stream_read_error,
        detail::text_stream_write_error,
        detail::text_state_getpos,
        detail::text_state_setpos,
        detail::text_state_seek_end);
}

} // namespace xer

#endif /* XER_BITS_FOPEN_H_INCLUDED_ */
