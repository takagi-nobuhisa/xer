/**
 * @file xer/bits/zip.h
 * @brief ZIP archive reading utilities.
 */

#pragma once

#ifndef XER_BITS_ZIP_H_INCLUDED_
#define XER_BITS_ZIP_H_INCLUDED_

#include <algorithm>
#include <array>
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

/**
 * @brief Move-only ZIP archive reader.
 */
class zip_archive {
public:
    constexpr zip_archive() noexcept = default;

    zip_archive(const zip_archive&) = delete;
    auto operator=(const zip_archive&) -> zip_archive& = delete;

    constexpr zip_archive(zip_archive&&) noexcept = default;
    auto operator=(zip_archive&&) noexcept -> zip_archive& = default;

    constexpr zip_archive(
        binary_stream stream,
        std::uint64_t central_directory_offset,
        std::uint64_t central_directory_size,
        std::uint64_t entry_count) noexcept
        : stream_(std::move(stream)),
          central_directory_offset_(central_directory_offset),
          central_directory_size_(central_directory_size),
          next_entry_offset_(central_directory_offset),
          entry_count_(entry_count) {}

    [[nodiscard]] auto stream() noexcept -> binary_stream& { return stream_; }
    [[nodiscard]] constexpr auto central_directory_offset() const noexcept -> std::uint64_t { return central_directory_offset_; }
    [[nodiscard]] constexpr auto central_directory_size() const noexcept -> std::uint64_t { return central_directory_size_; }
    [[nodiscard]] constexpr auto next_entry_offset() const noexcept -> std::uint64_t { return next_entry_offset_; }
    constexpr auto set_next_entry_offset(std::uint64_t value) noexcept -> void { next_entry_offset_ = value; }
    [[nodiscard]] constexpr auto entry_count() const noexcept -> std::uint64_t { return entry_count_; }
    [[nodiscard]] constexpr auto next_entry_index() const noexcept -> std::uint64_t { return next_entry_index_; }
    constexpr auto increment_next_entry_index() noexcept -> void { ++next_entry_index_; }

private:
    binary_stream stream_;
    std::uint64_t central_directory_offset_ = 0;
    std::uint64_t central_directory_size_ = 0;
    std::uint64_t next_entry_offset_ = 0;
    std::uint64_t entry_count_ = 0;
    std::uint64_t next_entry_index_ = 0;
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
    if (archive.next_entry_index() >= archive.entry_count()) {
        return std::unexpected(make_error(error_t::end_of_file));
    }

    std::array<std::byte, 46> header{};
    const auto header_result = detail::zip_read_exact_at(
        archive.stream(),
        archive.next_entry_offset(),
        header);
    if (!header_result.has_value()) {
        return std::unexpected(header_result.error());
    }

    if (detail::zip_read_u32(header, 0u) != detail::zip_central_directory_file_header_signature) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const std::uint16_t flags = detail::zip_read_u16(header, 8u);
    const std::uint16_t method = detail::zip_read_u16(header, 10u);
    const std::uint32_t compressed_size32 = detail::zip_read_u32(header, 20u);
    const std::uint32_t uncompressed_size32 = detail::zip_read_u32(header, 24u);
    const std::uint16_t name_length = detail::zip_read_u16(header, 28u);
    const std::uint16_t extra_length = detail::zip_read_u16(header, 30u);
    const std::uint16_t comment_length = detail::zip_read_u16(header, 32u);
    const std::uint16_t disk_start = detail::zip_read_u16(header, 34u);
    const std::uint32_t local_header_offset32 = detail::zip_read_u32(header, 42u);

    if ((flags & detail::zip_flag_encrypted) != 0u || disk_start != 0u) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (compressed_size32 == 0xffffffffu || uncompressed_size32 == 0xffffffffu ||
        local_header_offset32 == 0xffffffffu) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const std::uint64_t variable_size = static_cast<std::uint64_t>(name_length) +
        static_cast<std::uint64_t>(extra_length) +
        static_cast<std::uint64_t>(comment_length);
    const std::uint64_t next_offset = archive.next_entry_offset() + header.size() + variable_size;
    if (next_offset < archive.next_entry_offset() ||
        next_offset > archive.central_directory_offset() + archive.central_directory_size()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::vector<std::byte> name_bytes(name_length);
    if (name_length != 0u) {
        const auto name_result = detail::zip_read_exact_at(
            archive.stream(),
            archive.next_entry_offset() + header.size(),
            name_bytes);
        if (!name_result.has_value()) {
            return std::unexpected(name_result.error());
        }
    }

    const auto name = detail::zip_bytes_to_u8string(name_bytes, flags);
    if (!name.has_value()) {
        return std::unexpected(name.error());
    }

    archive.set_next_entry_offset(next_offset);
    archive.increment_next_entry_index();

    return zip_entry(
        *name,
        uncompressed_size32,
        compressed_size32,
        method,
        flags,
        local_header_offset32);
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

} // namespace xer

#endif /* XER_BITS_ZIP_H_INCLUDED_ */
