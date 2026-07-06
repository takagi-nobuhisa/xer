/**
 * @file xer/bits/windows.h
 * @brief Internal Windows platform header inclusion helpers.
 */

#pragma once

#ifndef XER_BITS_WINDOWS_H_INCLUDED_
#define XER_BITS_WINDOWS_H_INCLUDED_

#if defined(_WIN32)

#ifndef NOMINMAX
#define NOMINMAX
#endif

/*
 * Do not define WIN32_LEAN_AND_MEAN here.  Some users intentionally rely on
 * Windows headers that are excluded by that macro.
 *
 * Instead, suppress only the legacy winsock.h inclusion from windows.h.
 * xer/socket.h includes winsock2.h explicitly when socket support is needed.
 */
#if !defined(_WINSOCK2API_) && !defined(_WINSOCKAPI_)
#define _WINSOCKAPI_
#define XER_BITS_WINDOWS_SUPPRESSED_WINSOCKAPI
#endif

#include <windows.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#endif

#endif
