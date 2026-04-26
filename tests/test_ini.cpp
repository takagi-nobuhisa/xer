#include <string>

#include <xer/assert.h>
#include <xer/error.h>
#include <xer/ini.h>

namespace {

void test_ini_decode_global_entries()
{
    const auto result = xer::ini_decode(u8"name = xer\nversion=0.2.0a3\n");

    xer_assert(result.has_value());
    xer_assert_eq(result->entries.size(), static_cast<std::size_t>(2));
    xer_assert_eq(result->entries[0].key, u8"name");
    xer_assert_eq(result->entries[0].value, u8"xer");
    xer_assert_eq(result->entries[1].key, u8"version");
    xer_assert_eq(result->entries[1].value, u8"0.2.0a3");
    xer_assert(result->sections.empty());
}

void test_ini_decode_sections()
{
    const auto result = xer::ini_decode(
        u8"; comment\n"
        u8"# comment\n"
        u8"title = sample\r\n"
        u8"\r\n"
        u8"[ main ]\n"
        u8"name = xer\n"
        u8"enabled = true\r"
        u8"[empty]\n");

    xer_assert(result.has_value());
    xer_assert_eq(result->entries.size(), static_cast<std::size_t>(1));
    xer_assert_eq(result->entries[0].key, u8"title");
    xer_assert_eq(result->entries[0].value, u8"sample");
    xer_assert_eq(result->sections.size(), static_cast<std::size_t>(2));
    xer_assert_eq(result->sections[0].name, u8"main");
    xer_assert_eq(result->sections[0].entries.size(), static_cast<std::size_t>(2));
    xer_assert_eq(result->sections[0].entries[0].key, u8"name");
    xer_assert_eq(result->sections[0].entries[0].value, u8"xer");
    xer_assert_eq(result->sections[0].entries[1].key, u8"enabled");
    xer_assert_eq(result->sections[0].entries[1].value, u8"true");
    xer_assert_eq(result->sections[1].name, u8"empty");
    xer_assert(result->sections[1].entries.empty());
}

void test_ini_decode_preserves_duplicates()
{
    const auto result = xer::ini_decode(
        u8"key=1\n"
        u8"key=2\n"
        u8"[section]\n"
        u8"key=3\n"
        u8"[section]\n"
        u8"key=4\n");

    xer_assert(result.has_value());
    xer_assert_eq(result->entries.size(), static_cast<std::size_t>(2));
    xer_assert_eq(result->entries[0].value, u8"1");
    xer_assert_eq(result->entries[1].value, u8"2");
    xer_assert_eq(result->sections.size(), static_cast<std::size_t>(2));
    xer_assert_eq(result->sections[0].name, u8"section");
    xer_assert_eq(result->sections[0].entries[0].value, u8"3");
    xer_assert_eq(result->sections[1].name, u8"section");
    xer_assert_eq(result->sections[1].entries[0].value, u8"4");
}

void test_ini_decode_does_not_interpret_quotes_or_inline_comments()
{
    const auto result = xer::ini_decode(
        u8"quoted = \"xer\"\n"
        u8"semi = value ; not a comment\n"
        u8"hash = value # not a comment\n");

    xer_assert(result.has_value());
    xer_assert_eq(result->entries[0].value, u8"\"xer\"");
    xer_assert_eq(result->entries[1].value, u8"value ; not a comment");
    xer_assert_eq(result->entries[2].value, u8"value # not a comment");
}

void test_ini_decode_rejects_invalid_lines()
{
    const auto no_equal = xer::ini_decode(u8"name\n");
    const auto empty_key = xer::ini_decode(u8" = value\n");
    const auto bad_section = xer::ini_decode(u8"[section\n");
    const auto empty_section = xer::ini_decode(u8"[]\n");

    xer_assert_not(no_equal.has_value());
    xer_assert_eq(no_equal.error().code, xer::error_t::invalid_argument);

    xer_assert_not(empty_key.has_value());
    xer_assert_eq(empty_key.error().code, xer::error_t::invalid_argument);

    xer_assert_not(bad_section.has_value());
    xer_assert_eq(bad_section.error().code, xer::error_t::invalid_argument);

    xer_assert_not(empty_section.has_value());
    xer_assert_eq(empty_section.error().code, xer::error_t::invalid_argument);
}

void test_ini_decode_rejects_invalid_utf8()
{
    const char8_t text[] = {
        u8'n',
        u8'a',
        u8'm',
        u8'e',
        u8'=',
        static_cast<char8_t>(0x80),
        u8'\n',
    };

    const auto result = xer::ini_decode(std::u8string_view(text, sizeof(text)));

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::encoding_error);
}

void test_ini_encode_global_entries_and_sections()
{
    xer::ini_file file;
    file.entries.push_back({u8"title", u8"sample"});
    file.sections.push_back({u8"main", {{u8"name", u8"xer"}, {u8"enabled", u8"true"}}});
    file.sections.push_back({u8"empty", {}});

    const auto result = xer::ini_encode(file);

    xer_assert(result.has_value());
    xer_assert_eq(
        *result,
        std::u8string(
            u8"title=sample\n"
            u8"\n"
            u8"[main]\n"
            u8"name=xer\n"
            u8"enabled=true\n"
            u8"\n"
            u8"[empty]\n"));
}

void test_ini_round_trip()
{
    const std::u8string source =
        u8"title=sample\n"
        u8"\n"
        u8"[main]\n"
        u8"name=xer\n"
        u8"enabled=true\n";

    const auto decoded = xer::ini_decode(source);
    xer_assert(decoded.has_value());

    const auto encoded = xer::ini_encode(*decoded);
    xer_assert(encoded.has_value());
    xer_assert_eq(*encoded, source);
}

void test_ini_encode_rejects_unrepresentable_values()
{
    xer::ini_file empty_key;
    empty_key.entries.push_back({u8"", u8"value"});

    xer::ini_file key_with_equal;
    key_with_equal.entries.push_back({u8"a=b", u8"value"});

    xer::ini_file value_with_outer_space;
    value_with_outer_space.entries.push_back({u8"key", u8" value"});

    xer::ini_file section_with_bracket;
    section_with_bracket.sections.push_back({u8"bad]name", {}});

    const auto result_empty_key = xer::ini_encode(empty_key);
    const auto result_key_with_equal = xer::ini_encode(key_with_equal);
    const auto result_value_with_outer_space = xer::ini_encode(value_with_outer_space);
    const auto result_section_with_bracket = xer::ini_encode(section_with_bracket);

    xer_assert_not(result_empty_key.has_value());
    xer_assert_eq(result_empty_key.error().code, xer::error_t::invalid_argument);

    xer_assert_not(result_key_with_equal.has_value());
    xer_assert_eq(result_key_with_equal.error().code, xer::error_t::invalid_argument);

    xer_assert_not(result_value_with_outer_space.has_value());
    xer_assert_eq(result_value_with_outer_space.error().code, xer::error_t::invalid_argument);

    xer_assert_not(result_section_with_bracket.has_value());
    xer_assert_eq(result_section_with_bracket.error().code, xer::error_t::invalid_argument);
}

void test_ini_encode_rejects_invalid_utf8()
{
    xer::ini_file file;
    file.entries.push_back({u8"key", std::u8string(1, static_cast<char8_t>(0x80))});

    const auto result = xer::ini_encode(file);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::encoding_error);
}

} // namespace

int main()
{
    test_ini_decode_global_entries();
    test_ini_decode_sections();
    test_ini_decode_preserves_duplicates();
    test_ini_decode_does_not_interpret_quotes_or_inline_comments();
    test_ini_decode_rejects_invalid_lines();
    test_ini_decode_rejects_invalid_utf8();
    test_ini_encode_global_entries_and_sections();
    test_ini_round_trip();
    test_ini_encode_rejects_unrepresentable_values();
    test_ini_encode_rejects_invalid_utf8();

    return 0;
}
