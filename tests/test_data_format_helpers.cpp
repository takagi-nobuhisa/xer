#include <string>

#include <xer/assert.h>
#include <xer/error.h>
#include <xer/ini.h>
#include <xer/json.h>
#include <xer/path.h>
#include <xer/stdio.h>
#include <xer/toml.h>

namespace {

void test_json_find()
{
    const auto decoded = xer::json_decode(u8"{\"name\":\"xer\",\"version\":5}");
    xer_assert(decoded.has_value());

    const auto* name = xer::json_find(*decoded, u8"name");
    const auto* missing = xer::json_find(*decoded, u8"missing");

    xer_assert(name != nullptr);
    xer_assert(name->is_string());
    xer_assert_eq(name->as_string(), u8"xer");
    xer_assert(missing == nullptr);
}

void test_toml_find()
{
    const auto decoded = xer::toml_decode(
        u8"[project]\n"
        u8"name = \"xer\"\n"
        u8"[project.build]\n"
        u8"number = 5\n");
    xer_assert(decoded.has_value());

    const auto* name = xer::toml_find(*decoded, u8"project.name");
    const auto* number = xer::toml_find(*decoded, u8"project.build.number");
    const auto* missing = xer::toml_find(*decoded, u8"project.missing");

    xer_assert(name != nullptr);
    xer_assert(name->is_string());
    xer_assert_eq(*name->as_string(), u8"xer");

    xer_assert(number != nullptr);
    xer_assert(number->is_integer());
    xer_assert_eq(*number->as_integer(), static_cast<std::int64_t>(5));

    xer_assert(missing == nullptr);
}

void test_ini_find()
{
    const auto decoded = xer::ini_decode(
        u8"title = sample\n"
        u8"[project]\n"
        u8"name = xer\n");
    xer_assert(decoded.has_value());

    const auto* title = xer::ini_find(*decoded, u8"title");
    const auto* name = xer::ini_find(*decoded, u8"project", u8"name");
    const auto* missing = xer::ini_find(*decoded, u8"project", u8"missing");

    xer_assert(title != nullptr);
    xer_assert_eq(title->value, u8"sample");

    xer_assert(name != nullptr);
    xer_assert_eq(name->value, u8"xer");

    xer_assert(missing == nullptr);
}

void test_json_load_save()
{
    const xer::path filename(u8"test_data_format_json.tmp");

    const xer::json_value::object_type object = {
        {u8"name", xer::json_value(u8"xer")},
    };

    const auto saved = xer::json_save(filename, xer::json_value(object));
    xer_assert(saved.has_value());

    const auto loaded = xer::json_load(filename);
    xer_assert(loaded.has_value());

    const auto* name = xer::json_find(*loaded, u8"name");
    xer_assert(name != nullptr);
    xer_assert_eq(name->as_string(), u8"xer");

    static_cast<void>(xer::remove(filename));
}

void test_toml_load_save()
{
    const xer::path filename(u8"test_data_format_toml.tmp");

    xer::toml_table table;
    table.push_back({u8"name", xer::toml_value(u8"xer")});

    const auto saved = xer::toml_save(filename, xer::toml_value(std::move(table)));
    xer_assert(saved.has_value());

    const auto loaded = xer::toml_load(filename);
    xer_assert(loaded.has_value());

    const auto* name = xer::toml_find(*loaded, u8"name");
    xer_assert(name != nullptr);
    xer_assert_eq(*name->as_string(), u8"xer");

    static_cast<void>(xer::remove(filename));
}

void test_ini_load_save()
{
    const xer::path filename(u8"test_data_format_ini.tmp");

    xer::ini_file file;
    file.entries.push_back({u8"name", u8"xer"});

    const auto saved = xer::ini_save(filename, file);
    xer_assert(saved.has_value());

    const auto loaded = xer::ini_load(filename);
    xer_assert(loaded.has_value());

    const auto* name = xer::ini_find(*loaded, u8"name");
    xer_assert(name != nullptr);
    xer_assert_eq(name->value, u8"xer");

    static_cast<void>(xer::remove(filename));
}

void test_load_io_error_has_no_parse_detail()
{
    const auto result = xer::json_load(xer::path(u8"missing_data_format_json.tmp"));

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().reason, xer::parse_error_reason::none);
    xer_assert_eq(result.error().line, static_cast<std::size_t>(0));
    xer_assert_eq(result.error().column, static_cast<std::size_t>(0));
}

} // namespace

int main()
{
    test_json_find();
    test_toml_find();
    test_ini_find();
    test_json_load_save();
    test_toml_load_save();
    test_ini_load_save();
    test_load_io_error_has_no_parse_detail();

    return 0;
}
