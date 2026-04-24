// XER_EXAMPLE_BEGIN: process_null_stdio
//
// This example spawns a child process with all standard streams connected
// to the null device.
//
// Expected output:
// child exit code: 0

#include <xer/process.h>
#include <xer/stdio.h>

namespace {

[[nodiscard]] auto make_process_options() -> xer::process_options
{
#if defined(_WIN32)
    return xer::process_options {
        xer::path(u8"C:/Windows/System32/cmd.exe"),
        {u8"/C", u8"echo discarded stdout & echo discarded stderr 1>&2"},
        xer::process_stdio::null,
        xer::process_stdio::null,
        xer::process_stdio::null};
#else
    return xer::process_options {
        xer::path(u8"/bin/sh"),
        {u8"-c", u8"printf discarded_stdout; printf discarded_stderr >&2"},
        xer::process_stdio::null,
        xer::process_stdio::null,
        xer::process_stdio::null};
#endif
}

} // namespace

auto main() -> int
{
    auto spawned = xer::process_spawn(make_process_options());
    if (!spawned.has_value()) {
        return 1;
    }

    auto result = xer::process_wait(spawned->proc);
    if (!result.has_value()) {
        return 1;
    }

    if (!xer::printf(u8"child exit code: %d\n", result->exit_code)) {
        return 1;
    }

    return result->exit_code == 0 ? 0 : 1;
}

// XER_EXAMPLE_END: process_null_stdio
