// XER_EXAMPLE_BEGIN: process_stderr_pipe
//
// This example reads a child process's standard error through a binary stream.
//
// Expected output:
// child stderr: ping

#include <array>
#include <cstddef>
#include <span>

#include <xer/process.h>
#include <xer/stdio.h>

namespace {

[[nodiscard]] auto make_stderr_process_options() -> xer::process_options
{
#if defined(_WIN32)
    return xer::process_options {
        xer::path(u8"C:/Windows/System32/cmd.exe"),
        {u8"/C", u8"echo ping 1>&2"},
        xer::process_stdio::inherit,
        xer::process_stdio::inherit,
        xer::process_stdio::pipe};
#else
    return xer::process_options {
        xer::path(u8"/bin/sh"),
        {u8"-c", u8"printf ping >&2"},
        xer::process_stdio::inherit,
        xer::process_stdio::inherit,
        xer::process_stdio::pipe};
#endif
}

[[nodiscard]] auto byte_matches(std::byte value, char expected) noexcept -> bool
{
    return std::to_integer<unsigned char>(value) == static_cast<unsigned char>(expected);
}

} // namespace

auto main() -> int
{
    auto spawned = xer::process_spawn(make_stderr_process_options());
    if (!spawned || !spawned->stderr_stream) {
        return 1;
    }

    std::array<std::byte, 4> buffer {};
    auto read = xer::fread(std::span<std::byte>(buffer), *spawned->stderr_stream);
    if (!read || *read != buffer.size()) {
        return 1;
    }

    if (!byte_matches(buffer[0], 'p') || !byte_matches(buffer[1], 'i') ||
        !byte_matches(buffer[2], 'n') || !byte_matches(buffer[3], 'g')) {
        return 1;
    }

    auto result = xer::process_wait(spawned->proc);
    if (!result || result->exit_code != 0) {
        return 1;
    }

    if (!xer::puts(u8"child stderr: ping")) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: process_stderr_pipe
