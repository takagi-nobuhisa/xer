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

/**
 * @brief Fine-grained reason for a text parsing failure.
 *
 * This enumeration is used as the structured reason field of
 * @ref parse_error_detail. The value @ref parse_error_reason::none means that
 * no parse-position detail is available. It is mainly used when an API such as
 * a load helper returns @c result<T,parse_error_detail> but fails before
 * parsing begins, for example because file I/O failed.
 */
enum class parse_error_reason {
    /** No parse detail is available. */
    none,
    /** The input does not match the expected grammar. */
    invalid_syntax,
    /** The input encoding is invalid for the parser. */
    invalid_encoding,
    /** A token is malformed or not valid at the current position. */
    invalid_token,
    /** A key or key-like name is malformed. */
    invalid_key,
    /** A key appears more than once where duplicates are not allowed. */
    duplicate_key,
    /** A table appears more than once where duplicates are not allowed. */
    duplicate_table,
    /** A string literal is malformed. */
    invalid_string,
    /** An escape sequence is malformed. */
    invalid_escape,
    /** A Unicode escape sequence is malformed or invalid. */
    invalid_unicode_escape,
    /** A numeric literal is malformed. */
    invalid_number,
    /** An integer literal is outside the representable range. */
    integer_out_of_range,
    /** A date or time literal is malformed. */
    invalid_date_time,
    /** An array value is malformed. */
    invalid_array,
    /** A table or table-like value is malformed. */
    invalid_table,
};

/**
 * @brief Common detail payload for text parse errors.
 *
 * The position members describe a location in the original UTF-8 input.
 * @c offset is a zero-based UTF-8 code-unit offset. @c line and @c column are
 * one-based when @c reason is not @ref parse_error_reason::none.
 *
 * When @c reason is @ref parse_error_reason::none, the position members are
 * intentionally zero. This represents a failure that has no parse location,
 * such as an I/O error reported by a load helper before decoding starts.
 */
struct parse_error_detail {
    /** Zero-based UTF-8 code-unit offset, or zero when no position is available. */
    std::size_t offset = 0;
    /** One-based line number, or zero when no position is available. */
    std::size_t line = 0;
    /** One-based UTF-8 code-unit column, or zero when no position is available. */
    std::size_t column = 0;
    /** Fine-grained parse reason, or @c none when this detail is unused. */
    parse_error_reason reason = parse_error_reason::none;
};

} // namespace xer

#endif /* XER_BITS_PARSE_H_INCLUDED_ */
