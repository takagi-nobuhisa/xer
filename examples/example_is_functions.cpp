// XER_EXAMPLE_BEGIN: is_functions_basic
//
// This example compares several character classification functions
// from <xer/ctype.h>, including xer::isctype with alpha and
// latin1_alpha.
//
// Expected output:
// isalpha('A')         = true
// isalpha('1')         = false
// isdigit('1')         = true
// isdigit('A')         = false
// isspace(' ')         = true
// isspace('A')         = false
// isascii('A')         = true
// isascii('Ä')         = false
// islatin1_alpha('Ä')  = true
// islatin1_alpha('1')  = false
// isctype('A', alpha)  = true
// isctype('Ä', alpha)  = false
// isctype('Ä', latin1_alpha) = true

#include <xer/ctype.h>
#include <xer/stdio.h>

namespace {

auto print_result(std::u8string_view label, bool value) -> int
{
    if (!xer::puts(label)) {
        return 1;
    }

    if (!xer::puts(value ? u8"true" : u8"false")) {
        return 1;
    }

    return 0;
}

} // namespace

auto main() -> int
{
    if (print_result(u8"isalpha('A')         = ", xer::isalpha(U'A')) != 0) {
        return 1;
    }
    if (print_result(u8"isalpha('1')         = ", xer::isalpha(U'1')) != 0) {
        return 1;
    }

    if (print_result(u8"isdigit('1')         = ", xer::isdigit(U'1')) != 0) {
        return 1;
    }
    if (print_result(u8"isdigit('A')         = ", xer::isdigit(U'A')) != 0) {
        return 1;
    }

    if (print_result(u8"isspace(' ')         = ", xer::isspace(U' ')) != 0) {
        return 1;
    }
    if (print_result(u8"isspace('A')         = ", xer::isspace(U'A')) != 0) {
        return 1;
    }

    if (print_result(u8"isascii('A')         = ", xer::isascii(U'A')) != 0) {
        return 1;
    }
    if (print_result(u8"isascii('Ä')         = ", xer::isascii(U'Ä')) != 0) {
        return 1;
    }

    if (print_result(u8"islatin1_alpha('Ä')  = ", xer::islatin1_alpha(U'Ä')) != 0) {
        return 1;
    }
    if (print_result(u8"islatin1_alpha('1')  = ", xer::islatin1_alpha(U'1')) != 0) {
        return 1;
    }

    if (print_result(u8"isctype('A', alpha)  = ",
                     xer::isctype(U'A', xer::ctype_id::alpha)) != 0) {
        return 1;
    }
    if (print_result(u8"isctype('Ä', alpha)  = ",
                     xer::isctype(U'Ä', xer::ctype_id::alpha)) != 0) {
        return 1;
    }
    if (print_result(u8"isctype('Ä', latin1_alpha) = ",
                     xer::isctype(U'Ä', xer::ctype_id::latin1_alpha)) != 0) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: is_functions_basic
