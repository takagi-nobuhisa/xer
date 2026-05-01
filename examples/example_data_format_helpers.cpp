// XER_EXAMPLE_BEGIN: data_format_helpers
//
// This example shows JSON/TOML/INI find helpers and load/save helpers.
//
// Expected output:
// xer
// xer
// xer
// XER_EXAMPLE_END: data_format_helpers

#include <xer/ini.h>
#include <xer/json.h>
#include <xer/path.h>
#include <xer/stdio.h>
#include <xer/toml.h>

namespace {

auto print_line(std::u8string_view text) -> bool
{
    return xer::puts(text).has_value();
}

} // namespace

auto main() -> int
{
    const xer::path json_file(u8"example_data_format.json");
    const xer::path toml_file(u8"example_data_format.toml");
    const xer::path ini_file_path(u8"example_data_format.ini");

    const xer::json_value::object_type json_object = {
        {u8"name", xer::json_value(u8"xer")},
    };
    if (!xer::json_save(json_file, xer::json_value(json_object)).has_value()) {
        return 1;
    }

    xer::toml_table toml_table_value;
    toml_table_value.push_back({u8"name", xer::toml_value(u8"xer")});
    if (!xer::toml_save(toml_file, xer::toml_value(toml_table_value)).has_value()) {
        return 1;
    }

    xer::ini_file ini_value;
    ini_value.entries.push_back({u8"name", u8"xer"});
    if (!xer::ini_save(ini_file_path, ini_value).has_value()) {
        return 1;
    }

    const auto json_loaded = xer::json_load(json_file);
    const auto toml_loaded = xer::toml_load(toml_file);
    const auto ini_loaded = xer::ini_load(ini_file_path);
    if (!json_loaded.has_value() || !toml_loaded.has_value() || !ini_loaded.has_value()) {
        return 1;
    }

    const auto* json_name = xer::json_find(*json_loaded, u8"name");
    const auto* toml_name = xer::toml_find(*toml_loaded, u8"name");
    const auto* ini_name = xer::ini_find(*ini_loaded, u8"name");
    if (json_name == nullptr || toml_name == nullptr || ini_name == nullptr) {
        return 1;
    }

    if (!print_line(json_name->as_string())) {
        return 1;
    }

    if (const auto* value = toml_name->as_string(); value == nullptr || !print_line(*value)) {
        return 1;
    }

    if (!print_line(ini_name->value)) {
        return 1;
    }

    static_cast<void>(xer::remove(json_file));
    static_cast<void>(xer::remove(toml_file));
    static_cast<void>(xer::remove(ini_file_path));

    return 0;
}
