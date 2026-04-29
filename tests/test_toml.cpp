#include <cstdint>
#include <string>
#include <string_view>

#include <xer/assert.h>
#include <xer/error.h>
#include <xer/toml.h>

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

void test_toml_decode_basic_values()
{
    const auto result = xer::toml_decode(
        u8"title = \"xer\"\n"
        u8"enabled = true\n"
        u8"count = 123\n"
        u8"ratio = 1.5\n");

    xer_assert(result.has_value());
    xer_assert(result->is_table());

    const auto* table = result->as_table();
    xer_assert(table != nullptr);
    xer_assert_eq(table->size(), static_cast<std::size_t>(4));

    const auto* title = find_key(*table, u8"title");
    const auto* enabled = find_key(*table, u8"enabled");
    const auto* count = find_key(*table, u8"count");
    const auto* ratio = find_key(*table, u8"ratio");

    xer_assert(title != nullptr);
    xer_assert(enabled != nullptr);
    xer_assert(count != nullptr);
    xer_assert(ratio != nullptr);

    xer_assert(title->is_string());
    xer_assert(enabled->is_bool());
    xer_assert(count->is_integer());
    xer_assert(ratio->is_float());

    xer_assert_eq(*title->as_string(), u8"xer");
    xer_assert_eq(*enabled->as_bool(), true);
    xer_assert_eq(*count->as_integer(), static_cast<std::int64_t>(123));
    xer_assert_eq(*ratio->as_float(), 1.5);
}

void test_toml_decode_array()
{
    const auto result = xer::toml_decode(
        u8"ports = [8000, 8001, 8002]\n"
        u8"mixed = [\"xer\", true, 3]\n");

    xer_assert(result.has_value());

    const auto* table = result->as_table();
    xer_assert(table != nullptr);

    const auto* ports = find_key(*table, u8"ports");
    const auto* mixed = find_key(*table, u8"mixed");

    xer_assert(ports != nullptr);
    xer_assert(mixed != nullptr);
    xer_assert(ports->is_array());
    xer_assert(mixed->is_array());

    const auto* port_array = ports->as_array();
    const auto* mixed_array = mixed->as_array();

    xer_assert(port_array != nullptr);
    xer_assert(mixed_array != nullptr);

    xer_assert_eq(port_array->size(), static_cast<std::size_t>(3));
    xer_assert_eq(*(*port_array)[0].as_integer(), static_cast<std::int64_t>(8000));
    xer_assert_eq(*(*port_array)[1].as_integer(), static_cast<std::int64_t>(8001));
    xer_assert_eq(*(*port_array)[2].as_integer(), static_cast<std::int64_t>(8002));

    xer_assert_eq(mixed_array->size(), static_cast<std::size_t>(3));
    xer_assert_eq(*(*mixed_array)[0].as_string(), u8"xer");
    xer_assert_eq(*(*mixed_array)[1].as_bool(), true);
    xer_assert_eq(*(*mixed_array)[2].as_integer(), static_cast<std::int64_t>(3));
}

void test_toml_decode_table()
{
    const auto result = xer::toml_decode(
        u8"title = \"sample\"\n"
        u8"\n"
        u8"[project]\n"
        u8"name = \"xer\"\n"
        u8"version = \"0.2.0a3\"\n");

    xer_assert(result.has_value());

    const auto* root = result->as_table();
    xer_assert(root != nullptr);

    const auto* title = find_key(*root, u8"title");
    const auto* project = find_key(*root, u8"project");

    xer_assert(title != nullptr);
    xer_assert(project != nullptr);
    xer_assert_eq(*title->as_string(), u8"sample");

    xer_assert(project->is_table());

    const auto* project_table = project->as_table();
    xer_assert(project_table != nullptr);

    const auto* name = find_key(*project_table, u8"name");
    const auto* version = find_key(*project_table, u8"version");

    xer_assert(name != nullptr);
    xer_assert(version != nullptr);
    xer_assert_eq(*name->as_string(), u8"xer");
    xer_assert_eq(*version->as_string(), u8"0.2.0a3");
}

void test_toml_decode_comments_and_escapes()
{
    const auto result = xer::toml_decode(
        u8"# comment\n"
        u8"name = \"x#r\" # comment\n"
        u8"text = \"a\\nb\"\n");

    xer_assert(result.has_value());

    const auto* table = result->as_table();
    xer_assert(table != nullptr);

    const auto* name = find_key(*table, u8"name");
    const auto* text = find_key(*table, u8"text");

    xer_assert(name != nullptr);
    xer_assert(text != nullptr);

    xer_assert_eq(*name->as_string(), u8"x#r");
    xer_assert_eq(*text->as_string(), u8"a\nb");
}

void test_toml_decode_extended_numbers()
{
    const auto result = xer::toml_decode(
        u8"decimal = 1_000_000\n"
        u8"positive = +123\n"
        u8"negative = -123\n"
        u8"hex = 0xDEAD_BEEF\n"
        u8"octal = 0o755\n"
        u8"binary = 0b1010_0101\n"
        u8"float_value = 1_000.25_5\n"
        u8"exponent = +1.0e-3\n");

    xer_assert(result.has_value());

    const auto* table = result->as_table();
    xer_assert(table != nullptr);

    const auto* decimal = find_key(*table, u8"decimal");
    const auto* positive = find_key(*table, u8"positive");
    const auto* negative = find_key(*table, u8"negative");
    const auto* hex = find_key(*table, u8"hex");
    const auto* octal = find_key(*table, u8"octal");
    const auto* binary = find_key(*table, u8"binary");
    const auto* float_value = find_key(*table, u8"float_value");
    const auto* exponent = find_key(*table, u8"exponent");

    xer_assert(decimal != nullptr);
    xer_assert(positive != nullptr);
    xer_assert(negative != nullptr);
    xer_assert(hex != nullptr);
    xer_assert(octal != nullptr);
    xer_assert(binary != nullptr);
    xer_assert(float_value != nullptr);
    xer_assert(exponent != nullptr);

    xer_assert_eq(*decimal->as_integer(), static_cast<std::int64_t>(1000000));
    xer_assert_eq(*positive->as_integer(), static_cast<std::int64_t>(123));
    xer_assert_eq(*negative->as_integer(), static_cast<std::int64_t>(-123));
    xer_assert_eq(*hex->as_integer(), static_cast<std::int64_t>(0xDEADBEEF));
    xer_assert_eq(*octal->as_integer(), static_cast<std::int64_t>(493));
    xer_assert_eq(*binary->as_integer(), static_cast<std::int64_t>(165));
    xer_assert_eq(*float_value->as_float(), 1000.255);
    xer_assert_eq(*exponent->as_float(), 0.001);
}

void test_toml_decode_rejects_invalid_numbers()
{
    const auto double_separator = xer::toml_decode(u8"a = 1__000\n");
    const auto trailing_separator = xer::toml_decode(u8"a = 1000_\n");
    const auto prefixed_separator = xer::toml_decode(u8"a = 0x_DEAD\n");
    const auto bad_binary_digit = xer::toml_decode(u8"a = 0b102\n");
    const auto bad_octal_digit = xer::toml_decode(u8"a = 0o778\n");
    const auto bad_float_fraction = xer::toml_decode(u8"a = 1._0\n");
    const auto bad_float_exponent = xer::toml_decode(u8"a = 1.0e_3\n");
    const auto infinity = xer::toml_decode(u8"a = inf\n");
    const auto not_a_number = xer::toml_decode(u8"a = nan\n");

    xer_assert_not(double_separator.has_value());
    xer_assert_eq(double_separator.error().code, xer::error_t::invalid_argument);

    xer_assert_not(trailing_separator.has_value());
    xer_assert_eq(trailing_separator.error().code, xer::error_t::invalid_argument);

    xer_assert_not(prefixed_separator.has_value());
    xer_assert_eq(prefixed_separator.error().code, xer::error_t::invalid_argument);

    xer_assert_not(bad_binary_digit.has_value());
    xer_assert_eq(bad_binary_digit.error().code, xer::error_t::invalid_argument);

    xer_assert_not(bad_octal_digit.has_value());
    xer_assert_eq(bad_octal_digit.error().code, xer::error_t::invalid_argument);

    xer_assert_not(bad_float_fraction.has_value());
    xer_assert_eq(bad_float_fraction.error().code, xer::error_t::invalid_argument);

    xer_assert_not(bad_float_exponent.has_value());
    xer_assert_eq(bad_float_exponent.error().code, xer::error_t::invalid_argument);

    xer_assert_not(infinity.has_value());
    xer_assert_eq(infinity.error().code, xer::error_t::invalid_argument);

    xer_assert_not(not_a_number.has_value());
    xer_assert_eq(not_a_number.error().code, xer::error_t::invalid_argument);
}
void test_toml_decode_rejects_invalid_syntax()
{
    const auto no_equal = xer::toml_decode(u8"name\n");
    const auto empty_key = xer::toml_decode(u8" = 1\n");
    const auto bad_table = xer::toml_decode(u8"[project\n");
    const auto duplicate_key = xer::toml_decode(u8"name = \"a\"\nname = \"b\"\n");
    const auto duplicate_table = xer::toml_decode(u8"[a]\nkey = 1\n[a]\nkey = 2\n");
    const auto unsupported_dotted = xer::toml_decode(u8"a.b = 1\n");

    xer_assert_not(no_equal.has_value());
    xer_assert_eq(no_equal.error().code, xer::error_t::invalid_argument);

    xer_assert_not(empty_key.has_value());
    xer_assert_eq(empty_key.error().code, xer::error_t::invalid_argument);

    xer_assert_not(bad_table.has_value());
    xer_assert_eq(bad_table.error().code, xer::error_t::invalid_argument);

    xer_assert_not(duplicate_key.has_value());
    xer_assert_eq(duplicate_key.error().code, xer::error_t::invalid_argument);

    xer_assert_not(duplicate_table.has_value());
    xer_assert_eq(duplicate_table.error().code, xer::error_t::invalid_argument);

    xer_assert_not(unsupported_dotted.has_value());
    xer_assert_eq(unsupported_dotted.error().code, xer::error_t::invalid_argument);
}

void test_toml_decode_rejects_invalid_utf8()
{
    const char8_t text[] = {
        u8'n',
        u8'a',
        u8'm',
        u8'e',
        u8' ',
        u8'=',
        u8' ',
        u8'"',
        static_cast<char8_t>(0x80),
        u8'"',
        u8'\n',
    };

    const auto result = xer::toml_decode(std::u8string_view(text, sizeof(text)));

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::encoding_error);
}

void test_toml_encode_basic_document()
{
    xer::toml_table project;
    project.push_back({u8"name", xer::toml_value(u8"xer")});
    project.push_back({u8"version", xer::toml_value(u8"0.2.0a3")});

    xer::toml_array ports;
    ports.push_back(xer::toml_value(static_cast<std::int64_t>(8000)));
    ports.push_back(xer::toml_value(static_cast<std::int64_t>(8001)));

    xer::toml_table root;
    root.push_back({u8"title", xer::toml_value(u8"sample")});
    root.push_back({u8"enabled", xer::toml_value(true)});
    root.push_back({u8"count", xer::toml_value(static_cast<std::int64_t>(123))});
    root.push_back({u8"ratio", xer::toml_value(1.5)});
    root.push_back({u8"ports", xer::toml_value(std::move(ports))});
    root.push_back({u8"project", xer::toml_value(std::move(project))});

    const auto result = xer::toml_encode(xer::toml_value(std::move(root)));

    xer_assert(result.has_value());
    xer_assert_eq(
        *result,
        std::u8string(
            u8"title = \"sample\"\n"
            u8"enabled = true\n"
            u8"count = 123\n"
            u8"ratio = 1.5\n"
            u8"ports = [8000, 8001]\n"
            u8"\n"
            u8"[project]\n"
            u8"name = \"xer\"\n"
            u8"version = \"0.2.0a3\"\n"));
}

void test_toml_round_trip()
{
    const std::u8string source =
        u8"title = \"sample\"\n"
        u8"enabled = true\n"
        u8"count = 123\n"
        u8"ratio = 1.5\n"
        u8"ports = [8000, 8001]\n"
        u8"\n"
        u8"[project]\n"
        u8"name = \"xer\"\n"
        u8"version = \"0.2.0a3\"\n";

    const auto decoded = xer::toml_decode(source);
    xer_assert(decoded.has_value());

    const auto encoded = xer::toml_encode(*decoded);
    xer_assert(encoded.has_value());
    xer_assert_eq(*encoded, source);
}

void test_toml_encode_rejects_invalid_values()
{
    xer::toml_table bad_key_root;
    bad_key_root.push_back(
        {u8"bad.key", xer::toml_value(static_cast<std::int64_t>(1))});

    xer::toml_table nested;
    nested.push_back({u8"inner", xer::toml_value(xer::toml_table{})});

    xer::toml_table nested_root;
    nested_root.push_back({u8"outer", xer::toml_value(std::move(nested))});

    const auto bad_key =
        xer::toml_encode(xer::toml_value(std::move(bad_key_root)));
    const auto nested_table =
        xer::toml_encode(xer::toml_value(std::move(nested_root)));
    const auto not_table = xer::toml_encode(xer::toml_value(u8"text"));

    xer_assert_not(bad_key.has_value());
    xer_assert_eq(bad_key.error().code, xer::error_t::invalid_argument);

    xer_assert_not(nested_table.has_value());
    xer_assert_eq(nested_table.error().code, xer::error_t::invalid_argument);

    xer_assert_not(not_table.has_value());
    xer_assert_eq(not_table.error().code, xer::error_t::invalid_argument);
}

} // namespace

auto main() -> int
{
    test_toml_decode_basic_values();
    test_toml_decode_array();
    test_toml_decode_table();
    test_toml_decode_comments_and_escapes();
    test_toml_decode_extended_numbers();
    test_toml_decode_rejects_invalid_numbers();
    test_toml_decode_rejects_invalid_syntax();
    test_toml_decode_rejects_invalid_utf8();
    test_toml_encode_basic_document();
    test_toml_round_trip();
    test_toml_encode_rejects_invalid_values();

    return 0;
}
