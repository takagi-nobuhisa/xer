#include <cmath>
#include <limits>
#include <string>

#include <xer/assert.h>
#include <xer/error.h>
#include <xer/json.h>

namespace {

void test_json_decode_null()
{
    const auto result = xer::json_decode(u8"null");

    xer_assert(result.has_value());
    xer_assert(result->is_null());
}

void test_json_decode_boolean()
{
    const auto result_true = xer::json_decode(u8"true");
    const auto result_false = xer::json_decode(u8"false");

    xer_assert(result_true.has_value());
    xer_assert(result_true->is_bool());
    xer_assert(result_true->as_bool());

    xer_assert(result_false.has_value());
    xer_assert(result_false->is_bool());
    xer_assert_not(result_false->as_bool());
}

void test_json_decode_number_as_double()
{
    const auto result = xer::json_decode(u8"-12.5e1");

    xer_assert(result.has_value());
    xer_assert(result->is_number());
    xer_assert_eq(result->as_number(), -125.0);
}

void test_json_decode_string_with_escapes()
{
    const auto result = xer::json_decode(u8"\"A\\nB\\u3042\"");

    xer_assert(result.has_value());
    xer_assert(result->is_string());
    xer_assert_eq(result->as_string(), u8"A\nBあ");
}

void test_json_decode_array()
{
    const auto result = xer::json_decode(u8"[null,true,1.5,\"x\"]");

    xer_assert(result.has_value());
    xer_assert(result->is_array());
    xer_assert_eq(result->as_array().size(), static_cast<std::size_t>(4));
    xer_assert(result->as_array()[0].is_null());
    xer_assert(result->as_array()[1].as_bool());
    xer_assert_eq(result->as_array()[2].as_number(), 1.5);
    xer_assert_eq(result->as_array()[3].as_string(), u8"x");
}

void test_json_decode_object_preserves_order()
{
    const auto result = xer::json_decode(u8"{\"b\":1,\"a\":2}");

    xer_assert(result.has_value());
    xer_assert(result->is_object());
    xer_assert_eq(result->as_object().size(), static_cast<std::size_t>(2));
    xer_assert_eq(result->as_object()[0].first, u8"b");
    xer_assert_eq(result->as_object()[1].first, u8"a");
    xer_assert_eq(result->as_object()[0].second.as_number(), 1.0);
    xer_assert_eq(result->as_object()[1].second.as_number(), 2.0);
}

void test_json_decode_surrogate_pair()
{
    const auto result = xer::json_decode(u8"\"\\uD83D\\uDE00\"");

    xer_assert(result.has_value());
    xer_assert_eq(result->as_string(), u8"😀");
}

void test_json_decode_rejects_trailing_comma()
{
    const auto result = xer::json_decode(u8"[1,]");

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_json_decode_rejects_invalid_unicode_escape()
{
    const auto result = xer::json_decode(u8"\"\\uD800\"");

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::encoding_error);
}

void test_json_encode_scalar_values()
{
    const auto result_null = xer::json_encode(xer::json_value(nullptr));
    const auto result_bool = xer::json_encode(xer::json_value(true));
    const auto result_number = xer::json_encode(xer::json_value(1.25));
    const auto result_string = xer::json_encode(xer::json_value(u8"A\nあ"));

    xer_assert(result_null.has_value());
    xer_assert_eq(*result_null, u8"null");

    xer_assert(result_bool.has_value());
    xer_assert_eq(*result_bool, u8"true");

    xer_assert(result_number.has_value());
    xer_assert_eq(*result_number, u8"1.25");

    xer_assert(result_string.has_value());
    xer_assert_eq(*result_string, u8"\"A\\nあ\"");
}

void test_json_encode_array_and_object()
{
    xer::json_value::array_type array = {
        xer::json_value(nullptr),
        xer::json_value(false),
        xer::json_value(2.0),
    };

    xer::json_value::object_type object = {
        {u8"z", xer::json_value(std::move(array))},
        {u8"a", xer::json_value(u8"x")},
    };

    const auto result = xer::json_encode(xer::json_value(std::move(object)));

    xer_assert(result.has_value());
    xer_assert_eq(*result, u8"{\"z\":[null,false,2],\"a\":\"x\"}");
}

void test_json_encode_rejects_non_finite_number()
{
    const auto result_nan = xer::json_encode(
        xer::json_value(std::numeric_limits<double>::quiet_NaN()));
    const auto result_inf = xer::json_encode(
        xer::json_value(std::numeric_limits<double>::infinity()));

    xer_assert_not(result_nan.has_value());
    xer_assert_eq(result_nan.error().code, xer::error_t::invalid_argument);

    xer_assert_not(result_inf.has_value());
    xer_assert_eq(result_inf.error().code, xer::error_t::invalid_argument);
}

void test_json_round_trip()
{
    const std::u8string source =
        u8"{\"name\":\"xer\",\"values\":[1,2.5,true,null,\"あ\"]}";

    const auto decoded = xer::json_decode(source);
    xer_assert(decoded.has_value());

    const auto encoded = xer::json_encode(*decoded);
    xer_assert(encoded.has_value());
    xer_assert_eq(*encoded, source);
}

void test_json_header_provides_aliases()
{
    xer::json_array array = {xer::json_value(1.0)};
    xer::json_object object = {{u8"k", xer::json_value(std::move(array))}};

    const xer::json_value value(std::move(object));
    xer_assert(value.is_object());
}

} // namespace

int main()
{
    test_json_decode_null();
    test_json_decode_boolean();
    test_json_decode_number_as_double();
    test_json_decode_string_with_escapes();
    test_json_decode_array();
    test_json_decode_object_preserves_order();
    test_json_decode_surrogate_pair();
    test_json_decode_rejects_trailing_comma();
    test_json_decode_rejects_invalid_unicode_escape();
    test_json_encode_scalar_values();
    test_json_encode_array_and_object();
    test_json_encode_rejects_non_finite_number();
    test_json_round_trip();
    test_json_header_provides_aliases();

    return 0;
}
