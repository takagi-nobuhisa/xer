/**
 * @file tests/test_tcl.cpp
 * @brief Tests for Tcl/Tk integration helpers.
 */

// XER_TEST_FEATURES: tcltk

#include <exception>
#include <string>
#include <string_view>
#include <thread>

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

void test_tk_obj_string_and_int_var()
{
    auto interp = make_interpreter();

    auto name = xer::tk::make_string_obj(u8"xer");
    xer_assert(name.has_value());

    const auto set_name = xer::tk::set_var(interp, u8"name", *name);
    xer_assert(set_name.has_value());

    const auto get_name = xer::tk::get_var(interp, u8"name");
    xer_assert(get_name.has_value());
    xer_assert_eq(*get_name, std::u8string(u8"xer"));

    auto count = xer::tk::make_int_obj(42);
    const auto set_count = xer::tk::set_var(interp, u8"count", count);
    xer_assert(set_count.has_value());

    const auto get_count = xer::tk::get_var(interp, u8"count");
    xer_assert(get_count.has_value());
    xer_assert_eq(*get_count, std::u8string(u8"42"));
}

void test_tk_obj_list_var()
{
    auto interp = make_interpreter();

    auto list = xer::tk::make_list_obj({u8"first value", u8"second", u8"third"});
    xer_assert(list.has_value());

    const auto set = xer::tk::set_var(interp, u8"argv", *list);
    xer_assert(set.has_value());

    const auto length = xer::tk::eval(interp, u8"llength $argv");
    xer_assert(length.has_value());
    xer_assert_eq(*length, std::u8string(u8"3"));

    const auto first = xer::tk::eval(interp, u8"lindex $argv 0");
    xer_assert(first.has_value());
    xer_assert_eq(*first, std::u8string(u8"first value"));
}

void test_tk_obj_copy_keeps_reference()
{
    auto interp = make_interpreter();

    auto original = xer::tk::make_string_obj(u8"copied");
    xer_assert(original.has_value());

    xer::tk::obj copy = *original;
    original = xer::tk::make_string_obj(u8"other");
    xer_assert(original.has_value());

    const auto set = xer::tk::set_var(interp, u8"value", copy);
    xer_assert(set.has_value());

    const auto get = xer::tk::get_var(interp, u8"value");
    xer_assert(get.has_value());
    xer_assert_eq(*get, std::u8string(u8"copied"));
}

void test_tk_create_command_obj_argument_and_result()
{
    auto interp = make_interpreter();

    const auto created = xer::tk::create_command(
        interp,
        u8"xer_obj_echo",
        [](xer::tk::obj value) -> xer::tk::obj {
            return value;
        });
    xer_assert(created.has_value());

    const auto result = xer::tk::eval(interp, u8"xer_obj_echo hello");
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"hello"));
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


void test_tk_create_command_string_view_argument()
{
    auto interp = make_interpreter();

    std::u8string copied;

    const auto created = xer::tk::create_command(
        interp,
        u8"xer_view",
        [&copied](std::u8string_view value) -> std::u8string_view {
            // The view is valid only during this callback invocation.
            // Copy it when the value must outlive the callback.
            copied = std::u8string(value);
            return value.substr(0, 3);
        });
    xer_assert(created.has_value());

    const auto result = xer::tk::eval(interp, u8"xer_view example");
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"exa"));
    xer_assert_eq(copied, std::u8string(u8"example"));
}

void test_tk_create_command_bool_argument_and_result()
{
    auto interp = make_interpreter();

    const auto created = xer::tk::create_command(
        interp,
        u8"xer_not",
        [](bool value) -> bool {
            return !value;
        });
    xer_assert(created.has_value());

    const auto result = xer::tk::eval(interp, u8"xer_not false");
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"1"));
}

void test_tk_create_command_unsigned_and_floating_arguments()
{
    auto interp = make_interpreter();

    const auto created = xer::tk::create_command(
        interp,
        u8"xer_mix",
        [](unsigned int count, double value) -> double {
            return static_cast<double>(count) + value;
        });
    xer_assert(created.has_value());

    const auto result = xer::tk::eval(interp, u8"xer_mix 3 2.5");
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"5.5"));
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

void test_tk_interpreter_worker_thread()
{
    std::exception_ptr failure;

    std::thread worker([&failure] {
        try {
            auto interp = make_interpreter();

            const auto created = xer::tk::create_command(
                interp,
                u8"worker_add",
                [](int a, int b) -> int {
                    return a + b;
                });
            xer_assert(created.has_value());

            const auto result = xer::tk::eval(interp, u8"worker_add 10 20");
            xer_assert(result.has_value());
            xer_assert_eq(*result, std::u8string(u8"30"));

            // The interpreter is intentionally destroyed inside this worker
            // thread. This test does not share one interpreter across threads.
        } catch (...) {
            failure = std::current_exception();
        }
    });

    worker.join();

    if (failure != nullptr) {
        std::rethrow_exception(failure);
    }
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
    test_tk_obj_string_and_int_var();
    test_tk_obj_list_var();
    test_tk_obj_copy_keeps_reference();
    test_tk_create_command_obj_argument_and_result();
    test_tk_to_native_handle();
    test_tk_create_command_integer_result();
    test_tk_create_command_string_result();
    test_tk_create_command_void_result();
    test_tk_create_command_string_view_argument();
    test_tk_create_command_bool_argument_and_result();
    test_tk_create_command_unsigned_and_floating_arguments();
    test_tk_create_command_result_return();
    test_tk_interpreter_worker_thread();

    return 0;
}
