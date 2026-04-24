/**
 * @file tests/test_typeinfo.cpp
 * @brief Tests for xer/typeinfo.h.
 */

#include <map>
#include <string>
#include <string_view>
#include <typeindex>
#include <typeinfo>
#include <utility>

#include <xer/assert.h>
#include <xer/typeinfo.h>

namespace {

struct sample_type {
};

/**
 * @brief Tests that xer_typeid accepts a type operand.
 */
auto test_xer_typeid_with_type() -> void
{
    const auto info = xer_typeid(int);

    xer_assert_eq(info.name(), std::u8string(u8"int"));
    xer_assert(info.index() == std::type_index(typeid(int)));
}

/**
 * @brief Tests that xer_typeid accepts a template type operand containing a comma.
 */
auto test_xer_typeid_with_template_type() -> void
{
    const auto info = xer_typeid(std::pair<int, long>);
    const auto name = info.name();

    xer_assert(name.find(u8"pair") != std::u8string::npos);
    xer_assert(info.index() == std::type_index(typeid(std::pair<int, long>)));
}

/**
 * @brief Tests that xer_typeid accepts an expression operand.
 */
auto test_xer_typeid_with_expression() -> void
{
    const sample_type value;
    const auto info = xer_typeid(value);
    const auto name = info.name();

    xer_assert(name.find(u8"sample_type") != std::u8string::npos);
    xer_assert(info.index() == std::type_index(typeid(sample_type)));
}

/**
 * @brief Tests equality and inequality.
 */
auto test_comparison_equality() -> void
{
    const auto int_type_1 = xer_typeid(int);
    const auto int_type_2 = xer_typeid(int);
    const auto long_type = xer_typeid(long);

    xer_assert(int_type_1 == int_type_2);
    xer_assert_not(int_type_1 != int_type_2);
    xer_assert(int_type_1 != long_type);
    xer_assert_not(int_type_1 == long_type);
}

/**
 * @brief Tests ordering by using type_info as a std::map key.
 */
auto test_ordering_for_map_key() -> void
{
    std::map<xer::type_info, int> values;

    values.emplace(xer_typeid(int), 1);
    values.emplace(xer_typeid(sample_type), 2);
    values.emplace(xer_typeid(long), 3);

    xer_assert_eq(values.at(xer_typeid(int)), 1);
    xer_assert_eq(values.at(xer_typeid(sample_type)), 2);
    xer_assert_eq(values.at(xer_typeid(long)), 3);
}

/**
 * @brief Tests raw_name() against std::type_index::name().
 */
auto test_raw_name() -> void
{
    const auto info = xer_typeid(sample_type);

    xer_assert_eq(
        std::string_view(info.raw_name()),
        std::string_view(std::type_index(typeid(sample_type)).name()));
}

} // namespace

/**
 * @brief Entry point.
 * @return Zero on success.
 */
auto main() -> int
{
    test_xer_typeid_with_type();
    test_xer_typeid_with_template_type();
    test_xer_typeid_with_expression();
    test_comparison_equality();
    test_ordering_for_map_key();
    test_raw_name();

    return 0;
}
