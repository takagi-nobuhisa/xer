/**
 * @file tests/test_tcl.cpp
 * @brief Tests for Tcl/Tk integration helpers.
 */

// XER_TEST_FEATURES: tcltk

#include <string>
#include <string_view>

#include <xer/assert.h>
#include <xer/error.h>
#include <xer/tk.h>

namespace {

[[nodiscard]] auto make_interpreter() -> xer::tk::interpreter
{
    auto interp = xer::tk::interpreter::create();
    xer_assert(interp.has_value());
    xer_assert(interp->valid());
    return std::move(*interp);
}

void test_tk_result_code_constants()
{
    xer_assert_eq(xer::tk::result_ok, TCL_OK);
    xer_assert_eq(xer::tk::result_error, TCL_ERROR);
    xer_assert_eq(xer::tk::result_break, TCL_BREAK);
    xer_assert_eq(xer::tk::var_global_only, TCL_GLOBAL_ONLY);
    xer_assert_eq(xer::tk::eval_direct, TCL_EVAL_DIRECT);
}

void test_tk_eval_success()
{
    auto interp = make_interpreter();

    const auto result = xer::tk::eval(interp, u8"expr {1 + 2}");

    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"3"));
}

void test_tk_eval_error_code()
{
    auto interp = make_interpreter();

    const auto result = xer::tk::eval(interp, u8"break");

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().result_code, xer::tk::result_error);
}

void test_tk_eval_custom_control_code()
{
    auto interp = make_interpreter();

    const auto created = xer::tk::create_command(
        interp,
        u8"xer_break",
        []() -> xer::result<void, xer::tk::error_detail> {
            return std::unexpected(xer::make_error<xer::tk::error_detail>(
                xer::error_t::runtime_error,
                xer::tk::error_detail{xer::tk::result_break}));
        });
    xer_assert(created.has_value());

    const auto result = xer::tk::eval(
        interp,
        u8"set count 0\n"
        u8"foreach x {1 2 3} {\n"
        u8"    incr count\n"
        u8"    xer_break\n"
        u8"    set count 99\n"
        u8"}\n"
        u8"set count");

    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"1"));
}

void test_tk_result_helpers()
{
    auto interp = make_interpreter();

    const auto result = xer::tk::eval(interp, u8"set message hello");
    xer_assert(result.has_value());
    xer_assert_eq(xer::tk::get_result(interp), std::u8string(u8"hello"));

    xer::tk::reset_result(interp);
    xer_assert_eq(xer::tk::get_result(interp), std::u8string(u8""));
}

void test_tk_set_get_var()
{
    auto interp = make_interpreter();

    const auto set = xer::tk::set_var(interp, u8"name", u8"xer");
    xer_assert(set.has_value());

    const auto get = xer::tk::get_var(interp, u8"name");
    xer_assert(get.has_value());
    xer_assert_eq(*get, std::u8string(u8"xer"));
}

void test_tk_set_get_array_var()
{
    auto interp = make_interpreter();

    const auto set = xer::tk::set_var(interp, u8"config", u8"title", u8"XER");
    xer_assert(set.has_value());

    const auto get = xer::tk::get_var(interp, u8"config", u8"title");
    xer_assert(get.has_value());
    xer_assert_eq(*get, std::u8string(u8"XER"));
}

void test_tk_to_native_handle()
{
    auto interp = make_interpreter();

    Tcl_Interp* const raw = xer::tk::to_native_handle(interp);
    xer_assert(raw != nullptr);
}

void test_tk_create_command_integer_result()
{
    auto interp = make_interpreter();

    const auto created = xer::tk::create_command(
        interp,
        u8"xer_add",
        [](int a, int b) -> int {
            return a + b;
        });
    xer_assert(created.has_value());

    const auto result = xer::tk::eval(interp, u8"xer_add 10 20");
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"30"));
}

void test_tk_create_command_string_result()
{
    auto interp = make_interpreter();

    const auto created = xer::tk::create_command(
        interp,
        u8"xer_echo",
        [](std::u8string value) -> std::u8string {
            return value;
        });
    xer_assert(created.has_value());

    const auto result = xer::tk::eval(interp, u8"xer_echo hello");
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"hello"));
}

void test_tk_create_command_void_result()
{
    auto interp = make_interpreter();

    const auto created = xer::tk::create_command(
        interp,
        u8"xer_store",
        [&interp](std::u8string value) -> void {
            static_cast<void>(xer::tk::set_var(interp, u8"stored", value));
        });
    xer_assert(created.has_value());

    const auto result = xer::tk::eval(interp, u8"xer_store value");
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8""));

    const auto stored = xer::tk::get_var(interp, u8"stored");
    xer_assert(stored.has_value());
    xer_assert_eq(*stored, std::u8string(u8"value"));
}

void test_tk_create_command_result_return()
{
    auto interp = make_interpreter();

    const auto created = xer::tk::create_command(
        interp,
        u8"xer_checked",
        [](int value) -> xer::result<int, xer::tk::error_detail> {
            if (value < 0) {
                return std::unexpected(xer::make_error<xer::tk::error_detail>(
                    xer::error_t::invalid_argument,
                    xer::tk::error_detail{xer::tk::result_error}));
            }

            return value * 2;
        });
    xer_assert(created.has_value());

    const auto ok = xer::tk::eval(interp, u8"xer_checked 21");
    xer_assert(ok.has_value());
    xer_assert_eq(*ok, std::u8string(u8"42"));

    const auto failed = xer::tk::eval(interp, u8"xer_checked -1");
    xer_assert_not(failed.has_value());
    xer_assert_eq(failed.error().result_code, xer::tk::result_error);
}

} // namespace

auto main() -> int
{
    test_tk_result_code_constants();
    test_tk_eval_success();
    test_tk_eval_error_code();
    test_tk_eval_custom_control_code();
    test_tk_result_helpers();
    test_tk_set_get_var();
    test_tk_set_get_array_var();
    test_tk_to_native_handle();
    test_tk_create_command_integer_result();
    test_tk_create_command_string_result();
    test_tk_create_command_void_result();
    test_tk_create_command_result_return();

    return 0;
}
