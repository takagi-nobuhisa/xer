// XER_EXAMPLE_BEGIN: tk_window_basic
//
// This example uses xer::tk::main to initialize Tcl/Tk, creates a minimal
// window, and closes it automatically after a short delay so that it can be run
// by automated tests.
//
// Expected output:
// Opening Tk window...
// Tk window closed.

// XER_TEST_FEATURES: tcltk

#include <expected>

#include <xer/error.h>
#include <xer/stdio.h>
#include <xer/tk.h>

auto main() -> int
{
    const auto result = xer::tk::main([](xer::tk::interpreter& interp) -> xer::result<void> {
        if (!xer::puts(u8"Opening Tk window...").has_value()) {
            return std::unexpected(xer::make_error(xer::error_t::io_error));
        }

        const auto setup = xer::tk::eval(
            interp,
            u8"wm title . {XER Tk Example}\n"
            u8"label .message -text {Hello from XER/Tk} -padx 24 -pady 16\n"
            u8"pack .message\n"
            u8"after 500 {destroy .}\n");
        if (!setup.has_value()) {
            return std::unexpected(xer::make_error(xer::error_t::runtime_error));
        }

        return {};
    });

    if (!result.has_value()) {
        return 1;
    }

    if (!xer::puts(u8"Tk window closed.").has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: tk_window_basic
