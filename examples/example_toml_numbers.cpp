#include <xer/stdio.h>
#include <xer/toml.h>

#include <cstdint>
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
    // XER_EXAMPLE_BEGIN: toml_numbers
    //
    // This example decodes TOML numeric literals with signs, non-decimal
    // integer prefixes, and numeric separators.
    //
    // Expected output:
    // decimal = 1000000
    // hex = 255
    // binary = 165

    const auto decoded = xer::toml_decode(
        u8"decimal = 1_000_000\n"
        u8"hex = 0xFF\n"
        u8"binary = 0b1010_0101\n"
        u8"ratio = 1_000.25_5\n");

    if (!decoded.has_value()) {
        return 1;
    }

    const auto* root = decoded->as_table();
    if (root == nullptr) {
        return 1;
    }

    const auto* decimal = find_key(*root, u8"decimal");
    const auto* hex = find_key(*root, u8"hex");
    const auto* binary = find_key(*root, u8"binary");
    const auto* ratio = find_key(*root, u8"ratio");

    if (decimal == nullptr || hex == nullptr || binary == nullptr ||
        ratio == nullptr || !decimal->is_integer() || !hex->is_integer() ||
        !binary->is_integer() || !ratio->is_float()) {
        return 1;
    }

    if (!xer::printf(u8"decimal = %@\n", *decimal->as_integer()).has_value()) {
        return 1;
    }

    if (!xer::printf(u8"hex = %@\n", *hex->as_integer()).has_value()) {
        return 1;
    }

    if (!xer::printf(u8"binary = %@\n", *binary->as_integer()).has_value()) {
        return 1;
    }



    // XER_EXAMPLE_END: toml_numbers

    return 0;
}
