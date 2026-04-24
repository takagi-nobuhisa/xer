// XER_EXAMPLE_BEGIN: ctype_unicode
//
// This example checks whether characters are valid Unicode scalar values
// and whether they are in the Unicode BMP.
//
// Expected output:
// あ is Unicode
// あ is in the Unicode BMP
// 𠮷 is Unicode
// 𠮷 is not in the Unicode BMP
// surrogate is not Unicode

#include <xer/ctype.h>
#include <xer/stdio.h>

auto main() -> int {
    if (xer::isctype(U'あ', xer::ctype_id::unicode)) {
        if (!xer::puts(u8"あ is Unicode").has_value()) {
            return 1;
        }
    }

    if (xer::isctype(U'あ', xer::ctype_id::unicode_bmp)) {
        if (!xer::puts(u8"あ is in the Unicode BMP").has_value()) {
            return 1;
        }
    }

    if (xer::isctype(U'𠮷', xer::ctype_id::unicode)) {
        if (!xer::puts(u8"𠮷 is Unicode").has_value()) {
            return 1;
        }
    }

    if (!xer::isctype(U'𠮷', xer::ctype_id::unicode_bmp)) {
        if (!xer::puts(u8"𠮷 is not in the Unicode BMP").has_value()) {
            return 1;
        }
    }

    if (!xer::isctype(static_cast<char32_t>(0xD800),
                      xer::ctype_id::unicode)) {
        if (!xer::puts(u8"surrogate is not Unicode").has_value()) {
            return 1;
        }
    }

    return 0;
}

// XER_EXAMPLE_END: ctype_unicode
