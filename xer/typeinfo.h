/**
 * @file xer/typeinfo.h
 * @brief Public header for XER type information helpers.
 */

#pragma once

#ifndef XER_TYPEINFO_H_INCLUDED_
#define XER_TYPEINFO_H_INCLUDED_

#include <xer/bits/typeinfo.h>

/**
 * @brief Creates xer::type_info from the same operand accepted by typeid.
 *
 * This macro is variadic so that type operands containing commas, such as
 * template specializations, can be passed without extra wrapping.
 */
#define xer_typeid(...) ::xer::type_info(typeid(__VA_ARGS__))

#endif /* XER_TYPEINFO_H_INCLUDED_ */
