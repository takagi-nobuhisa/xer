/**
 * @file tests/test_braille_symbols.cpp
 * @brief Tests for braille sign constants.
 */

#include <xer/assert.h>
#include <xer/braille.h>

namespace {

void test_general_braille_indicators()
{
    static_assert(xer::braille::numeric_indicator == u8"⠼");
    static_assert(xer::braille::alphabetic_indicator == u8"⠰");
    static_assert(xer::braille::capital_indicator == u8"⠠");
    static_assert(xer::braille::double_capital_indicator == u8"⠠⠠");

    xer_assert(xer::braille::numeric_indicator.size() == 3);
    xer_assert(xer::braille::alphabetic_indicator.size() == 3);
    xer_assert(xer::braille::capital_indicator.size() == 3);
    xer_assert(xer::braille::double_capital_indicator.size() == 6);
}

void test_information_processing_braille_indicators()
{
    namespace ip = xer::braille::information_processing;

    static_assert(ip::lowercase_indicator == u8"⠰");
    static_assert(ip::uppercase_indicator == u8"⠠");
    static_assert(ip::single_uppercase_indicator == u8"⠠");
    static_assert(ip::double_uppercase_indicator == u8"⠠⠠");
    static_assert(ip::numeric_indicator == u8"⠼");

    xer_assert(ip::lowercase_indicator == xer::braille::alphabetic_indicator);
    xer_assert(ip::uppercase_indicator == xer::braille::capital_indicator);
    xer_assert(ip::single_uppercase_indicator == xer::braille::capital_indicator);
    xer_assert(ip::double_uppercase_indicator == xer::braille::double_capital_indicator);
    xer_assert(ip::numeric_indicator == xer::braille::numeric_indicator);
}

} // namespace

auto main() -> int
{
    test_general_braille_indicators();
    test_information_processing_braille_indicators();
    return 0;
}
