/**
 * @file tests/test_process.cpp
 * @brief Tests for process spawning, waiting, and standard-stream pipes.
 */

#include <array>
#include <cstddef>
#include <span>
#include <string>

#include <xer/assert.h>
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

[[nodiscard]] auto make_stdout_process_options() -> xer::process_options
{
#if defined(_WIN32)
    return xer::process_options {
        xer::path(u8"C:/Windows/System32/cmd.exe"),
        {u8"/C", u8"echo ping"},
        xer::process_stdio::inherit,
        xer::process_stdio::pipe,
        xer::process_stdio::inherit};
#else
    return xer::process_options {
        xer::path(u8"/bin/sh"),
        {u8"-c", u8"printf ping"},
        xer::process_stdio::inherit,
        xer::process_stdio::pipe,
        xer::process_stdio::inherit};
#endif
}

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

[[nodiscard]] auto make_stdin_stdout_process_options() -> xer::process_options
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


[[nodiscard]] auto make_null_stdio_process_options() -> xer::process_options
{
#if defined(_WIN32)
    return xer::process_options {
        xer::path(u8"C:/Windows/System32/cmd.exe"),
        {u8"/C", u8"echo stdout & echo stderr 1>&2"},
        xer::process_stdio::null,
        xer::process_stdio::null,
        xer::process_stdio::null};
#else
    return xer::process_options {
        xer::path(u8"/bin/sh"),
        {u8"-c", u8"printf stdout; printf stderr >&2"},
        xer::process_stdio::null,
        xer::process_stdio::null,
        xer::process_stdio::null};
#endif
}

[[nodiscard]] auto byte_matches(std::byte value, char expected) noexcept -> bool
{
    return std::to_integer<unsigned char>(value) == static_cast<unsigned char>(expected);
}

void test_process_spawn_and_wait_success()
{
    auto spawned = xer::process_spawn(make_exit_process_options(0));
    xer_assert(spawned.has_value());
    xer_assert(spawned->proc.is_open());
    xer_assert_not(spawned->stdin_stream.has_value());
    xer_assert_not(spawned->stdout_stream.has_value());
    xer_assert_not(spawned->stderr_stream.has_value());

    auto result = xer::process_wait(spawned->proc);
    xer_assert(result.has_value());
    xer_assert_eq(result->exit_code, 0);
    xer_assert_not(spawned->proc.is_open());
}

void test_process_spawn_and_wait_nonzero_exit()
{
    auto spawned = xer::process_spawn(make_exit_process_options(7));
    xer_assert(spawned.has_value());

    auto result = xer::process_wait(spawned->proc);
    xer_assert(result.has_value());
    xer_assert_eq(result->exit_code, 7);
}

void test_process_stdout_pipe()
{
    auto spawned = xer::process_spawn(make_stdout_process_options());
    xer_assert(spawned.has_value());
    xer_assert(spawned->stdout_stream.has_value());

    std::array<std::byte, 4> buffer {};
    auto read = xer::fread(std::span<std::byte>(buffer), *spawned->stdout_stream);
    xer_assert(read.has_value());
    xer_assert_eq(*read, buffer.size());

    xer_assert(byte_matches(buffer[0], 'p'));
    xer_assert(byte_matches(buffer[1], 'i'));
    xer_assert(byte_matches(buffer[2], 'n'));
    xer_assert(byte_matches(buffer[3], 'g'));

    auto result = xer::process_wait(spawned->proc);
    xer_assert(result.has_value());
    xer_assert_eq(result->exit_code, 0);
}

void test_process_stderr_pipe()
{
    auto spawned = xer::process_spawn(make_stderr_process_options());
    xer_assert(spawned.has_value());
    xer_assert_not(spawned->stdout_stream.has_value());
    xer_assert(spawned->stderr_stream.has_value());

    std::array<std::byte, 4> buffer {};
    auto read = xer::fread(std::span<std::byte>(buffer), *spawned->stderr_stream);
    xer_assert(read.has_value());
    xer_assert_eq(*read, buffer.size());

    xer_assert(byte_matches(buffer[0], 'p'));
    xer_assert(byte_matches(buffer[1], 'i'));
    xer_assert(byte_matches(buffer[2], 'n'));
    xer_assert(byte_matches(buffer[3], 'g'));

    auto result = xer::process_wait(spawned->proc);
    xer_assert(result.has_value());
    xer_assert_eq(result->exit_code, 0);
}

void test_process_stdin_stdout_pipe()
{
    auto spawned = xer::process_spawn(make_stdin_stdout_process_options());
    xer_assert(spawned.has_value());
    xer_assert(spawned->stdin_stream.has_value());
    xer_assert(spawned->stdout_stream.has_value());

    constexpr std::array<std::byte, 4> input {
        std::byte {'p'},
        std::byte {'i'},
        std::byte {'n'},
        std::byte {'g'},
    };

    auto written = xer::fwrite(std::span<const std::byte>(input), *spawned->stdin_stream);
    xer_assert(written.has_value());
    xer_assert_eq(*written, input.size());

    auto closed = xer::fclose(*spawned->stdin_stream);
    xer_assert(closed.has_value());

    std::array<std::byte, 4> buffer {};
    auto read = xer::fread(std::span<std::byte>(buffer), *spawned->stdout_stream);
    xer_assert(read.has_value());
    xer_assert_eq(*read, buffer.size());

    xer_assert(byte_matches(buffer[0], 'p'));
    xer_assert(byte_matches(buffer[1], 'i'));
    xer_assert(byte_matches(buffer[2], 'n'));
    xer_assert(byte_matches(buffer[3], 'g'));

    auto result = xer::process_wait(spawned->proc);
    xer_assert(result.has_value());
    xer_assert_eq(result->exit_code, 0);
}

void test_process_wait_empty_process_fails()
{
    xer::process process;

    const auto result = xer::process_wait(process);
    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::process_error);
}

void test_process_null_stdio()
{
    auto spawned = xer::process_spawn(make_null_stdio_process_options());
    xer_assert(spawned.has_value());
    xer_assert(spawned->proc.is_open());
    xer_assert_not(spawned->stdin_stream.has_value());
    xer_assert_not(spawned->stdout_stream.has_value());
    xer_assert_not(spawned->stderr_stream.has_value());

    auto result = xer::process_wait(spawned->proc);
    xer_assert(result.has_value());
    xer_assert_eq(result->exit_code, 0);
}

} // namespace

auto main() -> int
{
    test_process_spawn_and_wait_success();
    test_process_spawn_and_wait_nonzero_exit();
    test_process_stdout_pipe();
    test_process_stderr_pipe();
    test_process_stdin_stdout_pipe();
    test_process_wait_empty_process_fails();
    test_process_null_stdio();

    return 0;
}
