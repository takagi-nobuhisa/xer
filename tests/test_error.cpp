/**
 * @file tests/test_error.cpp
 * @brief Simple tests for xer/error.h.
 */

#include <concepts>
#include <cstdint>
#include <source_location>
#include <string>
#include <tuple>
#include <utility>

#include <xer/error.h>

namespace {

struct file_error_detail {
    std::u8string path;
    std::size_t offset;

    constexpr explicit file_error_detail(std::tuple<std::u8string, std::size_t> value) noexcept
        : path(std::move(std::get<0>(value))),
          offset(std::get<1>(value)) {}
};

void test_error_void() {
    const auto e = xer::make_error(xer::error_t::inval);

    static_assert(std::same_as<decltype(e), const xer::error<void>>);

    if (e.code != xer::error_t::inval) {
        throw "test_error_void: code mismatch";
    }
}

void test_error_scalar_detail() {
    const auto e = xer::make_error<int>(xer::error_t::range, 123);

    static_assert(std::same_as<decltype(e), const xer::error<int>>);

    if (e.code != xer::error_t::range) {
        throw "test_error_scalar_detail: code mismatch";
    }
    if (e.detail != 123) {
        throw "test_error_scalar_detail: detail mismatch";
    }
}

void test_error_class_detail() {
    const auto e = xer::make_error<file_error_detail>(
        xer::error_t::noent,
        std::tuple<std::u8string, std::size_t>{u8"foo.txt", 42});

    static_assert(std::same_as<decltype(e), const xer::error<file_error_detail>>);

    if (e.code != xer::error_t::noent) {
        throw "test_error_class_detail: code mismatch";
    }
    if (e.path != u8"foo.txt") {
        throw "test_error_class_detail: path mismatch";
    }
    if (e.offset != 42) {
        throw "test_error_class_detail: offset mismatch";
    }
}

void test_base_conversion() {
    const auto e = xer::make_error<file_error_detail>(
        xer::error_t::io,
        std::tuple<std::u8string, std::size_t>{u8"bar.bin", 7});

    const xer::error<void>& base = e;
    const file_error_detail& detail = e;

    if (base.code != xer::error_t::io) {
        throw "test_base_conversion: base code mismatch";
    }
    if (detail.path != u8"bar.bin") {
        throw "test_base_conversion: detail path mismatch";
    }
    if (detail.offset != 7) {
        throw "test_base_conversion: detail offset mismatch";
    }
}



void test_error_encoding_error() {
    const auto e = xer::make_error(xer::error_t::encoding_error);

    if (e.code != xer::error_t::encoding_error) {
        throw "test_error_encoding_error: code mismatch";
    }
}

void test_error_io_error() {
    const auto e = xer::make_error(xer::error_t::io_error);

    if (e.code != xer::error_t::io_error) {
        throw "test_error_io_error: code mismatch";
    }
}

void test_source_location_void() {
    constexpr std::uint_least32_t expected_line = __LINE__ + 1;
    const auto e = xer::make_error(xer::error_t::inval);

    if (e.location.line() != expected_line) {
        throw "test_source_location_void: line mismatch";
    }
}

void test_source_location_scalar_detail() {
    constexpr std::uint_least32_t expected_line = __LINE__ + 1;
    const auto e = xer::make_error<int>(xer::error_t::range, 123);

    if (e.location.line() != expected_line) {
        throw "test_source_location_scalar_detail: line mismatch";
    }
}

void test_source_location_class_detail() {
    constexpr std::uint_least32_t expected_line = __LINE__ + 1;
    const auto e = xer::make_error<file_error_detail>(
        xer::error_t::noent,
        std::tuple<std::u8string, std::size_t>{u8"foo.txt", 42});

    if (e.location.line() != expected_line) {
        throw "test_source_location_class_detail: line mismatch";
    }
}

void test_source_location_explicit() {
    const auto here = std::source_location::current();
    const auto e = xer::make_error(xer::error_t::io, here);

    if (e.location.line() != here.line()) {
        throw "test_source_location_explicit: line mismatch";
    }
    if (e.location.column() != here.column()) {
        throw "test_source_location_explicit: column mismatch";
    }
}

} // namespace

int main() {
    test_error_void();
    test_error_scalar_detail();
    test_error_class_detail();
    test_base_conversion();
    test_error_encoding_error();
    test_error_io_error();
    test_source_location_void();
    test_source_location_scalar_detail();
    test_source_location_class_detail();
    test_source_location_explicit();
}
