// XER_EXAMPLE_BEGIN: json_basic
//
// This example shows basic usage of xer::json_decode
// and xer::json_encode.
//
// Expected output:
// decoded = 2 hello
// encoded = {"ok":true,"items":[1.5,"x"]}

#include <xer/json.h>
#include <xer/stdio.h>

auto main() -> int
{
    const auto decoded =
        xer::json_decode(u8"{\"count\":2,\"message\":\"hello\"}");
    if (!decoded) {
        return 1;
    }

    if (!decoded->is_object()) {
        return 1;
    }

    const auto& object = decoded->as_object();
    if (object.size() != 2) {
        return 1;
    }

    if (!object[0].second.is_number()) {
        return 1;
    }

    if (!object[1].second.is_string()) {
        return 1;
    }

    if (!xer::printf(
            u8"decoded = %.0f %s\n",
            object[0].second.as_number(),
            object[1].second.as_string().data())) {
        return 1;
    }

    xer::json_value::array_type items = {
        xer::json_value(1.5),
        xer::json_value(u8"x"),
    };

    xer::json_value::object_type out = {
        {u8"ok", xer::json_value(true)},
        {u8"items", xer::json_value(std::move(items))},
    };

    const auto encoded = xer::json_encode(xer::json_value(std::move(out)));
    if (!encoded) {
        return 1;
    }

    if (!xer::printf(u8"encoded = %s\n", encoded->c_str())) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: json_basic
