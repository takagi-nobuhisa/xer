/**
 * @file xer/bits/parse.h
 * @brief Defines common parse error detail types.
 */

#pragma once

#ifndef XER_BITS_PARSE_H_INCLUDED_
#define XER_BITS_PARSE_H_INCLUDED_

#include <cstddef>

#include <xer/bits/common.h>

namespace xer {

enum class parse_error_reason {
    none,
    invalid_syntax,
    invalid_encoding,
    invalid_token,
    invalid_key,
    duplicate_key,
    duplicate_table,
    invalid_string,
    invalid_escape,
    invalid_unicode_escape,
    invalid_number,
    integer_out_of_range,
    invalid_date_time,
    invalid_array,
    invalid_table,
};

struct parse_error_detail {
    std::size_t offset = 0;
    std::size_t line = 0;
    std::size_t column = 0;
    parse_error_reason reason = parse_error_reason::none;
};

} // namespace xer

#endif /* XER_BITS_PARSE_H_INCLUDED_ */
