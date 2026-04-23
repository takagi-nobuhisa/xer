// XER_EXAMPLE_BEGIN: time_format_basic
//
// This example shows basic usage of xer::ctime and xer::strftime.
//
// Expected output:
// The program prints:
// - a ctime-style time string
// - a strftime-formatted time string

#include <xer/stdio.h>
#include <xer/time.h>

auto main() -> int
{
    const auto now = xer::time();
    if (!now) {
        return 1;
    }

    const auto local = xer::localtime(*now);
    if (!local) {
        return 1;
    }

    const auto ctime_text = xer::ctime(*now);

    if (!xer::fputs(u8"ctime    = ", xer_stdout)) {
        return 1;
    }

    if (!xer::puts(ctime_text)) {
        return 1;
    }

    const auto formatted = xer::strftime(u8"%Y-%m-%d %H:%M:%S", *local);
    if (!formatted) {
        return 1;
    }

    if (!xer::fputs(u8"strftime = ", xer_stdout)) {
        return 1;
    }

    if (!xer::puts(*formatted)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: time_format_basic
