/**
 * @file tests/test_cmdline.cpp
 * @brief Tests for command-line argument utilities.
 */

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include <xer/assert.h>
#include <xer/cmdline.h>

namespace {

void test_cmdline_basic_storage()
{
    const xer::cmdline line({
        std::u8string(u8"program"),
        std::u8string(u8"--name=xer"),
        std::u8string(u8"value"),
    });

    xer_assert_eq(line.size(), static_cast<std::size_t>(3));
    xer_assert_not(line.empty());

    const auto arg0 = line.at(0);
    xer_assert(arg0.has_value());
    xer_assert_eq(*arg0, std::u8string_view(u8"program"));

    const auto arg1 = line.at(1);
    xer_assert(arg1.has_value());
    xer_assert_eq(*arg1, std::u8string_view(u8"--name=xer"));

    const auto raw_args = line.args();
    xer_assert_eq(raw_args.size(), static_cast<std::size_t>(3));
    xer_assert_eq(raw_args[2], std::u8string(u8"value"));
}

void test_cmdline_at_out_of_range()
{
    const xer::cmdline line({std::u8string(u8"program")});

    const auto arg = line.at(1);
    xer_assert_not(arg.has_value());
    xer_assert_eq(arg.error().code, xer::error_t::out_of_range);
}

void test_parse_arg_option_without_value()
{
    const auto parsed = xer::parse_arg(u8"--verbose");

    xer_assert_eq(parsed.first, std::u8string_view(u8"verbose"));
    xer_assert_eq(parsed.second, std::u8string_view(u8""));
}

void test_parse_arg_option_with_value()
{
    const auto parsed = xer::parse_arg(u8"--name=xer");

    xer_assert_eq(parsed.first, std::u8string_view(u8"name"));
    xer_assert_eq(parsed.second, std::u8string_view(u8"xer"));
}

void test_parse_arg_option_with_empty_value()
{
    const auto parsed = xer::parse_arg(u8"--name=");

    xer_assert_eq(parsed.first, std::u8string_view(u8"name"));
    xer_assert_eq(parsed.second, std::u8string_view(u8""));
}

void test_parse_arg_plain_value()
{
    const auto parsed = xer::parse_arg(u8"value");

    xer_assert_eq(parsed.first, std::u8string_view(u8""));
    xer_assert_eq(parsed.second, std::u8string_view(u8"value"));
}

void test_parse_arg_single_hyphen_value()
{
    const auto parsed = xer::parse_arg(u8"-name");

    xer_assert_eq(parsed.first, std::u8string_view(u8""));
    xer_assert_eq(parsed.second, std::u8string_view(u8"-name"));
}

void test_parse_arg_double_hyphen_value()
{
    const auto parsed = xer::parse_arg(u8"--");

    xer_assert_eq(parsed.first, std::u8string_view(u8""));
    xer_assert_eq(parsed.second, std::u8string_view(u8"--"));
}

void test_parse_arg_empty_name_value()
{
    const auto parsed = xer::parse_arg(u8"--=value");

    xer_assert_eq(parsed.first, std::u8string_view(u8""));
    xer_assert_eq(parsed.second, std::u8string_view(u8"--=value"));
}

void test_get_cmdline()
{
    const auto line = xer::get_cmdline();

    xer_assert(line.has_value());
    xer_assert_not(line->empty());

    const auto first = line->at(0);
    xer_assert(first.has_value());
}

} // namespace

auto main() -> int
{
    test_cmdline_basic_storage();
    test_cmdline_at_out_of_range();
    test_parse_arg_option_without_value();
    test_parse_arg_option_with_value();
    test_parse_arg_option_with_empty_value();
    test_parse_arg_plain_value();
    test_parse_arg_single_hyphen_value();
    test_parse_arg_double_hyphen_value();
    test_parse_arg_empty_name_value();
    test_get_cmdline();

    return 0;
}
