#include <cmath>
#include <cstdint>
#include <limits>
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


void test_toml_decode_quoted_and_dotted_keys()
{
    const auto result = xer::toml_decode(
        u8"\"site.name\" = \"xer\"\n"
        u8"'literal.key' = 10\n"
        u8"server.port = 8080\n"
        u8"database.\"connection.pool\".size = 4\n");

    xer_assert(result.has_value());

    const auto* root = result->as_table();
    xer_assert(root != nullptr);

    const auto* site_name = find_key(*root, u8"site.name");
    const auto* literal_key = find_key(*root, u8"literal.key");
    const auto* server = find_key(*root, u8"server");
    const auto* database = find_key(*root, u8"database");

    xer_assert(site_name != nullptr);
    xer_assert(literal_key != nullptr);
    xer_assert(server != nullptr);
    xer_assert(database != nullptr);

    xer_assert_eq(*site_name->as_string(), u8"xer");
    xer_assert_eq(*literal_key->as_integer(), static_cast<std::int64_t>(10));

    const auto* server_table = server->as_table();
    xer_assert(server_table != nullptr);

    const auto* port = find_key(*server_table, u8"port");
    xer_assert(port != nullptr);
    xer_assert_eq(*port->as_integer(), static_cast<std::int64_t>(8080));

    const auto* database_table = database->as_table();
    xer_assert(database_table != nullptr);

    const auto* pool = find_key(*database_table, u8"connection.pool");
    xer_assert(pool != nullptr);

    const auto* pool_table = pool->as_table();
    xer_assert(pool_table != nullptr);

    const auto* size = find_key(*pool_table, u8"size");
    xer_assert(size != nullptr);
    xer_assert_eq(*size->as_integer(), static_cast<std::int64_t>(4));
}

void test_toml_decode_nested_tables()
{
    const auto result = xer::toml_decode(
        u8"[project.package]\n"
        u8"name = \"xer\"\n"
        u8"\n"
        u8"[project]\n"
        u8"enabled = true\n"
        u8"\n"
        u8"[project.\"build.target\"]\n"
        u8"name = \"ucrt64\"\n");

    xer_assert(result.has_value());

    const auto* root = result->as_table();
    xer_assert(root != nullptr);

    const auto* project = find_key(*root, u8"project");
    xer_assert(project != nullptr);

    const auto* project_table = project->as_table();
    xer_assert(project_table != nullptr);

    const auto* enabled = find_key(*project_table, u8"enabled");
    const auto* package = find_key(*project_table, u8"package");
    const auto* build_target = find_key(*project_table, u8"build.target");

    xer_assert(enabled != nullptr);
    xer_assert(package != nullptr);
    xer_assert(build_target != nullptr);
    xer_assert_eq(*enabled->as_bool(), true);

    const auto* package_table = package->as_table();
    const auto* build_target_table = build_target->as_table();

    xer_assert(package_table != nullptr);
    xer_assert(build_target_table != nullptr);

    const auto* package_name = find_key(*package_table, u8"name");
    const auto* target_name = find_key(*build_target_table, u8"name");

    xer_assert(package_name != nullptr);
    xer_assert(target_name != nullptr);
    xer_assert_eq(*package_name->as_string(), u8"xer");
    xer_assert_eq(*target_name->as_string(), u8"ucrt64");
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

void test_toml_decode_extended_strings()
{
    const auto result = xer::toml_decode(
        u8"literal = 'C:\\Users\\xer'\n"
        u8"unicode = \"A\\u03C0\\U0001F600\"\n"
        u8"multiline_basic = \"\"\"\nline1\nline2\"\"\"\n"
        u8"multiline_literal = '''\nC:\\Users\\xer\n'''\n");

    xer_assert(result.has_value());

    const auto* table = result->as_table();
    xer_assert(table != nullptr);

    const auto* literal = find_key(*table, u8"literal");
    const auto* unicode = find_key(*table, u8"unicode");
    const auto* multiline_basic = find_key(*table, u8"multiline_basic");
    const auto* multiline_literal = find_key(*table, u8"multiline_literal");

    xer_assert(literal != nullptr);
    xer_assert(unicode != nullptr);
    xer_assert(multiline_basic != nullptr);
    xer_assert(multiline_literal != nullptr);

    xer_assert_eq(*literal->as_string(), u8"C:\\Users\\xer");
    xer_assert_eq(*unicode->as_string(), u8"Aπ😀");
    xer_assert_eq(*multiline_basic->as_string(), u8"line1\nline2");
    xer_assert_eq(*multiline_literal->as_string(), u8"C:\\Users\\xer\n");
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

void test_toml_decode_special_floats()
{
    const auto result = xer::toml_decode(
        u8"positive_inf = inf\n"
        u8"negative_inf = -inf\n"
        u8"not_a_number = nan\n");

    xer_assert(result.has_value());

    const auto* table = result->as_table();
    xer_assert(table != nullptr);

    const auto* positive_inf = find_key(*table, u8"positive_inf");
    const auto* negative_inf = find_key(*table, u8"negative_inf");
    const auto* not_a_number = find_key(*table, u8"not_a_number");

    xer_assert(positive_inf != nullptr);
    xer_assert(negative_inf != nullptr);
    xer_assert(not_a_number != nullptr);

    xer_assert(std::isinf(*positive_inf->as_float()));
    xer_assert(*positive_inf->as_float() > 0.0);
    xer_assert(std::isinf(*negative_inf->as_float()));
    xer_assert(*negative_inf->as_float() < 0.0);
    xer_assert(std::isnan(*not_a_number->as_float()));
}


void test_toml_decode_inline_tables()
{
    const auto result = xer::toml_decode(
        u8"point = { x = 1, y = 2 }\n"
        u8"package = { name = \"xer\", metadata.version = \"0.2.0a5\" }\n"
        u8"items = [{ name = \"one\" }, { name = \"two\", enabled = true }]\n");

    xer_assert(result.has_value());

    const auto* root = result->as_table();
    xer_assert(root != nullptr);

    const auto* point = find_key(*root, u8"point");
    const auto* package = find_key(*root, u8"package");
    const auto* items = find_key(*root, u8"items");

    xer_assert(point != nullptr);
    xer_assert(package != nullptr);
    xer_assert(items != nullptr);

    const auto* point_table = point->as_table();
    xer_assert(point_table != nullptr);

    const auto* x = find_key(*point_table, u8"x");
    const auto* y = find_key(*point_table, u8"y");

    xer_assert(x != nullptr);
    xer_assert(y != nullptr);
    xer_assert_eq(*x->as_integer(), static_cast<std::int64_t>(1));
    xer_assert_eq(*y->as_integer(), static_cast<std::int64_t>(2));

    const auto* package_table = package->as_table();
    xer_assert(package_table != nullptr);

    const auto* name = find_key(*package_table, u8"name");
    const auto* metadata = find_key(*package_table, u8"metadata");

    xer_assert(name != nullptr);
    xer_assert(metadata != nullptr);
    xer_assert_eq(*name->as_string(), u8"xer");

    const auto* metadata_table = metadata->as_table();
    xer_assert(metadata_table != nullptr);

    const auto* version = find_key(*metadata_table, u8"version");
    xer_assert(version != nullptr);
    xer_assert_eq(*version->as_string(), u8"0.2.0a5");

    const auto* array = items->as_array();
    xer_assert(array != nullptr);
    xer_assert_eq(array->size(), static_cast<std::size_t>(2));

    const auto* first = (*array)[0].as_table();
    const auto* second = (*array)[1].as_table();

    xer_assert(first != nullptr);
    xer_assert(second != nullptr);

    const auto* first_name = find_key(*first, u8"name");
    const auto* second_name = find_key(*second, u8"name");
    const auto* second_enabled = find_key(*second, u8"enabled");

    xer_assert(first_name != nullptr);
    xer_assert(second_name != nullptr);
    xer_assert(second_enabled != nullptr);

    xer_assert_eq(*first_name->as_string(), u8"one");
    xer_assert_eq(*second_name->as_string(), u8"two");
    xer_assert_eq(*second_enabled->as_bool(), true);
}

void test_toml_decode_rejects_invalid_inline_tables()
{
    const auto no_equal = xer::toml_decode(u8"value = { a }\n");
    const auto duplicate_key = xer::toml_decode(u8"value = { a = 1, a = 2 }\n");
    const auto trailing_comma = xer::toml_decode(u8"value = { a = 1, }\n");
    const auto scalar_conflict = xer::toml_decode(u8"value = { a = 1, a.b = 2 }\n");

    xer_assert_not(no_equal.has_value());
    xer_assert_not(duplicate_key.has_value());
    xer_assert_not(trailing_comma.has_value());
    xer_assert_not(scalar_conflict.has_value());
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
}
void test_toml_decode_rejects_invalid_syntax()
{
    const auto no_equal = xer::toml_decode(u8"name\n");
    const auto empty_key = xer::toml_decode(u8" = 1\n");
    const auto bad_table = xer::toml_decode(u8"[project\n");
    const auto duplicate_key = xer::toml_decode(u8"name = \"a\"\nname = \"b\"\n");
    const auto duplicate_table = xer::toml_decode(u8"[a]\nkey = 1\n[a]\nkey = 2\n");
    const auto duplicate_dotted_key = xer::toml_decode(u8"a.b = 1\na.b = 2\n");
    const auto scalar_table_conflict = xer::toml_decode(u8"a = 1\na.b = 2\n");
    const auto duplicate_nested_table =
        xer::toml_decode(u8"[a.b]\nkey = 1\n[a.b]\nkey = 2\n");

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

    xer_assert_not(duplicate_dotted_key.has_value());
    xer_assert_eq(duplicate_dotted_key.error().code, xer::error_t::invalid_argument);

    xer_assert_not(scalar_table_conflict.has_value());
    xer_assert_eq(scalar_table_conflict.error().code, xer::error_t::invalid_argument);

    xer_assert_not(duplicate_nested_table.has_value());
    xer_assert_eq(duplicate_nested_table.error().code, xer::error_t::invalid_argument);
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

void test_toml_encode_special_floats()
{
    xer::toml_table root;
    root.push_back({
        u8"positive_inf",
        xer::toml_value(std::numeric_limits<double>::infinity())});
    root.push_back({
        u8"negative_inf",
        xer::toml_value(-std::numeric_limits<double>::infinity())});
    root.push_back({
        u8"not_a_number",
        xer::toml_value(std::numeric_limits<double>::quiet_NaN())});

    const auto result = xer::toml_encode(xer::toml_value(std::move(root)));

    xer_assert(result.has_value());
    xer_assert_eq(
        *result,
        std::u8string(
            u8"positive_inf = inf\n"
            u8"negative_inf = -inf\n"
            u8"not_a_number = nan\n"));
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

void test_toml_encode_quoted_keys_and_nested_tables()
{
    xer::toml_table metadata;
    metadata.push_back({u8"build.target", xer::toml_value(u8"ucrt64")});

    xer::toml_table project;
    project.push_back({u8"name", xer::toml_value(u8"xer")});
    project.push_back({u8"metadata", xer::toml_value(std::move(metadata))});

    xer::toml_table root;
    root.push_back({u8"site.name", xer::toml_value(u8"xer")});
    root.push_back({u8"project", xer::toml_value(std::move(project))});

    const auto result = xer::toml_encode(xer::toml_value(std::move(root)));

    xer_assert(result.has_value());
    xer_assert_eq(
        *result,
        std::u8string(
            u8"\"site.name\" = \"xer\"\n"
            u8"\n"
            u8"[project]\n"
            u8"name = \"xer\"\n"
            u8"\n"
            u8"[project.metadata]\n"
            u8"\"build.target\" = \"ucrt64\"\n"));
}


void test_toml_encode_array_of_tables()
{
    xer::toml_table first;
    first.push_back({u8"name", xer::toml_value(u8"one")});

    xer::toml_table second;
    second.push_back({u8"name", xer::toml_value(u8"two")});
    second.push_back({u8"enabled", xer::toml_value(true)});

    xer::toml_array items;
    items.push_back(xer::toml_value(std::move(first)));
    items.push_back(xer::toml_value(std::move(second)));

    xer::toml_table root;
    root.push_back({u8"items", xer::toml_value(std::move(items))});

    const auto result = xer::toml_encode(xer::toml_value(std::move(root)));

    xer_assert(result.has_value());
    xer_assert_eq(
        *result,
        std::u8string(
            u8"[[items]]\n"
            u8"name = \"one\"\n"
            u8"\n"
            u8"[[items]]\n"
            u8"name = \"two\"\n"
            u8"enabled = true\n"));
}


void test_toml_decode_date_time_values()
{
    const auto result = xer::toml_decode(
        u8"date = 2026-04-30\n"
        u8"time = 23:59:58.123456\n"
        u8"local = 2026-04-30T23:59:58\n"
        u8"offset = 2026-04-30T23:59:58+09:00\n"
        u8"utc = 2026-04-30T14:59:58Z\n");

    xer_assert(result.has_value());

    const auto* root = result->as_table();
    xer_assert(root != nullptr);

    const auto* date = find_key(*root, u8"date");
    const auto* time = find_key(*root, u8"time");
    const auto* local = find_key(*root, u8"local");
    const auto* offset = find_key(*root, u8"offset");
    const auto* utc = find_key(*root, u8"utc");

    xer_assert(date != nullptr);
    xer_assert(time != nullptr);
    xer_assert(local != nullptr);
    xer_assert(offset != nullptr);
    xer_assert(utc != nullptr);

    const auto* date_value = date->as_local_date();
    const auto* time_value = time->as_local_time();
    const auto* local_value = local->as_local_datetime();
    const auto* offset_value = offset->as_offset_datetime();
    const auto* utc_value = utc->as_offset_datetime();

    xer_assert(date_value != nullptr);
    xer_assert(time_value != nullptr);
    xer_assert(local_value != nullptr);
    xer_assert(offset_value != nullptr);
    xer_assert(utc_value != nullptr);

    xer_assert_eq(date_value->year, 2026);
    xer_assert_eq(date_value->month, 4);
    xer_assert_eq(date_value->day, 30);
    xer_assert_eq(time_value->microsec, 123456);
    xer_assert_eq(local_value->date.year, 2026);
    xer_assert_eq(offset_value->offset_minutes, 9 * 60);
    xer_assert_eq(utc_value->offset_minutes, 0);
}

void test_toml_decode_array_of_tables()
{
    const auto result = xer::toml_decode(
        u8"[[products]]\n"
        u8"name = \"Hammer\"\n"
        u8"\n"
        u8"[[products]]\n"
        u8"name = \"Nail\"\n"
        u8"\n"
        u8"[[products.colors]]\n"
        u8"name = \"silver\"\n");

    xer_assert(result.has_value());

    const auto* root = result->as_table();
    xer_assert(root != nullptr);

    const auto* products = find_key(*root, u8"products");
    xer_assert(products != nullptr);

    const auto* array = products->as_array();
    xer_assert(array != nullptr);
    xer_assert_eq(array->size(), static_cast<std::size_t>(2));

    const auto* first = (*array)[0].as_table();
    const auto* second = (*array)[1].as_table();
    xer_assert(first != nullptr);
    xer_assert(second != nullptr);

    const auto* first_name = find_key(*first, u8"name");
    const auto* second_name = find_key(*second, u8"name");
    const auto* colors = find_key(*second, u8"colors");

    xer_assert(first_name != nullptr);
    xer_assert(second_name != nullptr);
    xer_assert(colors != nullptr);
    xer_assert_eq(*first_name->as_string(), u8"Hammer");
    xer_assert_eq(*second_name->as_string(), u8"Nail");

    const auto* color_array = colors->as_array();
    xer_assert(color_array != nullptr);
    xer_assert_eq(color_array->size(), static_cast<std::size_t>(1));

    const auto* color_table = (*color_array)[0].as_table();
    xer_assert(color_table != nullptr);

    const auto* color_name = find_key(*color_table, u8"name");
    xer_assert(color_name != nullptr);
    xer_assert_eq(*color_name->as_string(), u8"silver");
}

void test_toml_decode_reports_parse_error_detail()
{
    const auto result = xer::toml_decode(
        u8"name = \"ok\"\n"
        u8"bad\n");

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
    xer_assert_eq(result.error().line, static_cast<std::size_t>(2));
    xer_assert_eq(result.error().column, static_cast<std::size_t>(1));
}

void test_toml_encode_date_time_and_array_of_tables()
{
    xer::toml_table first;
    first.push_back({u8"name", xer::toml_value(u8"Hammer")});
    first.push_back({u8"available", xer::toml_value(xer::toml_local_date{2026, 4, 30})});

    xer::toml_table second;
    second.push_back({u8"name", xer::toml_value(u8"Nail")});
    second.push_back({u8"created", xer::toml_value(xer::toml_offset_datetime{
        xer::toml_local_date{2026, 4, 30},
        xer::toml_local_time{23, 59, 58, 123000},
        9 * 60})});

    xer::toml_array products;
    products.push_back(xer::toml_value(std::move(first)));
    products.push_back(xer::toml_value(std::move(second)));

    xer::toml_table root;
    root.push_back({u8"products", xer::toml_value(std::move(products))});

    const auto result = xer::toml_encode(xer::toml_value(std::move(root)));

    xer_assert(result.has_value());
    xer_assert(result->find(u8"[[products]]") != std::u8string::npos);
    xer_assert(result->find(u8"available = 2026-04-30") != std::u8string::npos);
    xer_assert(result->find(u8"created = 2026-04-30T23:59:58.123+09:00") != std::u8string::npos);
}


void test_toml_encode_rejects_invalid_values()
{
    xer::toml_table bad_key_root;
    bad_key_root.push_back(
        {std::u8string(u8"bad\nkey"), xer::toml_value(static_cast<std::int64_t>(1))});

    const auto bad_key =
        xer::toml_encode(xer::toml_value(std::move(bad_key_root)));
    const auto not_table = xer::toml_encode(xer::toml_value(u8"text"));

    xer_assert_not(bad_key.has_value());
    xer_assert_eq(bad_key.error().code, xer::error_t::invalid_argument);

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
    test_toml_decode_quoted_and_dotted_keys();
    test_toml_decode_nested_tables();
    test_toml_decode_extended_strings();
    test_toml_decode_extended_numbers();
    test_toml_decode_special_floats();
    test_toml_decode_date_time_values();
    test_toml_decode_inline_tables();
    test_toml_decode_array_of_tables();
    test_toml_decode_reports_parse_error_detail();
    test_toml_decode_rejects_invalid_inline_tables();
    test_toml_decode_rejects_invalid_numbers();
    test_toml_decode_rejects_invalid_syntax();
    test_toml_decode_rejects_invalid_utf8();
    test_toml_encode_basic_document();
    test_toml_encode_special_floats();
    test_toml_encode_quoted_keys_and_nested_tables();
    test_toml_encode_array_of_tables();
    test_toml_encode_date_time_and_array_of_tables();
    test_toml_round_trip();
    test_toml_encode_rejects_invalid_values();

    return 0;
}
