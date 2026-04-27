/**
 * @file xer/bits/cmdline.h
 * @brief Command-line argument handling implementation.
 */

#pragma once

#ifndef XER_BITS_CMDLINE_H_INCLUDED_
#define XER_BITS_CMDLINE_H_INCLUDED_

#include <cerrno>
#include <cstdio>
#include <cstddef>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <xer/bits/common.h>
#include <xer/bits/text_encoding_common.h>
#include <xer/error.h>

#ifdef _WIN32
#    include <windows.h>
#endif

namespace xer {

using cmdline_arg = std::pair<std::u8string_view, std::u8string_view>;

/**
 * @brief Holds command-line arguments as UTF-8 strings.
 *
 * `cmdline` owns an argv-like sequence of UTF-8 strings. It does not parse
 * options by itself. Use `parse_arg` when an individual argument should be
 * interpreted according to XER's simple command-line rule.
 */
class cmdline {
public:
    cmdline() = default;

    /**
     * @brief Constructs a command-line object from UTF-8 arguments.
     *
     * @param args Command-line arguments.
     */
    explicit cmdline(std::vector<std::u8string> args)
        : args_(std::move(args))
    {
    }

    /**
     * @brief Returns the number of arguments.
     *
     * @return Number of stored arguments.
     */
    [[nodiscard]] auto size() const noexcept -> std::size_t
    {
        return args_.size();
    }

    /**
     * @brief Returns whether no arguments are stored.
     *
     * @return true if no arguments are stored.
     */
    [[nodiscard]] auto empty() const noexcept -> bool
    {
        return args_.empty();
    }

    /**
     * @brief Returns all raw arguments.
     *
     * @return Span over stored UTF-8 argument strings.
     */
    [[nodiscard]] auto args() const noexcept -> std::span<const std::u8string>
    {
        return std::span<const std::u8string>(args_.data(), args_.size());
    }

    /**
     * @brief Returns one raw argument.
     *
     * @param index Argument index.
     * @return Argument view on success.
     */
    [[nodiscard]] auto at(std::size_t index) const -> result<std::u8string_view>
    {
        if (index >= args_.size()) {
            return std::unexpected(make_error(error_t::out_of_range));
        }

        return std::u8string_view(args_[index]);
    }

private:
    std::vector<std::u8string> args_;
};

/**
 * @brief Parses one command-line argument according to XER's simple rule.
 *
 * The accepted option forms are `--name` and `--name=value`. A single-leading
 * hyphen form such as `-x` is not an option in XER and is treated as an
 * ordinary value. `--name=` is intentionally treated the same as `--name`,
 * because distinguishing "no value" from "empty value" is not valuable enough
 * for this small command-line helper.
 *
 * If the argument is an option, the returned pair is `{name, value}`. If the
 * argument is an ordinary value, the returned pair is `{empty, value}`.
 *
 * @param value Raw command-line argument.
 * @return Parsed command-line argument view pair.
 */
[[nodiscard]] inline auto parse_arg(std::u8string_view value) noexcept -> cmdline_arg
{
    constexpr std::u8string_view option_prefix = u8"--";

    if (!value.starts_with(option_prefix) || value.size() <= option_prefix.size()) {
        return {std::u8string_view(), value};
    }

    const std::size_t name_begin = option_prefix.size();
    if (value[name_begin] == u8'=') {
        return {std::u8string_view(), value};
    }

    const std::size_t separator = value.find(u8'=', name_begin);
    if (separator == std::u8string_view::npos) {
        return {value.substr(name_begin), std::u8string_view()};
    }

    return {
        value.substr(name_begin, separator - name_begin),
        value.substr(separator + 1),
    };
}

namespace detail {

#ifndef _WIN32

[[nodiscard]] inline auto cmdline_errno_error() noexcept -> error_t
{
    return errno == 0 ? error_t::io_error : static_cast<error_t>(errno);
}

[[nodiscard]] inline auto append_linux_cmdline_arg(
    std::vector<std::u8string>& args,
    std::string_view value) -> result<void>
{
    if (!is_valid_utf8(value)) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    args.push_back(to_u8string(value));
    return {};
}

[[nodiscard]] inline auto read_linux_cmdline_bytes() -> result<std::vector<char>>
{
    FILE* file = std::fopen("/proc/self/cmdline", "rb");
    if (file == nullptr) {
        return std::unexpected(make_error(cmdline_errno_error()));
    }

    std::vector<char> bytes;
    char buffer[4096];

    for (;;) {
        const std::size_t read_size = std::fread(buffer, 1, sizeof(buffer), file);
        if (read_size > 0) {
            bytes.insert(bytes.end(), buffer, buffer + read_size);
        }

        if (read_size < sizeof(buffer)) {
            if (std::ferror(file) != 0) {
                const error_t error_code = cmdline_errno_error();
                static_cast<void>(std::fclose(file));
                return std::unexpected(make_error(error_code));
            }

            break;
        }
    }

    if (std::fclose(file) != 0) {
        return std::unexpected(make_error(cmdline_errno_error()));
    }

    return bytes;
}

[[nodiscard]] inline auto parse_linux_cmdline_bytes(
    std::span<const char> bytes) -> result<cmdline>
{
    std::vector<std::u8string> args;
    std::size_t start = 0;

    for (std::size_t i = 0; i < bytes.size(); ++i) {
        if (bytes[i] != '\0') {
            continue;
        }

        const auto appended = append_linux_cmdline_arg(
            args,
            std::string_view(bytes.data() + start, i - start));
        if (!appended.has_value()) {
            return std::unexpected(appended.error());
        }

        start = i + 1;
    }

    if (start < bytes.size()) {
        const auto appended = append_linux_cmdline_arg(
            args,
            std::string_view(bytes.data() + start, bytes.size() - start));
        if (!appended.has_value()) {
            return std::unexpected(appended.error());
        }
    }

    return cmdline(std::move(args));
}

#else

using command_line_to_argv_w_type = LPWSTR* (WINAPI*)(LPCWSTR, int*);

class shell32_library {
public:
    shell32_library() = default;

    shell32_library(const shell32_library&) = delete;
    auto operator=(const shell32_library&) -> shell32_library& = delete;

    ~shell32_library()
    {
        if (handle_ != nullptr) {
            static_cast<void>(::FreeLibrary(handle_));
        }
    }

    [[nodiscard]] auto open() noexcept -> result<void>
    {
        handle_ = ::LoadLibraryW(L"Shell32.dll");
        if (handle_ == nullptr) {
            return std::unexpected(make_error(error_t::runtime_error));
        }

        return {};
    }

    [[nodiscard]] auto command_line_to_argv_w() const noexcept
        -> result<command_line_to_argv_w_type>
    {
        if (handle_ == nullptr) {
            return std::unexpected(make_error(error_t::runtime_error));
        }

        const FARPROC proc = ::GetProcAddress(handle_, "CommandLineToArgvW");
        if (proc == nullptr) {
            return std::unexpected(make_error(error_t::runtime_error));
        }

        return reinterpret_cast<command_line_to_argv_w_type>(proc);
    }

private:
    HMODULE handle_ = nullptr;
};

class local_memory_guard {
public:
    explicit local_memory_guard(HLOCAL handle) noexcept
        : handle_(handle)
    {
    }

    local_memory_guard(const local_memory_guard&) = delete;
    auto operator=(const local_memory_guard&) -> local_memory_guard& = delete;

    ~local_memory_guard()
    {
        if (handle_ != nullptr) {
            static_cast<void>(::LocalFree(handle_));
        }
    }

private:
    HLOCAL handle_ = nullptr;
};

#endif

} // namespace detail

/**
 * @brief Gets the current process command-line arguments.
 *
 * On Windows, this function obtains the command line through `GetCommandLineW`
 * and splits it with `CommandLineToArgvW`. The latter is loaded dynamically
 * from Shell32.dll so that users of this header-only library do not need an
 * additional explicit link option merely by including this header.
 *
 * On Linux, this function reads `/proc/self/cmdline` as NUL-separated byte
 * strings and requires each argument to be valid UTF-8.
 *
 * @return Command-line arguments on success.
 */
[[nodiscard]] inline auto get_cmdline() -> result<cmdline>
{
#ifdef _WIN32
    detail::shell32_library shell32;
    const auto opened = shell32.open();
    if (!opened.has_value()) {
        return std::unexpected(opened.error());
    }

    const auto command_line_to_argv_w = shell32.command_line_to_argv_w();
    if (!command_line_to_argv_w.has_value()) {
        return std::unexpected(command_line_to_argv_w.error());
    }

    int argc = 0;
    LPWSTR* const argv = (*command_line_to_argv_w)(::GetCommandLineW(), &argc);
    if (argv == nullptr || argc < 0) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    detail::local_memory_guard argv_guard(reinterpret_cast<HLOCAL>(argv));

    std::vector<std::u8string> args;
    args.reserve(static_cast<std::size_t>(argc));

    for (int i = 0; i < argc; ++i) {
        const auto converted = detail::wstring_to_utf8(std::wstring_view(argv[i]));
        if (!converted.has_value()) {
            return std::unexpected(converted.error());
        }

        args.push_back(*converted);
    }

    return cmdline(std::move(args));
#else
    const auto bytes = detail::read_linux_cmdline_bytes();
    if (!bytes.has_value()) {
        return std::unexpected(bytes.error());
    }

    return detail::parse_linux_cmdline_bytes(
        std::span<const char>(bytes->data(), bytes->size()));
#endif
}

} // namespace xer

#endif /* XER_BITS_CMDLINE_H_INCLUDED_ */
