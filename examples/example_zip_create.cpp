#include <cstddef>
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

} // namespace

// XER_EXAMPLE_BEGIN: zip_create
//
// This example creates a ZIP archive, adds one in-memory entry, commits it,
// and opens the archive again to verify the stored entry.
//
// Expected output:
// Entry: hello.txt
// Body: hello from created zip

auto main() -> int
{
    const auto body = as_bytes("hello from created zip\n");

    auto writer = xer::zip_create(u8"created.zip");
    if (!writer.has_value()) {
        return 1;
    }

    if (!xer::zip_add_from_bytes(*writer, u8"hello.txt", body).has_value()) {
        return 1;
    }

    if (!xer::zip_commit(*writer).has_value()) {
        return 1;
    }

    auto reader = xer::zip_open(u8"created.zip");
    if (!reader.has_value()) {
        return 1;
    }

    auto entry = xer::zip_read(*reader);
    if (!entry.has_value()) {
        return 1;
    }

    auto name = xer::zip_entry_name(*entry);
    auto data = xer::zip_entry_read(*reader, *entry);
    if (!name.has_value() || !data.has_value()) {
        return 1;
    }

    std::u8string text;
    text.reserve(data->size());
    for (const auto b : *data) {
        text.push_back(static_cast<char8_t>(b));
    }

    if (!xer::printf(u8"Entry: %@\n", *name).has_value()) {
        return 1;
    }
    if (!xer::printf(u8"Body: %@", text).has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: zip_create
