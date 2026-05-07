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
#include <limits>
#include <mutex>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include <tcl.h>

#include <xer/bits/common.h>
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

/**
 * @brief Returns the native Tcl interpreter handle.
 *
 * This is an escape hatch for direct Tcl/Tk C API use. Ordinary code should
 * prefer the functions in `xer::tk`.
 */
[[nodiscard]] inline auto to_native_handle(interpreter& interp) noexcept -> Tcl_Interp*;

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

class obj_ref {
public:
    obj_ref() noexcept = default;

    explicit obj_ref(Tcl_Obj* obj) noexcept
        : obj_(obj)
    {
        if (obj_ != nullptr) {
            Tcl_IncrRefCount(obj_);
        }
    }

    obj_ref(const obj_ref& other) noexcept
        : obj_(other.obj_)
    {
        if (obj_ != nullptr) {
            Tcl_IncrRefCount(obj_);
        }
    }

    auto operator=(const obj_ref& other) noexcept -> obj_ref&
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

    obj_ref(obj_ref&& other) noexcept
        : obj_(other.obj_)
    {
        other.obj_ = nullptr;
    }

    auto operator=(obj_ref&& other) noexcept -> obj_ref&
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

    ~obj_ref()
    {
        if (obj_ != nullptr) {
            Tcl_DecrRefCount(obj_);
        }
    }

    [[nodiscard]] auto get() const noexcept -> Tcl_Obj*
    {
        return obj_;
    }

    [[nodiscard]] auto release() noexcept -> Tcl_Obj*
    {
        Tcl_Obj* const result = obj_;
        obj_ = nullptr;
        return result;
    }

private:
    Tcl_Obj* obj_ = nullptr;
};

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

[[nodiscard]] inline auto make_string_obj(std::u8string_view value)
    -> result<obj_ref, error_detail>
{
    const auto obj = new_string_obj(value);
    if (!obj.has_value()) {
        return std::unexpected(obj.error());
    }

    return obj_ref(*obj);
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

template<class T>
[[nodiscard]] auto get_argument(Tcl_Interp* interp, Tcl_Obj* obj)
    -> result<std::remove_cvref_t<T>, error_detail>
{
    using U = std::remove_cvref_t<T>;

    if constexpr (std::same_as<U, Tcl_Obj*>) {
        return obj;
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
    auto script_obj = detail::make_string_obj(script);
    if (!script_obj.has_value()) {
        return std::unexpected(script_obj.error());
    }

    Tcl_Interp* const raw = to_native_handle(interp);
    if (raw == nullptr) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    const int code = Tcl_EvalObjEx(raw, script_obj->get(), flags);
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
    auto name_obj = detail::make_string_obj(name);
    if (!name_obj.has_value()) {
        return std::unexpected(name_obj.error());
    }

    auto value_obj = detail::make_string_obj(value);
    if (!value_obj.has_value()) {
        return std::unexpected(value_obj.error());
    }

    Tcl_Interp* const raw = to_native_handle(interp);
    if (raw == nullptr) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    Tcl_Obj* const result = Tcl_ObjSetVar2(
        raw,
        name_obj->get(),
        nullptr,
        value_obj->get(),
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
    auto name1_obj = detail::make_string_obj(name1);
    if (!name1_obj.has_value()) {
        return std::unexpected(name1_obj.error());
    }

    auto name2_obj = detail::make_string_obj(name2);
    if (!name2_obj.has_value()) {
        return std::unexpected(name2_obj.error());
    }

    auto value_obj = detail::make_string_obj(value);
    if (!value_obj.has_value()) {
        return std::unexpected(value_obj.error());
    }

    Tcl_Interp* const raw = to_native_handle(interp);
    if (raw == nullptr) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    Tcl_Obj* const result = Tcl_ObjSetVar2(
        raw,
        name1_obj->get(),
        name2_obj->get(),
        value_obj->get(),
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
    auto name_obj = detail::make_string_obj(name);
    if (!name_obj.has_value()) {
        return std::unexpected(name_obj.error());
    }

    Tcl_Interp* const raw = to_native_handle(interp);
    if (raw == nullptr) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    Tcl_Obj* const result = Tcl_ObjGetVar2(
        raw,
        name_obj->get(),
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
    auto name1_obj = detail::make_string_obj(name1);
    if (!name1_obj.has_value()) {
        return std::unexpected(name1_obj.error());
    }

    auto name2_obj = detail::make_string_obj(name2);
    if (!name2_obj.has_value()) {
        return std::unexpected(name2_obj.error());
    }

    Tcl_Interp* const raw = to_native_handle(interp);
    if (raw == nullptr) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    Tcl_Obj* const result = Tcl_ObjGetVar2(
        raw,
        name1_obj->get(),
        name2_obj->get(),
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

} // namespace xer::tk

#endif /* XER_BITS_TCL_H_INCLUDED_ */
