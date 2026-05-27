#include <cstddef>
#include <cstdint>
#include <fstream>
#include <span>
#include <string_view>
#include <vector>

#include <xer/stdio.h>
#include <xer/zip.h>

// XER_TEST_FEATURES: zip

namespace {

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

void append_ascii(std::vector<std::byte>& out, std::string_view text)
{
    for (const char c : text) {
        out.push_back(static_cast<std::byte>(static_cast<unsigned char>(c)));
    }
}

void write_minimal_zip()
{
    constexpr std::string_view name = "hello.txt";
    constexpr std::string_view text = "hello from zip\n";

    std::vector<std::byte> out;

    append_u32(out, 0x04034b50u);
    append_u16(out, 20u);
    append_u16(out, 0x0800u);
    append_u16(out, 0u);
    append_u16(out, 0u);
    append_u16(out, 0u);
    append_u32(out, 0u);
    append_u32(out, static_cast<std::uint32_t>(text.size()));
    append_u32(out, static_cast<std::uint32_t>(text.size()));
    append_u16(out, static_cast<std::uint16_t>(name.size()));
    append_u16(out, 0u);
    append_ascii(out, name);
    append_ascii(out, text);

    const auto central_offset = static_cast<std::uint32_t>(out.size());

    append_u32(out, 0x02014b50u);
    append_u16(out, 20u);
    append_u16(out, 20u);
    append_u16(out, 0x0800u);
    append_u16(out, 0u);
    append_u16(out, 0u);
    append_u16(out, 0u);
    append_u32(out, 0u);
    append_u32(out, static_cast<std::uint32_t>(text.size()));
    append_u32(out, static_cast<std::uint32_t>(text.size()));
    append_u16(out, static_cast<std::uint16_t>(name.size()));
    append_u16(out, 0u);
    append_u16(out, 0u);
    append_u16(out, 0u);
    append_u16(out, 0u);
    append_u32(out, 0u);
    append_u32(out, 0u);
    append_ascii(out, name);

    const auto central_size = static_cast<std::uint32_t>(out.size()) - central_offset;

    append_u32(out, 0x06054b50u);
    append_u16(out, 0u);
    append_u16(out, 0u);
    append_u16(out, 1u);
    append_u16(out, 1u);
    append_u32(out, central_size);
    append_u32(out, central_offset);
    append_u16(out, 0u);

    std::ofstream file("sample.zip", std::ios::binary);
    file.write(reinterpret_cast<const char*>(out.data()), static_cast<std::streamsize>(out.size()));
}

} // namespace

// XER_EXAMPLE_BEGIN: zip_basic
//
// This example opens a ZIP archive, reads one entry, and expands its contents.
//
// Expected output:
// Entry: hello.txt
// Size: 15
// Method: store
// Body: hello from zip

auto main() -> int
{
    write_minimal_zip();

    auto archive = xer::zip_open(u8"sample.zip");
    if (!archive.has_value()) {
        return 1;
    }

    auto entry = xer::zip_read(*archive);
    if (!entry.has_value()) {
        return 1;
    }

    auto name = xer::zip_entry_name(*entry);
    auto size = xer::zip_entry_filesize(*entry);
    auto method = xer::zip_entry_compression_method(*entry);
    auto body = xer::zip_entry_read(*archive, *entry);
    if (!name.has_value() || !size.has_value() || !method.has_value() || !body.has_value()) {
        return 1;
    }

    std::u8string text;
    text.reserve(body->size());
    for (const auto b : *body) {
        text.push_back(static_cast<char8_t>(b));
    }

    if (!xer::printf(u8"Entry: %@\n", *name).has_value()) {
        return 1;
    }
    if (!xer::printf(u8"Size: %@\n", *size).has_value()) {
        return 1;
    }
    if (!xer::printf(u8"Method: %@\n", *method).has_value()) {
        return 1;
    }
    if (!xer::printf(u8"Body: %@", text).has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: zip_basic
