/**
 * @file xer/bits/zip.h
 * @brief ZIP archive reading and writing utilities.
 */

#pragma once

#ifndef XER_BITS_ZIP_H_INCLUDED_
#define XER_BITS_ZIP_H_INCLUDED_

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <zlib.h>

#include <xer/bits/binary_stream.h>
#include <xer/bits/binary_stream_io.h>
#include <xer/bits/fclose.h>
#include <xer/bits/fflush.h>
#include <xer/bits/file_contents.h>
#include <xer/bits/file_entry.h>
#include <xer/bits/fopen.h>
#include <xer/bits/stream_position.h>
#include <xer/bits/stream_position_io.h>
#include <xer/bits/text_encoding_common.h>
#include <xer/error.h>
#include <xer/path.h>

namespace xer {

namespace detail {

inline constexpr std::uint32_t zip_local_file_header_signature = 0x04034b50u;
inline constexpr std::uint32_t zip_central_directory_file_header_signature = 0x02014b50u;
inline constexpr std::uint32_t zip_end_of_central_directory_signature = 0x06054b50u;
inline constexpr std::uint16_t zip_method_store = 0u;
inline constexpr std::uint16_t zip_method_deflate = 8u;
inline constexpr std::uint16_t zip_flag_encrypted = 0x0001u;
inline constexpr std::uint16_t zip_flag_utf8_name = 0x0800u;
inline constexpr std::size_t zip_eocd_min_size = 22u;
inline constexpr std::size_t zip_max_comment_size = 65535u;

[[nodiscard]] constexpr auto zip_read_u16(std::span<const std::byte> data, std::size_t offset) noexcept
    -> std::uint16_t
{
    return static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(static_cast<unsigned char>(data[offset])) |
        (static_cast<std::uint16_t>(static_cast<unsigned char>(data[offset + 1u])) << 8u));
}

[[nodiscard]] constexpr auto zip_read_u32(std::span<const std::byte> data, std::size_t offset) noexcept
    -> std::uint32_t
{
    return static_cast<std::uint32_t>(
        static_cast<std::uint32_t>(static_cast<unsigned char>(data[offset])) |
        (static_cast<std::uint32_t>(static_cast<unsigned char>(data[offset + 1u])) << 8u) |
        (static_cast<std::uint32_t>(static_cast<unsigned char>(data[offset + 2u])) << 16u) |
        (static_cast<std::uint32_t>(static_cast<unsigned char>(data[offset + 3u])) << 24u));
}


inline auto zip_append_u16(std::vector<std::byte>& out, std::uint16_t value) -> void
{
    out.push_back(static_cast<std::byte>(value & 0xffu));
    out.push_back(static_cast<std::byte>((value >> 8u) & 0xffu));
}

inline auto zip_append_u32(std::vector<std::byte>& out, std::uint32_t value) -> void
{
    zip_append_u16(out, static_cast<std::uint16_t>(value & 0xffffu));
    zip_append_u16(out, static_cast<std::uint16_t>((value >> 16u) & 0xffffu));
}

[[nodiscard]] inline auto zip_u64_to_u32(std::uint64_t value) noexcept -> result<std::uint32_t>
{
    if (value > static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max())) {
        return std::unexpected(make_error(error_t::out_of_range));
    }

    return static_cast<std::uint32_t>(value);
}

[[nodiscard]] inline auto zip_size_to_u16(std::size_t value) noexcept -> result<std::uint16_t>
{
    if (value > static_cast<std::size_t>(std::numeric_limits<std::uint16_t>::max())) {
        return std::unexpected(make_error(error_t::length_error));
    }

    return static_cast<std::uint16_t>(value);
}

[[nodiscard]] inline auto zip_uint64_to_int64(std::uint64_t value) noexcept -> result<std::int64_t>
{
    if (value > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
        return std::unexpected(make_error(error_t::out_of_range));
    }

    return static_cast<std::int64_t>(value);
}

[[nodiscard]] inline auto zip_uint64_to_size(std::uint64_t value) noexcept -> result<std::size_t>
{
    if (value > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
        return std::unexpected(make_error(error_t::out_of_range));
    }

    return static_cast<std::size_t>(value);
}

[[nodiscard]] inline auto zip_read_exact(binary_stream& stream, std::span<std::byte> buffer) noexcept
    -> result<void>
{
    std::size_t offset = 0;

    while (offset < buffer.size()) {
        const auto read_result = fread(buffer.subspan(offset), stream);
        if (!read_result.has_value()) {
            return std::unexpected(read_result.error().code == error_t::end_of_file
                ? make_error(error_t::invalid_argument)
                : read_result.error());
        }

        offset += *read_result;
    }

    return {};
}


[[nodiscard]] inline auto zip_write_exact(binary_stream& stream, std::span<const std::byte> buffer)
    -> result<void>
{
    std::size_t offset = 0;

    while (offset < buffer.size()) {
        const auto write_result = fwrite(buffer.subspan(offset), stream);
        if (!write_result.has_value()) {
            return std::unexpected(write_result.error());
        }

        if (*write_result == 0u) {
            return std::unexpected(make_error(error_t::io_error));
        }

        offset += *write_result;
    }

    return {};
}

[[nodiscard]] inline auto zip_read_exact_at(
    binary_stream& stream,
    std::uint64_t offset,
    std::span<std::byte> buffer) noexcept -> result<void>
{
    const auto signed_offset = zip_uint64_to_int64(offset);
    if (!signed_offset.has_value()) {
        return std::unexpected(signed_offset.error());
    }

    const auto seek_result = fseek(stream, *signed_offset, seek_set);
    if (!seek_result.has_value()) {
        return std::unexpected(seek_result.error());
    }

    return zip_read_exact(stream, buffer);
}

[[nodiscard]] inline auto zip_bytes_to_u8string(
    std::span<const std::byte> bytes,
    std::uint16_t flags) -> result<std::u8string>
{
    std::string narrow;
    narrow.reserve(bytes.size());

    for (const std::byte b : bytes) {
        narrow.push_back(static_cast<char>(static_cast<unsigned char>(b)));
    }

    if ((flags & zip_flag_utf8_name) != 0u && !is_valid_utf8(narrow)) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    if ((flags & zip_flag_utf8_name) == 0u && !is_valid_utf8(narrow)) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    return to_u8string(narrow);
}

[[nodiscard]] inline auto zip_is_directory_name(std::u8string_view entry_name) noexcept -> bool
{
    return !entry_name.empty() && entry_name.back() == u8'/';
}

[[nodiscard]] inline auto zip_is_safe_entry_name(std::u8string_view entry_name) noexcept -> bool
{
    if (entry_name.empty()) {
        return false;
    }

    for (const char8_t ch : entry_name) {
        if (ch == u8'\0') {
            return false;
        }
    }

    path normalized(entry_name);
    const std::u8string_view value = normalized.str();
    if (value.empty() || is_absolute(normalized) || is_drive_relative_path(value)) {
        return false;
    }

    std::size_t offset = 0;
    while (offset < value.size()) {
        const std::size_t next = value.find(u8'/', offset);
        const std::u8string_view component = next == std::u8string_view::npos
            ? value.substr(offset)
            : value.substr(offset, next - offset);

        if (component.empty()) {
            return false;
        }

        if (component == u8"." || component == u8"..") {
            return false;
        }

        if (next == std::u8string_view::npos) {
            break;
        }

        offset = next + 1u;
    }

    return true;
}

[[nodiscard]] inline auto zip_make_directories(const path& directory) -> result<void>
{
    if (directory.str().empty() || is_dir(directory)) {
        return {};
    }

    const auto parent = parent_path(directory);
    if (parent.has_value()) {
        const auto parent_result = zip_make_directories(*parent);
        if (!parent_result.has_value()) {
            return std::unexpected(parent_result.error());
        }
    } else if (parent.error().code != error_t::not_found) {
        return std::unexpected(parent.error());
    }

    const auto created = mkdir(directory);
    if (!created.has_value() && !is_dir(directory)) {
        return std::unexpected(created.error());
    }

    return {};
}

[[nodiscard]] inline auto zip_make_parent_directories(const path& filename) -> result<void>
{
    const auto parent = parent_path(filename);
    if (!parent.has_value()) {
        return parent.error().code == error_t::not_found
            ? result<void>()
            : std::unexpected(parent.error());
    }

    return zip_make_directories(*parent);
}

[[nodiscard]] inline auto zip_join_safe_entry_path(
    const path& destination_dir,
    std::u8string_view entry_name) -> result<path>
{
    if (!zip_is_safe_entry_name(entry_name)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    auto result = destination_dir / path(entry_name);
    if (!result.has_value()) {
        return std::unexpected(result.error());
    }

    return *result;
}

[[nodiscard]] inline auto zip_inflate_raw(
    std::span<const std::byte> compressed,
    std::uint64_t uncompressed_size) -> result<std::vector<std::byte>>
{
    const auto output_size = zip_uint64_to_size(uncompressed_size);
    if (!output_size.has_value()) {
        return std::unexpected(output_size.error());
    }

    std::vector<std::byte> output(*output_size);
    if (compressed.empty() && output.empty()) {
        return output;
    }

    z_stream stream{};
    int code = ::inflateInit2(&stream, -MAX_WBITS);
    if (code != Z_OK) {
        return std::unexpected(make_error(error_t::io_error));
    }

    stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(compressed.data()));
    stream.avail_in = static_cast<uInt>(std::min<std::size_t>(
        compressed.size(),
        static_cast<std::size_t>(std::numeric_limits<uInt>::max())));
    stream.next_out = reinterpret_cast<Bytef*>(output.data());
    stream.avail_out = static_cast<uInt>(std::min<std::size_t>(
        output.size(),
        static_cast<std::size_t>(std::numeric_limits<uInt>::max())));

    std::size_t input_offset = stream.avail_in;
    std::size_t output_offset = stream.avail_out;

    for (;;) {
        if (stream.avail_in == 0 && input_offset < compressed.size()) {
            const std::size_t chunk = std::min<std::size_t>(
                compressed.size() - input_offset,
                static_cast<std::size_t>(std::numeric_limits<uInt>::max()));
            stream.next_in = const_cast<Bytef*>(
                reinterpret_cast<const Bytef*>(compressed.data() + input_offset));
            stream.avail_in = static_cast<uInt>(chunk);
            input_offset += chunk;
        }

        if (stream.avail_out == 0 && output_offset < output.size()) {
            const std::size_t chunk = std::min<std::size_t>(
                output.size() - output_offset,
                static_cast<std::size_t>(std::numeric_limits<uInt>::max()));
            stream.next_out = reinterpret_cast<Bytef*>(output.data() + output_offset);
            stream.avail_out = static_cast<uInt>(chunk);
            output_offset += chunk;
        }

        code = ::inflate(&stream, Z_NO_FLUSH);
        if (code == Z_STREAM_END) {
            break;
        }

        if (code != Z_OK) {
            static_cast<void>(::inflateEnd(&stream));
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        if (stream.avail_in == 0 && input_offset >= compressed.size() &&
            stream.avail_out != 0) {
            static_cast<void>(::inflateEnd(&stream));
            return std::unexpected(make_error(error_t::invalid_argument));
        }
    }

    code = ::inflateEnd(&stream);
    if (code != Z_OK) {
        return std::unexpected(make_error(error_t::io_error));
    }

    if (stream.total_out != uncompressed_size) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }


    return output;
}

[[nodiscard]] inline auto zip_deflate_raw(std::span<const std::byte> data)
    -> result<std::vector<std::byte>>
{
    const uLong bound = ::compressBound(static_cast<uLong>(data.size()));
    if (bound > static_cast<uLong>(std::numeric_limits<std::size_t>::max())) {
        return std::unexpected(make_error(error_t::length_error));
    }

    std::vector<std::byte> output(static_cast<std::size_t>(bound));

    z_stream stream{};
    int code = ::deflateInit2(
        &stream,
        Z_DEFAULT_COMPRESSION,
        Z_DEFLATED,
        -MAX_WBITS,
        8,
        Z_DEFAULT_STRATEGY);
    if (code != Z_OK) {
        return std::unexpected(make_error(error_t::io_error));
    }

    stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(data.data()));
    stream.avail_in = static_cast<uInt>(std::min<std::size_t>(
        data.size(),
        static_cast<std::size_t>(std::numeric_limits<uInt>::max())));
    stream.next_out = reinterpret_cast<Bytef*>(output.data());
    stream.avail_out = static_cast<uInt>(std::min<std::size_t>(
        output.size(),
        static_cast<std::size_t>(std::numeric_limits<uInt>::max())));

    std::size_t input_offset = stream.avail_in;
    std::size_t output_offset = stream.avail_out;

    for (;;) {
        if (stream.avail_in == 0 && input_offset < data.size()) {
            const std::size_t chunk = std::min<std::size_t>(
                data.size() - input_offset,
                static_cast<std::size_t>(std::numeric_limits<uInt>::max()));
            stream.next_in = const_cast<Bytef*>(
                reinterpret_cast<const Bytef*>(data.data() + input_offset));
            stream.avail_in = static_cast<uInt>(chunk);
            input_offset += chunk;
        }

        if (stream.avail_out == 0 && output_offset < output.size()) {
            const std::size_t chunk = std::min<std::size_t>(
                output.size() - output_offset,
                static_cast<std::size_t>(std::numeric_limits<uInt>::max()));
            stream.next_out = reinterpret_cast<Bytef*>(output.data() + output_offset);
            stream.avail_out = static_cast<uInt>(chunk);
            output_offset += chunk;
        }

        code = ::deflate(&stream, input_offset >= data.size() ? Z_FINISH : Z_NO_FLUSH);
        if (code == Z_STREAM_END) {
            break;
        }

        if (code != Z_OK) {
            static_cast<void>(::deflateEnd(&stream));
            return std::unexpected(make_error(error_t::io_error));
        }

        if (stream.avail_out == 0 && output_offset >= output.size()) {
            static_cast<void>(::deflateEnd(&stream));
            return std::unexpected(make_error(error_t::length_error));
        }
    }

    const auto total_out = stream.total_out;
    code = ::deflateEnd(&stream);
    if (code != Z_OK) {
        return std::unexpected(make_error(error_t::io_error));
    }

    output.resize(static_cast<std::size_t>(total_out));
    return output;
}

struct zip_central_record {
    std::u8string name;
    std::uint32_t crc32 = 0;
    std::uint32_t compressed_size = 0;
    std::uint32_t uncompressed_size = 0;
    std::uint16_t compression_method = 0;
    std::uint16_t flags = 0;
    std::uint32_t local_header_offset = 0;
};

struct zip_central_entry {
    std::u8string name;
    std::uint64_t uncompressed_size = 0;
    std::uint64_t compressed_size = 0;
    std::uint16_t compression_method = 0;
    std::uint16_t flags = 0;
    std::uint64_t local_header_offset = 0;
    std::uint64_t next_offset = 0;
};

} // namespace detail

/**
 * @brief ZIP archive entry metadata.
 */
class zip_entry {
public:
    constexpr zip_entry() noexcept = default;

    constexpr zip_entry(
        std::u8string name,
        std::uint64_t uncompressed_size,
        std::uint64_t compressed_size,
        std::uint16_t compression_method,
        std::uint16_t flags,
        std::uint64_t local_header_offset) noexcept
        : name_(std::move(name)),
          uncompressed_size_(uncompressed_size),
          compressed_size_(compressed_size),
          compression_method_(compression_method),
          flags_(flags),
          local_header_offset_(local_header_offset) {}

    [[nodiscard]] auto name() const noexcept -> const std::u8string& { return name_; }
    [[nodiscard]] constexpr auto uncompressed_size() const noexcept -> std::uint64_t { return uncompressed_size_; }
    [[nodiscard]] constexpr auto compressed_size() const noexcept -> std::uint64_t { return compressed_size_; }
    [[nodiscard]] constexpr auto compression_method_id() const noexcept -> std::uint16_t { return compression_method_; }
    [[nodiscard]] constexpr auto flags() const noexcept -> std::uint16_t { return flags_; }
    [[nodiscard]] constexpr auto local_header_offset() const noexcept -> std::uint64_t { return local_header_offset_; }

private:
    std::u8string name_;
    std::uint64_t uncompressed_size_ = 0;
    std::uint64_t compressed_size_ = 0;
    std::uint16_t compression_method_ = 0;
    std::uint16_t flags_ = 0;
    std::uint64_t local_header_offset_ = 0;
};

namespace detail {

[[nodiscard]] inline auto zip_entry_from_central_entry(
    const zip_central_entry& central_entry) -> zip_entry
{
    return zip_entry(
        central_entry.name,
        central_entry.uncompressed_size,
        central_entry.compressed_size,
        central_entry.compression_method,
        central_entry.flags,
        central_entry.local_header_offset);
}

} // namespace detail

/**
 * @brief Move-only ZIP archive handle.
 */
class zip_archive {
public:
    zip_archive() noexcept = default;

    zip_archive(const zip_archive&) = delete;
    auto operator=(const zip_archive&) -> zip_archive& = delete;

    zip_archive(zip_archive&&) noexcept = default;
    auto operator=(zip_archive&&) noexcept -> zip_archive& = default;

    zip_archive(
        binary_stream stream,
        std::uint64_t central_directory_offset,
        std::uint64_t central_directory_size,
        std::uint64_t entry_count) noexcept
        : stream_(std::move(stream)),
          mode_(mode::read),
          central_directory_offset_(central_directory_offset),
          central_directory_size_(central_directory_size),
          next_entry_offset_(central_directory_offset),
          entry_count_(entry_count) {}

    explicit zip_archive(binary_stream stream) noexcept
        : stream_(std::move(stream)),
          mode_(mode::write) {}

    [[nodiscard]] auto stream() noexcept -> binary_stream& { return stream_; }
    [[nodiscard]] constexpr auto is_reading() const noexcept -> bool { return mode_ == mode::read; }
    [[nodiscard]] constexpr auto is_writing() const noexcept -> bool { return mode_ == mode::write; }
    [[nodiscard]] constexpr auto is_committed() const noexcept -> bool { return committed_; }
    constexpr auto set_committed() noexcept -> void { committed_ = true; }
    [[nodiscard]] constexpr auto central_directory_offset() const noexcept -> std::uint64_t { return central_directory_offset_; }
    [[nodiscard]] constexpr auto central_directory_size() const noexcept -> std::uint64_t { return central_directory_size_; }
    [[nodiscard]] constexpr auto next_entry_offset() const noexcept -> std::uint64_t { return next_entry_offset_; }
    constexpr auto set_next_entry_offset(std::uint64_t value) noexcept -> void { next_entry_offset_ = value; }
    [[nodiscard]] constexpr auto entry_count() const noexcept -> std::uint64_t { return entry_count_; }
    [[nodiscard]] constexpr auto next_entry_index() const noexcept -> std::uint64_t { return next_entry_index_; }
    constexpr auto increment_next_entry_index() noexcept -> void { ++next_entry_index_; }
    [[nodiscard]] auto central_records() noexcept -> std::vector<detail::zip_central_record>& { return central_records_; }
    [[nodiscard]] auto central_records() const noexcept -> const std::vector<detail::zip_central_record>& { return central_records_; }

private:
    enum class mode {
        empty,
        read,
        write
    };

    binary_stream stream_;
    mode mode_ = mode::empty;
    bool committed_ = false;
    std::uint64_t central_directory_offset_ = 0;
    std::uint64_t central_directory_size_ = 0;
    std::uint64_t next_entry_offset_ = 0;
    std::uint64_t entry_count_ = 0;
    std::uint64_t next_entry_index_ = 0;
    std::vector<detail::zip_central_record> central_records_;
};

/**
 * @brief Opens a ZIP archive for reading.
 *
 * This initial reader supports ordinary non-ZIP64 archives with a single-disk
 * central directory. Entry data may be stored or deflated.
 *
 * @param filename ZIP archive path.
 * @return Open ZIP archive on success.
 */
[[nodiscard]] inline auto zip_open(std::u8string_view filename) -> result<zip_archive>
{
    auto stream = fopen(path(filename), "r");
    if (!stream.has_value()) {
        return std::unexpected(stream.error());
    }

    const auto seek_end_result = fseek(*stream, 0, seek_end);
    if (!seek_end_result.has_value()) {
        return std::unexpected(seek_end_result.error());
    }

    const auto file_size = ftell(*stream);
    if (!file_size.has_value()) {
        return std::unexpected(file_size.error());
    }

    if (*file_size < detail::zip_eocd_min_size) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const std::uint64_t search_size_u64 = std::min<std::uint64_t>(
        *file_size,
        detail::zip_eocd_min_size + detail::zip_max_comment_size);
    const auto search_size = detail::zip_uint64_to_size(search_size_u64);
    if (!search_size.has_value()) {
        return std::unexpected(search_size.error());
    }

    std::vector<std::byte> tail(*search_size);
    const std::uint64_t tail_offset = *file_size - search_size_u64;
    const auto tail_result = detail::zip_read_exact_at(*stream, tail_offset, tail);
    if (!tail_result.has_value()) {
        return std::unexpected(tail_result.error());
    }

    std::size_t eocd_offset = std::string_view::npos;
    for (std::size_t i = tail.size() - detail::zip_eocd_min_size + 1u; i-- > 0u;) {
        if (detail::zip_read_u32(tail, i) != detail::zip_end_of_central_directory_signature) {
            continue;
        }

        const std::uint16_t comment_length = detail::zip_read_u16(tail, i + 20u);
        if (i + detail::zip_eocd_min_size + comment_length == tail.size()) {
            eocd_offset = i;
            break;
        }
    }

    if (eocd_offset == std::string_view::npos) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const std::uint16_t disk_number = detail::zip_read_u16(tail, eocd_offset + 4u);
    const std::uint16_t central_dir_disk = detail::zip_read_u16(tail, eocd_offset + 6u);
    const std::uint16_t entries_on_disk = detail::zip_read_u16(tail, eocd_offset + 8u);
    const std::uint16_t total_entries16 = detail::zip_read_u16(tail, eocd_offset + 10u);
    const std::uint32_t central_size32 = detail::zip_read_u32(tail, eocd_offset + 12u);
    const std::uint32_t central_offset32 = detail::zip_read_u32(tail, eocd_offset + 16u);

    if (disk_number != 0u || central_dir_disk != 0u || entries_on_disk != total_entries16) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (total_entries16 == 0xffffu || central_size32 == 0xffffffffu || central_offset32 == 0xffffffffu) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const std::uint64_t central_offset = central_offset32;
    const std::uint64_t central_size = central_size32;
    if (central_offset > *file_size || central_size > *file_size - central_offset) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return zip_archive(std::move(*stream), central_offset, central_size, total_entries16);
}

/**
 * @brief Creates a ZIP archive for writing.
 *
 * The archive is not complete until zip_commit is called.
 *
 * @param filename ZIP archive path.
 * @return Open ZIP archive writer on success.
 */
[[nodiscard]] inline auto zip_create(std::u8string_view filename) -> result<zip_archive>
{
    auto stream = fopen(path(filename), "w");
    if (!stream.has_value()) {
        return std::unexpected(stream.error());
    }

    return zip_archive(std::move(*stream));
}

namespace detail {

[[nodiscard]] inline auto zip_read_central_entry_at(
    zip_archive& archive,
    std::uint64_t entry_offset) -> result<zip_central_entry>
{
    if (!archive.is_reading()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (entry_offset < archive.central_directory_offset() ||
        entry_offset >= archive.central_directory_offset() + archive.central_directory_size()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::array<std::byte, 46> header{};
    const auto header_result = zip_read_exact_at(archive.stream(), entry_offset, header);
    if (!header_result.has_value()) {
        return std::unexpected(header_result.error());
    }

    if (zip_read_u32(header, 0u) != zip_central_directory_file_header_signature) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const std::uint16_t flags = zip_read_u16(header, 8u);
    const std::uint16_t method = zip_read_u16(header, 10u);
    const std::uint32_t compressed_size32 = zip_read_u32(header, 20u);
    const std::uint32_t uncompressed_size32 = zip_read_u32(header, 24u);
    const std::uint16_t name_length = zip_read_u16(header, 28u);
    const std::uint16_t extra_length = zip_read_u16(header, 30u);
    const std::uint16_t comment_length = zip_read_u16(header, 32u);
    const std::uint16_t disk_start = zip_read_u16(header, 34u);
    const std::uint32_t local_header_offset32 = zip_read_u32(header, 42u);

    if ((flags & zip_flag_encrypted) != 0u || disk_start != 0u) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (compressed_size32 == 0xffffffffu || uncompressed_size32 == 0xffffffffu ||
        local_header_offset32 == 0xffffffffu) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const std::uint64_t variable_size = static_cast<std::uint64_t>(name_length) +
        static_cast<std::uint64_t>(extra_length) +
        static_cast<std::uint64_t>(comment_length);
    const std::uint64_t next_offset = entry_offset + header.size() + variable_size;
    if (next_offset < entry_offset ||
        next_offset > archive.central_directory_offset() + archive.central_directory_size()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::vector<std::byte> name_bytes(name_length);
    if (name_length != 0u) {
        const auto name_result = zip_read_exact_at(
            archive.stream(),
            entry_offset + header.size(),
            name_bytes);
        if (!name_result.has_value()) {
            return std::unexpected(name_result.error());
        }
    }

    const auto name = zip_bytes_to_u8string(name_bytes, flags);
    if (!name.has_value()) {
        return std::unexpected(name.error());
    }

    return zip_central_entry{
        *name,
        uncompressed_size32,
        compressed_size32,
        method,
        flags,
        local_header_offset32,
        next_offset};
}

} // namespace detail

/**
 * @brief Reads the next ZIP entry metadata from an archive.
 *
 * Reaching the end of the central directory is reported as
 * `error_t::end_of_file`.
 *
 * @param archive ZIP archive.
 * @return Next ZIP entry on success.
 */
[[nodiscard]] inline auto zip_read(zip_archive& archive) -> result<zip_entry>
{
    if (!archive.is_reading()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (archive.next_entry_index() >= archive.entry_count()) {
        return std::unexpected(make_error(error_t::end_of_file));
    }

    const auto central_entry = detail::zip_read_central_entry_at(
        archive,
        archive.next_entry_offset());
    if (!central_entry.has_value()) {
        return std::unexpected(central_entry.error());
    }

    archive.set_next_entry_offset(central_entry->next_offset);
    archive.increment_next_entry_index();

    return detail::zip_entry_from_central_entry(*central_entry);
}

[[nodiscard]] inline auto zip_entry_name(const zip_entry& entry) -> result<std::u8string>
{
    return entry.name();
}

[[nodiscard]] inline auto zip_entry_filesize(const zip_entry& entry) -> result<std::uint64_t>
{
    return entry.uncompressed_size();
}

[[nodiscard]] inline auto zip_entry_compressed_size(const zip_entry& entry) -> result<std::uint64_t>
{
    return entry.compressed_size();
}

[[nodiscard]] inline auto zip_entry_compression_method(const zip_entry& entry) -> result<std::u8string>
{
    switch (entry.compression_method_id()) {
    case detail::zip_method_store:
        return std::u8string(u8"store");
    case detail::zip_method_deflate:
        return std::u8string(u8"deflate");
    default:
        return std::u8string(u8"unknown");
    }
}

/**
 * @brief Reads and expands a ZIP entry body.
 *
 * The initial implementation supports stored entries and raw deflate entries.
 * Unsupported compression methods are reported as `error_t::invalid_argument`.
 *
 * @param archive ZIP archive that owns the entry data.
 * @param entry Entry metadata obtained from zip_read.
 * @return Uncompressed entry bytes on success.
 */
[[nodiscard]] inline auto zip_entry_read(zip_archive& archive, const zip_entry& entry)
    -> result<std::vector<std::byte>>
{
    if (!archive.is_reading()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::array<std::byte, 30> local_header{};
    const auto local_result = detail::zip_read_exact_at(
        archive.stream(),
        entry.local_header_offset(),
        local_header);
    if (!local_result.has_value()) {
        return std::unexpected(local_result.error());
    }

    if (detail::zip_read_u32(local_header, 0u) != detail::zip_local_file_header_signature) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const std::uint16_t local_name_length = detail::zip_read_u16(local_header, 26u);
    const std::uint16_t local_extra_length = detail::zip_read_u16(local_header, 28u);
    const std::uint64_t data_offset = entry.local_header_offset() + local_header.size() +
        static_cast<std::uint64_t>(local_name_length) +
        static_cast<std::uint64_t>(local_extra_length);

    const auto compressed_size = detail::zip_uint64_to_size(entry.compressed_size());
    if (!compressed_size.has_value()) {
        return std::unexpected(compressed_size.error());
    }

    std::vector<std::byte> compressed(*compressed_size);
    if (!compressed.empty()) {
        const auto data_result = detail::zip_read_exact_at(archive.stream(), data_offset, compressed);
        if (!data_result.has_value()) {
            return std::unexpected(data_result.error());
        }
    }

    switch (entry.compression_method_id()) {
    case detail::zip_method_store:
        if (entry.compressed_size() != entry.uncompressed_size()) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }
        return compressed;

    case detail::zip_method_deflate:
        return detail::zip_inflate_raw(compressed, entry.uncompressed_size());

    default:
        return std::unexpected(make_error(error_t::invalid_argument));
    }
}

/**
 * @brief Locates a ZIP entry by name.
 *
 * This function scans the central directory and returns the first entry whose
 * name exactly matches `entry_name`. The sequential position used by `zip_read`
 * is not changed. Missing entries are reported as `error_t::not_found`.
 *
 * @param archive ZIP archive.
 * @param entry_name Entry name to locate.
 * @return Matching ZIP entry on success.
 */
[[nodiscard]] inline auto zip_locate_name(zip_archive& archive, std::u8string_view entry_name)
    -> result<zip_entry>
{
    if (!archive.is_reading()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::uint64_t offset = archive.central_directory_offset();
    for (std::uint64_t i = 0; i < archive.entry_count(); ++i) {
        const auto central_entry = detail::zip_read_central_entry_at(archive, offset);
        if (!central_entry.has_value()) {
            return std::unexpected(central_entry.error());
        }

        if (central_entry->name == entry_name) {
            return detail::zip_entry_from_central_entry(*central_entry);
        }

        offset = central_entry->next_offset;
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Locates a ZIP entry by name and reads its expanded body.
 *
 * This convenience function is equivalent to calling `zip_locate_name` and then
 * `zip_entry_read`.
 *
 * @param archive ZIP archive.
 * @param entry_name Entry name to read.
 * @return Uncompressed entry bytes on success.
 */
[[nodiscard]] inline auto zip_entry_read_by_name(
    zip_archive& archive,
    std::u8string_view entry_name) -> result<std::vector<std::byte>>
{
    auto entry = zip_locate_name(archive, entry_name);
    if (!entry.has_value()) {
        return std::unexpected(entry.error());
    }

    return zip_entry_read(archive, *entry);
}

/**
 * @brief Extracts one ZIP entry to a specified path.
 *
 * Parent directories of @p target_filename are created as needed. If @p entry
 * is a directory entry, @p target_filename is created as a directory.
 *
 * @param archive ZIP archive.
 * @param entry Entry metadata.
 * @param target_filename Destination path.
 * @return Empty success value on success.
 */
[[nodiscard]] inline auto zip_entry_extract(
    zip_archive& archive,
    const zip_entry& entry,
    std::u8string_view target_filename) -> result<void>
{
    if (!archive.is_reading()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const path target(target_filename);
    if (detail::zip_is_directory_name(entry.name())) {
        return detail::zip_make_directories(target);
    }

    const auto parent_result = detail::zip_make_parent_directories(target);
    if (!parent_result.has_value()) {
        return std::unexpected(parent_result.error());
    }

    const auto data = zip_entry_read(archive, entry);
    if (!data.has_value()) {
        return std::unexpected(data.error());
    }

    return file_put_contents(target, *data);
}

/**
 * @brief Extracts one ZIP entry below a destination directory.
 *
 * The entry name is treated as a relative path inside the destination
 * directory. Absolute paths, drive-relative paths, empty components, `.`
 * components, and `..` components are rejected as `error_t::invalid_argument`.
 *
 * @param archive ZIP archive.
 * @param entry Entry metadata.
 * @param destination_dir Destination directory.
 * @return Empty success value on success.
 */
[[nodiscard]] inline auto zip_entry_extract_to(
    zip_archive& archive,
    const zip_entry& entry,
    std::u8string_view destination_dir) -> result<void>
{
    if (!archive.is_reading()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const auto destination = detail::zip_join_safe_entry_path(path(destination_dir), entry.name());
    if (!destination.has_value()) {
        return std::unexpected(destination.error());
    }

    return zip_entry_extract(archive, entry, destination->str());
}

/**
 * @brief Extracts all ZIP entries below a destination directory.
 *
 * This function scans the central directory directly and does not change the
 * sequential position used by `zip_read`. Entry names are checked with the same
 * path-safety rules as `zip_entry_extract_to`.
 *
 * @param archive ZIP archive.
 * @param destination_dir Destination directory.
 * @return Empty success value on success.
 */
[[nodiscard]] inline auto zip_extract_to(
    zip_archive& archive,
    std::u8string_view destination_dir) -> result<void>
{
    if (!archive.is_reading()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const auto root_result = detail::zip_make_directories(path(destination_dir));
    if (!root_result.has_value()) {
        return std::unexpected(root_result.error());
    }

    std::uint64_t offset = archive.central_directory_offset();
    for (std::uint64_t i = 0; i < archive.entry_count(); ++i) {
        const auto central_entry = detail::zip_read_central_entry_at(archive, offset);
        if (!central_entry.has_value()) {
            return std::unexpected(central_entry.error());
        }

        const auto entry = detail::zip_entry_from_central_entry(*central_entry);
        const auto extracted = zip_entry_extract_to(archive, entry, destination_dir);
        if (!extracted.has_value()) {
            return std::unexpected(extracted.error());
        }

        offset = central_entry->next_offset;
    }

    return {};
}

/**
 * @brief Adds an in-memory byte sequence to a ZIP archive.
 *
 * Entry data is compressed with deflate. The initial writer creates ordinary
 * non-ZIP64 single-disk archives with UTF-8 entry names.
 *
 * @param archive ZIP archive writer.
 * @param entry_name Entry name stored in the archive.
 * @param data Uncompressed entry data.
 * @return Empty success value on success.
 */
[[nodiscard]] inline auto zip_add_from_bytes(
    zip_archive& archive,
    std::u8string_view entry_name,
    std::span<const std::byte> data) -> result<void>
{
    if (!archive.is_writing() || archive.is_committed()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (entry_name.empty()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::string name_narrow;
    name_narrow.reserve(entry_name.size());
    for (const char8_t c : entry_name) {
        name_narrow.push_back(static_cast<char>(c));
    }

    if (!detail::is_valid_utf8(name_narrow)) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    const auto name_length = detail::zip_size_to_u16(name_narrow.size());
    if (!name_length.has_value()) {
        return std::unexpected(name_length.error());
    }

    const auto uncompressed_size32 = detail::zip_u64_to_u32(data.size());
    if (!uncompressed_size32.has_value()) {
        return std::unexpected(uncompressed_size32.error());
    }

    if (archive.central_records().size() >= static_cast<std::size_t>(std::numeric_limits<std::uint16_t>::max())) {
        return std::unexpected(make_error(error_t::length_error));
    }

    const auto compressed = detail::zip_deflate_raw(data);
    if (!compressed.has_value()) {
        return std::unexpected(compressed.error());
    }

    const auto compressed_size32 = detail::zip_u64_to_u32(compressed->size());
    if (!compressed_size32.has_value()) {
        return std::unexpected(compressed_size32.error());
    }

    const auto local_offset64 = ftell(archive.stream());
    if (!local_offset64.has_value()) {
        return std::unexpected(local_offset64.error());
    }

    const auto local_offset32 = detail::zip_u64_to_u32(*local_offset64);
    if (!local_offset32.has_value()) {
        return std::unexpected(local_offset32.error());
    }

    const std::uint32_t crc = static_cast<std::uint32_t>(::crc32(
        0,
        reinterpret_cast<const Bytef*>(data.data()),
        static_cast<uInt>(std::min<std::size_t>(
            data.size(),
            static_cast<std::size_t>(std::numeric_limits<uInt>::max())))));

    std::uint32_t full_crc = crc;
    if (data.size() > static_cast<std::size_t>(std::numeric_limits<uInt>::max())) {
        uLong running_crc = 0;
        std::size_t offset = 0;
        while (offset < data.size()) {
            const std::size_t chunk = std::min<std::size_t>(
                data.size() - offset,
                static_cast<std::size_t>(std::numeric_limits<uInt>::max()));
            running_crc = ::crc32(
                running_crc,
                reinterpret_cast<const Bytef*>(data.data() + offset),
                static_cast<uInt>(chunk));
            offset += chunk;
        }
        full_crc = static_cast<std::uint32_t>(running_crc);
    }

    std::vector<std::byte> local_header;
    local_header.reserve(30u + name_narrow.size());
    detail::zip_append_u32(local_header, detail::zip_local_file_header_signature);
    detail::zip_append_u16(local_header, 20u);
    detail::zip_append_u16(local_header, detail::zip_flag_utf8_name);
    detail::zip_append_u16(local_header, detail::zip_method_deflate);
    detail::zip_append_u16(local_header, 0u);
    detail::zip_append_u16(local_header, 0u);
    detail::zip_append_u32(local_header, full_crc);
    detail::zip_append_u32(local_header, *compressed_size32);
    detail::zip_append_u32(local_header, *uncompressed_size32);
    detail::zip_append_u16(local_header, *name_length);
    detail::zip_append_u16(local_header, 0u);
    for (const char c : name_narrow) {
        local_header.push_back(static_cast<std::byte>(static_cast<unsigned char>(c)));
    }

    const auto header_written = detail::zip_write_exact(archive.stream(), local_header);
    if (!header_written.has_value()) {
        return std::unexpected(header_written.error());
    }

    const auto body_written = detail::zip_write_exact(archive.stream(), *compressed);
    if (!body_written.has_value()) {
        return std::unexpected(body_written.error());
    }

    archive.central_records().push_back(detail::zip_central_record{
        std::u8string(entry_name),
        full_crc,
        *compressed_size32,
        *uncompressed_size32,
        detail::zip_method_deflate,
        detail::zip_flag_utf8_name,
        *local_offset32});

    return {};
}

/**
 * @brief Adds a file to a ZIP archive.
 *
 * The source file is read into memory and then compressed into one ZIP entry.
 *
 * @param archive ZIP archive writer.
 * @param source_path Source file path.
 * @param entry_name Entry name stored in the archive.
 * @return Empty success value on success.
 */
[[nodiscard]] inline auto zip_add_file(
    zip_archive& archive,
    std::u8string_view source_path,
    std::u8string_view entry_name) -> result<void>
{
    const auto data = file_get_contents(path(source_path));
    if (!data.has_value()) {
        return std::unexpected(data.error());
    }

    return zip_add_from_bytes(archive, entry_name, *data);
}

/**
 * @brief Finalizes a ZIP archive writer.
 *
 * This function writes the central directory and closes the underlying stream.
 * Write-side errors can occur during finalization, so callers should invoke
 * this function explicitly instead of relying only on destruction.
 *
 * @param archive ZIP archive writer.
 * @return Empty success value on success.
 */
[[nodiscard]] inline auto zip_commit(zip_archive& archive) -> result<void>
{
    if (!archive.is_writing() || archive.is_committed()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const auto central_offset64 = ftell(archive.stream());
    if (!central_offset64.has_value()) {
        return std::unexpected(central_offset64.error());
    }

    const auto central_offset32 = detail::zip_u64_to_u32(*central_offset64);
    if (!central_offset32.has_value()) {
        return std::unexpected(central_offset32.error());
    }

    std::vector<std::byte> central;
    for (const auto& record : archive.central_records()) {
        std::string name_narrow;
        name_narrow.reserve(record.name.size());
        for (const char8_t c : record.name) {
            name_narrow.push_back(static_cast<char>(c));
        }

        const auto name_length = detail::zip_size_to_u16(name_narrow.size());
        if (!name_length.has_value()) {
            return std::unexpected(name_length.error());
        }

        detail::zip_append_u32(central, detail::zip_central_directory_file_header_signature);
        detail::zip_append_u16(central, 20u);
        detail::zip_append_u16(central, 20u);
        detail::zip_append_u16(central, record.flags);
        detail::zip_append_u16(central, record.compression_method);
        detail::zip_append_u16(central, 0u);
        detail::zip_append_u16(central, 0u);
        detail::zip_append_u32(central, record.crc32);
        detail::zip_append_u32(central, record.compressed_size);
        detail::zip_append_u32(central, record.uncompressed_size);
        detail::zip_append_u16(central, *name_length);
        detail::zip_append_u16(central, 0u);
        detail::zip_append_u16(central, 0u);
        detail::zip_append_u16(central, 0u);
        detail::zip_append_u16(central, 0u);
        detail::zip_append_u32(central, 0u);
        detail::zip_append_u32(central, record.local_header_offset);
        for (const char c : name_narrow) {
            central.push_back(static_cast<std::byte>(static_cast<unsigned char>(c)));
        }
    }

    const auto central_written = detail::zip_write_exact(archive.stream(), central);
    if (!central_written.has_value()) {
        return std::unexpected(central_written.error());
    }

    const auto central_size32 = detail::zip_u64_to_u32(central.size());
    if (!central_size32.has_value()) {
        return std::unexpected(central_size32.error());
    }

    const auto entry_count = detail::zip_size_to_u16(archive.central_records().size());
    if (!entry_count.has_value()) {
        return std::unexpected(entry_count.error());
    }

    std::vector<std::byte> eocd;
    eocd.reserve(detail::zip_eocd_min_size);
    detail::zip_append_u32(eocd, detail::zip_end_of_central_directory_signature);
    detail::zip_append_u16(eocd, 0u);
    detail::zip_append_u16(eocd, 0u);
    detail::zip_append_u16(eocd, *entry_count);
    detail::zip_append_u16(eocd, *entry_count);
    detail::zip_append_u32(eocd, *central_size32);
    detail::zip_append_u32(eocd, *central_offset32);
    detail::zip_append_u16(eocd, 0u);

    const auto eocd_written = detail::zip_write_exact(archive.stream(), eocd);
    if (!eocd_written.has_value()) {
        return std::unexpected(eocd_written.error());
    }

    const auto flushed = fflush(archive.stream());
    if (!flushed.has_value()) {
        return std::unexpected(flushed.error());
    }

    const auto closed = fclose(archive.stream());
    if (!closed.has_value()) {
        return std::unexpected(closed.error());
    }

    archive.set_committed();
    return {};
}

} // namespace xer

#endif /* XER_BITS_ZIP_H_INCLUDED_ */
