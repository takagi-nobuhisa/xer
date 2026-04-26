/**
 * @file examples/example_diag.cpp
 * @brief Example of XER diagnostic tracing and logging.
 */

// XER_EXAMPLE_BEGIN: diag_basic
//
// This example writes one trace line and one CSV log record.
//
// Expected output is written to the standard error stream.
// The trace line is fixed:
// [general][40] value (int) = 42
//
// The log line starts with the current local timestamp and then uses CSV fields:
// YYYY-MM-DD HH:MM:SS.mmm,io,30,"opened sample.txt"

#include <xer/diag.h>

namespace {

constexpr int value = 42;

} // namespace

auto main() -> int
{
    xer::set_trace_level(xer::diag_debug);
    xer_trace(xer::diag_category::general, xer::diag_debug, value);

    xer_log(xer::diag_category::io, xer::diag_info, u8"opened sample.txt");

    return 0;
}

// XER_EXAMPLE_END: diag_basic
