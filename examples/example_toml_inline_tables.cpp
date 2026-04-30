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
    // XER_EXAMPLE_BEGIN: toml_inline_tables
    //
    // This example decodes inline tables and arrays containing inline tables.
    //
    // Expected output:
    // point = 1,2
    // first item = one

    const auto decoded = xer::toml_decode(
        u8"point = { x = 1, y = 2 }\n"
        u8"items = [{ name = \"one\" }, { name = \"two\" }]\n");

    if (!decoded.has_value()) {
        return 1;
    }

    const auto* root = decoded->as_table();
    if (root == nullptr) {
        return 1;
    }

    const auto* point = find_key(*root, u8"point");
    const auto* items = find_key(*root, u8"items");
    if (point == nullptr || items == nullptr) {
        return 1;
    }

    const auto* point_table = point->as_table();
    const auto* item_array = items->as_array();
    if (point_table == nullptr || item_array == nullptr || item_array->empty()) {
        return 1;
    }

    const auto* x = find_key(*point_table, u8"x");
    const auto* y = find_key(*point_table, u8"y");
    const auto* first_item = (*item_array)[0].as_table();
    if (x == nullptr || y == nullptr || first_item == nullptr) {
        return 1;
    }

    const auto* first_name = find_key(*first_item, u8"name");
    if (first_name == nullptr || !x->is_integer() || !y->is_integer() ||
        !first_name->is_string()) {
        return 1;
    }

    if (!xer::printf(
            u8"point = %@,%@\n",
            static_cast<long long>(*x->as_integer()),
            static_cast<long long>(*y->as_integer()))
             .has_value()) {
        return 1;
    }

    if (!xer::printf(u8"first item = %@\n", *first_name->as_string())
             .has_value()) {
        return 1;
    }

    // XER_EXAMPLE_END: toml_inline_tables

    return 0;
}
