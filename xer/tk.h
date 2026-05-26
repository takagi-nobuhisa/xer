/**
 * @file xer/tk.h
 * @brief Public Tcl/Tk integration utilities.
 */

#pragma once

#ifndef XER_TK_H_INCLUDED_
#define XER_TK_H_INCLUDED_

#if defined(__has_include)
#    if __has_include(<tcl.h>) && __has_include(<tk.h>)
#        define XER_TK_HEADERS_AVAILABLE_ 1
#    else
#        define XER_TK_HEADERS_AVAILABLE_ 0
#    endif
#else
#    define XER_TK_HEADERS_AVAILABLE_ 1
#endif

#if XER_TK_HEADERS_AVAILABLE_
#    include <xer/bits/tcl.h>
#    include <xer/bits/tk.h>
#else
#    if !__has_include(<tcl.h>)
#        error "xer/tk.h requires the Tcl C API header <tcl.h>. On Debian-like systems, add an include path such as -I/usr/include/tcl."
#    endif
#    if !__has_include(<tk.h>)
#        error "xer/tk.h requires the Tk C API header <tk.h>. On Debian-like systems, add an include path such as -I/usr/include/tcl."
#    endif
#endif

#endif /* XER_TK_H_INCLUDED_ */
