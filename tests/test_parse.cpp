/**
 * @file tests/test_parse.cpp
 * @brief Simple tests for xer/parse.h.
 */

#include <concepts>
#include <cstddef>

#include <xer/error.h>
#include <xer/parse.h>

namespace {

void test_parse_error_reason_values()
{
    static_assert(std::same_as<decltype(xer::parse_error_reason::none), xer::parse_error_reason>);

    if (xer::parse_error_reason::none == xer::parse_error_reason::invalid_syntax) {
        throw "test_parse_error_reason_values: enum values are not distinct";
    }
    if (xer::parse_error_reason::invalid_magic == xer::parse_error_reason::truncated_input) {
        throw "test_parse_error_reason_values: binary reasons are not distinct";
    }
}

void test_parse_error_detail_default()
{
    const xer::parse_error_detail detail;

    if (detail.offset != 0) {
        throw "test_parse_error_detail_default: offset mismatch";
    }
    if (detail.line != 0) {
        throw "test_parse_error_detail_default: line mismatch";
    }
    if (detail.column != 0) {
        throw "test_parse_error_detail_default: column mismatch";
    }
    if (detail.reason != xer::parse_error_reason::none) {
        throw "test_parse_error_detail_default: reason mismatch";
    }
}

void test_parse_error_detail_position()
{
    const xer::parse_error_detail detail{
        .offset = 12,
        .line = 3,
        .column = 5,
        .reason = xer::parse_error_reason::invalid_token,
    };

    if (detail.offset != 12) {
        throw "test_parse_error_detail_position: offset mismatch";
    }
    if (detail.line != 3) {
        throw "test_parse_error_detail_position: line mismatch";
    }
    if (detail.column != 5) {
        throw "test_parse_error_detail_position: column mismatch";
    }
    if (detail.reason != xer::parse_error_reason::invalid_token) {
        throw "test_parse_error_detail_position: reason mismatch";
    }
}

void test_parse_error_detail_with_error()
{
    const auto e = xer::make_error<xer::parse_error_detail>(
        xer::error_t::invalid_argument,
        xer::parse_error_detail{
            .offset = 4,
            .line = 1,
            .column = 5,
            .reason = xer::parse_error_reason::invalid_number,
        });

    static_assert(std::same_as<decltype(e), const xer::error<xer::parse_error_detail>>);

    if (e.code != xer::error_t::invalid_argument) {
        throw "test_parse_error_detail_with_error: code mismatch";
    }
    if (e.offset != 4) {
        throw "test_parse_error_detail_with_error: offset mismatch";
    }
    if (e.line != 1) {
        throw "test_parse_error_detail_with_error: line mismatch";
    }
    if (e.column != 5) {
        throw "test_parse_error_detail_with_error: column mismatch";
    }
    if (e.reason != xer::parse_error_reason::invalid_number) {
        throw "test_parse_error_detail_with_error: reason mismatch";
    }
}

void test_parse_error_detail_no_position_error()
{
    const auto e = xer::make_error<xer::parse_error_detail>(
        xer::error_t::io,
        xer::parse_error_detail{});

    if (e.code != xer::error_t::io) {
        throw "test_parse_error_detail_no_position_error: code mismatch";
    }
    if (e.offset != 0 || e.line != 0 || e.column != 0) {
        throw "test_parse_error_detail_no_position_error: position mismatch";
    }
    if (e.reason != xer::parse_error_reason::none) {
        throw "test_parse_error_detail_no_position_error: reason mismatch";
    }
}

} // namespace

auto main() -> int
{
    test_parse_error_reason_values();
    test_parse_error_detail_default();
    test_parse_error_detail_position();
    test_parse_error_detail_with_error();
    test_parse_error_detail_no_position_error();
}
