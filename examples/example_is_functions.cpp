// XER_EXAMPLE_BEGIN: is_functions_basic
//
// This example compares several character classification functions
// from <xer/ctype.h>, including xer::isctype with alpha and
// latin1_alpha.
//
// Expected output:
// isalpha('A'): true
// isalpha('1'): false
// isdigit('1'): true
// isdigit('A'): false
// isspace(' '): true
// isspace('A'): false
// isascii('A'): true
// isascii('Ä'): false
// islatin1_alpha('Ä'): true
// islatin1_alpha('1'): false
// isctype('A', alpha): true
// isctype('Ä', alpha): false
// isctype('Ä', latin1_alpha): true

#include <xer/ctype.h>
#include <xer/diag.h>

auto main() -> int
{
    if (!xer_print(u8"isalpha('A')", xer::isalpha(U'A'))) {
        return 1;
    }
    if (!xer_print(u8"isalpha('1')", xer::isalpha(U'1'))) {
        return 1;
    }

    if (!xer_print(u8"isdigit('1')", xer::isdigit(U'1'))) {
        return 1;
    }
    if (!xer_print(u8"isdigit('A')", xer::isdigit(U'A'))) {
        return 1;
    }

    if (!xer_print(u8"isspace(' ')", xer::isspace(U' '))) {
        return 1;
    }
    if (!xer_print(u8"isspace('A')", xer::isspace(U'A'))) {
        return 1;
    }

    if (!xer_print(u8"isascii('A')", xer::isascii(U'A'))) {
        return 1;
    }
    if (!xer_print(u8"isascii('Ä')", xer::isascii(U'Ä'))) {
        return 1;
    }

    if (!xer_print(u8"islatin1_alpha('Ä')", xer::islatin1_alpha(U'Ä'))) {
        return 1;
    }
    if (!xer_print(u8"islatin1_alpha('1')", xer::islatin1_alpha(U'1'))) {
        return 1;
    }

    if (!xer_print(u8"isctype('A', alpha)", xer::isctype(U'A', xer::ctype_id::alpha))) {
        return 1;
    }
    if (!xer_print(u8"isctype('Ä', alpha)", xer::isctype(U'Ä', xer::ctype_id::alpha))) {
        return 1;
    }
    if (!xer_print(
            u8"isctype('Ä', latin1_alpha)",
            xer::isctype(U'Ä', xer::ctype_id::latin1_alpha))) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: is_functions_basic
