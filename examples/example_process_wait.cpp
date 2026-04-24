// XER_EXAMPLE_BEGIN: process_wait
//
// This example spawns a child process and waits for it to finish.
//
// Expected output:
// exit code: 0

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
    auto spawned = xer::process_spawn(make_exit_process_options(0));
    if (!spawned) {
        return 1;
    }

    auto result = xer::process_wait(spawned->proc);
    if (!result) {
        return 1;
    }

    if (!xer::printf(u8"exit code: %d\n", result->exit_code)) {
        return 1;
    }

    return result->exit_code == 0 ? 0 : 1;
}

// XER_EXAMPLE_END: process_wait
