// XER_EXAMPLE_BEGIN: process_stdin_pipe
//
// This example writes to a child process's standard input through a binary
// stream and reads the echoed data from its standard output.
//
// Expected output:
// child echoed: ping

#include <array>
#include <cstddef>
#include <span>

#include <xer/process.h>
#include <xer/stdio.h>

namespace {

[[nodiscard]] auto make_echo_process_options() -> xer::process_options
{
#if defined(_WIN32)
    return xer::process_options {
        xer::path(u8"C:/Windows/System32/cmd.exe"),
        {u8"/C", u8"more"},
        xer::process_stdio::pipe,
        xer::process_stdio::pipe,
        xer::process_stdio::inherit};
#else
    return xer::process_options {
        xer::path(u8"/bin/cat"),
        {},
        xer::process_stdio::pipe,
        xer::process_stdio::pipe,
        xer::process_stdio::inherit};
#endif
}

[[nodiscard]] auto byte_matches(std::byte value, char expected) noexcept -> bool
{
    return std::to_integer<unsigned char>(value) == static_cast<unsigned char>(expected);
}

} // namespace

auto main() -> int
{
    auto spawned = xer::process_spawn(make_echo_process_options());
    if (!spawned || !spawned->stdin_stream || !spawned->stdout_stream) {
        return 1;
    }

    constexpr std::array<std::byte, 4> input {
        std::byte {'p'},
        std::byte {'i'},
        std::byte {'n'},
        std::byte {'g'},
    };

    auto written = xer::fwrite(std::span<const std::byte>(input), *spawned->stdin_stream);
    if (!written || *written != input.size()) {
        return 1;
    }

    if (!xer::fclose(*spawned->stdin_stream)) {
        return 1;
    }

    std::array<std::byte, 4> buffer {};
    auto read = xer::fread(std::span<std::byte>(buffer), *spawned->stdout_stream);
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

    if (!xer::puts(u8"child echoed: ping")) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: process_stdin_pipe
