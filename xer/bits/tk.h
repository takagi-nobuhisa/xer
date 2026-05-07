/**
 * @file xer/bits/tk.h
 * @brief Tk-side implementation for XER Tcl/Tk integration.
 */

#pragma once

#ifndef XER_BITS_TK_H_INCLUDED_
#define XER_BITS_TK_H_INCLUDED_

#include <expected>

#include <tk.h>

#include <xer/bits/common.h>
#include <xer/bits/tcl.h>
#include <xer/error.h>

namespace xer::tk {

using event_flag_t = int;

inline constexpr event_flag_t event_all = TCL_ALL_EVENTS;
inline constexpr event_flag_t event_window = TCL_WINDOW_EVENTS;
inline constexpr event_flag_t event_file = TCL_FILE_EVENTS;
inline constexpr event_flag_t event_timer = TCL_TIMER_EVENTS;
inline constexpr event_flag_t event_idle = TCL_IDLE_EVENTS;
inline constexpr event_flag_t event_dont_wait = TCL_DONT_WAIT;

/**
 * @brief Initializes both Tcl and Tk for an interpreter.
 */
[[nodiscard]] inline auto init(interpreter& interp) -> result<void, error_detail>
{
    Tcl_Interp* const raw = to_native_handle(interp);
    if (raw == nullptr) {
        return std::unexpected(detail::make_error(error_t::invalid_argument));
    }

    int code = Tcl_Init(raw);
    if (code != TCL_OK) {
        return std::unexpected(detail::make_error(error_t::runtime_error, code));
    }

    code = Tk_Init(raw);
    if (code != TCL_OK) {
        return std::unexpected(detail::make_error(error_t::runtime_error, code));
    }

    return {};
}

/**
 * @brief Runs the Tk main event loop.
 */
inline auto main_loop() -> void
{
    Tk_MainLoop();
}

/**
 * @brief Processes one Tcl/Tk event.
 */
inline auto do_one_event(event_flag_t flags = event_all) -> int
{
    return Tcl_DoOneEvent(flags);
}

} // namespace xer::tk

#endif /* XER_BITS_TK_H_INCLUDED_ */
