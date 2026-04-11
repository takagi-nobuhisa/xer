/**
 * @file tests/test_getenv.cpp
 * @brief Tests for xer/bits/getenv.h.
 */

#include <cstdlib>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

#include <xer/assert.h>
#include <xer/bits/getenv.h>

namespace {

#ifdef _WIN32

bool set_environment_variable(const wchar_t* name, const wchar_t* value) {
    return SetEnvironmentVariableW(name, value) != 0;
}

bool unset_environment_variable(const wchar_t* name) {
    return SetEnvironmentVariableW(name, nullptr) != 0;
}

#else

bool set_environment_variable(const char* name, const char* value) {
    return ::setenv(name, value, 1) == 0;
}

bool unset_environment_variable(const char* name) {
    return ::unsetenv(name) == 0;
}

#endif

void test_empty_name() {
    const auto result = xer::getenv(u8"");
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_missing_environment() {
    constexpr auto name = u8"XER_TEST_GETENV_MISSING";

#ifdef _WIN32
    xer_assert(unset_environment_variable(L"XER_TEST_GETENV_MISSING"));
#else
    unset_environment_variable("XER_TEST_GETENV_MISSING");
#endif

    const auto result = xer::getenv(name);
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::not_found);
}

void test_ascii_environment() {
    constexpr auto name = u8"XER_TEST_GETENV_ASCII";
    constexpr auto value = u8"hello world";

#ifdef _WIN32
    xer_assert(set_environment_variable(L"XER_TEST_GETENV_ASCII", L"hello world"));
#else
    xer_assert(set_environment_variable("XER_TEST_GETENV_ASCII", "hello world"));
#endif

    const auto result = xer::getenv(name);
    xer_assert(result.has_value());
    xer_assert_eq(result.value(), std::u8string(value));

#ifdef _WIN32
    xer_assert(unset_environment_variable(L"XER_TEST_GETENV_ASCII"));
#else
    xer_assert(unset_environment_variable("XER_TEST_GETENV_ASCII"));
#endif
}

void test_utf8_environment() {
    constexpr auto name = u8"XER_TEST_GETENV_UTF8";
    constexpr auto value = u8"日本語🙂";

#ifdef _WIN32
    xer_assert(set_environment_variable(L"XER_TEST_GETENV_UTF8", L"日本語🙂"));
#else
    xer_assert(set_environment_variable(
        "XER_TEST_GETENV_UTF8",
        reinterpret_cast<const char*>(value)));
#endif

    const auto result = xer::getenv(name);
    xer_assert(result.has_value());
    xer_assert_eq(result.value(), std::u8string(value));

#ifdef _WIN32
    xer_assert(unset_environment_variable(L"XER_TEST_GETENV_UTF8"));
#else
    xer_assert(unset_environment_variable("XER_TEST_GETENV_UTF8"));
#endif
}

} // namespace

int main() {
    test_empty_name();
    test_missing_environment();
    test_ascii_environment();
    test_utf8_environment();
    return 0;
}
