/**
 * @file xer/bits/process.h
 * @brief Internal child process management facilities.
 */

#pragma once

#ifndef XER_BITS_PROCESS_H_INCLUDED_
#define XER_BITS_PROCESS_H_INCLUDED_

#include <cstdint>
#include <expected>
#include <new>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <xer/bits/binary_stream.h>
#include <xer/error.h>
#include <xer/path.h>

#if defined(_WIN32)
#    if !defined(NOMINMAX)
#        define NOMINMAX
#    endif
#    include <windows.h>
#else
#    include <cerrno>
#    include <cstdlib>
#    include <fcntl.h>
#    include <sys/types.h>
#    include <sys/wait.h>
#    include <unistd.h>
#endif

namespace xer {

/**
 * @brief Specifies how a standard stream of a child process is connected.
 */
enum class process_stdio {
    /**
     * @brief Inherit the corresponding stream from the parent process.
     */
    inherit,

    /**
     * @brief Connect the stream to a null device.
     */
    null,

    /**
     * @brief Connect the stream to a pipe.
     */
    pipe
};

/**
 * @brief Options used to spawn a child process.
 */
struct process_options {
    /**
     * @brief Program path to execute.
     */
    path program;

    /**
     * @brief Command-line arguments excluding argv[0].
     */
    std::vector<std::u8string> arguments;

    /**
     * @brief Standard input handling.
     */
    process_stdio stdin_mode = process_stdio::inherit;

    /**
     * @brief Standard output handling.
     */
    process_stdio stdout_mode = process_stdio::inherit;

    /**
     * @brief Standard error handling.
     */
    process_stdio stderr_mode = process_stdio::inherit;
};

/**
 * @brief Result of waiting for a child process.
 */
struct process_result {
    /**
     * @brief Exit code, or 128 + signal number on POSIX signal termination.
     */
    int exit_code = 0;
};

class process;
struct process_spawn_result;

namespace detail {

[[nodiscard]] inline auto process_stdio_options_are_supported(
    const process_options& options) noexcept -> bool
{
    return (options.stdin_mode == process_stdio::inherit ||
            options.stdin_mode == process_stdio::null ||
            options.stdin_mode == process_stdio::pipe) &&
           (options.stdout_mode == process_stdio::inherit ||
            options.stdout_mode == process_stdio::null ||
            options.stdout_mode == process_stdio::pipe) &&
           (options.stderr_mode == process_stdio::inherit ||
            options.stderr_mode == process_stdio::null ||
            options.stderr_mode == process_stdio::pipe);
}

#if defined(_WIN32)

[[nodiscard]] inline auto quote_windows_command_line_argument(
    std::wstring_view value) -> std::wstring
{
    if (value.empty()) {
        return L"\"\"";
    }

    bool needs_quotes = false;
    for (wchar_t ch : value) {
        if (ch == L' ' || ch == L'\t' || ch == L'\n' || ch == L'\v' || ch == L'\"') {
            needs_quotes = true;
            break;
        }
    }

    if (!needs_quotes) {
        return std::wstring(value);
    }

    std::wstring result;
    result.push_back(L'\"');

    std::size_t backslash_count = 0;
    for (wchar_t ch : value) {
        if (ch == L'\\') {
            ++backslash_count;
            continue;
        }

        if (ch == L'\"') {
            result.append(backslash_count * 2 + 1, L'\\');
            result.push_back(ch);
            backslash_count = 0;
            continue;
        }

        result.append(backslash_count, L'\\');
        backslash_count = 0;
        result.push_back(ch);
    }

    result.append(backslash_count * 2, L'\\');
    result.push_back(L'\"');
    return result;
}

[[nodiscard]] inline auto utf8_argument_to_wstring(
    std::u8string_view value) -> result<std::wstring>
{
    const auto converted = detail::utf8_to_wstring(detail::to_byte_string(value));
    if (!converted.has_value()) {
        return std::unexpected(converted.error());
    }

    return *converted;
}

[[nodiscard]] inline auto build_windows_command_line(
    const native_path_string& program,
    const std::vector<std::u8string>& arguments) -> result<std::wstring>
{
    std::wstring command_line = quote_windows_command_line_argument(program);

    for (std::u8string_view argument : arguments) {
        const auto converted = utf8_argument_to_wstring(argument);
        if (!converted.has_value()) {
            return std::unexpected(converted.error());
        }

        command_line.push_back(L' ');
        command_line += quote_windows_command_line_argument(*converted);
    }

    return command_line;
}

struct process_pipe_stream_state {
    HANDLE handle = nullptr;
};

inline auto close_process_pipe_handle(HANDLE& handle) noexcept -> void
{
    if (handle != nullptr && handle != INVALID_HANDLE_VALUE) {
        CloseHandle(handle);
        handle = nullptr;
    }
}

[[nodiscard]] inline auto open_process_null_handle(
    DWORD desired_access,
    SECURITY_ATTRIBUTES* security_attributes) noexcept -> HANDLE
{
    return CreateFileW(
        L"NUL",
        desired_access,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        security_attributes,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
}

inline auto process_pipe_close(binary_stream_handle_t handle) noexcept -> int
{
    auto* const state = reinterpret_cast<process_pipe_stream_state*>(handle);
    if (state == nullptr) {
        return -1;
    }

    int result = 0;
    if (state->handle == nullptr || state->handle == INVALID_HANDLE_VALUE) {
        result = -1;
    } else if (CloseHandle(state->handle) == FALSE) {
        result = -1;
    }

    delete state;
    return result;
}

inline auto process_pipe_read(binary_stream_handle_t handle, void* s, int n) noexcept -> int
{
    if (s == nullptr || n < 0) {
        return -1;
    }

    auto* const state = reinterpret_cast<process_pipe_stream_state*>(handle);
    if (state == nullptr || state->handle == nullptr || state->handle == INVALID_HANDLE_VALUE) {
        return -1;
    }

    if (n == 0) {
        return 0;
    }

    DWORD read_size = 0;
    if (ReadFile(state->handle, s, static_cast<DWORD>(n), &read_size, nullptr) == FALSE) {
        return GetLastError() == ERROR_BROKEN_PIPE ? 0 : -1;
    }

    return static_cast<int>(read_size);
}

inline auto process_pipe_write(binary_stream_handle_t handle, const void* s, int n) noexcept -> int
{
    if (s == nullptr || n < 0) {
        return -1;
    }

    auto* const state = reinterpret_cast<process_pipe_stream_state*>(handle);
    if (state == nullptr || state->handle == nullptr || state->handle == INVALID_HANDLE_VALUE) {
        return -1;
    }

    if (n == 0) {
        return 0;
    }

    DWORD written_size = 0;
    if (WriteFile(state->handle, s, static_cast<DWORD>(n), &written_size, nullptr) == FALSE) {
        return GetLastError() == ERROR_BROKEN_PIPE ? 0 : -1;
    }

    return static_cast<int>(written_size);
}

#else

[[nodiscard]] inline auto utf8_argument_to_string(
    std::u8string_view value) -> result<std::string>
{
    const auto result = detail::to_byte_string(value);
    if (!detail::is_valid_utf8(result)) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    return result;
}

struct process_pipe_stream_state {
    int fd = -1;
};

inline auto close_process_pipe_fd(int& fd) noexcept -> void
{
    if (fd >= 0) {
        (void)::close(fd);
        fd = -1;
    }
}

[[nodiscard]] inline auto open_process_null_fd(int flags) noexcept -> int
{
    return ::open("/dev/null", flags);
}

inline auto process_pipe_close(binary_stream_handle_t handle) noexcept -> int
{
    auto* const state = reinterpret_cast<process_pipe_stream_state*>(handle);
    if (state == nullptr) {
        return -1;
    }

    int result = 0;
    if (state->fd < 0) {
        result = -1;
    } else {
        int closed = -1;
        do {
            closed = ::close(state->fd);
        } while (closed < 0 && errno == EINTR);

        if (closed < 0) {
            result = -1;
        }
    }

    delete state;
    return result;
}

inline auto process_pipe_read(binary_stream_handle_t handle, void* s, int n) noexcept -> int
{
    if (s == nullptr || n < 0) {
        return -1;
    }

    auto* const state = reinterpret_cast<process_pipe_stream_state*>(handle);
    if (state == nullptr || state->fd < 0) {
        return -1;
    }

    if (n == 0) {
        return 0;
    }

    ssize_t result = -1;
    do {
        result = ::read(state->fd, s, static_cast<std::size_t>(n));
    } while (result < 0 && errno == EINTR);

    return result < 0 ? -1 : static_cast<int>(result);
}

inline auto process_pipe_write(binary_stream_handle_t handle, const void* s, int n) noexcept -> int
{
    if (s == nullptr || n < 0) {
        return -1;
    }

    auto* const state = reinterpret_cast<process_pipe_stream_state*>(handle);
    if (state == nullptr || state->fd < 0) {
        return -1;
    }

    if (n == 0) {
        return 0;
    }

    ssize_t result = -1;
    do {
        result = ::write(state->fd, s, static_cast<std::size_t>(n));
    } while (result < 0 && errno == EINTR);

    return result < 0 ? -1 : static_cast<int>(result);
}

#endif

inline auto process_pipe_seek(binary_stream_handle_t, std::int64_t, int) noexcept -> int
{
    return -1;
}

inline auto process_pipe_tell(binary_stream_handle_t) noexcept -> std::int64_t
{
    return -1;
}

#if defined(_WIN32)
[[nodiscard]] inline auto make_process_pipe_stream(HANDLE handle) -> result<binary_stream>
{
    auto* state = new (std::nothrow) process_pipe_stream_state {handle};
    if (state == nullptr) {
        close_process_pipe_handle(handle);
        return std::unexpected(make_error(error_t::nomem));
    }

    return binary_stream(
        reinterpret_cast<binary_stream_handle_t>(state),
        process_pipe_close,
        process_pipe_read,
        process_pipe_write,
        process_pipe_seek,
        process_pipe_tell);
}
#else
[[nodiscard]] inline auto make_process_pipe_stream(int fd) -> result<binary_stream>
{
    auto* state = new (std::nothrow) process_pipe_stream_state {fd};
    if (state == nullptr) {
        close_process_pipe_fd(fd);
        return std::unexpected(make_error(error_t::nomem));
    }

    return binary_stream(
        reinterpret_cast<binary_stream_handle_t>(state),
        process_pipe_close,
        process_pipe_read,
        process_pipe_write,
        process_pipe_seek,
        process_pipe_tell);
}
#endif

} // namespace detail

/**
 * @brief Move-only handle for a child process.
 */
class process {
public:
    /**
     * @brief Constructs an empty process handle.
     */
    process() noexcept = default;

    process(const process&) = delete;
    auto operator=(const process&) -> process& = delete;

    /**
     * @brief Moves a process handle.
     *
     * @param other Source process handle.
     */
    process(process&& other) noexcept
    {
        move_from(other);
    }

    /**
     * @brief Move-assigns a process handle.
     *
     * @param other Source process handle.
     * @return This object.
     */
    auto operator=(process&& other) noexcept -> process&
    {
        if (this != &other) {
            close_handle();
            move_from(other);
        }

        return *this;
    }

    /**
     * @brief Destroys the process handle.
     *
     * The destructor releases the native handle owned by this object, but it
     * does not wait for process termination. Call process_wait explicitly when
     * the exit status is needed and to avoid leaving a zombie process on POSIX.
     */
    ~process()
    {
        close_handle();
    }

    /**
     * @brief Returns whether this object currently owns a process handle.
     *
     * @return true if open, otherwise false.
     */
    [[nodiscard]] auto is_open() const noexcept -> bool
    {
#if defined(_WIN32)
        return handle_ != nullptr;
#else
        return pid_ > 0;
#endif
    }

private:
#if defined(_WIN32)
    explicit process(HANDLE handle) noexcept
        : handle_(handle)
    {
    }

    void close_handle() noexcept
    {
        if (handle_ != nullptr) {
            CloseHandle(handle_);
            handle_ = nullptr;
        }
    }

    void move_from(process& other) noexcept
    {
        handle_ = other.handle_;
        other.handle_ = nullptr;
    }

    HANDLE handle_ = nullptr;
#else
    explicit process(pid_t pid) noexcept
        : pid_(pid)
    {
    }

    void close_handle() noexcept
    {
        pid_ = -1;
    }

    void move_from(process& other) noexcept
    {
        pid_ = other.pid_;
        other.pid_ = -1;
    }

    pid_t pid_ = -1;
#endif

    friend auto process_spawn(const process_options& options) noexcept -> result<process_spawn_result>;
    friend auto process_wait(process& value) noexcept -> result<process_result>;
};

/**
 * @brief Result of spawning a child process.
 */
struct process_spawn_result {
    /**
     * @brief Spawned child process.
     */
    process proc;

    /**
     * @brief Parent-side stream connected to the child process's standard input.
     */
    std::optional<binary_stream> stdin_stream;

    /**
     * @brief Parent-side stream connected to the child process's standard output.
     */
    std::optional<binary_stream> stdout_stream;

    /**
     * @brief Parent-side stream connected to the child process's standard error.
     */
    std::optional<binary_stream> stderr_stream;
};

/**
 * @brief Spawns a child process.
 *
 * The program is executed directly without going through a command shell.
 * Arguments are passed as separate command-line arguments.
 *
 * process_stdio::null and process_stdio::pipe are supported for
 * stdin_mode, stdout_mode, and stderr_mode.
 *
 * @param options Spawn options.
 * @return Spawn result on success.
 */
[[nodiscard]] inline auto process_spawn(
    const process_options& options) noexcept -> result<process_spawn_result>
{
    if (!detail::process_stdio_options_are_supported(options)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const auto native_program = to_native_path(options.program);
    if (!native_program.has_value()) {
        return std::unexpected(native_program.error());
    }
#if defined(_WIN32)
    auto command_line = detail::build_windows_command_line(*native_program, options.arguments);
    if (!command_line.has_value()) {
        return std::unexpected(command_line.error());
    }

    HANDLE stdin_read = nullptr;
    HANDLE stdin_write = nullptr;
    HANDLE stdout_read = nullptr;
    HANDLE stdout_write = nullptr;
    HANDLE stderr_read = nullptr;
    HANDLE stderr_write = nullptr;
    HANDLE stdin_null = nullptr;
    HANDLE stdout_null = nullptr;
    HANDLE stderr_null = nullptr;
    const bool pipe_stdin = options.stdin_mode == process_stdio::pipe;
    const bool pipe_stdout = options.stdout_mode == process_stdio::pipe;
    const bool pipe_stderr = options.stderr_mode == process_stdio::pipe;
    const bool null_stdin = options.stdin_mode == process_stdio::null;
    const bool null_stdout = options.stdout_mode == process_stdio::null;
    const bool null_stderr = options.stderr_mode == process_stdio::null;

    SECURITY_ATTRIBUTES security_attributes{};
    security_attributes.nLength = sizeof(security_attributes);
    security_attributes.lpSecurityDescriptor = nullptr;
    security_attributes.bInheritHandle = TRUE;

    auto close_child_setup_handles = [&]() noexcept -> void {
        detail::close_process_pipe_handle(stdin_read);
        detail::close_process_pipe_handle(stdin_write);
        detail::close_process_pipe_handle(stdout_read);
        detail::close_process_pipe_handle(stdout_write);
        detail::close_process_pipe_handle(stderr_read);
        detail::close_process_pipe_handle(stderr_write);
        detail::close_process_pipe_handle(stdin_null);
        detail::close_process_pipe_handle(stdout_null);
        detail::close_process_pipe_handle(stderr_null);
    };

    if (pipe_stdin) {
        if (CreatePipe(&stdin_read, &stdin_write, &security_attributes, 0) == FALSE) {
            close_child_setup_handles();
            return std::unexpected(make_error(error_t::process_error));
        }

        if (SetHandleInformation(stdin_write, HANDLE_FLAG_INHERIT, 0) == FALSE) {
            close_child_setup_handles();
            return std::unexpected(make_error(error_t::process_error));
        }
    } else if (null_stdin) {
        stdin_null = detail::open_process_null_handle(GENERIC_READ, &security_attributes);
        if (stdin_null == INVALID_HANDLE_VALUE) {
            stdin_null = nullptr;
            close_child_setup_handles();
            return std::unexpected(make_error(error_t::process_error));
        }
    }

    if (pipe_stdout) {
        if (CreatePipe(&stdout_read, &stdout_write, &security_attributes, 0) == FALSE) {
            close_child_setup_handles();
            return std::unexpected(make_error(error_t::process_error));
        }

        if (SetHandleInformation(stdout_read, HANDLE_FLAG_INHERIT, 0) == FALSE) {
            close_child_setup_handles();
            return std::unexpected(make_error(error_t::process_error));
        }
    } else if (null_stdout) {
        stdout_null = detail::open_process_null_handle(GENERIC_WRITE, &security_attributes);
        if (stdout_null == INVALID_HANDLE_VALUE) {
            stdout_null = nullptr;
            close_child_setup_handles();
            return std::unexpected(make_error(error_t::process_error));
        }
    }

    if (pipe_stderr) {
        if (CreatePipe(&stderr_read, &stderr_write, &security_attributes, 0) == FALSE) {
            close_child_setup_handles();
            return std::unexpected(make_error(error_t::process_error));
        }

        if (SetHandleInformation(stderr_read, HANDLE_FLAG_INHERIT, 0) == FALSE) {
            close_child_setup_handles();
            return std::unexpected(make_error(error_t::process_error));
        }
    } else if (null_stderr) {
        stderr_null = detail::open_process_null_handle(GENERIC_WRITE, &security_attributes);
        if (stderr_null == INVALID_HANDLE_VALUE) {
            stderr_null = nullptr;
            close_child_setup_handles();
            return std::unexpected(make_error(error_t::process_error));
        }
    }

    STARTUPINFOW startup_info{};
    startup_info.cb = sizeof(startup_info);
    if (pipe_stdin || pipe_stdout || pipe_stderr || null_stdin || null_stdout || null_stderr) {
        startup_info.dwFlags = STARTF_USESTDHANDLES;
        startup_info.hStdInput = pipe_stdin ? stdin_read : (null_stdin ? stdin_null : GetStdHandle(STD_INPUT_HANDLE));
        startup_info.hStdOutput = pipe_stdout ? stdout_write : (null_stdout ? stdout_null : GetStdHandle(STD_OUTPUT_HANDLE));
        startup_info.hStdError = pipe_stderr ? stderr_write : (null_stderr ? stderr_null : GetStdHandle(STD_ERROR_HANDLE));
    }

    PROCESS_INFORMATION process_information{};

    std::wstring mutable_command_line = *command_line;
    const BOOL created = CreateProcessW(
        native_program->c_str(),
        mutable_command_line.data(),
        nullptr,
        nullptr,
        (pipe_stdin || pipe_stdout || pipe_stderr || null_stdin || null_stdout || null_stderr) ? TRUE : FALSE,
        0,
        nullptr,
        nullptr,
        &startup_info,
        &process_information);

    if (pipe_stdin) {
        detail::close_process_pipe_handle(stdin_read);
    }
    if (pipe_stdout) {
        detail::close_process_pipe_handle(stdout_write);
    }
    if (pipe_stderr) {
        detail::close_process_pipe_handle(stderr_write);
    }
    detail::close_process_pipe_handle(stdin_null);
    detail::close_process_pipe_handle(stdout_null);
    detail::close_process_pipe_handle(stderr_null);

    if (created == FALSE) {
        detail::close_process_pipe_handle(stdin_write);
        detail::close_process_pipe_handle(stdout_read);
        detail::close_process_pipe_handle(stderr_read);
        return std::unexpected(make_error(error_t::process_error));
    }

    CloseHandle(process_information.hThread);

    std::optional<binary_stream> stdin_stream;
    if (pipe_stdin) {
        auto stream = detail::make_process_pipe_stream(stdin_write);
        if (!stream.has_value()) {
            CloseHandle(process_information.hProcess);
            detail::close_process_pipe_handle(stdout_read);
            detail::close_process_pipe_handle(stderr_read);
            return std::unexpected(stream.error());
        }
        stdin_write = nullptr;
        stdin_stream.emplace(std::move(*stream));
    }

    std::optional<binary_stream> stdout_stream;
    if (pipe_stdout) {
        auto stream = detail::make_process_pipe_stream(stdout_read);
        if (!stream.has_value()) {
            CloseHandle(process_information.hProcess);
            detail::close_process_pipe_handle(stderr_read);
            return std::unexpected(stream.error());
        }
        stdout_read = nullptr;
        stdout_stream.emplace(std::move(*stream));
    }

    std::optional<binary_stream> stderr_stream;
    if (pipe_stderr) {
        auto stream = detail::make_process_pipe_stream(stderr_read);
        if (!stream.has_value()) {
            CloseHandle(process_information.hProcess);
            return std::unexpected(stream.error());
        }
        stderr_read = nullptr;
        stderr_stream.emplace(std::move(*stream));
    }

    return process_spawn_result {
        process(process_information.hProcess),
        std::move(stdin_stream),
        std::move(stdout_stream),
        std::move(stderr_stream)};
#else
    std::vector<std::string> native_arguments;
    native_arguments.reserve(options.arguments.size() + 1);
    native_arguments.push_back(*native_program);

    for (std::u8string_view argument : options.arguments) {
        const auto converted = detail::utf8_argument_to_string(argument);
        if (!converted.has_value()) {
            return std::unexpected(converted.error());
        }

        native_arguments.push_back(*converted);
    }

    std::vector<char*> argv;
    argv.reserve(native_arguments.size() + 1);
    for (std::string& argument : native_arguments) {
        argv.push_back(argument.data());
    }
    argv.push_back(nullptr);

    int stdin_pipe[2] = {-1, -1};
    int stdout_pipe[2] = {-1, -1};
    int stderr_pipe[2] = {-1, -1};
    int stdin_null = -1;
    int stdout_null = -1;
    int stderr_null = -1;
    const bool pipe_stdin = options.stdin_mode == process_stdio::pipe;
    const bool pipe_stdout = options.stdout_mode == process_stdio::pipe;
    const bool pipe_stderr = options.stderr_mode == process_stdio::pipe;
    const bool null_stdin = options.stdin_mode == process_stdio::null;
    const bool null_stdout = options.stdout_mode == process_stdio::null;
    const bool null_stderr = options.stderr_mode == process_stdio::null;

    auto close_child_setup_fds = [&]() noexcept -> void {
        detail::close_process_pipe_fd(stdin_pipe[0]);
        detail::close_process_pipe_fd(stdin_pipe[1]);
        detail::close_process_pipe_fd(stdout_pipe[0]);
        detail::close_process_pipe_fd(stdout_pipe[1]);
        detail::close_process_pipe_fd(stderr_pipe[0]);
        detail::close_process_pipe_fd(stderr_pipe[1]);
        detail::close_process_pipe_fd(stdin_null);
        detail::close_process_pipe_fd(stdout_null);
        detail::close_process_pipe_fd(stderr_null);
    };

    if (pipe_stdin && pipe(stdin_pipe) < 0) {
        close_child_setup_fds();
        return std::unexpected(make_error(error_t::process_error));
    }
    if (null_stdin) {
        stdin_null = detail::open_process_null_fd(O_RDONLY);
        if (stdin_null < 0) {
            close_child_setup_fds();
            return std::unexpected(make_error(error_t::process_error));
        }
    }

    if (pipe_stdout && pipe(stdout_pipe) < 0) {
        close_child_setup_fds();
        return std::unexpected(make_error(error_t::process_error));
    }
    if (null_stdout) {
        stdout_null = detail::open_process_null_fd(O_WRONLY);
        if (stdout_null < 0) {
            close_child_setup_fds();
            return std::unexpected(make_error(error_t::process_error));
        }
    }

    if (pipe_stderr && pipe(stderr_pipe) < 0) {
        close_child_setup_fds();
        return std::unexpected(make_error(error_t::process_error));
    }
    if (null_stderr) {
        stderr_null = detail::open_process_null_fd(O_WRONLY);
        if (stderr_null < 0) {
            close_child_setup_fds();
            return std::unexpected(make_error(error_t::process_error));
        }
    }

    const pid_t pid = fork();
    if (pid < 0) {
        close_child_setup_fds();
        return std::unexpected(make_error(error_t::process_error));
    }

    if (pid == 0) {
        if (pipe_stdin) {
            detail::close_process_pipe_fd(stdin_pipe[1]);
            if (dup2(stdin_pipe[0], STDIN_FILENO) < 0) {
                _exit(127);
            }
            detail::close_process_pipe_fd(stdin_pipe[0]);
        } else if (null_stdin) {
            if (dup2(stdin_null, STDIN_FILENO) < 0) {
                _exit(127);
            }
        }

        if (pipe_stdout) {
            detail::close_process_pipe_fd(stdout_pipe[0]);
            if (dup2(stdout_pipe[1], STDOUT_FILENO) < 0) {
                _exit(127);
            }
            detail::close_process_pipe_fd(stdout_pipe[1]);
        } else if (null_stdout) {
            if (dup2(stdout_null, STDOUT_FILENO) < 0) {
                _exit(127);
            }
        }

        if (pipe_stderr) {
            detail::close_process_pipe_fd(stderr_pipe[0]);
            if (dup2(stderr_pipe[1], STDERR_FILENO) < 0) {
                _exit(127);
            }
            detail::close_process_pipe_fd(stderr_pipe[1]);
        } else if (null_stderr) {
            if (dup2(stderr_null, STDERR_FILENO) < 0) {
                _exit(127);
            }
        }

        close_child_setup_fds();
        execv(native_arguments.front().c_str(), argv.data());
        _exit(127);
    }

    detail::close_process_pipe_fd(stdin_null);
    detail::close_process_pipe_fd(stdout_null);
    detail::close_process_pipe_fd(stderr_null);

    std::optional<binary_stream> stdin_stream;
    if (pipe_stdin) {
        detail::close_process_pipe_fd(stdin_pipe[0]);
        auto stream = detail::make_process_pipe_stream(stdin_pipe[1]);
        if (!stream.has_value()) {
            detail::close_process_pipe_fd(stdout_pipe[0]);
            detail::close_process_pipe_fd(stdout_pipe[1]);
            detail::close_process_pipe_fd(stderr_pipe[0]);
            detail::close_process_pipe_fd(stderr_pipe[1]);
            return std::unexpected(stream.error());
        }
        stdin_pipe[1] = -1;
        stdin_stream.emplace(std::move(*stream));
    }

    std::optional<binary_stream> stdout_stream;
    if (pipe_stdout) {
        detail::close_process_pipe_fd(stdout_pipe[1]);
        auto stream = detail::make_process_pipe_stream(stdout_pipe[0]);
        if (!stream.has_value()) {
            detail::close_process_pipe_fd(stderr_pipe[0]);
            detail::close_process_pipe_fd(stderr_pipe[1]);
            return std::unexpected(stream.error());
        }
        stdout_pipe[0] = -1;
        stdout_stream.emplace(std::move(*stream));
    }

    std::optional<binary_stream> stderr_stream;
    if (pipe_stderr) {
        detail::close_process_pipe_fd(stderr_pipe[1]);
        auto stream = detail::make_process_pipe_stream(stderr_pipe[0]);
        if (!stream.has_value()) {
            return std::unexpected(stream.error());
        }
        stderr_pipe[0] = -1;
        stderr_stream.emplace(std::move(*stream));
    }

    return process_spawn_result {
        process(pid),
        std::move(stdin_stream),
        std::move(stdout_stream),
        std::move(stderr_stream)};
#endif
}

/**
 * @brief Waits for a child process to terminate.
 *
 * @param value Process handle.
 * @return Process result on success.
 */
[[nodiscard]] inline auto process_wait(process& value) noexcept -> result<process_result>
{
    if (!value.is_open()) {
        return std::unexpected(make_error(error_t::process_error));
    }

#if defined(_WIN32)
    const DWORD wait_result = WaitForSingleObject(value.handle_, INFINITE);
    if (wait_result != WAIT_OBJECT_0) {
        value.close_handle();
        return std::unexpected(make_error(error_t::process_error));
    }

    DWORD exit_code = 0;
    if (GetExitCodeProcess(value.handle_, &exit_code) == FALSE) {
        value.close_handle();
        return std::unexpected(make_error(error_t::process_error));
    }

    value.close_handle();
    return process_result {static_cast<int>(exit_code)};
#else
    int status = 0;
    pid_t waited = -1;
    do {
        waited = waitpid(value.pid_, &status, 0);
    } while (waited < 0 && errno == EINTR);

    if (waited < 0) {
        value.close_handle();
        return std::unexpected(make_error(error_t::process_error));
    }

    value.close_handle();

    if (WIFEXITED(status)) {
        return process_result {WEXITSTATUS(status)};
    }

    if (WIFSIGNALED(status)) {
        return process_result {128 + WTERMSIG(status)};
    }

    return std::unexpected(make_error(error_t::process_error));
#endif
}

} // namespace xer

#endif /* XER_BITS_PROCESS_H_INCLUDED_ */
