/**
 * @file tests/test_path.cpp
 * @brief Tests for xer/path.h.
 */

#include <string>
#include <string_view>

#include <xer/assert.h>
#include <xer/path.h>

namespace {

void test_default_constructor() {
    const xer::path value;
    xer_assert_eq(value.str(), std::u8string_view(u8""));
}

void test_u8string_view_constructor_normalizes_backslash() {
    const xer::path value(u8"foo\\bar\\baz.txt");
    xer_assert_eq(value.str(), std::u8string_view(u8"foo/bar/baz.txt"));
}

void test_char8_pointer_constructor_normalizes_backslash() {
    const xer::path value(u8"alpha\\beta");
    xer_assert_eq(value.str(), std::u8string_view(u8"alpha/beta"));
}

void test_null_char8_pointer_constructor_creates_empty_path() {
    const xer::path value(static_cast<const char8_t*>(nullptr));
    xer_assert_eq(value.str(), std::u8string_view(u8""));
}

void test_to_native_path_ascii() {
    const xer::path value(u8"dir/subdir/file.txt");
    const auto result = xer::to_native_path(value);

    xer_assert(result.has_value());

#ifdef _WIN32
    xer_assert_eq(result.value(), std::wstring(L"dir\\subdir\\file.txt"));
#else
    xer_assert_eq(result.value(), std::string("dir/subdir/file.txt"));
#endif
}

void test_to_native_path_utf8() {
    const xer::path value(u8"日本語/🙂.txt");
    const auto result = xer::to_native_path(value);

    xer_assert(result.has_value());

#ifdef _WIN32
    xer_assert_eq(result.value(), std::wstring(L"日本語\\🙂.txt"));
#else
    xer_assert_eq(result.value(), std::string(reinterpret_cast<const char*>(u8"日本語/🙂.txt")));
#endif
}

void test_from_native_path_ascii_view() {
#ifdef _WIN32
    const auto result = xer::from_native_path(std::wstring_view(L"dir\\subdir\\file.txt"));
#else
    const auto result = xer::from_native_path(std::string_view("dir/subdir/file.txt"));
#endif

    xer_assert(result.has_value());
    xer_assert_eq(result.value().str(), std::u8string_view(u8"dir/subdir/file.txt"));
}

void test_from_native_path_utf8_view() {
#ifdef _WIN32
    const auto result = xer::from_native_path(std::wstring_view(L"日本語\\🙂.txt"));
#else
    const auto result = xer::from_native_path(std::string_view(reinterpret_cast<const char*>(u8"日本語/🙂.txt")));
#endif

    xer_assert(result.has_value());
    xer_assert_eq(result.value().str(), std::u8string_view(u8"日本語/🙂.txt"));
}

void test_from_native_path_pointer() {
#ifdef _WIN32
    const auto result = xer::from_native_path(L"foo\\bar");
#else
    const auto result = xer::from_native_path("foo/bar");
#endif

    xer_assert(result.has_value());
    xer_assert_eq(result.value().str(), std::u8string_view(u8"foo/bar"));
}

void test_from_native_path_null_pointer() {
    const auto result = xer::from_native_path(static_cast<const xer::native_path_char_t*>(nullptr));
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_round_trip() {
    const xer::path original(u8"one/two/日本語🙂.dat");

    const auto native = xer::to_native_path(original);
    xer_assert(native.has_value());

    const auto restored = xer::from_native_path(std::basic_string_view<xer::native_path_char_t>(native.value()));
    xer_assert(restored.has_value());
    xer_assert_eq(restored.value().str(), original.str());
}

void test_from_native_path_invalid_sequence() {
#ifdef _WIN32
    const wchar_t invalid_value[] = {0xd800, 0x0000};
    const auto result = xer::from_native_path(invalid_value);
#else
    const std::string invalid_value("\x80", 1);
    const auto result = xer::from_native_path(std::string_view(invalid_value));
#endif

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::ilseq);
}

} // namespace

int main() {
    test_default_constructor();
    test_u8string_view_constructor_normalizes_backslash();
    test_char8_pointer_constructor_normalizes_backslash();
    test_null_char8_pointer_constructor_creates_empty_path();
    test_to_native_path_ascii();
    test_to_native_path_utf8();
    test_from_native_path_ascii_view();
    test_from_native_path_utf8_view();
    test_from_native_path_pointer();
    test_from_native_path_null_pointer();
    test_round_trip();
    test_from_native_path_invalid_sequence();
    return 0;
}
