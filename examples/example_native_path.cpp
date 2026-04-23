// XER_EXAMPLE_BEGIN: native_path_basic
//
// This example shows basic usage of xer::to_native_path
// and xer::from_native_path.
//
// Expected output:
// native conversion = ok
// round trip = dir/subdir/file.txt

#include <string_view>

#include <xer/path.h>
#include <xer/stdio.h>

auto main() -> int
{
    const xer::path original(u8"dir/subdir/file.txt");

    const auto native = xer::to_native_path(original);
    if (!native) {
        return 1;
    }

#ifdef _WIN32
    bool converted_ok = false;
    for (const auto ch : *native) {
        if (ch == L'\\') {
            converted_ok = true;
            break;
        }
    }
#else
    bool converted_ok = false;
    for (const auto ch : *native) {
        if (ch == '/') {
            converted_ok = true;
            break;
        }
    }
#endif

    if (!xer::puts(converted_ok ? u8"native conversion = ok"
                                : u8"native conversion = unexpected")) {
        return 1;
    }

    const auto restored =
        xer::from_native_path(std::basic_string_view<xer::native_path_char_t>(*native));
    if (!restored) {
        return 1;
    }

    if (!xer::fputs(u8"round trip = ", xer_stdout)) {
        return 1;
    }

    if (!xer::puts(restored->str())) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: native_path_basic
