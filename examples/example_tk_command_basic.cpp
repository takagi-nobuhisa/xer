#include <string>
#include <string_view>

#include <xer/stdio.h>
#include <xer/tk.h>

// XER_EXAMPLE_BEGIN: tk_command_basic
//
// This example creates a Tcl interpreter, registers a C++ callable as a Tcl
// command, and uses Tcl variables through the XER Tcl/Tk wrapper.
//
// It intentionally does not call xer::tk::init, because no Tk widget is created
// in this example.
//
// Expected output:
// 10 + 20 = 30
// name = xer
// upper = XER

// XER_TEST_FEATURES: tcltk

namespace {

[[nodiscard]] auto print_result(std::u8string_view label, std::u8string_view value)
    -> bool
{
    return xer::printf(u8"%@ = %@\n", label, value).has_value();
}

} // namespace

auto main() -> int
{
    auto interp = xer::tk::interpreter::create();
    if (!interp.has_value()) {
        return 1;
    }

    const auto command = xer::tk::create_command(
        *interp,
        u8"add",
        [](int a, int b) -> int {
            return a + b;
        });
    if (!command.has_value()) {
        return 1;
    }

    const auto sum = xer::tk::eval(*interp, u8"add 10 20");
    if (!sum.has_value()) {
        return 1;
    }

    if (!xer::printf(u8"10 + 20 = %@\n", *sum).has_value()) {
        return 1;
    }

    // Remove the command explicitly so that the C++ callable is released while
    // the interpreter is still actively being used by this example.
    if (!xer::tk::eval(*interp, u8"rename add {}").has_value()) {
        return 1;
    }

    if (!xer::tk::set_var(*interp, u8"name", u8"xer").has_value()) {
        return 1;
    }

    const auto name = xer::tk::get_var(*interp, u8"name");
    if (!name.has_value()) {
        return 1;
    }

    if (!print_result(u8"name", *name)) {
        return 1;
    }

    const auto upper = xer::tk::eval(*interp, u8"string toupper $name");
    if (!upper.has_value()) {
        return 1;
    }

    if (!print_result(u8"upper", *upper)) {
        return 1;
    }

    xer::tk::reset_result(*interp);
    return 0;
}

// XER_EXAMPLE_END: tk_command_basic
