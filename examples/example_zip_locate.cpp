#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

#include <xer/stdio.h>
#include <xer/zip.h>

// XER_TEST_FEATURES: zip

namespace {

[[nodiscard]] auto as_bytes(std::string_view text) -> std::vector<std::byte>
{
    std::vector<std::byte> result;
    result.reserve(text.size());

    for (const char c : text) {
        result.push_back(static_cast<std::byte>(static_cast<unsigned char>(c)));
    }

    return result;
}

[[nodiscard]] auto as_u8string(std::span<const std::byte> data) -> std::u8string
{
    std::u8string result;
    result.reserve(data.size());

    for (const std::byte b : data) {
        result.push_back(static_cast<char8_t>(b));
    }

    return result;
}

} // namespace

// XER_EXAMPLE_BEGIN: zip_locate
//
// This example creates a ZIP archive, locates an entry by name, and reads it.
//
// Expected output:
// Entry: docs/readme.txt
// Body: located entry body

auto main() -> int
{
    auto writer = xer::zip_create(u8"located.zip");
    if (!writer.has_value()) {
        return 1;
    }

    const auto first = as_bytes("first body\n");
    const auto target = as_bytes("located entry body\n");

    if (!xer::zip_add_from_bytes(*writer, u8"first.txt", first).has_value()) {
        return 1;
    }
    if (!xer::zip_add_from_bytes(*writer, u8"docs/readme.txt", target).has_value()) {
        return 1;
    }
    if (!xer::zip_commit(*writer).has_value()) {
        return 1;
    }

    auto archive = xer::zip_open(u8"located.zip");
    if (!archive.has_value()) {
        return 1;
    }

    auto entry = xer::zip_locate_name(*archive, u8"docs/readme.txt");
    if (!entry.has_value()) {
        return 1;
    }

    auto name = xer::zip_entry_name(*entry);
    auto data = xer::zip_entry_read(*archive, *entry);
    if (!name.has_value() || !data.has_value()) {
        return 1;
    }

    if (!xer::printf(u8"Entry: %@\n", *name).has_value()) {
        return 1;
    }
    if (!xer::printf(u8"Body: %@", as_u8string(*data)).has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: zip_locate
