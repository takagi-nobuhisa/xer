// XER_EXAMPLE_BEGIN: parse_basic
//
// This example shows how to attach xer::parse_error_detail to a result.
// The detail object reports a parser-specific reason and a source position.
//
// Expected output:
// Valid name: alpha_1
// Parse failed at line 1, column 4: invalid_token

#include <cstddef>
#include <iostream>
#include <string_view>

#include <xer/error.h>
#include <xer/parse.h>

namespace {

auto reason_name(xer::parse_error_reason reason) -> const char*
{
    switch (reason) {
    case xer::parse_error_reason::none:
        return "none";
    case xer::parse_error_reason::invalid_token:
        return "invalid_token";
    default:
        return "other";
    }
}

auto is_name_first(char8_t c) -> bool
{
    return (c >= u8'a' && c <= u8'z') || (c >= u8'A' && c <= u8'Z') || c == u8'_';
}

auto is_name_rest(char8_t c) -> bool
{
    return is_name_first(c) || (c >= u8'0' && c <= u8'9');
}

auto parse_ascii_name(std::u8string_view text)
    -> xer::result<std::u8string_view, xer::parse_error_detail>
{
    if (text.empty() || !is_name_first(text.front())) {
        return std::unexpected(xer::make_error<xer::parse_error_detail>(
            xer::error_t::invalid_argument,
            xer::parse_error_detail{
                .offset = 0,
                .line = 1,
                .column = 1,
                .reason = xer::parse_error_reason::invalid_token,
            }));
    }

    for (std::size_t i = 1; i < text.size(); ++i) {
        if (!is_name_rest(text[i])) {
            return std::unexpected(xer::make_error<xer::parse_error_detail>(
                xer::error_t::invalid_argument,
                xer::parse_error_detail{
                    .offset = i,
                    .line = 1,
                    .column = i + 1,
                    .reason = xer::parse_error_reason::invalid_token,
                }));
        }
    }

    return text;
}

} // namespace

auto main() -> int
{
    const auto valid = parse_ascii_name(u8"alpha_1");
    if (!valid.has_value()) {
        return 1;
    }

    std::cout << "Valid name: ";
    for (const auto c : *valid) {
        std::cout << static_cast<char>(c);
    }
    std::cout << '\n';

    const auto invalid = parse_ascii_name(u8"abc-123");
    if (invalid.has_value()) {
        return 1;
    }

    const auto& error = invalid.error();
    std::cout << "Parse failed at line " << error.line
              << ", column " << error.column << ": "
              << reason_name(error.reason) << '\n';

    return 0;
}

// XER_EXAMPLE_END: parse_basic
