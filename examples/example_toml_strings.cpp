#include <cmath>
#include <xer/stdio.h>
#include <xer/toml.h>

#include <string_view>

namespace {

auto find_key(const xer::toml_table& table, std::u8string_view key)
    -> const xer::toml_value*
{
    for (const auto& entry : table) {
        if (entry.first == key) {
            return &entry.second;
        }
    }

    return nullptr;
}

} // namespace

auto main() -> int
{
    // XER_EXAMPLE_BEGIN: toml_strings
    //
    // This example decodes TOML literal strings, multiline strings, Unicode
    // escapes, and special floating-point values.
    //
    // Expected output:
    // path = C:\\Users\\xer
    // greeting = Hello, π!
    // poem = first line
    // second line
    // status = inf

    const auto decoded = xer::toml_decode(
        u8"path = 'C:\\Users\\xer'\n"
        u8"greeting = \"Hello, \\u03C0!\"\n"
        u8"poem = \"\"\"\nfirst line\nsecond line\"\"\"\n"
        u8"status = inf\n");

    if (!decoded.has_value()) {
        return 1;
    }

    const auto* root = decoded->as_table();
    if (root == nullptr) {
        return 1;
    }

    const auto* path = find_key(*root, u8"path");
    const auto* greeting = find_key(*root, u8"greeting");
    const auto* poem = find_key(*root, u8"poem");
    const auto* status = find_key(*root, u8"status");

    if (path == nullptr || greeting == nullptr || poem == nullptr ||
        status == nullptr || !path->is_string() || !greeting->is_string() ||
        !poem->is_string() || !status->is_float()) {
        return 1;
    }

    if (!std::isinf(*status->as_float())) {
        return 1;
    }

    if (!xer::printf(u8"path = %@\n", *path->as_string()).has_value()) {
        return 1;
    }

    if (!xer::printf(u8"greeting = %@\n", *greeting->as_string()).has_value()) {
        return 1;
    }

    if (!xer::printf(u8"poem = %@\n", *poem->as_string()).has_value()) {
        return 1;
    }

    if (!xer::puts(u8"status = inf").has_value()) {
        return 1;
    }

    // XER_EXAMPLE_END: toml_strings

    return 0;
}
