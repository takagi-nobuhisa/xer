/**
 * @file tests/test_stdfloat.cpp
 * @brief Tests for xer::stdfloat facilities.
 */

#include <concepts>
#include <limits>
#include <type_traits>

#include <xer/assert.h>
#include <xer/stdfloat.h>

namespace {

template<typename T>
concept usable_floating_type =
    std::is_floating_point_v<T> ||
#if XER_HAS_FLOAT128_T
    std::same_as<T, xer::float128_t> ||
#endif
#if XER_HAS_DECIMAL32_T
    std::same_as<T, xer::decimal32_t> ||
#endif
#if XER_HAS_DECIMAL64_T
    std::same_as<T, xer::decimal64_t> ||
#endif
#if XER_HAS_DECIMAL128_T
    std::same_as<T, xer::decimal128_t> ||
#endif
    false;

template<typename T>
void assert_type_is_usable_floating_type() {
    static_assert(usable_floating_type<T>);
}

void test_fixed_binary_float_aliases() {
    assert_type_is_usable_floating_type<xer::float32_t>();
    assert_type_is_usable_floating_type<xer::float64_t>();

    static_assert(sizeof(xer::float32_t) >= sizeof(float));
    static_assert(sizeof(xer::float64_t) >= sizeof(double));
}

void test_optional_binary_float_aliases() {
#if XER_HAS_FLOAT16_T
    assert_type_is_usable_floating_type<xer::float16_t>();
#endif

#if XER_HAS_BFLOAT16_T
    assert_type_is_usable_floating_type<xer::bfloat16_t>();
#endif

#if XER_HAS_FLOAT80_T
    assert_type_is_usable_floating_type<xer::float80_t>();
    static_assert(std::same_as<xer::float80_t, long double>);
#endif

#if XER_HAS_FLOAT128_T
    assert_type_is_usable_floating_type<xer::float128_t>();
#endif
}

void test_least_binary_float_aliases() {
    assert_type_is_usable_floating_type<xer::float_least16_t>();
    assert_type_is_usable_floating_type<xer::float_least32_t>();
    assert_type_is_usable_floating_type<xer::float_least64_t>();

#if XER_HAS_FLOAT16_T
    static_assert(std::same_as<xer::float_least16_t, xer::float16_t>);
#else
    static_assert(std::same_as<xer::float_least16_t, xer::float32_t>);
#endif

    static_assert(std::same_as<xer::float_least32_t, xer::float32_t>);
    static_assert(std::same_as<xer::float_least64_t, xer::float64_t>);

#if XER_HAS_FLOAT_LEAST80_T
    assert_type_is_usable_floating_type<xer::float_least80_t>();

#    if XER_HAS_FLOAT80_T
    static_assert(std::same_as<xer::float_least80_t, xer::float80_t>);
#    elif XER_HAS_FLOAT128_T
    static_assert(std::same_as<xer::float_least80_t, xer::float128_t>);
#    endif
#endif

#if XER_HAS_FLOAT_LEAST128_T
    assert_type_is_usable_floating_type<xer::float_least128_t>();
    static_assert(std::same_as<xer::float_least128_t, xer::float128_t>);
#endif
}

void test_fast_binary_float_aliases() {
    assert_type_is_usable_floating_type<xer::float_fast16_t>();
    assert_type_is_usable_floating_type<xer::float_fast32_t>();
    assert_type_is_usable_floating_type<xer::float_fast64_t>();

#if XER_HAS_FLOAT_FAST80_T
    assert_type_is_usable_floating_type<xer::float_fast80_t>();
#endif

#if XER_HAS_FLOAT_FAST128_T
    assert_type_is_usable_floating_type<xer::float_fast128_t>();
#endif
}

void test_floatmax_alias() {
    assert_type_is_usable_floating_type<xer::floatmax_t>();

#if XER_HAS_FLOAT128_T
    static_assert(std::same_as<xer::floatmax_t, xer::float128_t>);
#elif XER_HAS_FLOAT80_T
    static_assert(std::same_as<xer::floatmax_t, xer::float80_t>);
#else
    static_assert(std::same_as<xer::floatmax_t, long double>);
#endif
}

void test_numeric_limits_helpers_for_binary_float() {
    static_assert(xer::min_of<xer::float32_t> < xer::max_of<xer::float32_t>);
    static_assert(xer::min_of<xer::float64_t> < xer::max_of<xer::float64_t>);

    static_assert(xer::min_of<xer::float32_t> ==
                  std::numeric_limits<xer::float32_t>::lowest());
    static_assert(xer::max_of<xer::float32_t> ==
                  std::numeric_limits<xer::float32_t>::max());
}

void test_fixed_binary_float_literals() {
    using namespace xer::literals::floating_literals;

    const auto f32 = 1.5_f32;
    const auto f64 = 2.5_f64;
    const auto i32 = 3_f32;
    const auto i64 = 4_f64;

    static_assert(std::same_as<decltype(f32), const xer::float32_t>);
    static_assert(std::same_as<decltype(f64), const xer::float64_t>);
    static_assert(std::same_as<decltype(i32), const xer::float32_t>);
    static_assert(std::same_as<decltype(i64), const xer::float64_t>);

    xer_assert(f32 > static_cast<xer::float32_t>(1));
    xer_assert(f64 > static_cast<xer::float64_t>(2));
    xer_assert(i32 == static_cast<xer::float32_t>(3));
    xer_assert(i64 == static_cast<xer::float64_t>(4));

#if XER_HAS_FLOAT16_T
    const auto f16 = 1.5_f16;
    static_assert(std::same_as<decltype(f16), const xer::float16_t>);
#endif

#if XER_HAS_FLOAT80_T
    const auto f80 = 1.5_f80;
    static_assert(std::same_as<decltype(f80), const xer::float80_t>);
#endif

#if XER_HAS_FLOAT128_T
    const auto f128 = 1.5_f128;
    static_assert(std::same_as<decltype(f128), const xer::float128_t>);
#endif

#if XER_HAS_BFLOAT16_T
    const auto bf16 = 1.5_bf16;
    static_assert(std::same_as<decltype(bf16), const xer::bfloat16_t>);
#endif
}

void test_least_binary_float_literals() {
    using namespace xer::literals::floating_literals;

    const auto fl16 = 1.5_fl16;
    const auto fl32 = 2.5_fl32;
    const auto fl64 = 3.5_fl64;
    const auto il16 = 1_fl16;
    const auto il32 = 2_fl32;
    const auto il64 = 3_fl64;

    static_assert(std::same_as<decltype(fl16), const xer::float_least16_t>);
    static_assert(std::same_as<decltype(fl32), const xer::float_least32_t>);
    static_assert(std::same_as<decltype(fl64), const xer::float_least64_t>);
    static_assert(std::same_as<decltype(il16), const xer::float_least16_t>);
    static_assert(std::same_as<decltype(il32), const xer::float_least32_t>);
    static_assert(std::same_as<decltype(il64), const xer::float_least64_t>);

    xer_assert(fl16 > static_cast<xer::float_least16_t>(1));
    xer_assert(fl32 > static_cast<xer::float_least32_t>(2));
    xer_assert(fl64 > static_cast<xer::float_least64_t>(3));
    xer_assert(il16 == static_cast<xer::float_least16_t>(1));
    xer_assert(il32 == static_cast<xer::float_least32_t>(2));
    xer_assert(il64 == static_cast<xer::float_least64_t>(3));

#if XER_HAS_FLOAT_LEAST80_T
    const auto fl80 = 4.5_fl80;
    const auto il80 = 4_fl80;
    static_assert(std::same_as<decltype(fl80), const xer::float_least80_t>);
    static_assert(std::same_as<decltype(il80), const xer::float_least80_t>);
    xer_assert(fl80 > static_cast<xer::float_least80_t>(4));
    xer_assert(il80 == static_cast<xer::float_least80_t>(4));
#endif

#if XER_HAS_FLOAT_LEAST128_T
    const auto fl128 = 5.5_fl128;
    const auto il128 = 5_fl128;
    static_assert(std::same_as<decltype(fl128), const xer::float_least128_t>);
    static_assert(std::same_as<decltype(il128), const xer::float_least128_t>);
    xer_assert(fl128 > static_cast<xer::float_least128_t>(5));
    xer_assert(il128 == static_cast<xer::float_least128_t>(5));
#endif
}

void test_decimal_aliases_if_available() {
#if XER_HAS_DECIMAL32_T
    assert_type_is_usable_floating_type<xer::decimal32_t>();
    assert_type_is_usable_floating_type<xer::decimal_least32_t>();
    assert_type_is_usable_floating_type<xer::decimal_fast32_t>();

    static_assert(std::same_as<xer::decimal_least32_t, xer::decimal32_t>);
    static_assert(std::same_as<xer::decimal_fast32_t, xer::decimal32_t>);
#endif

#if XER_HAS_DECIMAL64_T
    assert_type_is_usable_floating_type<xer::decimal64_t>();
    assert_type_is_usable_floating_type<xer::decimal_least64_t>();
    assert_type_is_usable_floating_type<xer::decimal_fast64_t>();

    static_assert(std::same_as<xer::decimal_least64_t, xer::decimal64_t>);
    static_assert(std::same_as<xer::decimal_fast64_t, xer::decimal64_t>);
#endif

#if XER_HAS_DECIMAL128_T
    assert_type_is_usable_floating_type<xer::decimal128_t>();
    assert_type_is_usable_floating_type<xer::decimal_least128_t>();
    assert_type_is_usable_floating_type<xer::decimal_fast128_t>();
    assert_type_is_usable_floating_type<xer::decimalmax_t>();

    static_assert(std::same_as<xer::decimal_least128_t, xer::decimal128_t>);
    static_assert(std::same_as<xer::decimal_fast128_t, xer::decimal128_t>);
    static_assert(std::same_as<xer::decimalmax_t, xer::decimal128_t>);
#endif
}

} // namespace

auto main() -> int {
    test_fixed_binary_float_aliases();
    test_optional_binary_float_aliases();
    test_least_binary_float_aliases();
    test_fast_binary_float_aliases();
    test_floatmax_alias();
    test_numeric_limits_helpers_for_binary_float();
    test_fixed_binary_float_literals();
    test_least_binary_float_literals();
    test_decimal_aliases_if_available();
    return 0;
}
