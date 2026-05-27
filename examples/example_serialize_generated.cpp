#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <xer/serialize.h>
#include <xer/stdio.h>

#include "example_serialize_generated.hpp"

// XER_EXAMPLE_BEGIN: serialize_generated
//
// This example uses a header generated from example_serialize_generated_schema.php.
//
// Regenerate the header with:
//   php php/generate_xfer_struct.php \
//       examples/example_serialize_generated_schema.php \
//       examples/example_serialize_generated.hpp
//
// Expected output:
// id: 1001
// name: generated
// payload bytes: 4
// score count: 2
// history count: 3

auto main() -> int
{
    xer_example::generated_record source{
        1001u,
        u8"generated",
        {std::byte{0x10}, std::byte{0x20}, std::byte{0x30}, std::byte{0x40}},
        {{u8"math", 91.5}, {u8"science", 88.25}},
        {7u, 8u, 9u},
        {1u, 2u, 3u, 4u},
    };

    xer::binary_output_archive output;
    if (auto r = xfer(output, source); !r) {
        return 1;
    }

    const auto bytes = output.release();

    xer_example::generated_record restored{};
    xer::binary_input_archive input(bytes);
    if (auto r = xfer(input, restored); !r) {
        return 1;
    }
    if (!input.empty()) {
        return 1;
    }

    if (!xer::printf(u8"id: %@\n", restored.id).has_value()) {
        return 1;
    }
    if (!xer::printf(u8"name: %@\n", restored.name).has_value()) {
        return 1;
    }
    if (!xer::printf(u8"payload bytes: %@\n", restored.payload.size()).has_value()) {
        return 1;
    }
    if (!xer::printf(u8"score count: %@\n", restored.scores.size()).has_value()) {
        return 1;
    }
    if (!xer::printf(u8"history count: %@\n", restored.history.size()).has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: serialize_generated
