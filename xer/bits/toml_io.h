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
