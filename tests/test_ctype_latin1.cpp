#include <expected>

#include <xer/assert.h>
#include <xer/ctype.h>
#include <xer/error.h>

namespace {

void test_latin1_upper_includes_ascii()
{
    xer_assert(xer::isctype(U'A', xer::ctype_id::latin1_upper));
    xer_assert(xer::isctype(U'Z', xer::ctype_id::latin1_upper));
    xer_assert_not(xer::isctype(U'a', xer::ctype_id::latin1_upper));
}

void test_latin1_lower_includes_ascii()
{
    xer_assert(xer::isctype(U'a', xer::ctype_id::latin1_lower));
    xer_assert(xer::isctype(U'z', xer::ctype_id::latin1_lower));
    xer_assert_not(xer::isctype(U'A', xer::ctype_id::latin1_lower));
}

void test_latin1_alpha_includes_ascii()
{
    xer_assert(xer::isctype(U'A', xer::ctype_id::latin1_alpha));
    xer_assert(xer::isctype(U'z', xer::ctype_id::latin1_alpha));
    xer_assert_not(xer::isctype(U'0', xer::ctype_id::latin1_alpha));
    xer_assert_not(xer::isctype(U'!', xer::ctype_id::latin1_alpha));
}

void test_latin1_alnum_includes_ascii_digits()
{
    xer_assert(xer::isctype(U'A', xer::ctype_id::latin1_alnum));
    xer_assert(xer::isctype(U'z', xer::ctype_id::latin1_alnum));
    xer_assert(xer::isctype(U'0', xer::ctype_id::latin1_alnum));
    xer_assert(xer::isctype(U'9', xer::ctype_id::latin1_alnum));
    xer_assert_not(xer::isctype(U'!', xer::ctype_id::latin1_alnum));
}

void test_latin1_upper_letters()
{
    xer_assert(xer::isctype(U'\u00c0', xer::ctype_id::latin1_upper));
    xer_assert(xer::isctype(U'\u00d6', xer::ctype_id::latin1_upper));
    xer_assert(xer::isctype(U'\u00d8', xer::ctype_id::latin1_upper));
    xer_assert(xer::isctype(U'\u00de', xer::ctype_id::latin1_upper));

    xer_assert_not(xer::isctype(U'\u00d7', xer::ctype_id::latin1_upper));
    xer_assert_not(xer::isctype(U'\u00df', xer::ctype_id::latin1_upper));
    xer_assert_not(xer::isctype(U'\u00e0', xer::ctype_id::latin1_upper));
}

void test_latin1_lower_letters()
{
    xer_assert(xer::isctype(U'\u00e0', xer::ctype_id::latin1_lower));
    xer_assert(xer::isctype(U'\u00f6', xer::ctype_id::latin1_lower));
    xer_assert(xer::isctype(U'\u00f8', xer::ctype_id::latin1_lower));
    xer_assert(xer::isctype(U'\u00ff', xer::ctype_id::latin1_lower));
    xer_assert(xer::isctype(U'\u00aa', xer::ctype_id::latin1_lower));
    xer_assert(xer::isctype(U'\u00ba', xer::ctype_id::latin1_lower));
    xer_assert(xer::isctype(U'\u00df', xer::ctype_id::latin1_lower));

    xer_assert_not(xer::isctype(U'\u00d7', xer::ctype_id::latin1_lower));
    xer_assert_not(xer::isctype(U'\u00f7', xer::ctype_id::latin1_lower));
    xer_assert_not(xer::isctype(U'\u00c9', xer::ctype_id::latin1_lower));
}

void test_latin1_alpha_letters()
{
    xer_assert(xer::isctype(U'\u00c9', xer::ctype_id::latin1_alpha));
    xer_assert(xer::isctype(U'\u00e9', xer::ctype_id::latin1_alpha));
    xer_assert(xer::isctype(U'\u00df', xer::ctype_id::latin1_alpha));
    xer_assert(xer::isctype(U'\u00aa', xer::ctype_id::latin1_alpha));
    xer_assert(xer::isctype(U'\u00ba', xer::ctype_id::latin1_alpha));

    xer_assert_not(xer::isctype(U'\u00d7', xer::ctype_id::latin1_alpha));
    xer_assert_not(xer::isctype(U'\u00f7', xer::ctype_id::latin1_alpha));
    xer_assert_not(xer::isctype(U'\u00b2', xer::ctype_id::latin1_alpha));
}

void test_latin1_alnum_letters_and_digits()
{
    xer_assert(xer::isctype(U'\u00c9', xer::ctype_id::latin1_alnum));
    xer_assert(xer::isctype(U'\u00e9', xer::ctype_id::latin1_alnum));
    xer_assert(xer::isctype(U'\u00df', xer::ctype_id::latin1_alnum));
    xer_assert(xer::isctype(U'7', xer::ctype_id::latin1_alnum));

    xer_assert_not(xer::isctype(U'\u00b2', xer::ctype_id::latin1_alnum));
    xer_assert_not(xer::isctype(U'\u00d7', xer::ctype_id::latin1_alnum));
    xer_assert_not(xer::isctype(U' ', xer::ctype_id::latin1_alnum));
}

void test_latin1_print_includes_ascii_print()
{
    xer_assert(xer::isctype(U' ', xer::ctype_id::latin1_print));
    xer_assert(xer::isctype(U'~', xer::ctype_id::latin1_print));
    xer_assert_not(xer::isctype(U'\n', xer::ctype_id::latin1_print));
}

void test_latin1_graph_includes_ascii_graph()
{
    xer_assert(xer::isctype(U'!', xer::ctype_id::latin1_graph));
    xer_assert(xer::isctype(U'~', xer::ctype_id::latin1_graph));
    xer_assert_not(xer::isctype(U' ', xer::ctype_id::latin1_graph));
}

void test_latin1_print_extended_range()
{
    xer_assert(xer::isctype(U'\u00a0', xer::ctype_id::latin1_print));
    xer_assert(xer::isctype(U'\u00ad', xer::ctype_id::latin1_print));
    xer_assert(xer::isctype(U'\u00ff', xer::ctype_id::latin1_print));

    xer_assert_not(xer::isctype(U'\u009f', xer::ctype_id::latin1_print));
    xer_assert_not(xer::isctype(U'\u0100', xer::ctype_id::latin1_print));
}

void test_latin1_graph_extended_range()
{
    xer_assert(xer::isctype(U'\u00a1', xer::ctype_id::latin1_graph));
    xer_assert(xer::isctype(U'\u00ad', xer::ctype_id::latin1_graph));
    xer_assert(xer::isctype(U'\u00ff', xer::ctype_id::latin1_graph));

    xer_assert_not(xer::isctype(U'\u00a0', xer::ctype_id::latin1_graph));
    xer_assert_not(xer::isctype(U'\u009f', xer::ctype_id::latin1_graph));
    xer_assert_not(xer::isctype(U'\u0100', xer::ctype_id::latin1_graph));
}

void test_ascii_only_kinds_remain_ascii_only()
{
    xer_assert_not(xer::isctype(U'\u00c9', xer::ctype_id::upper));
    xer_assert_not(xer::isctype(U'\u00e9', xer::ctype_id::lower));
    xer_assert_not(xer::isctype(U'\u00df', xer::ctype_id::alpha));
    xer_assert_not(xer::isctype(U'\u00a1', xer::ctype_id::graph));
    xer_assert_not(xer::isctype(U'\u00a0', xer::ctype_id::print));
}

void test_non_latin1_code_points_are_not_matched()
{
    xer_assert_not(xer::isctype(U'\u0100', xer::ctype_id::latin1_upper));
    xer_assert_not(xer::isctype(U'\u0101', xer::ctype_id::latin1_lower));
    xer_assert_not(xer::isctype(U'\u03a9', xer::ctype_id::latin1_alpha));
    xer_assert_not(xer::isctype(U'\u3042', xer::ctype_id::latin1_print));
    xer_assert_not(xer::isctype(U'\u4e00', xer::ctype_id::latin1_graph));
}

void test_toctrans_latin1_lowercase_includes_ascii()
{
    const auto r1 = xer::toctrans(U'A', xer::ctrans_id::latin1_lower);
    const auto r2 = xer::toctrans(U'Z', xer::ctrans_id::latin1_lower);
    const auto r3 = xer::toctrans(U'a', xer::ctrans_id::latin1_lower);
    const auto r4 = xer::toctrans(U'0', xer::ctrans_id::latin1_lower);
    const auto r5 = xer::toctrans(U'!', xer::ctrans_id::latin1_lower);

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

void test_toctrans_latin1_uppercase_includes_ascii()
{
    const auto r1 = xer::toctrans(U'a', xer::ctrans_id::latin1_upper);
    const auto r2 = xer::toctrans(U'z', xer::ctrans_id::latin1_upper);
    const auto r3 = xer::toctrans(U'A', xer::ctrans_id::latin1_upper);
    const auto r4 = xer::toctrans(U'0', xer::ctrans_id::latin1_upper);
    const auto r5 = xer::toctrans(U'!', xer::ctrans_id::latin1_upper);

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

void test_toctrans_latin1_lowercase_basic_letters()
{
    const auto r1 = xer::toctrans(U'\u00c0', xer::ctrans_id::latin1_lower);
    const auto r2 = xer::toctrans(U'\u00d6', xer::ctrans_id::latin1_lower);
    const auto r3 = xer::toctrans(U'\u00d8', xer::ctrans_id::latin1_lower);
    const auto r4 = xer::toctrans(U'\u00de', xer::ctrans_id::latin1_lower);

    xer_assert(r1.has_value());
    xer_assert(r2.has_value());
    xer_assert(r3.has_value());
    xer_assert(r4.has_value());

    xer_assert_eq(*r1, U'\u00e0');
    xer_assert_eq(*r2, U'\u00f6');
    xer_assert_eq(*r3, U'\u00f8');
    xer_assert_eq(*r4, U'\u00fe');
}

void test_toctrans_latin1_uppercase_basic_letters()
{
    const auto r1 = xer::toctrans(U'\u00e0', xer::ctrans_id::latin1_upper);
    const auto r2 = xer::toctrans(U'\u00f6', xer::ctrans_id::latin1_upper);
    const auto r3 = xer::toctrans(U'\u00f8', xer::ctrans_id::latin1_upper);
    const auto r4 = xer::toctrans(U'\u00fe', xer::ctrans_id::latin1_upper);

    xer_assert(r1.has_value());
    xer_assert(r2.has_value());
    xer_assert(r3.has_value());
    xer_assert(r4.has_value());

    xer_assert_eq(*r1, U'\u00c0');
    xer_assert_eq(*r2, U'\u00d6');
    xer_assert_eq(*r3, U'\u00d8');
    xer_assert_eq(*r4, U'\u00de');
}

void test_toctrans_latin1_preserves_non_convertible_supported_chars()
{
    const auto r1 = xer::toctrans(U'\u00d7', xer::ctrans_id::latin1_lower);
    const auto r2 = xer::toctrans(U'\u00f7', xer::ctrans_id::latin1_upper);
    const auto r3 = xer::toctrans(U'\u00aa', xer::ctrans_id::latin1_upper);
    const auto r4 = xer::toctrans(U'\u00ba', xer::ctrans_id::latin1_lower);
    const auto r5 = xer::toctrans(U'\u00a0', xer::ctrans_id::latin1_upper);

    xer_assert(r1.has_value());
    xer_assert(r2.has_value());
    xer_assert(r3.has_value());
    xer_assert(r4.has_value());
    xer_assert(r5.has_value());

    xer_assert_eq(*r1, U'\u00d7');
    xer_assert_eq(*r2, U'\u00f7');
    xer_assert_eq(*r3, U'\u00aa');
    xer_assert_eq(*r4, U'\u00ba');
    xer_assert_eq(*r5, U'\u00a0');
}

void test_toctrans_latin1_sharp_s_special_case()
{
    const auto r1 = xer::toctrans(U'\u00df', xer::ctrans_id::latin1_upper);
    const auto r2 = xer::toctrans(U'\u1e9e', xer::ctrans_id::latin1_lower);

    xer_assert(r1.has_value());
    xer_assert(r2.has_value());

    xer_assert_eq(*r1, U'\u1e9e');
    xer_assert_eq(*r2, U'\u00df');
}

void test_toctrans_ascii_modes_remain_ascii_only()
{
    const auto r1 = xer::toctrans(U'\u00c9', xer::ctrans_id::lower);
    const auto r2 = xer::toctrans(U'\u00e9', xer::ctrans_id::upper);
    const auto r3 = xer::toctrans(U'\u00df', xer::ctrans_id::upper);

    xer_assert_not(r1.has_value());
    xer_assert_not(r2.has_value());
    xer_assert_not(r3.has_value());

    xer_assert_eq(r1.error().code, xer::error_t::invalid_argument);
    xer_assert_eq(r2.error().code, xer::error_t::invalid_argument);
    xer_assert_eq(r3.error().code, xer::error_t::invalid_argument);
}

void test_toctrans_latin1_rejects_non_latin1_code_points()
{
    const auto r1 = xer::toctrans(U'\u0100', xer::ctrans_id::latin1_lower);
    const auto r2 = xer::toctrans(U'\u03a9', xer::ctrans_id::latin1_upper);
    const auto r3 = xer::toctrans(U'\u3042', xer::ctrans_id::latin1_lower);

    xer_assert_not(r1.has_value());
    xer_assert_not(r2.has_value());
    xer_assert_not(r3.has_value());

    xer_assert_eq(r1.error().code, xer::error_t::invalid_argument);
    xer_assert_eq(r2.error().code, xer::error_t::invalid_argument);
    xer_assert_eq(r3.error().code, xer::error_t::invalid_argument);
}

void test_type_properties()
{
    static_assert(std::same_as<
                  decltype(xer::toctrans(U'A', xer::ctrans_id::latin1_lower)),
                  std::expected<char32_t, xer::error<void>>>);

    static_assert(std::same_as<
                  decltype(xer::toctrans(U'a', xer::ctrans_id::latin1_upper)),
                  std::expected<char32_t, xer::error<void>>>);
}

constexpr auto test_constexpr() -> bool
{
    if (!xer::toctrans(U'A', xer::ctrans_id::latin1_lower).has_value()) {
        return false;
    }
    if (*xer::toctrans(U'A', xer::ctrans_id::latin1_lower) != U'a') {
        return false;
    }
    if (!xer::toctrans(U'\u00c9', xer::ctrans_id::latin1_lower)
             .has_value()) {
        return false;
    }
    if (*xer::toctrans(U'\u00c9', xer::ctrans_id::latin1_lower) !=
        U'\u00e9') {
        return false;
    }
    if (!xer::toctrans(U'\u00df', xer::ctrans_id::latin1_upper)
             .has_value()) {
        return false;
    }
    if (*xer::toctrans(U'\u00df', xer::ctrans_id::latin1_upper) !=
        U'\u1e9e') {
        return false;
    }
    if (!xer::toctrans(U'\u1e9e', xer::ctrans_id::latin1_lower)
             .has_value()) {
        return false;
    }
    if (*xer::toctrans(U'\u1e9e', xer::ctrans_id::latin1_lower) !=
        U'\u00df') {
        return false;
    }
    if (xer::toctrans(U'\u3042', xer::ctrans_id::latin1_lower)
            .has_value()) {
        return false;
    }

    return true;
}

static_assert(test_constexpr());

} // namespace

int main()
{
    test_latin1_upper_includes_ascii();
    test_latin1_lower_includes_ascii();
    test_latin1_alpha_includes_ascii();
    test_latin1_alnum_includes_ascii_digits();

    test_latin1_upper_letters();
    test_latin1_lower_letters();
    test_latin1_alpha_letters();
    test_latin1_alnum_letters_and_digits();

    test_latin1_print_includes_ascii_print();
    test_latin1_graph_includes_ascii_graph();
    test_latin1_print_extended_range();
    test_latin1_graph_extended_range();

    test_ascii_only_kinds_remain_ascii_only();
    test_non_latin1_code_points_are_not_matched();

    test_toctrans_latin1_lowercase_includes_ascii();
    test_toctrans_latin1_uppercase_includes_ascii();
    test_toctrans_latin1_lowercase_basic_letters();
    test_toctrans_latin1_uppercase_basic_letters();
    test_toctrans_latin1_preserves_non_convertible_supported_chars();
    test_toctrans_latin1_sharp_s_special_case();
    test_toctrans_ascii_modes_remain_ascii_only();
    test_toctrans_latin1_rejects_non_latin1_code_points();
    test_type_properties();

    return 0;
}
