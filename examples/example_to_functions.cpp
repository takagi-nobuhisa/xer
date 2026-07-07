// XER_EXAMPLE_BEGIN: to_functions_basic
//
// This example compares several character conversion functions
// from <xer/ctype.h>, including xer::toctrans with lower and
// latin1_upper.
//
// Expected output:
// tolower('A'): a
// tolower('1'): 1
// toupper('a'): A
// toupper('1'): 1
// tolower('Ä'): error(invalid_argument)
// toctrans('A', lower): a
// toctrans('a', upper): A
// toctrans('ä', upper): error(invalid_argument)
// toctrans('ä', latin1_upper): Ä

#include <xer/ctype.h>
#include <xer/diagnostics.h>


auto main() -> int
{
    if (!xer_print(u8"tolower('A')", xer::tolower(U'A'))) {
        return 1;
    }

    if (!xer_print(u8"tolower('1')", xer::tolower(U'1'))) {
        return 1;
    }

    if (!xer_print(u8"toupper('a')", xer::toupper(U'a'))) {
        return 1;
    }

    if (!xer_print(u8"toupper('1')", xer::toupper(U'1'))) {
        return 1;
    }

    if (!xer_print(u8"tolower('Ä')", xer::tolower(U'Ä'))) {
        return 1;
    }

    if (!xer_print(u8"toctrans('A', lower)", xer::toctrans(U'A', xer::ctrans_id::lower))) {
        return 1;
    }

    if (!xer_print(u8"toctrans('a', upper)", xer::toctrans(U'a', xer::ctrans_id::upper))) {
        return 1;
    }

    if (!xer_print(u8"toctrans('ä', upper)", xer::toctrans(U'ä', xer::ctrans_id::upper))) {
        return 1;
    }

    if (!xer_print(u8"toctrans('ä', latin1_upper)", xer::toctrans(U'ä', xer::ctrans_id::latin1_upper))) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: to_functions_basic
