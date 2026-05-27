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

[[nodiscard]] auto as_u8string(std::span<const std::byte> bytes) -> std::u8string
{
    std::u8string result;
    result.reserve(bytes.size());

    for (const auto b : bytes) {
        result.push_back(static_cast<char8_t>(b));
    }

    return result;
}

} // namespace

// XER_EXAMPLE_BEGIN: zip_extract
//
// This example creates a ZIP archive, extracts all entries to a directory,
// and reads one extracted file back.
//
// Expected output:
// Extracted: hello from extracted zip

auto main() -> int
{
    const auto body = as_bytes("hello from extracted zip\n");

    auto writer = xer::zip_create(u8"extract_source.zip");
    if (!writer.has_value()) {
        return 1;
    }

    if (!xer::zip_add_from_bytes(*writer, u8"docs/hello.txt", body).has_value()) {
        return 1;
    }

    if (!xer::zip_commit(*writer).has_value()) {
        return 1;
    }

    auto reader = xer::zip_open(u8"extract_source.zip");
    if (!reader.has_value()) {
        return 1;
    }

    if (!xer::zip_extract_to(*reader, u8"extract_out").has_value()) {
        return 1;
    }

    auto extracted = xer::file_get_contents(xer::path(u8"extract_out/docs/hello.txt"));
    if (!extracted.has_value()) {
        return 1;
    }

    if (!xer::printf(u8"Extracted: %@", as_u8string(*extracted)).has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: zip_extract
