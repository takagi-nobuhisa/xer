/**
 * @file tests/test_ctype.cpp
 * @brief Tests for xer/ctype.h (ASCII-only version).
 */

#include <concepts>
#include <expected>

#include <xer/assert.h>
#include <xer/ctype.h>

namespace {

/**
 * @brief Tests ASCII classification functions with positive cases.
 */
void test_is_functions_positive()
{
    xer_assert(xer::isascii(U'A'));
    xer_assert(xer::isalpha(U'A'));
    xer_assert(xer::isalpha(U'z'));
    xer_assert(xer::isupper(U'A'));
    xer_assert(xer::islower(U'z'));
    xer_assert(xer::isdigit(U'0'));
    xer_assert(xer::isdigit(U'9'));
    xer_assert(xer::isalnum(U'A'));
    xer_assert(xer::isalnum(U'7'));
    xer_assert(xer::isspace(U' '));
    xer_assert(xer::isspace(U'\n'));
    xer_assert(xer::isblank(U' '));
    xer_assert(xer::isblank(U'\t'));
    xer_assert(xer::iscntrl(U'\0'));
    xer_assert(xer::iscntrl(U'\x1f'));
    xer_assert(xer::iscntrl(U'\x7f'));
    xer_assert(xer::isprint(U' '));
    xer_assert(xer::isprint(U'~'));
    xer_assert(xer::isgraph(U'!'));
    xer_assert(xer::isgraph(U'~'));
    xer_assert(xer::ispunct(U'!'));
    xer_assert(xer::ispunct(U'~'));
    xer_assert(xer::isxdigit(U'0'));
    xer_assert(xer::isxdigit(U'9'));
    xer_assert(xer::isxdigit(U'A'));
    xer_assert(xer::isxdigit(U'F'));
    xer_assert(xer::isxdigit(U'a'));
    xer_assert(xer::isxdigit(U'f'));
    xer_assert(xer::isoctal(U'0'));
    xer_assert(xer::isoctal(U'7'));
    xer_assert(xer::isbinary(U'0'));
    xer_assert(xer::isbinary(U'1'));
}

/**
 * @brief Tests ASCII classification functions with negative cases.
 */
void test_is_functions_negative()
{
    xer_assert_not(xer::isascii(U'あ'));
    xer_assert_not(xer::isalpha(U'0'));
    xer_assert_not(xer::isalpha(U'_'));
    xer_assert_not(xer::isupper(U'a'));
    xer_assert_not(xer::islower(U'A'));
    xer_assert_not(xer::isdigit(U'A'));
    xer_assert_not(xer::isalnum(U'_'));
    xer_assert_not(xer::isspace(U'A'));
    xer_assert_not(xer::isblank(U'\n'));
    xer_assert_not(xer::iscntrl(U' '));
    xer_assert_not(xer::isprint(U'\n'));
    xer_assert_not(xer::isgraph(U' '));
    xer_assert_not(xer::ispunct(U'A'));
    xer_assert_not(xer::ispunct(U'0'));
    xer_assert_not(xer::ispunct(U' '));
    xer_assert_not(xer::isxdigit(U'G'));
    xer_assert_not(xer::isxdigit(U'g'));
    xer_assert_not(xer::isoctal(U'8'));
    xer_assert_not(xer::isoctal(U'9'));
    xer_assert_not(xer::isbinary(U'2'));
    xer_assert_not(xer::isbinary(U'A'));
}

/**
 * @brief Tests boundary values for ASCII classification.
 */
void test_is_functions_boundaries()
{
    xer_assert(xer::isascii(U'\0'));
    xer_assert(xer::isascii(U'\x7f'));
    xer_assert_not(xer::isascii(U'\x80'));

    xer_assert(xer::iscntrl(U'\0'));
    xer_assert(xer::iscntrl(U'\x1f'));
    xer_assert_not(xer::iscntrl(U'\x20'));
    xer_assert_not(xer::iscntrl(U'\x7e'));
    xer_assert(xer::iscntrl(U'\x7f'));

    xer_assert_not(xer::isprint(U'\x1f'));
    xer_assert(xer::isprint(U'\x20'));
    xer_assert(xer::isprint(U'\x7e'));
    xer_assert_not(xer::isprint(U'\x7f'));

    xer_assert_not(xer::isgraph(U'\x20'));
    xer_assert(xer::isgraph(U'\x21'));
    xer_assert(xer::isgraph(U'\x7e'));
    xer_assert_not(xer::isgraph(U'\x7f'));
}

/**
 * @brief Tests dynamic ASCII classification.
 */
void test_isctype()
{
    xer_assert(xer::isctype(U'A', xer::ctype_id::alpha));
    xer_assert(xer::isctype(U'9', xer::ctype_id::digit));
    xer_assert(xer::isctype(U'B', xer::ctype_id::alnum));
    xer_assert(xer::isctype(U'z', xer::ctype_id::lower));
    xer_assert(xer::isctype(U'Q', xer::ctype_id::upper));
    xer_assert(xer::isctype(U' ', xer::ctype_id::space));
    xer_assert(xer::isctype(U'\t', xer::ctype_id::blank));
    xer_assert(xer::isctype(U'\n', xer::ctype_id::cntrl));
    xer_assert(xer::isctype(U' ', xer::ctype_id::print));
    xer_assert(xer::isctype(U'!', xer::ctype_id::graph));
    xer_assert(xer::isctype(U'!', xer::ctype_id::punct));
    xer_assert(xer::isctype(U'F', xer::ctype_id::xdigit));
    xer_assert(xer::isctype(U'\x7f', xer::ctype_id::ascii));
    xer_assert(xer::isctype(U'7', xer::ctype_id::octal));
    xer_assert(xer::isctype(U'1', xer::ctype_id::binary));

    xer_assert_not(xer::isctype(U'あ', xer::ctype_id::ascii));
    xer_assert_not(xer::isctype(U'G', xer::ctype_id::xdigit));
    xer_assert_not(xer::isctype(U'8', xer::ctype_id::octal));
    xer_assert_not(xer::isctype(U'2', xer::ctype_id::binary));
    xer_assert_not(xer::isctype(U' ', xer::ctype_id::graph));
}

/**
 * @brief Tests successful lowercase conversion.
 */
void test_tolower_success()
{
    const auto r1 = xer::tolower(U'A');
    const auto r2 = xer::tolower(U'Z');
    const auto r3 = xer::tolower(U'a');
    const auto r4 = xer::tolower(U'0');
    const auto r5 = xer::tolower(U'!');

    xer_assert(r1.has_value());
    xer_assert(r2.has_value());
    xer_assert(r3.has_value());
    xer_assert(r4.has_value());
    xer_assert(r5.has_value());

    xer_assert_eq(*r1, U'a');
    xer_assert_eq(*r2, U'z');
    xer_assert_eq(*r3, U'a');
    xer_assert_eq(*r4, U'0');
    xer_assert_eq(*r5, U'!');
}

/**
 * @brief Tests successful uppercase conversion.
 */
void test_toupper_success()
{
    const auto r1 = xer::toupper(U'a');
    const auto r2 = xer::toupper(U'z');
    const auto r3 = xer::toupper(U'A');
    const auto r4 = xer::toupper(U'0');
    const auto r5 = xer::toupper(U'!');

    xer_assert(r1.has_value());
    xer_assert(r2.has_value());
    xer_assert(r3.has_value());
    xer_assert(r4.has_value());
    xer_assert(r5.has_value());

    xer_assert_eq(*r1, U'A');
    xer_assert_eq(*r2, U'Z');
    xer_assert_eq(*r3, U'A');
    xer_assert_eq(*r4, U'0');
    xer_assert_eq(*r5, U'!');
}

/**
 * @brief Tests failed ASCII conversions for non-ASCII input.
 */
void test_to_functions_failure()
{
    const auto r1 = xer::tolower(U'あ');
    const auto r2 = xer::toupper(U'Ω');

    xer_assert_not(r1.has_value());
    xer_assert_not(r2.has_value());

    xer_assert_eq(r1.error().code, xer::error_t::invalid_argument);
    xer_assert_eq(r2.error().code, xer::error_t::invalid_argument);
}

/**
 * @brief Tests dynamic ASCII conversion.
 */
void test_toctrans()
{
    const auto r1 = xer::toctrans(U'A', xer::ctrans_id::lower);
    const auto r2 = xer::toctrans(U'z', xer::ctrans_id::upper);
    const auto r3 = xer::toctrans(U'0', xer::ctrans_id::lower);
    const auto r4 = xer::toctrans(U'!', xer::ctrans_id::upper);
    const auto r5 = xer::toctrans(U'あ', xer::ctrans_id::lower);

    xer_assert(r1.has_value());
    xer_assert(r2.has_value());
    xer_assert(r3.has_value());
    xer_assert(r4.has_value());
    xer_assert_not(r5.has_value());

    xer_assert_eq(*r1, U'a');
    xer_assert_eq(*r2, U'Z');
    xer_assert_eq(*r3, U'0');
    xer_assert_eq(*r4, U'!');
    xer_assert_eq(r5.error().code, xer::error_t::invalid_argument);
}

/**
 * @brief Tests basic type properties.
 */
void test_type_properties()
{
    static_assert(std::same_as<decltype(xer::isalpha(U'A')), bool>);
    static_assert(std::same_as<decltype(xer::isctype(U'A', xer::ctype_id::alpha)), bool>);

    static_assert(std::same_as<
        decltype(xer::tolower(U'A')),
        std::expected<char32_t, xer::error<void>>>);

    static_assert(std::same_as<
        decltype(xer::toupper(U'A')),
        std::expected<char32_t, xer::error<void>>>);

    static_assert(std::same_as<
        decltype(xer::toctrans(U'A', xer::ctrans_id::lower)),
        std::expected<char32_t, xer::error<void>>>);
}

/**
 * @brief Tests constexpr evaluation.
 */
constexpr bool test_constexpr()
{
    if (!xer::isalpha(U'A')) {
        return false;
    }
    if (!xer::isdigit(U'9')) {
        return false;
    }
    if (xer::isalpha(U'0')) {
        return false;
    }
    if (!xer::tolower(U'A').has_value()) {
        return false;
    }
    if (*xer::tolower(U'A') != U'a') {
        return false;
    }
    if (!xer::toupper(U'z').has_value()) {
        return false;
    }
    if (*xer::toupper(U'z') != U'Z') {
        return false;
    }
    if (xer::tolower(U'あ').has_value()) {
        return false;
    }
    if (!xer::isctype(U'F', xer::ctype_id::xdigit)) {
        return false;
    }
    if (!xer::toctrans(U'b', xer::ctrans_id::upper).has_value()) {
        return false;
    }
    if (*xer::toctrans(U'b', xer::ctrans_id::upper) != U'B') {
        return false;
    }

    return true;
}

static_assert(test_constexpr());

} // namespace

int main()
{
    test_is_functions_positive();
    test_is_functions_negative();
    test_is_functions_boundaries();
    test_isctype();
    test_tolower_success();
    test_toupper_success();
    test_to_functions_failure();
    test_toctrans();
    test_type_properties();
}
