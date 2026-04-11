/**
 * @file tests/test_assert.cpp
 * @brief Tests for xer/assert.h.
 */

#include <stdexcept>
#include <string>
#include <string_view>

#include <xer/assert.h>

namespace {

/**
 * @brief Checks that the message contains the specified substring.
 * @param message Full message.
 * @param needle Substring to search for.
 */
void require_contains(std::string_view message, std::string_view needle)
{
    xer_assert(message.find(needle) != std::string_view::npos);
}

/**
 * @brief Checks that an assertion failure was thrown and returns the message.
 * @param f Test target.
 * @return Failure message.
 */
template<typename f_t>
[[nodiscard]] std::string expect_assertion_failure(f_t&& f)
{
    try {
        std::forward<f_t>(f)();
    } catch (const xer::assertion_error& e) {
        return e.what();
    }

    throw std::runtime_error("xer::assertion_error was not thrown");
}

/**
 * @brief Tests successful boolean assertions.
 */
void test_assert_success()
{
    xer_assert(true);
    xer_assert(1 + 1 == 2);
    xer_assert_not(false);
    xer_assert_not(1 + 1 == 3);
}

/**
 * @brief Tests failed xer_assert.
 */
void test_assert_failure()
{
    const std::string message = expect_assertion_failure([] {
        xer_assert(false);
    });

    require_contains(message, "xer_assert failed");
    require_contains(message, "false");
    require_contains(message, "expected true");
}

/**
 * @brief Tests failed xer_assert_not.
 */
void test_assert_not_failure()
{
    const std::string message = expect_assertion_failure([] {
        xer_assert_not(true);
    });

    require_contains(message, "xer_assert_not failed");
    require_contains(message, "true");
    require_contains(message, "expected false");
}

/**
 * @brief Tests successful equality assertions.
 */
void test_assert_eq_success()
{
    xer_assert_eq(0, 0);
    xer_assert_eq(42, 42);

    const std::string s1 = "alpha";
    const std::string s2 = "alpha";
    xer_assert_eq(s1, s2);

    const char* p1 = "hello";
    const char* p2 = "hello";
    xer_assert_eq(std::string_view(p1), std::string_view(p2));
}

/**
 * @brief Tests failed xer_assert_eq with integers.
 */
void test_assert_eq_failure_int()
{
    const std::string message = expect_assertion_failure([] {
        xer_assert_eq(1, 2);
    });

    require_contains(message, "xer_assert_eq failed");
    require_contains(message, "1 == 2");
    require_contains(message, "lhs=1");
    require_contains(message, "rhs=2");
}

/**
 * @brief Tests failed xer_assert_eq with bool values.
 */
void test_assert_eq_failure_bool()
{
    const std::string message = expect_assertion_failure([] {
        xer_assert_eq(false, true);
    });

    require_contains(message, "xer_assert_eq failed");
    require_contains(message, "lhs=false");
    require_contains(message, "rhs=true");
}

/**
 * @brief Tests successful inequality assertions.
 */
void test_assert_ne_success()
{
    xer_assert_ne(1, 2);
    xer_assert_ne(false, true);

    const std::string s1 = "alpha";
    const std::string s2 = "beta";
    xer_assert_ne(s1, s2);
}

/**
 * @brief Tests failed xer_assert_ne.
 */
void test_assert_ne_failure()
{
    const std::string message = expect_assertion_failure([] {
        xer_assert_ne(7, 7);
    });

    require_contains(message, "xer_assert_ne failed");
    require_contains(message, "7 != 7");
    require_contains(message, "lhs=7");
    require_contains(message, "rhs=7");
}

/**
 * @brief Tests successful less-than assertions.
 */
void test_assert_lt_success()
{
    xer_assert_lt(1, 2);
    xer_assert_lt(-5, -4);

    const std::string a = "abc";
    const std::string b = "abd";
    xer_assert_lt(a, b);
}

/**
 * @brief Tests failed xer_assert_lt.
 */
void test_assert_lt_failure()
{
    const std::string message = expect_assertion_failure([] {
        xer_assert_lt(5, 3);
    });

    require_contains(message, "xer_assert_lt failed");
    require_contains(message, "5 < 3");
    require_contains(message, "lhs=5");
    require_contains(message, "rhs=3");
}

/**
 * @brief Tests that operands are evaluated only once.
 */
void test_single_evaluation()
{
    int lhs = 0;
    int rhs = 1;

    xer_assert_eq(lhs++, 0);
    xer_assert_eq(rhs++, 1);

    xer_assert_eq(lhs, 1);
    xer_assert_eq(rhs, 2);

    int x = 0;
    int y = 0;

    const std::string message = expect_assertion_failure([&] {
        xer_assert_eq(x++, y++ + 1);
    });

    xer_assert_eq(x, 1);
    xer_assert_eq(y, 1);
    require_contains(message, "xer_assert_eq failed");
}

/**
 * @brief Throws a standard exception.
 */
void throw_runtime_error()
{
    throw std::runtime_error("runtime");
}

/**
 * @brief Throws a logic exception.
 */
void throw_logic_error()
{
    throw std::logic_error("logic");
}

/**
 * @brief Throws a non-standard exception.
 */
void throw_non_std_exception()
{
    throw 123;
}

/**
 * @brief Does not throw.
 */
void do_nothing()
{
}

/**
 * @brief Tests successful xer_assert_throw.
 */
void test_assert_throw_success()
{
    xer_assert_throw(throw_runtime_error(), std::runtime_error);
    xer_assert_throw(throw_logic_error(), std::exception);
    xer_assert_throw(throw_non_std_exception(), ...);
}

/**
 * @brief Tests xer_assert_throw when no exception is thrown.
 */
void test_assert_throw_no_exception()
{
    const std::string message = expect_assertion_failure([] {
        xer_assert_throw(do_nothing(), std::runtime_error);
    });

    require_contains(message, "xer_assert_throw failed");
    require_contains(message, "do_nothing()");
    require_contains(message, "no exception thrown");
}

/**
 * @brief Tests xer_assert_throw when an unexpected exception type is thrown.
 */
void test_assert_throw_unexpected_type()
{
    const std::string message = expect_assertion_failure([] {
        xer_assert_throw(throw_logic_error(), std::runtime_error);
    });

    require_contains(message, "xer_assert_throw failed");
    require_contains(message, "throw_logic_error()");
    require_contains(message, "unexpected exception type");
}

/**
 * @brief Tests successful xer_assert_nothrow.
 */
void test_assert_nothrow_success()
{
    xer_assert_nothrow(do_nothing());
    xer_assert_nothrow((void)0);
    xer_assert_nothrow(xer_assert_eq(1, 1));
}

/**
 * @brief Tests xer_assert_nothrow with std::exception.
 */
void test_assert_nothrow_std_exception()
{
    const std::string message = expect_assertion_failure([] {
        xer_assert_nothrow(throw_runtime_error());
    });

    require_contains(message, "xer_assert_nothrow failed");
    require_contains(message, "throw_runtime_error()");
    require_contains(message, "runtime");
}

/**
 * @brief Tests xer_assert_nothrow with a non-standard exception.
 */
void test_assert_nothrow_non_std_exception()
{
    const std::string message = expect_assertion_failure([] {
        xer_assert_nothrow(throw_non_std_exception());
    });

    require_contains(message, "xer_assert_nothrow failed");
    require_contains(message, "throw_non_std_exception()");
    require_contains(message, "unexpected exception thrown");
}

/**
 * @brief Tests UTF-8 narrow string diagnostics.
 */
void test_utf8_narrow_string_message()
{
    const std::u8string lhs = u8"あ";
    const std::u8string rhs = u8"い";

    const std::string message = expect_assertion_failure([&] {
        xer_assert_eq(lhs, rhs);
    });

    require_contains(message, "xer_assert_eq failed");
    require_contains(message, "lhs=あ");
    require_contains(message, "rhs=い");
}

/**
 * @brief Tests signed char UTF-8 string diagnostics.
 */
void test_signed_char_string_message()
{
    static constexpr signed char lhs_data[] = {
        static_cast<signed char>(0xE3),
        static_cast<signed char>(0x81),
        static_cast<signed char>(0x82),
        0
    };
    static constexpr signed char rhs_data[] = {
        static_cast<signed char>(0xE3),
        static_cast<signed char>(0x81),
        static_cast<signed char>(0x84),
        0
    };

    const signed char* lhs = lhs_data;
    const signed char* rhs = rhs_data;

    const std::string message = expect_assertion_failure([&] {
        xer_assert_eq(lhs, rhs);
    });

    require_contains(message, "xer_assert_eq failed");
    require_contains(message, "lhs=あ");
    require_contains(message, "rhs=い");
}

/**
 * @brief Tests unsigned char CP932 string diagnostics.
 */
void test_unsigned_char_cp932_string_message()
{
    static constexpr unsigned char lhs_data[] = {0x82, 0xA0, 0x00}; // あ
    static constexpr unsigned char rhs_data[] = {0x82, 0xA2, 0x00}; // い

    const unsigned char* lhs = lhs_data;
    const unsigned char* rhs = rhs_data;

    const std::string message = expect_assertion_failure([&] {
        xer_assert_eq(lhs, rhs);
    });

    require_contains(message, "xer_assert_eq failed");
    require_contains(message, "lhs=あ");
    require_contains(message, "rhs=い");
}

/**
 * @brief Tests char16_t string diagnostics.
 */
void test_char16_string_message()
{
    const char16_t* lhs = u"あ";
    const char16_t* rhs = u"い";

    const std::string message = expect_assertion_failure([&] {
        xer_assert_eq(lhs, rhs);
    });

    require_contains(message, "xer_assert_eq failed");
    require_contains(message, "lhs=あ");
    require_contains(message, "rhs=い");
}

/**
 * @brief Tests char32_t string diagnostics.
 */
void test_char32_string_message()
{
    const char32_t* lhs = U"あ";
    const char32_t* rhs = U"い";

    const std::string message = expect_assertion_failure([&] {
        xer_assert_eq(lhs, rhs);
    });

    require_contains(message, "xer_assert_eq failed");
    require_contains(message, "lhs=あ");
    require_contains(message, "rhs=い");
}

/**
 * @brief Tests wchar_t string diagnostics.
 */
void test_wchar_string_message()
{
    const wchar_t* lhs = L"あ";
    const wchar_t* rhs = L"い";

    const std::string message = expect_assertion_failure([&] {
        xer_assert_eq(lhs, rhs);
    });

    require_contains(message, "xer_assert_eq failed");
    require_contains(message, "lhs=あ");
    require_contains(message, "rhs=い");
}

/**
 * @brief Tests unsigned char single-byte diagnostics.
 */
void test_unsigned_char_value_message()
{
    const unsigned char lhs = 0x82;
    const unsigned char rhs = 0x83;

    const std::string message = expect_assertion_failure([&] {
        xer_assert_eq(lhs, rhs);
    });

    require_contains(message, "xer_assert_eq failed");
    require_contains(message, "lhs=130");
    require_contains(message, "rhs=131");
}

} // namespace

/**
 * @brief Program entry point.
 * @return Exit status.
 */
int main()
{
    test_assert_success();
    test_assert_failure();
    test_assert_not_failure();

    test_assert_eq_success();
    test_assert_eq_failure_int();
    test_assert_eq_failure_bool();

    test_assert_ne_success();
    test_assert_ne_failure();

    test_assert_lt_success();
    test_assert_lt_failure();

    test_single_evaluation();

    test_assert_throw_success();
    test_assert_throw_no_exception();
    test_assert_throw_unexpected_type();

    test_assert_nothrow_success();
    test_assert_nothrow_std_exception();
    test_assert_nothrow_non_std_exception();

    test_utf8_narrow_string_message();
    test_signed_char_string_message();
    test_unsigned_char_cp932_string_message();
    test_char16_string_message();
    test_char32_string_message();
    test_wchar_string_message();
    test_unsigned_char_value_message();

    return 0;
}
