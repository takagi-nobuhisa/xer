#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <zlib.h>

#include <xer/assert.h>
#include <xer/error.h>
#include <xer/zip.h>

// XER_TEST_FEATURES: zip

namespace {

struct sample_entry {
    std::u8string name;
    std::vector<std::byte> data;
    std::uint16_t method;
};

[[nodiscard]] auto bytes(std::string_view text) -> std::vector<std::byte>
{
    std::vector<std::byte> result;
    result.reserve(text.size());

    for (const char c : text) {
        result.push_back(static_cast<std::byte>(static_cast<unsigned char>(c)));
    }

    return result;
}

void append_u16(std::vector<std::byte>& out, std::uint16_t value)
{
    out.push_back(static_cast<std::byte>(value & 0xffu));
    out.push_back(static_cast<std::byte>((value >> 8u) & 0xffu));
}

void append_u32(std::vector<std::byte>& out, std::uint32_t value)
{
    append_u16(out, static_cast<std::uint16_t>(value & 0xffffu));
    append_u16(out, static_cast<std::uint16_t>((value >> 16u) & 0xffffu));
}

void append_bytes(std::vector<std::byte>& out, std::span<const std::byte> bytes_)
{
    out.insert(out.end(), bytes_.begin(), bytes_.end());
}

[[nodiscard]] auto narrow(std::u8string_view text) -> std::string
{
    std::string result;
    result.reserve(text.size());

    for (const char8_t c : text) {
        result.push_back(static_cast<char>(c));
    }

    return result;
}

[[nodiscard]] auto deflate_raw(std::span<const std::byte> data) -> std::vector<std::byte>
{
    z_stream stream{};
    xer_assert_eq(::deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY), Z_OK);

    std::vector<std::byte> output(data.size() + 64u);
    stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(data.data()));
    stream.avail_in = static_cast<uInt>(data.size());
    stream.next_out = reinterpret_cast<Bytef*>(output.data());
    stream.avail_out = static_cast<uInt>(output.size());

    const int code = ::deflate(&stream, Z_FINISH);
    xer_assert_eq(code, Z_STREAM_END);
    output.resize(stream.total_out);
    xer_assert_eq(::deflateEnd(&stream), Z_OK);
    return output;
}

void write_file(std::string_view filename, std::span<const std::byte> data)
{
    std::ofstream out(std::string(filename), std::ios::binary);
    xer_assert(out.good());
    out.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    xer_assert(out.good());
}

void make_zip(std::string_view filename, std::span<const sample_entry> entries)
{
    struct central_record {
        sample_entry entry;
        std::vector<std::byte> compressed;
        std::uint32_t crc;
        std::uint32_t local_offset;
    };

    std::vector<std::byte> out;
    std::vector<central_record> central;

    for (const auto& entry : entries) {
        const auto name = narrow(entry.name);
        const auto compressed = entry.method == 8u ? deflate_raw(entry.data) : entry.data;
        const auto crc = static_cast<std::uint32_t>(::crc32(0, reinterpret_cast<const Bytef*>(entry.data.data()), static_cast<uInt>(entry.data.size())));
        const auto local_offset = static_cast<std::uint32_t>(out.size());

        append_u32(out, 0x04034b50u);
        append_u16(out, 20u);
        append_u16(out, 0x0800u);
        append_u16(out, entry.method);
        append_u16(out, 0u);
        append_u16(out, 0u);
        append_u32(out, crc);
        append_u32(out, static_cast<std::uint32_t>(compressed.size()));
        append_u32(out, static_cast<std::uint32_t>(entry.data.size()));
        append_u16(out, static_cast<std::uint16_t>(name.size()));
        append_u16(out, 0u);
        for (const char c : name) {
            out.push_back(static_cast<std::byte>(static_cast<unsigned char>(c)));
        }
        append_bytes(out, compressed);

        central.push_back(central_record{entry, compressed, crc, local_offset});
    }

    const auto central_offset = static_cast<std::uint32_t>(out.size());

    for (const auto& record : central) {
        const auto name = narrow(record.entry.name);
        append_u32(out, 0x02014b50u);
        append_u16(out, 20u);
        append_u16(out, 20u);
        append_u16(out, 0x0800u);
        append_u16(out, record.entry.method);
        append_u16(out, 0u);
        append_u16(out, 0u);
        append_u32(out, record.crc);
        append_u32(out, static_cast<std::uint32_t>(record.compressed.size()));
        append_u32(out, static_cast<std::uint32_t>(record.entry.data.size()));
        append_u16(out, static_cast<std::uint16_t>(name.size()));
        append_u16(out, 0u);
        append_u16(out, 0u);
        append_u16(out, 0u);
        append_u16(out, 0u);
        append_u32(out, 0u);
        append_u32(out, record.local_offset);
        for (const char c : name) {
            out.push_back(static_cast<std::byte>(static_cast<unsigned char>(c)));
        }
    }

    const auto central_size = static_cast<std::uint32_t>(out.size()) - central_offset;

    append_u32(out, 0x06054b50u);
    append_u16(out, 0u);
    append_u16(out, 0u);
    append_u16(out, static_cast<std::uint16_t>(central.size()));
    append_u16(out, static_cast<std::uint16_t>(central.size()));
    append_u32(out, central_size);
    append_u32(out, central_offset);
    append_u16(out, 0u);

    write_file(filename, out);
}

void assert_bytes_eq(std::span<const std::byte> lhs, std::span<const std::byte> rhs)
{
    xer_assert_eq(lhs.size(), rhs.size());

    for (std::size_t i = 0; i < lhs.size(); ++i) {
        xer_assert(lhs[i] == rhs[i]);
    }
}

void test_zip_read_store_and_deflate()
{
    const std::array entries = {
        sample_entry{u8"hello.txt", bytes("hello from zip\n"), 0u},
        sample_entry{u8"data/message.txt", bytes("deflated text data"), 8u},
    };
    make_zip("sample.zip", entries);

    auto archive = xer::zip_open(u8"sample.zip");
    xer_assert(archive.has_value());

    auto first = xer::zip_read(*archive);
    xer_assert(first.has_value());
    auto first_name = xer::zip_entry_name(*first);
    auto first_size = xer::zip_entry_filesize(*first);
    auto first_compressed_size = xer::zip_entry_compressed_size(*first);
    auto first_method = xer::zip_entry_compression_method(*first);
    auto first_body = xer::zip_entry_read(*archive, *first);
    xer_assert(first_name.has_value());
    xer_assert(first_size.has_value());
    xer_assert(first_compressed_size.has_value());
    xer_assert(first_method.has_value());
    xer_assert(first_body.has_value());
    xer_assert_eq(*first_name, u8"hello.txt");
    xer_assert_eq(*first_size, entries[0].data.size());
    xer_assert_eq(*first_compressed_size, entries[0].data.size());
    xer_assert_eq(*first_method, u8"store");
    assert_bytes_eq(*first_body, entries[0].data);

    auto second = xer::zip_read(*archive);
    xer_assert(second.has_value());
    auto second_name = xer::zip_entry_name(*second);
    auto second_size = xer::zip_entry_filesize(*second);
    auto second_method = xer::zip_entry_compression_method(*second);
    auto second_body = xer::zip_entry_read(*archive, *second);
    xer_assert(second_name.has_value());
    xer_assert(second_size.has_value());
    xer_assert(second_method.has_value());
    xer_assert(second_body.has_value());
    xer_assert_eq(*second_name, u8"data/message.txt");
    xer_assert_eq(*second_size, entries[1].data.size());
    xer_assert_eq(*second_method, u8"deflate");
    assert_bytes_eq(*second_body, entries[1].data);

    const auto end = xer::zip_read(*archive);
    xer_assert_not(end.has_value());
    xer_assert_eq(end.error().code, xer::error_t::end_of_file);
}

void test_zip_open_rejects_invalid_file()
{
    write_file("not-a-zip.zip", bytes("not a zip file"));

    const auto archive = xer::zip_open(u8"not-a-zip.zip");
    xer_assert_not(archive.has_value());
    xer_assert_eq(archive.error().code, xer::error_t::invalid_argument);
}

} // namespace

auto main() -> int
{
    test_zip_read_store_and_deflate();
    test_zip_open_rejects_invalid_file();
}
