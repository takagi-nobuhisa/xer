/**
 * @file xer/bits/toml_io.h
 * @brief TOML load and save helpers.
 */

#pragma once

#ifndef XER_BITS_TOML_IO_H_INCLUDED_
#define XER_BITS_TOML_IO_H_INCLUDED_

#include <expected>

#include <xer/bits/file_contents.h>
#include <xer/bits/toml.h>
#include <xer/error.h>
#include <xer/parse.h>
#include <xer/path.h>
#include <xer/stdio.h>

namespace xer {

/**
 * @brief Loads a UTF-8 TOML file and decodes it.
 *
 * This convenience function combines UTF-8 file reading with @ref toml_decode.
 * If file I/O fails before parsing begins, the returned error uses
 * @ref parse_error_reason::none and leaves the position members of
 * @ref parse_error_detail at zero.
 *
 * @param filename File path to read.
 * @return Decoded TOML value on success.
 */
[[nodiscard]] inline auto toml_load(const path& filename)
    -> result<toml_value, parse_error_detail>
{
    const auto text = file_get_contents(filename, encoding_t::utf8);
    if (!text.has_value()) {
        return std::unexpected(make_error<parse_error_detail>(
            text.error().code,
            parse_error_detail{}));
    }

    return toml_decode(*text);
}

/**
 * @brief Encodes a TOML value and saves it as UTF-8 text.
 *
 * @param filename File path to write.
 * @param value TOML value to encode.
 * @return Empty success result on success.
 */
[[nodiscard]] inline auto toml_save(
    const path& filename,
    const toml_value& value) -> result<void>
{
    const auto text = toml_encode(value);
    if (!text.has_value()) {
        return std::unexpected(text.error());
    }

    return file_put_contents(filename, *text, encoding_t::utf8);
}

} // namespace xer

#endif /* XER_BITS_TOML_IO_H_INCLUDED_ */
