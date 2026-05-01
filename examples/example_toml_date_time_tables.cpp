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
    // XER_EXAMPLE_BEGIN: toml_date_time_tables
    //
    // This example decodes TOML date/time values and array-of-tables.
    //
    // Expected output:
    // release year = 2026
    // first product = Hammer

    const auto decoded = xer::toml_decode(
        u8"released = 2026-04-30\n"
        u8"\n"
        u8"[[products]]\n"
        u8"name = \"Hammer\"\n"
        u8"\n"
        u8"[[products]]\n"
        u8"name = \"Nail\"\n");

    if (!decoded.has_value()) {
        const auto& error = decoded.error();
        return error.line == 0 ? 1 : 1;
    }

    const auto* root = decoded->as_table();
    if (root == nullptr) {
        return 1;
    }

    const auto* released = find_key(*root, u8"released");
    const auto* products = find_key(*root, u8"products");
    if (released == nullptr || products == nullptr) {
        return 1;
    }

    const auto* date = released->as_local_date();
    const auto* product_array = products->as_array();
    if (date == nullptr || product_array == nullptr || product_array->empty()) {
        return 1;
    }

    const auto* first_product = (*product_array)[0].as_table();
    if (first_product == nullptr) {
        return 1;
    }

    const auto* first_name = find_key(*first_product, u8"name");
    if (first_name == nullptr || !first_name->is_string()) {
        return 1;
    }

    if (!xer::printf(u8"release year = %@\n", date->year).has_value()) {
        return 1;
    }

    if (!xer::printf(u8"first product = %@\n", *first_name->as_string()).has_value()) {
        return 1;
    }

    // XER_EXAMPLE_END: toml_date_time_tables

    return 0;
}
