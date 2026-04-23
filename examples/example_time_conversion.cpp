// XER_EXAMPLE_BEGIN: time_conversion_basic
//
// This example shows basic usage of xer::time,
// xer::localtime, xer::gmtime, xer::mktime, and xer::difftime.
//
// Expected output:
// The program prints:
// - the current time_t value
// - the local broken-down time
// - the UTC broken-down time
// - the time_t value reconstructed from localtime
// - the difference between the original and reconstructed values

#include <xer/stdio.h>
#include <xer/time.h>

namespace {

auto print_tm(std::u8string_view label, const xer::tm& value) -> int
{
    if (!xer::printf(
            u8"%s%04d-%02d-%02d %02d:%02d:%02d\n",
            label.data(),
            value.tm_year + 1900,
            value.tm_mon + 1,
            value.tm_mday,
            value.tm_hour,
            value.tm_min,
            value.tm_sec)) {
        return 1;
    }

    return 0;
}

} // namespace

auto main() -> int
{
    const auto now = xer::time();
    if (!now) {
        return 1;
    }

    if (!xer::printf(u8"time      = %.6f\n", *now)) {
        return 1;
    }

    const auto local = xer::localtime(*now);
    if (!local) {
        return 1;
    }

    if (print_tm(u8"localtime = ", *local) != 0) {
        return 1;
    }

    const auto utc = xer::gmtime(*now);
    if (!utc) {
        return 1;
    }

    if (print_tm(u8"gmtime    = ", *utc) != 0) {
        return 1;
    }

    const auto reconstructed = xer::mktime(*local);
    if (!reconstructed) {
        return 1;
    }

    if (!xer::printf(u8"mktime    = %.6f\n", *reconstructed)) {
        return 1;
    }

    const auto difference = xer::difftime(*reconstructed, *now);
    if (!xer::printf(u8"difftime  = %.6f\n", difference)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: time_conversion_basic
