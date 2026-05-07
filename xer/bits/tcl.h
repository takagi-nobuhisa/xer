/**
 * @file xer/bits/tcl.h
 * @brief Tcl-side implementation for XER Tcl/Tk integration.
 */

#pragma once

#ifndef XER_BITS_TCL_H_INCLUDED_
#define XER_BITS_TCL_H_INCLUDED_

#include <concepts>
#include <cstddef>
#include <expected>
#include <functional>
#include <initializer_list>
#include <limits>
#include <mutex>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <tcl.h>

#include <xer/bits/common.h>
#include <xer/bits/environs.h>
#include <xer/cmdline.h>
#include <xer/error.h>

namespace xer::tk {

using result_code_t = int;
using eval_flag_t = int;
using var_flag_t = int;

inline constexpr result_code_t result_ok = TCL_OK;
inline constexpr result_code_t result_error = TCL_ERROR;
inline constexpr result_code_t result_return = TCL_RETURN;
inline constexpr result_code_t result_break = TCL_BREAK;
inline constexpr result_code_t result_continue = TCL_CONTINUE;

inline constexpr eval_flag_t eval_direct = TCL_EVAL_DIRECT;

inline constexpr var_flag_t var_none = 0;
inline constexpr var_flag_t var_global_only = TCL_GLOBAL_ONLY;
inline constexpr var_flag_t var_namespace_only = TCL_NAMESPACE_ONLY;
inline constexpr var_flag_t var_leave_error_msg = TCL_LEAVE_ERR_MSG;
inline constexpr var_flag_t var_append_value = TCL_APPEND_VALUE;
inline constexpr var_flag_t var_list_element = TCL_LIST_ELEMENT;

/**
 * @brief Additional Tcl/Tk error detail.
 */
struct error_detail {
    result_code_t result_code;
};

class interpreter;
class obj;

/**
 * @brief Returns the native Tcl interpreter handle.
 *
 * This is an escape hatch for direct Tcl/Tk C API use. Ordinary code should
 * prefer the functions in `xer::tk`.
 *
 * The returned handle follows the same thread-affinity rules as `interpreter`.
 */
[[nodiscard]] inline auto to_native_handle(interpreter& interp) noexcept -> Tcl_Interp*;

/**
 * @brief Returns the native Tcl object handle as a borrowed pointer.
 *
 * This is an escape hatch for direct Tcl C API use. The returned pointer is
 * borrowed from the `obj` object and remains valid only while that `obj` keeps
 * its owning reference. This function does not change the Tcl reference count.
 */
[[nodiscard]] inline auto to_native_handle(obj& value) noexcept -> Tcl_Obj*;

namespace detail {

[[nodiscard]] inline auto retain_obj(Tcl_Obj* value) noexcept -> obj;

} // namespace detail

/**
 * @brief RAII wrapper for `Tcl_Obj*`.
 *
 * `obj` owns one Tcl reference to the wrapped object. Construction and copying
 * increment the Tcl reference count. Destruction decrements it. Moving transfers
 * the owned reference without changing the Tcl reference count.
 *
 * The class has no public `Tcl_Obj*` constructor. Use factory functions such as
 * `make_string_obj`, `make_int_obj`, and `make_list_obj` to create objects.
 */
class obj {
public:
    obj() noexcept = default;

    obj(const obj& other) noexcept
        : obj_(other.obj_)
    {
        if (obj_ != nullptr) {
            Tcl_IncrRefCount(obj_);
        }
    }

    auto operator=(const obj& other) noexcept -> obj&
    {
        if (this == &other) {
            return *this;
        }

        if (other.obj_ != nullptr) {
            Tcl_IncrRefCount(other.obj_);
        }

        if (obj_ != nullptr) {
            Tcl_DecrRefCount(obj_);
        }

        obj_ = other.obj_;
        return *this;
    }

    obj(obj&& other) noexcept
        : obj_(other.obj_)
    {
        other.obj_ = nullptr;
    }

    auto operator=(obj&& other) noexcept -> obj&
    {
        if (this == &other) {
            return *this;
        }

        if (obj_ != nullptr) {
            Tcl_DecrRefCount(obj_);
        }

        obj_ = other.obj_;
        other.obj_ = nullptr;
        return *this;
    }

    ~obj()
    {
        if (obj_ != nullptr) {
            Tcl_DecrRefCount(obj_);
        }
    }

    [[nodiscard]] auto valid() const noexcept -> bool
    {
        return obj_ != nullptr;
    }

private:
    explicit obj(Tcl_Obj* value) noexcept
        : obj_(value)
    {
        if (obj_ != nullptr) {
            Tcl_IncrRefCount(obj_);
        }
    }

    Tcl_Obj* obj_ = nullptr;

    friend auto detail::retain_obj(Tcl_Obj* value) noexcept -> obj;
    friend auto to_native_handle(obj& value) noexcept -> Tcl_Obj*;
};

[[nodiscard]] inline auto to_native_handle(obj& value) noexcept -> Tcl_Obj*
{
    return value.obj_;
}

namespace detail {

[[nodiscard]] inline auto make_error(
    error_t code,
    result_code_t result_code = result_error) -> error<error_detail>
{
    return xer::make_error<error_detail>(code, error_detail{result_code});
}

[[nodiscard]] inline auto make_unexpected(
    error_t code,
    result_code_t result_code = result_error)
    -> std::unexpected<error<error_detail>>
{
    return std::unexpected(make_error(code, result_code));
}

[[nodiscard]] inline auto string_length_to_tcl_int(std::size_t size)
    -> result<int, error_detail>
{
    if (size > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        return make_unexpected(error_t::out_of_range);
    }

    return static_cast<int>(size);
}

[[nodiscard]] inline auto contains_null(std::u8string_view value) noexcept -> bool
{
    return value.find(char8_t{}) != std::u8string_view::npos;
}

[[nodiscard]] inline auto new_string_obj(std::u8string_view value)
    -> result<Tcl_Obj*, error_detail>
{
    const auto length = string_length_to_tcl_int(value.size());
    if (!length.has_value()) {
        return std::unexpected(length.error());
    }

    return Tcl_NewStringObj(
        reinterpret_cast<const char*>(value.data()),
        *length);
}

[[nodiscard]] inline auto retain_obj(Tcl_Obj* value) noexcept -> obj
{
    return obj(value);
}

[[nodiscard]] inline auto obj_to_u8string(Tcl_Obj* obj) -> std::u8string
{
    if (obj == nullptr) {
        return std::u8string();
    }

    int length = 0;
    const char* const text = Tcl_GetStringFromObj(obj, &length);
    if (text == nullptr || length <= 0) {
        return std::u8string();
    }

    return std::u8string(
        reinterpret_cast<const char8_t*>(text),
        static_cast<std::size_t>(length));
}

[[nodiscard]] inline auto make_command_name(std::u8string_view name)
    -> result<std::string, error_detail>
{
    if (contains_null(name)) {
        return make_unexpected(error_t::invalid_argument);
    }

    return std::string(
        reinterpret_cast<const char*>(name.data()),
        name.size());
}

template<class T>
struct dependent_false : std::false_type {};

template<class T>
struct function_traits;

template<class R, class... Args>
struct function_traits<R (*)(Args...)> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
};

template<class R, class... Args>
struct function_traits<R (&)(Args...)> : function_traits<R (*)(Args...)> {};

template<class R, class... Args>
struct function_traits<R(Args...)> : function_traits<R (*)(Args...)> {};

template<class C, class R, class... Args>
struct function_traits<R (C::*)(Args...)> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
};

template<class C, class R, class... Args>
struct function_traits<R (C::*)(Args...) const> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
};

template<class C, class R, class... Args>
struct function_traits<R (C::*)(Args...) noexcept>
    : function_traits<R (C::*)(Args...)> {};

template<class C, class R, class... Args>
struct function_traits<R (C::*)(Args...) const noexcept>
    : function_traits<R (C::*)(Args...) const> {};

template<class F>
struct function_traits : function_traits<decltype(&F::operator())> {};

template<class T>
struct expected_traits {
    static constexpr bool value = false;
};

template<class T, class Detail>
struct expected_traits<std::expected<T, error<Detail>>> {
    static constexpr bool value = true;
    using value_type = T;
    using detail_type = Detail;
};

template<class T>
inline constexpr bool expected_v = expected_traits<std::remove_cvref_t<T>>::value;

[[nodiscard]] inline auto tcl_error() -> error<error_detail>
{
    return make_error(error_t::runtime_error, result_error);
}

inline auto set_raw_result(Tcl_Interp* interp, Tcl_Obj* obj) noexcept -> void
{
    Tcl_SetObjResult(interp, obj);
}

inline auto set_string_result(Tcl_Interp* interp, std::u8string_view value)
    -> result<void, error_detail>
{
    const auto obj = new_string_obj(value);
    if (!obj.has_value()) {
        return std::unexpected(obj.error());
    }

    set_raw_result(interp, *obj);
    return {};
}

/**
 * @brief Converts one Tcl command argument to a C++ callback argument.
 *
 * A callback argument of type `std::u8string_view` is a borrowed view into
 * the string representation owned by the corresponding `Tcl_Obj`. The view is
 * valid only while the command callback is running. If the value must be
 * stored, returned later, or otherwise used after the callback returns, the
 * callback must copy it into an owning object such as `std::u8string`.
 */
template<class T>
[[nodiscard]] auto get_argument(Tcl_Interp* interp, Tcl_Obj* obj)
    -> result<std::remove_cvref_t<T>, error_detail>
{
    using U = std::remove_cvref_t<T>;

    if constexpr (std::same_as<U, Tcl_Obj*>) {
        return obj;
    } else if constexpr (std::same_as<U, xer::tk::obj>) {
        return retain_obj(obj);
    } else if constexpr (std::same_as<U, std::u8string>) {
        return obj_to_u8string(obj);
    } else if constexpr (std::same_as<U, std::u8string_view>) {
        int length = 0;
        const char* const text = Tcl_GetStringFromObj(obj, &length);
        if (text == nullptr) {
            return std::unexpected(tcl_error());
        }

        return std::u8string_view(
            reinterpret_cast<const char8_t*>(text),
            static_cast<std::size_t>(length));
    } else if constexpr (std::same_as<U, bool>) {
        int value = 0;
        if (Tcl_GetBooleanFromObj(interp, obj, &value) != TCL_OK) {
            return std::unexpected(tcl_error());
        }

        return value != 0;
    } else if constexpr (std::integral<U> && std::signed_integral<U>) {
        Tcl_WideInt value = 0;
        if (Tcl_GetWideIntFromObj(interp, obj, &value) != TCL_OK) {
            return std::unexpected(tcl_error());
        }

        if (value < static_cast<Tcl_WideInt>(std::numeric_limits<U>::min()) ||
            value > static_cast<Tcl_WideInt>(std::numeric_limits<U>::max())) {
            return make_unexpected(error_t::out_of_range);
        }

        return static_cast<U>(value);
    } else if constexpr (std::integral<U> && std::unsigned_integral<U>) {
        Tcl_WideInt value = 0;
        if (Tcl_GetWideIntFromObj(interp, obj, &value) != TCL_OK) {
            return std::unexpected(tcl_error());
        }

        if (value < 0 ||
            static_cast<unsigned long long>(value) >
                static_cast<unsigned long long>(std::numeric_limits<U>::max())) {
            return make_unexpected(error_t::out_of_range);
        }

        return static_cast<U>(value);
    } else if constexpr (std::floating_point<U>) {
        double value = 0.0;
        if (Tcl_GetDoubleFromObj(interp, obj, &value) != TCL_OK) {
            return std::unexpected(tcl_error());
        }

        return static_cast<U>(value);
    } else {
        static_assert(dependent_false<U>::value, "unsupported Tcl command argument type");
    }
}

template<class T>
[[nodiscard]] auto set_result_from_value(Tcl_Interp* interp, T&& value)
    -> result<void, error_detail>
{
    using U = std::remove_cvref_t<T>;

    if constexpr (std::same_as<U, Tcl_Obj*>) {
        set_raw_result(interp, value);
        return {};
    } else if constexpr (std::same_as<U, obj>) {
        set_raw_result(interp, to_native_handle(value));
        return {};
    } else if constexpr (std::same_as<U, std::u8string>) {
        return set_string_result(interp, std::u8string_view(value));
    } else if constexpr (std::same_as<U, std::u8string_view>) {
        return set_string_result(interp, value);
    } else if constexpr (std::same_as<U, const char8_t*> || std::same_as<U, char8_t*>) {
        return set_string_result(interp, std::u8string_view(value));
    } else if constexpr (std::same_as<U, bool>) {
        set_raw_result(interp, Tcl_NewBooleanObj(value ? 1 : 0));
        return {};
    } else if constexpr (std::integral<U> && std::signed_integral<U>) {
        set_raw_result(interp, Tcl_NewWideIntObj(static_cast<Tcl_WideInt>(value)));
        return {};
    } else if constexpr (std::integral<U> && std::unsigned_integral<U>) {
        if (value > static_cast<U>(std::numeric_limits<Tcl_WideInt>::max())) {
            return make_unexpected(error_t::out_of_range);
        }

        set_raw_result(interp, Tcl_NewWideIntObj(static_cast<Tcl_WideInt>(value)));
        return {};
    } else if constexpr (std::floating_point<U>) {
        set_raw_result(interp, Tcl_NewDoubleObj(static_cast<double>(value)));
        return {};
    } else {
        static_assert(dependent_false<U>::value, "unsupported Tcl command return type");
    }
}

inline auto propagate_command_error(Tcl_Interp* interp, const error<void>& error)
    -> int
{
    static_cast<void>(error);
    static_cast<void>(set_string_result(interp, u8"XER Tcl/Tk command failed"));
    return TCL_ERROR;
}

inline auto propagate_command_error(Tcl_Interp* interp, const error<error_detail>& error)
    -> int
{
    static_cast<void>(set_string_result(interp, u8"XER Tcl/Tk command failed"));
    return error.result_code;
}

template<class T>
inline auto set_result_from_callable_result(Tcl_Interp* interp, T&& value) -> int
{
    using U = std::remove_cvref_t<T>;

    if constexpr (std::same_as<U, void>) {
        Tcl_ResetResult(interp);
        return TCL_OK;
    } else if constexpr (expected_v<U>) {
        if (!value.has_value()) {
            return propagate_command_error(interp, value.error());
        }

        if constexpr (std::same_as<typename expected_traits<U>::value_type, void>) {
            Tcl_ResetResult(interp);
            return TCL_OK;
        } else {
            const auto set_result = set_result_from_value(interp, *value);
            if (!set_result.has_value()) {
                return propagate_command_error(interp, set_result.error());
            }
            return TCL_OK;
        }
    } else {
        const auto set_result = set_result_from_value(interp, std::forward<T>(value));
        if (!set_result.has_value()) {
            return propagate_command_error(interp, set_result.error());
        }
        return TCL_OK;
    }
}

template<class F, class Tuple, std::size_t... I>
inline auto invoke_command_with_tuple(
    F& callable,
    Tcl_Interp* interp,
    int objc,
    Tcl_Obj* const objv[],
    std::index_sequence<I...>) -> int
{
    if (objc != static_cast<int>(sizeof...(I) + 1)) {
        static_cast<void>(set_string_result(interp, u8"wrong # args"));
        return TCL_ERROR;
    }

    auto args = std::tuple<result<std::remove_cvref_t<std::tuple_element_t<I, Tuple>>, error_detail>...>(
        get_argument<std::tuple_element_t<I, Tuple>>(interp, objv[I + 1])...);

    const auto ok = (... && std::get<I>(args).has_value());
    if (!ok) {
        static_cast<void>(set_string_result(interp, u8"invalid argument"));
        return TCL_ERROR;
    }

    using return_type = typename function_traits<F>::result_type;

    if constexpr (std::same_as<return_type, void>) {
        std::invoke(callable, (*std::get<I>(args))...);
        Tcl_ResetResult(interp);
        return TCL_OK;
    } else {
        return set_result_from_callable_result(
            interp,
            std::invoke(callable, (*std::get<I>(args))...));
    }
}

template<class F>
inline auto command_proc(
    ClientData client_data,
    Tcl_Interp* interp,
    int objc,
    Tcl_Obj* const objv[]) -> int
{
    auto* const callable = static_cast<F*>(client_data);
    using traits = function_traits<F>;
    using args_tuple = typename traits::args_tuple;
    constexpr std::size_t arg_count = std::tuple_size_v<args_tuple>;

    return invoke_command_with_tuple<F, args_tuple>(
        *callable,
        interp,
        objc,
        objv,
        std::make_index_sequence<arg_count>());
}

template<class F>
inline auto command_delete_proc(ClientData client_data) -> void
{
    delete static_cast<F*>(client_data);
}

} // namespace detail

/**
 * @brief Move-only RAII wrapper for `Tcl_Interp*`.
 *
 * An interpreter is thread-affine. It should be created, initialized, used,
 * and destroyed on the same thread.
 *
 * XER does not require an interpreter to live on the program's main thread,
 * but it does not make one interpreter safely callable from arbitrary threads.
 */
class interpreter {
public:
    interpreter() noexcept = default;

    interpreter(const interpreter&) = delete;
    auto operator=(const interpreter&) -> interpreter& = delete;

    interpreter(interpreter&& other) noexcept
        : interp_(other.interp_)
    {
        other.interp_ = nullptr;
    }

    auto operator=(interpreter&& other) noexcept -> interpreter&
    {
        if (this == &other) {
            return *this;
        }

        if (interp_ != nullptr) {
            Tcl_DeleteInterp(interp_);
        }

        interp_ = other.interp_;
        other.interp_ = nullptr;
        return *this;
    }

    ~interpreter()
    {
        if (interp_ != nullptr) {
            Tcl_DeleteInterp(interp_);
        }
    }

    [[nodiscard]] static auto create() -> result<interpreter, error_detail>;

    [[nodiscard]] auto valid() const noexcept -> bool
    {
        return interp_ != nullptr;
    }

private:
    explicit interpreter(Tcl_Interp* interp) noexcept
        : interp_(interp)
    {
    }

    Tcl_Interp* interp_ = nullptr;

    friend auto to_native_handle(interpreter& interp) noexcept -> Tcl_Interp*;
};

[[nodiscard]] inline auto to_native_handle(interpreter& interp) noexcept -> Tcl_Interp*
{
    return interp.interp_;
}


/**
 * @brief Creates a Tcl string object from UTF-8 text.
 */
[[nodiscard]] inline auto make_string_obj(std::u8string_view value)
    -> result<obj, error_detail>
{
    const auto raw = detail::new_string_obj(value);
    if (!raw.has_value()) {
        return std::unexpected(raw.error());
    }

    return detail::retain_obj(*raw);
}

/**
 * @brief Creates a Tcl integer object.
 */
[[nodiscard]] inline auto make_int_obj(int value) -> obj
{
    return detail::retain_obj(Tcl_NewIntObj(value));
}

/**
 * @brief Creates a Tcl list object from UTF-8 text elements.
 */
[[nodiscard]] inline auto make_list_obj(std::span<const std::u8string_view> values)
    -> result<obj, error_detail>
{
    auto list = detail::retain_obj(Tcl_NewListObj(0, nullptr));

    for (const std::u8string_view value : values) {
        auto element = make_string_obj(value);
        if (!element.has_value()) {
            return std::unexpected(element.error());
        }

        if (Tcl_ListObjAppendElement(
                nullptr,
                to_native_handle(list),
                to_native_handle(*element)) != TCL_OK) {
            return std::unexpected(detail::make_error(error_t::runtime_error));
        }
    }

    return list;
}

/**
 * @brief Creates a Tcl list object from UTF-8 text elements.
 */
[[nodiscard]] inline auto make_list_obj(std::initializer_list<std::u8string_view> values)
    -> result<obj, error_detail>
{
    return make_list_obj(std::span<const std::u8string_view>(values.begin(), values.size()));
}

/**
 * @brief Initializes Tcl executable information from the current command line.
 */
[[nodiscard]] inline auto find_executable() -> result<void, error_detail>
{
    static std::mutex mutex;
    static bool initialized = false;

    std::lock_guard lock(mutex);

    if (initialized) {
        return {};
    }

    const auto line = get_cmdline();
    if (!line.has_value()) {
        return std::unexpected(detail::make_error(line.error().code));
    }

    const auto first = line->at(0);
    if (!first.has_value()) {
        return std::unexpected(detail::make_error(first.error().code));
    }

    const std::string executable(
        reinterpret_cast<const char*>(first->data()),
        first->size());

    Tcl_FindExecutable(executable.c_str());
    initialized = true;
    return {};
}

[[nodiscard]] inline auto interpreter::create() -> result<interpreter, error_detail>
{
    const auto executable = find_executable();
    if (!executable.has_value()) {
        return std::unexpected(executable.error());
    }

    Tcl_Interp* const raw = Tcl_CreateInterp();
    if (raw == nullptr) {
        return std::unexpected(detail::make_error(error_t::runtime_error));
    }

    return interpreter(raw);
}

/**
 * @brief Initializes Tcl and Tk for an interpreter.
 *
 * This function is defined in `xer/bits/tk.h` because it also calls `Tk_Init`.
 */
[[nodiscard]] inline auto init(interpreter& interp) -> result<void, error_detail>;

/**
 * @brief Runs the Tk main event loop.
 *
 * This function is defined in `xer/bits/tk.h`.
 */
inline auto main_loop() -> void;

/**
 * @brief Gets the current Tcl interpreter result as UTF-8 text.
 */
[[nodiscard]] inline auto get_result(interpreter& interp) -> std::u8string
{
    Tcl_Interp* const raw = to_native_handle(interp);
    if (raw == nullptr) {
        return std::u8string();
    }

    return detail::obj_to_u8string(Tcl_GetObjResult(raw));
}

/**
 * @brief Resets the current Tcl interpreter result.
 */
inline auto reset_result(interpreter& interp) noexcept -> void
{
    Tcl_Interp* const raw = to_native_handle(interp);
    if (raw != nullptr) {
        Tcl_ResetResult(raw);
    }
}

/**
 * @brief Evaluates Tcl script text using `Tcl_EvalObjEx`.
 */
[[nodiscard]] inline auto eval(
    interpreter& interp,
    std::u8string_view script,
    eval_flag_t flags = eval_direct) -> result<std::u8string, error_detail>
{
    auto script_obj = make_string_obj(script);
    if (!script_obj.has_value()) {
        return std::unexpected(script_obj.error());
    }

    Tcl_Interp* const raw = to_native_handle(interp);
    if (raw == nullptr) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    const int code = Tcl_EvalObjEx(raw, to_native_handle(*script_obj), flags);
    const auto text = get_result(interp);

    if (code != TCL_OK) {
        return std::unexpected(detail::make_error(error_t::runtime_error, code));
    }

    return text;
}

/**
 * @brief Sets a Tcl variable using `Tcl_ObjSetVar2`.
 */
[[nodiscard]] inline auto set_var(
    interpreter& interp,
    std::u8string_view name,
    std::u8string_view value,
    var_flag_t flags = var_global_only) -> result<void, error_detail>
{
    auto name_obj = make_string_obj(name);
    if (!name_obj.has_value()) {
        return std::unexpected(name_obj.error());
    }

    auto value_obj = make_string_obj(value);
    if (!value_obj.has_value()) {
        return std::unexpected(value_obj.error());
    }

    Tcl_Interp* const raw = to_native_handle(interp);
    if (raw == nullptr) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    Tcl_Obj* const result = Tcl_ObjSetVar2(
        raw,
        to_native_handle(*name_obj),
        nullptr,
        to_native_handle(*value_obj),
        flags);

    if (result == nullptr) {
        return std::unexpected(detail::make_error(error_t::runtime_error));
    }

    return {};
}

/**
 * @brief Sets a Tcl array variable element using `Tcl_ObjSetVar2`.
 */
[[nodiscard]] inline auto set_var(
    interpreter& interp,
    std::u8string_view name1,
    std::u8string_view name2,
    std::u8string_view value,
    var_flag_t flags = var_global_only) -> result<void, error_detail>
{
    auto name1_obj = make_string_obj(name1);
    if (!name1_obj.has_value()) {
        return std::unexpected(name1_obj.error());
    }

    auto name2_obj = make_string_obj(name2);
    if (!name2_obj.has_value()) {
        return std::unexpected(name2_obj.error());
    }

    auto value_obj = make_string_obj(value);
    if (!value_obj.has_value()) {
        return std::unexpected(value_obj.error());
    }

    Tcl_Interp* const raw = to_native_handle(interp);
    if (raw == nullptr) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    Tcl_Obj* const result = Tcl_ObjSetVar2(
        raw,
        to_native_handle(*name1_obj),
        to_native_handle(*name2_obj),
        to_native_handle(*value_obj),
        flags);

    if (result == nullptr) {
        return std::unexpected(detail::make_error(error_t::runtime_error));
    }

    return {};
}


/**
 * @brief Sets a Tcl variable to an existing Tcl object.
 */
[[nodiscard]] inline auto set_var(
    interpreter& interp,
    std::u8string_view name,
    obj& value,
    var_flag_t flags = var_global_only) -> result<void, error_detail>
{
    auto name_obj = make_string_obj(name);
    if (!name_obj.has_value()) {
        return std::unexpected(name_obj.error());
    }

    Tcl_Interp* const raw = to_native_handle(interp);
    if (raw == nullptr) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    Tcl_Obj* const result = Tcl_ObjSetVar2(
        raw,
        to_native_handle(*name_obj),
        nullptr,
        to_native_handle(value),
        flags);

    if (result == nullptr) {
        return std::unexpected(detail::make_error(error_t::runtime_error));
    }

    return {};
}

/**
 * @brief Sets a Tcl array variable element to an existing Tcl object.
 */
[[nodiscard]] inline auto set_var(
    interpreter& interp,
    std::u8string_view name1,
    std::u8string_view name2,
    obj& value,
    var_flag_t flags = var_global_only) -> result<void, error_detail>
{
    auto name1_obj = make_string_obj(name1);
    if (!name1_obj.has_value()) {
        return std::unexpected(name1_obj.error());
    }

    auto name2_obj = make_string_obj(name2);
    if (!name2_obj.has_value()) {
        return std::unexpected(name2_obj.error());
    }

    Tcl_Interp* const raw = to_native_handle(interp);
    if (raw == nullptr) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    Tcl_Obj* const result = Tcl_ObjSetVar2(
        raw,
        to_native_handle(*name1_obj),
        to_native_handle(*name2_obj),
        to_native_handle(value),
        flags);

    if (result == nullptr) {
        return std::unexpected(detail::make_error(error_t::runtime_error));
    }

    return {};
}

/**
 * @brief Gets a Tcl variable using `Tcl_ObjGetVar2`.
 */
[[nodiscard]] inline auto get_var(
    interpreter& interp,
    std::u8string_view name,
    var_flag_t flags = var_global_only) -> result<std::u8string, error_detail>
{
    auto name_obj = make_string_obj(name);
    if (!name_obj.has_value()) {
        return std::unexpected(name_obj.error());
    }

    Tcl_Interp* const raw = to_native_handle(interp);
    if (raw == nullptr) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    Tcl_Obj* const result = Tcl_ObjGetVar2(
        raw,
        to_native_handle(*name_obj),
        nullptr,
        flags);

    if (result == nullptr) {
        return std::unexpected(detail::make_error(error_t::runtime_error));
    }

    return detail::obj_to_u8string(result);
}

/**
 * @brief Gets a Tcl array variable element using `Tcl_ObjGetVar2`.
 */
[[nodiscard]] inline auto get_var(
    interpreter& interp,
    std::u8string_view name1,
    std::u8string_view name2,
    var_flag_t flags = var_global_only) -> result<std::u8string, error_detail>
{
    auto name1_obj = make_string_obj(name1);
    if (!name1_obj.has_value()) {
        return std::unexpected(name1_obj.error());
    }

    auto name2_obj = make_string_obj(name2);
    if (!name2_obj.has_value()) {
        return std::unexpected(name2_obj.error());
    }

    Tcl_Interp* const raw = to_native_handle(interp);
    if (raw == nullptr) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    Tcl_Obj* const result = Tcl_ObjGetVar2(
        raw,
        to_native_handle(*name1_obj),
        to_native_handle(*name2_obj),
        flags);

    if (result == nullptr) {
        return std::unexpected(detail::make_error(error_t::runtime_error));
    }

    return detail::obj_to_u8string(result);
}

/**
 * @brief Registers a C++ callable as a Tcl object command.
 */
template<class F>
[[nodiscard]] auto create_command(
    interpreter& interp,
    std::u8string_view name,
    F&& callable) -> result<void, error_detail>
{
    Tcl_Interp* const raw = to_native_handle(interp);
    if (raw == nullptr) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    const auto command_name = detail::make_command_name(name);
    if (!command_name.has_value()) {
        return std::unexpected(command_name.error());
    }

    using callable_type = std::decay_t<F>;
    auto* const stored = new callable_type(std::forward<F>(callable));

    Tcl_Command const command = Tcl_CreateObjCommand(
        raw,
        command_name->c_str(),
        detail::command_proc<callable_type>,
        static_cast<ClientData>(stored),
        detail::command_delete_proc<callable_type>);

    if (command == nullptr) {
        delete stored;
        return std::unexpected(detail::make_error(error_t::runtime_error));
    }

    return {};
}


namespace detail {

[[nodiscard]] inline auto make_plain_unexpected(error_t code)
    -> std::unexpected<error<void>>
{
    return std::unexpected(xer::make_error(code));
}

[[nodiscard]] inline auto make_plain_unexpected(const error<error_detail>& value)
    -> std::unexpected<error<void>>
{
    return std::unexpected(xer::make_error(value.code));
}

[[nodiscard]] inline auto set_argc(interpreter& interp, std::size_t argc)
    -> result<void, error_detail>
{
    if (argc > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        return std::unexpected(detail::make_error(error_t::out_of_range));
    }

    auto value = make_int_obj(static_cast<int>(argc));
    return set_var(interp, u8"argc", value);
}

[[nodiscard]] inline auto set_argv(interpreter& interp, std::span<const std::u8string> args)
    -> result<void, error_detail>
{
    std::vector<std::u8string_view> views;
    views.reserve(args.size());

    for (const auto& arg : args) {
        views.push_back(std::u8string_view(arg));
    }

    auto list = make_list_obj(std::span<const std::u8string_view>(views.data(), views.size()));
    if (!list.has_value()) {
        return std::unexpected(list.error());
    }

    return set_var(interp, u8"argv", *list);
}

[[nodiscard]] inline auto set_argv0(interpreter& interp, const cmdline& line)
    -> result<void, error_detail>
{
    const auto value = line.at(0);
    if (!value.has_value()) {
        return std::unexpected(detail::make_error(value.error().code));
    }

    return set_var(interp, u8"argv0", *value);
}

[[nodiscard]] inline auto set_environment_variables(interpreter& interp)
    -> result<void, error_detail>
{
    const auto environment = xer::get_environs();
    if (!environment.has_value()) {
        return std::unexpected(detail::make_error(environment.error().code));
    }

    for (const auto& entry : environment->entries()) {
        const auto set = set_var(
            interp,
            u8"env",
            std::u8string_view(entry.name),
            std::u8string_view(entry.value));
        if (!set.has_value()) {
            return std::unexpected(set.error());
        }
    }

    return {};
}

[[nodiscard]] inline auto set_main_variables(interpreter& interp) -> result<void, error_detail>
{
    const auto line = get_cmdline();
    if (!line.has_value()) {
        return std::unexpected(detail::make_error(line.error().code));
    }

    if (line->empty()) {
        return std::unexpected(detail::make_error(error_t::runtime_error));
    }

    const auto all_args = line->args();
    const auto script_args = all_args.subspan(1);

    const auto argc = set_argc(interp, script_args.size());
    if (!argc.has_value()) {
        return std::unexpected(argc.error());
    }

    const auto argv = set_argv(interp, script_args);
    if (!argv.has_value()) {
        return std::unexpected(argv.error());
    }

    const auto argv0 = set_argv0(interp, *line);
    if (!argv0.has_value()) {
        return std::unexpected(argv0.error());
    }

    const auto env = set_environment_variables(interp);
    if (!env.has_value()) {
        return std::unexpected(env.error());
    }

    return {};
}

} // namespace detail

/**
 * @brief Runs a Tcl/Tk application block without using `Tk_Main`.
 *
 * The callback is invoked after executable discovery, interpreter creation,
 * Tcl/Tk initialization, and setup of Tcl startup variables. If the callback
 * succeeds, `main_loop()` is entered. The interpreter is created, used, and
 * destroyed on the thread that calls this function.
 */
template<class F>
[[nodiscard]] auto main(F&& callback) -> result<void>
{
    using callback_result = std::invoke_result_t<F, interpreter&>;
    static_assert(
        std::same_as<std::remove_cvref_t<callback_result>, result<void>>,
        "xer::tk::main callback must return xer::result<void>");

    const auto executable = find_executable();
    if (!executable.has_value()) {
        return detail::make_plain_unexpected(executable.error());
    }

    auto interp = interpreter::create();
    if (!interp.has_value()) {
        return detail::make_plain_unexpected(interp.error());
    }

    const auto initialized = init(*interp);
    if (!initialized.has_value()) {
        return detail::make_plain_unexpected(initialized.error());
    }

    const auto variables = detail::set_main_variables(*interp);
    if (!variables.has_value()) {
        return detail::make_plain_unexpected(variables.error());
    }

    try {
        auto prepared = std::invoke(std::forward<F>(callback), *interp);
        if (!prepared.has_value()) {
            return std::unexpected(prepared.error());
        }
    } catch (...) {
        return std::unexpected(xer::make_error(error_t::runtime_error));
    }

    main_loop();
    return {};
}

} // namespace xer::tk

#endif /* XER_BITS_TCL_H_INCLUDED_ */
