/**
 * @file tests/test_environs.cpp
 * @brief Tests for xer::environs.
 */

#include <cstdlib>
#include <string>

#include <xer/assert.h>
#include <xer/stdlib.h>

namespace {

#ifdef _WIN32

auto set_environment_variable(const wchar_t* name, const wchar_t* value) -> bool
{
    std::wstring assignment(name);
    assignment.push_back(L'=');
    assignment.append(value);
    return _wputenv(assignment.c_str()) == 0;
}

auto unset_environment_variable(const wchar_t* name) -> bool
{
    std::wstring assignment(name);
    assignment.push_back(L'=');
    return _wputenv(assignment.c_str()) == 0;
}

#else

auto set_environment_variable(const char* name, const char* value) -> bool
{
    return ::setenv(name, value, 1) == 0;
}

auto unset_environment_variable(const char* name) -> bool
{
    return ::unsetenv(name) == 0;
}

#endif

void test_get_environs_returns_snapshot()
{
    const auto environment = xer::get_environs();
    xer_assert(environment.has_value());
}

void test_environs_find_ascii()
{
#ifdef _WIN32
    xer_assert(set_environment_variable(L"XER_TEST_ENVIRON_ASCII", L"hello world"));
#else
    xer_assert(set_environment_variable("XER_TEST_ENVIRON_ASCII", "hello world"));
#endif

    const auto environment = xer::get_environs();
    xer_assert(environment.has_value());

    const auto value = environment->find(u8"XER_TEST_ENVIRON_ASCII");
    xer_assert(value.has_value());
    xer_assert_eq(*value, std::u8string_view(u8"hello world"));

#ifdef _WIN32
    xer_assert(unset_environment_variable(L"XER_TEST_ENVIRON_ASCII"));
#else
    xer_assert(unset_environment_variable("XER_TEST_ENVIRON_ASCII"));
#endif
}

void test_environs_find_utf8()
{
    constexpr auto value = u8"日本語🙂";

#ifdef _WIN32
    xer_assert(set_environment_variable(L"XER_TEST_ENVIRON_UTF8", L"日本語🙂"));
#else
    xer_assert(set_environment_variable(
        "XER_TEST_ENVIRON_UTF8",
        reinterpret_cast<const char*>(value)));
#endif

    const auto environment = xer::get_environs();
    xer_assert(environment.has_value());

    const auto found = environment->find(u8"XER_TEST_ENVIRON_UTF8");
    xer_assert(found.has_value());
    xer_assert_eq(*found, std::u8string_view(value));

#ifdef _WIN32
    xer_assert(unset_environment_variable(L"XER_TEST_ENVIRON_UTF8"));
#else
    xer_assert(unset_environment_variable("XER_TEST_ENVIRON_UTF8"));
#endif
}

void test_environs_find_missing()
{
#ifdef _WIN32
    xer_assert(unset_environment_variable(L"XER_TEST_ENVIRON_MISSING"));
#else
    unset_environment_variable("XER_TEST_ENVIRON_MISSING");
#endif

    const auto environment = xer::get_environs();
    xer_assert(environment.has_value());

    const auto value = environment->find(u8"XER_TEST_ENVIRON_MISSING");
    xer_assert(!value.has_value());
    xer_assert_eq(value.error().code, xer::error_t::not_found);
}

void test_environs_empty_name()
{
    const auto environment = xer::get_environs();
    xer_assert(environment.has_value());

    const auto value = environment->find(u8"");
    xer_assert(!value.has_value());
    xer_assert_eq(value.error().code, xer::error_t::invalid_argument);
}

void test_environs_at_out_of_range()
{
    const auto environment = xer::get_environs();
    xer_assert(environment.has_value());

    const auto value = environment->at(environment->size());
    xer_assert(!value.has_value());
    xer_assert_eq(value.error().code, xer::error_t::out_of_range);
}

} // namespace

auto main() -> int
{
    test_get_environs_returns_snapshot();
    test_environs_find_ascii();
    test_environs_find_utf8();
    test_environs_find_missing();
    test_environs_empty_name();
    test_environs_at_out_of_range();
    return 0;
}
