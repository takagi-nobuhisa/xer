// XER_EXAMPLE_BEGIN: process_exit_code
//
// This example checks the exit code returned by a child process.
// A nonzero child exit code is still a successful process_wait result.
//
// Expected output:
// child exit code: 7

#include <string>

#include <xer/process.h>
#include <xer/stdio.h>

namespace {

[[nodiscard]] auto make_exit_process_options(int exit_code) -> xer::process_options
{
#if defined(_WIN32)
    return xer::process_options {
        xer::path(u8"C:/Windows/System32/cmd.exe"),
        {u8"/C", u8"exit", exit_code == 0 ? std::u8string(u8"0") : std::u8string(u8"7")}};
#else
    return xer::process_options {
        xer::path(u8"/bin/sh"),
        {u8"-c", exit_code == 0 ? std::u8string(u8"exit 0") : std::u8string(u8"exit 7")}};
#endif
}

} // namespace

auto main() -> int
{
    auto spawned = xer::process_spawn(make_exit_process_options(7));
    if (!spawned) {
        return 1;
    }

    auto result = xer::process_wait(spawned->proc);
    if (!result) {
        return 1;
    }

    if (result->exit_code != 7) {
        return 1;
    }

    if (!xer::printf(u8"child exit code: %d\n", result->exit_code)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: process_exit_code
