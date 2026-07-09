/**
 * @file xer/bits/dlfcn.h
 * @brief Internal dynamic shared object loading facilities.
 */

#pragma once

#ifndef XER_BITS_DLFCN_H_INCLUDED_
#define XER_BITS_DLFCN_H_INCLUDED_

#include <cstring>
#include <expected>
#include <memory>
#include <new>
#include <string>
#include <string_view>

#include <xer/bits/text_encoding_common.h>
#include <xer/error.h>
#include <xer/path.h>

#if defined(_WIN32)
#    include <xer/bits/windows.h>
#else
#    include <dlfcn.h>
#endif

namespace xer {

namespace detail {

#if defined(_WIN32)

using native_shared_library_handle_t = HMODULE;

[[nodiscard]] inline auto win32_dlfcn_error_to_error_t(DWORD value) noexcept -> error_t
{
    switch (value) {
    case ERROR_SUCCESS:
        return error_t::success;
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
    case ERROR_MOD_NOT_FOUND:
        return error_t::noent;
    case ERROR_ACCESS_DENIED:
        return error_t::acces;
    case ERROR_NOT_ENOUGH_MEMORY:
    case ERROR_OUTOFMEMORY:
        return error_t::nomem;
    case ERROR_INVALID_PARAMETER:
    case ERROR_INVALID_NAME:
    case ERROR_BAD_PATHNAME:
        return error_t::invalid_argument;
    case ERROR_BAD_EXE_FORMAT:
        return error_t::noexec;
    case ERROR_PROC_NOT_FOUND:
        return error_t::not_found;
    default:
        return error_t::io_error;
    }
}

inline auto close_native_shared_library(native_shared_library_handle_t handle) noexcept -> void
{
    if (handle != nullptr) {
        (void)::FreeLibrary(handle);
    }
}

#else

using native_shared_library_handle_t = void*;

inline auto close_native_shared_library(native_shared_library_handle_t handle) noexcept -> void
{
    if (handle != nullptr) {
        (void)::dlclose(handle);
    }
}

#endif

struct shared_library_state {
    native_shared_library_handle_t handle = nullptr;

    explicit shared_library_state(native_shared_library_handle_t handle_) noexcept
        : handle(handle_)
    {
    }

    shared_library_state(const shared_library_state&) = delete;
    auto operator=(const shared_library_state&) -> shared_library_state& = delete;

    ~shared_library_state() noexcept
    {
        close_native_shared_library(handle);
    }
};

[[nodiscard]] inline auto has_null_character(std::string_view value) noexcept -> bool
{
    return value.find('\0') != std::string_view::npos;
}

} // namespace detail

/**
 * @brief Copyable shared-library handle.
 *
 * `shared_library` is a lightweight shared handle. Copies refer to the same
 * native library handle, and the library is unloaded when the last handle is
 * released.
 */
class shared_library {
public:
    shared_library() = default;

private:
    explicit shared_library(
        std::shared_ptr<detail::shared_library_state> state) noexcept
        : state_(std::move(state))
    {
    }

    std::shared_ptr<detail::shared_library_state> state_;

    friend auto dlopen(std::string_view path) -> result<shared_library>;
    friend auto dlclose(shared_library& library) noexcept -> void;
    friend auto is_open(const shared_library& library) noexcept -> bool;
    friend auto native_handle(const shared_library& library) noexcept -> void*;
    friend auto dlsym(
        const shared_library& library,
        std::string_view name,
        void*& out) -> result<void>;
};

/**
 * @brief Loads a shared library.
 *
 * On POSIX platforms, this function uses `::dlopen` with
 * `RTLD_NOW | RTLD_LOCAL`. On Windows, it uses `LoadLibraryW`.
 *
 * @param path UTF-8 library path.
 * @return Loaded shared-library handle on success.
 */
[[nodiscard]] inline auto dlopen(std::string_view path) -> result<shared_library>
{
    if (path.empty() || detail::has_null_character(path)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const auto native_path = to_native_path(xer::path(detail::to_u8string(path)));
    if (!native_path.has_value()) {
        return std::unexpected(native_path.error());
    }

#if defined(_WIN32)
    const auto handle = ::LoadLibraryW(native_path->c_str());
    if (handle == nullptr) {
        return std::unexpected(
            make_error(detail::win32_dlfcn_error_to_error_t(::GetLastError())));
    }
#else
    const auto handle = ::dlopen(native_path->c_str(), RTLD_NOW | RTLD_LOCAL);
    if (handle == nullptr) {
        return std::unexpected(make_error(error_t::noent));
    }
#endif

    try {
        return shared_library(std::make_shared<detail::shared_library_state>(handle));
    } catch (...) {
        detail::close_native_shared_library(handle);
        return std::unexpected(make_error(error_t::nomem));
    }
}

/**
 * @brief Releases this handle's reference to a shared library.
 *
 * If other `shared_library` objects refer to the same native handle, the native
 * library remains loaded until the last reference is released.
 *
 * @param library Target shared-library handle.
 */
inline auto dlclose(shared_library& library) noexcept -> void
{
    library.state_.reset();
}

/**
 * @brief Returns whether the handle currently refers to a loaded library.
 *
 * @param library Target shared-library handle.
 * @return true if the handle is open.
 * @return false otherwise.
 */
[[nodiscard]] inline auto is_open(const shared_library& library) noexcept -> bool
{
    return library.state_ != nullptr && library.state_->handle != nullptr;
}

/**
 * @brief Returns the native library handle.
 *
 * @param library Target shared-library handle.
 * @return Native handle, or nullptr if the handle is empty.
 */
[[nodiscard]] inline auto native_handle(const shared_library& library) noexcept -> void*
{
    if (!is_open(library)) {
        return nullptr;
    }

    return reinterpret_cast<void*>(library.state_->handle);
}

/**
 * @brief Finds a symbol address in a shared library.
 *
 * @param library Target shared-library handle.
 * @param name Symbol name.
 * @param out Receives the raw symbol address on success.
 * @return Success on completion.
 */
[[nodiscard]] inline auto dlsym(
    const shared_library& library,
    std::string_view name,
    void*& out) -> result<void>
{
    out = nullptr;

    if (!is_open(library) || name.empty() || detail::has_null_character(name)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const std::string native_name(name);

#if defined(_WIN32)
    const auto proc = ::GetProcAddress(library.state_->handle, native_name.c_str());
    if (proc == nullptr) {
        return std::unexpected(
            make_error(detail::win32_dlfcn_error_to_error_t(::GetLastError())));
    }

    static_assert(sizeof(out) == sizeof(proc));
    std::memcpy(&out, &proc, sizeof(out));
    return {};
#else
    (void)::dlerror();
    void* const symbol = ::dlsym(library.state_->handle, native_name.c_str());
    const char* const error = ::dlerror();
    if (error != nullptr) {
        return std::unexpected(make_error(error_t::not_found));
    }

    out = symbol;
    return {};
#endif
}

/**
 * @brief Finds a typed function symbol in a shared library.
 *
 * The function pointer type is inferred from `out`, so callers do not have to
 * write an explicit cast at the call site.
 *
 * @tparam Function Function type.
 * @param library Target shared-library handle.
 * @param name Symbol name.
 * @param out Receives the typed function pointer on success.
 * @return Success on completion.
 */
template<class Function>
[[nodiscard]] inline auto dlsym(
    const shared_library& library,
    std::string_view name,
    Function*& out) -> result<void>
{
    out = nullptr;

    void* address = nullptr;
    auto result = dlsym(library, name, address);
    if (!result.has_value()) {
        return std::unexpected(result.error());
    }

    static_assert(sizeof(out) == sizeof(address));
    std::memcpy(&out, &address, sizeof(out));
    return {};
}

} // namespace xer

#endif // XER_BITS_DLFCN_H_INCLUDED_
