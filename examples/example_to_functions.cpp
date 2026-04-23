// XER_EXAMPLE_BEGIN: to_functions_basic
//
// This example compares several character conversion functions
// from <xer/ctype.h>, including xer::toctrans with lower and
// latin1_upper.
//
// Expected output:
// tolower('A')                = 'a'
// tolower('1')                = '1'
// toupper('a')                = 'A'
// toupper('1')                = '1'
// tolower('Ä')                = error
// toctrans('A', lower)        = 'a'
// toctrans('a', upper)        = 'A'
// toctrans('ä', upper)        = error
// toctrans('ä', latin1_upper) = 'Ä'

#include <xer/ctype.h>
#include <xer/stdio.h>

namespace {

auto print_line(std::u8string_view line) -> int
{
    if (!xer::puts(line).has_value()) {
        return 1;
    }

    return 0;
}

auto print_ascii_result(
    std::u8string_view label,
    const xer::result<char32_t>& value) -> int
{
    if (!value.has_value()) {
        std::u8string line(label);
        line += u8"error";
        return print_line(line);
    }

    std::u8string line(label);

    if (*value == U'a') {
        line += u8"'a'";
        return print_line(line);
    }

    if (*value == U'A') {
        line += u8"'A'";
        return print_line(line);
    }

    if (*value == U'1') {
        line += u8"'1'";
        return print_line(line);
    }

    return 1;
}

auto print_latin1_result(
    std::u8string_view label,
    const xer::result<char32_t>& value) -> int
{
    if (!value.has_value()) {
        std::u8string line(label);
        line += u8"error";
        return print_line(line);
    }

    std::u8string line(label);

    if (*value == U'Ä') {
        line += u8"'Ä'";
        return print_line(line);
    }

    return 1;
}

} // namespace

auto main() -> int
{
    if (print_ascii_result(u8"tolower('A')                = ", xer::tolower(U'A')) != 0) {
        return 1;
    }

    if (print_ascii_result(u8"tolower('1')                = ", xer::tolower(U'1')) != 0) {
        return 1;
    }

    if (print_ascii_result(u8"toupper('a')                = ", xer::toupper(U'a')) != 0) {
        return 1;
    }

    if (print_ascii_result(u8"toupper('1')                = ", xer::toupper(U'1')) != 0) {
        return 1;
    }

    if (print_ascii_result(u8"tolower('Ä')                = ", xer::tolower(U'Ä')) != 0) {
        return 1;
    }

    if (print_ascii_result(
            u8"toctrans('A', lower)        = ",
            xer::toctrans(U'A', xer::ctrans_id::lower)) != 0) {
        return 1;
    }

    if (print_ascii_result(
            u8"toctrans('a', upper)        = ",
            xer::toctrans(U'a', xer::ctrans_id::upper)) != 0) {
        return 1;
    }

    if (print_ascii_result(
            u8"toctrans('ä', upper)        = ",
            xer::toctrans(U'ä', xer::ctrans_id::upper)) != 0) {
        return 1;
    }

    if (print_latin1_result(
            u8"toctrans('ä', latin1_upper) = ",
            xer::toctrans(U'ä', xer::ctrans_id::latin1_upper)) != 0) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: to_functions_basic
