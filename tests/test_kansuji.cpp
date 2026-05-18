#include <array>
#include <cstdint>
#include <limits>

#include <xer/assert.h>
#include <xer/error.h>
#include <xer/kansuji.h>

namespace {

void test_to_kansuji_zero()
{
    xer_assert_eq(xer::to_kansuji(0, xer::k10), u8"0");
    xer_assert_eq(xer::to_kansuji(0, xer::k十), u8"零");
    xer_assert_eq(xer::to_kansuji(0, xer::k一〇), u8"〇");
    xer_assert_eq(xer::to_kansuji(0, xer::k拾), u8"零");
}

void test_to_kansuji_representative_styles()
{
    constexpr std::uint64_t value = UINT64_C(123456789012);

    xer_assert_eq(
        xer::to_kansuji(value, xer::k10),
        u8"1234億5678万9012");
    xer_assert_eq(
        xer::to_kansuji(value, xer::k十),
        u8"千二百三十四億五千六百七十八万九千十二");
    xer_assert_eq(
        xer::to_kansuji(value, xer::k一〇),
        u8"一二三四億五六七八万九〇一二");
    xer_assert_eq(
        xer::to_kansuji(value, xer::k拾),
        u8"壱千弐百参拾四億五千六百七拾八万九千壱拾弐");
}

void test_to_kansuji_ordinary_positional_omits_leading_one()
{
    xer_assert_eq(xer::to_kansuji(10, xer::k十), u8"十");
    xer_assert_eq(xer::to_kansuji(100, xer::k十), u8"百");
    xer_assert_eq(xer::to_kansuji(1000, xer::k十), u8"千");
    xer_assert_eq(xer::to_kansuji(110, xer::k十), u8"百十");
}

void test_to_kansuji_daiji_keeps_leading_ichi()
{
    xer_assert_eq(xer::to_kansuji(10, xer::k拾), u8"壱拾");
    xer_assert_eq(xer::to_kansuji(100, xer::k拾), u8"壱百");
    xer_assert_eq(xer::to_kansuji(1000, xer::k拾), u8"壱千");
    xer_assert_eq(xer::to_kansuji(110, xer::k拾), u8"壱百壱拾");
}

void test_to_kansuji_digitwise_uses_circle_zero()
{
    xer_assert_eq(xer::to_kansuji(10, xer::k一〇), u8"一〇");
    xer_assert_eq(xer::to_kansuji(2026, xer::k一〇), u8"二〇二六");
    xer_assert_eq(xer::to_kansuji(9012, xer::k一〇), u8"九〇一二");
}

void test_from_kansuji_whole_zero()
{
    const auto r1 = xer::from_kansuji(u8"0");
    const auto r2 = xer::from_kansuji(u8"零");
    const auto r3 = xer::from_kansuji(u8"〇");

    xer_assert(r1.has_value());
    xer_assert(r2.has_value());
    xer_assert(r3.has_value());

    xer_assert_eq(*r1, UINT64_C(0));
    xer_assert_eq(*r2, UINT64_C(0));
    xer_assert_eq(*r3, UINT64_C(0));
}

void test_from_kansuji_accepts_short_large_unit_groups()
{
    const auto r1 = xer::from_kansuji(u8"12億34万5");
    const auto r2 = xer::from_kansuji(u8"一二億三四万五");
    const auto r3 = xer::from_kansuji(u8"十二億三十四万五");

    xer_assert(r1.has_value());
    xer_assert(r2.has_value());
    xer_assert(r3.has_value());

    xer_assert_eq(*r1, UINT64_C(1200340005));
    xer_assert_eq(*r2, UINT64_C(1200340005));
    xer_assert_eq(*r3, UINT64_C(1200340005));
}

void test_from_kansuji_accepts_ordinary_leading_one_variants()
{
    const auto r1 = xer::from_kansuji(u8"十");
    const auto r2 = xer::from_kansuji(u8"一十");
    const auto r3 = xer::from_kansuji(u8"百");
    const auto r4 = xer::from_kansuji(u8"一百");
    const auto r5 = xer::from_kansuji(u8"千");
    const auto r6 = xer::from_kansuji(u8"一千");

    xer_assert(r1.has_value());
    xer_assert(r2.has_value());
    xer_assert(r3.has_value());
    xer_assert(r4.has_value());
    xer_assert(r5.has_value());
    xer_assert(r6.has_value());

    xer_assert_eq(*r1, UINT64_C(10));
    xer_assert_eq(*r2, UINT64_C(10));
    xer_assert_eq(*r3, UINT64_C(100));
    xer_assert_eq(*r4, UINT64_C(100));
    xer_assert_eq(*r5, UINT64_C(1000));
    xer_assert_eq(*r6, UINT64_C(1000));
}

void test_from_kansuji_accepts_practical_daiji()
{
    const auto r1 = xer::from_kansuji(u8"拾");
    const auto r2 = xer::from_kansuji(u8"壱拾");
    const auto r3 = xer::from_kansuji(u8"百拾");
    const auto r4 = xer::from_kansuji(u8"壱百壱拾");
    const auto r5 = xer::from_kansuji(u8"阡佰拾");
    const auto r6 = xer::from_kansuji(u8"壱阡壱佰壱拾");
    const auto r7 = xer::from_kansuji(u8"壱阡弐佰参拾四萬五阡六佰七拾八");

    xer_assert(r1.has_value());
    xer_assert(r2.has_value());
    xer_assert(r3.has_value());
    xer_assert(r4.has_value());
    xer_assert(r5.has_value());
    xer_assert(r6.has_value());
    xer_assert(r7.has_value());

    xer_assert_eq(*r1, UINT64_C(10));
    xer_assert_eq(*r2, UINT64_C(10));
    xer_assert_eq(*r3, UINT64_C(110));
    xer_assert_eq(*r4, UINT64_C(110));
    xer_assert_eq(*r5, UINT64_C(1110));
    xer_assert_eq(*r6, UINT64_C(1110));
    xer_assert_eq(*r7, UINT64_C(12345678));
}

void test_from_kansuji_rejects_invalid_text()
{
    const auto r1 = xer::from_kansuji(u8"");
    const auto r2 = xer::from_kansuji(u8"肆");
    const auto r3 = xer::from_kansuji(u8"万");
    const auto r4 = xer::from_kansuji(u8"億");
    const auto r5 = xer::from_kansuji(u8"一億万");
    const auto r6 = xer::from_kansuji(u8"一万億");
    const auto r7 = xer::from_kansuji(u8"一億〇");
    const auto r8 = xer::from_kansuji(u8"十百");
    const auto r9 = xer::from_kansuji(u8"十二三");

    xer_assert_not(r1.has_value());
    xer_assert_not(r2.has_value());
    xer_assert_not(r3.has_value());
    xer_assert_not(r4.has_value());
    xer_assert_not(r5.has_value());
    xer_assert_not(r6.has_value());
    xer_assert_not(r7.has_value());
    xer_assert_not(r8.has_value());
    xer_assert_not(r9.has_value());

    xer_assert_eq(r1.error().code, xer::error_t::invalid_argument);
    xer_assert_eq(r2.error().code, xer::error_t::invalid_argument);
    xer_assert_eq(r3.error().code, xer::error_t::invalid_argument);
    xer_assert_eq(r4.error().code, xer::error_t::invalid_argument);
    xer_assert_eq(r5.error().code, xer::error_t::invalid_argument);
    xer_assert_eq(r6.error().code, xer::error_t::invalid_argument);
    xer_assert_eq(r7.error().code, xer::error_t::invalid_argument);
    xer_assert_eq(r8.error().code, xer::error_t::invalid_argument);
    xer_assert_eq(r9.error().code, xer::error_t::invalid_argument);
}

void test_from_kansuji_rejects_zero_padded_groups()
{
    const auto r1 = xer::from_kansuji(u8"1億0001万1");
    const auto r2 = xer::from_kansuji(u8"一億〇〇〇一万一");
    const auto r3 = xer::from_kansuji(u8"00");
    const auto r4 = xer::from_kansuji(u8"〇〇");

    xer_assert_not(r1.has_value());
    xer_assert_not(r2.has_value());
    xer_assert_not(r3.has_value());
    xer_assert_not(r4.has_value());

    xer_assert_eq(r1.error().code, xer::error_t::invalid_argument);
    xer_assert_eq(r2.error().code, xer::error_t::invalid_argument);
    xer_assert_eq(r3.error().code, xer::error_t::invalid_argument);
    xer_assert_eq(r4.error().code, xer::error_t::invalid_argument);
}

void test_from_kansuji_reports_overflow()
{
    const auto result =
        xer::from_kansuji(u8"1844京6744兆737億955万1616");

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::overflow_error);
}

void test_kansuji_round_trip_all_styles()
{
    constexpr std::array<std::uint64_t, 14> values = {
        UINT64_C(0),
        UINT64_C(1),
        UINT64_C(9),
        UINT64_C(10),
        UINT64_C(11),
        UINT64_C(20),
        UINT64_C(101),
        UINT64_C(110),
        UINT64_C(9999),
        UINT64_C(10000),
        UINT64_C(10001),
        UINT64_C(123456789012),
        UINT64_C(1844674407370955161),
        std::numeric_limits<std::uint64_t>::max(),
    };

    constexpr std::array<xer::kansuji_style, 4> styles = {
        xer::k10,
        xer::k十,
        xer::k一〇,
        xer::k拾,
    };

    for (const auto value : values) {
        for (const auto style : styles) {
            const auto text = xer::to_kansuji(value, style);
            const auto parsed = xer::from_kansuji(text);

            xer_assert(parsed.has_value());
            xer_assert_eq(*parsed, value);
        }
    }
}

} // namespace

auto main() -> int
{
    test_to_kansuji_zero();
    test_to_kansuji_representative_styles();
    test_to_kansuji_ordinary_positional_omits_leading_one();
    test_to_kansuji_daiji_keeps_leading_ichi();
    test_to_kansuji_digitwise_uses_circle_zero();
    test_from_kansuji_whole_zero();
    test_from_kansuji_accepts_short_large_unit_groups();
    test_from_kansuji_accepts_ordinary_leading_one_variants();
    test_from_kansuji_accepts_practical_daiji();
    test_from_kansuji_rejects_invalid_text();
    test_from_kansuji_rejects_zero_padded_groups();
    test_from_kansuji_reports_overflow();
    test_kansuji_round_trip_all_styles();

    return 0;
}
