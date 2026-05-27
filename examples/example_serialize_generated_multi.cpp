#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <xer/serialize.h>
#include <xer/stdio.h>

#include "example_serialize_generated_multi.hpp"

// XER_EXAMPLE_BEGIN: serialize_generated_multi
//
// This example uses a schema file that generates multiple structs.
//
// Regenerate the header with:
//   php php/generate_xfer_struct.php \
//       examples/example_serialize_generated_multi_schema.php \
//       examples/example_serialize_generated_multi.hpp
//
// Expected output:
// version: 1
// kind: 7
// sample name: sensor
// value count: 3
// raw count: 8

auto main() -> int
{
    xer_example_multi::packet_header header{
        1u,
        7u,
        100u,
    };

    xer_example_multi::sensor_sample sample{
        42u,
        u8"sensor",
        {1.5f, 2.5f, 3.5f},
        {{u8"offset", 0.125}, {u8"scale", 2.0}},
        {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u},
    };

    xer::binary_output_archive output;
    if (auto r = xfer(output, header); !r) {
        return 1;
    }
    if (auto r = xfer(output, sample); !r) {
        return 1;
    }

    const auto bytes = output.release();

    xer_example_multi::packet_header restored_header{};
    xer_example_multi::sensor_sample restored_sample{};

    xer::binary_input_archive input(bytes);
    if (auto r = xfer(input, restored_header); !r) {
        return 1;
    }
    if (auto r = xfer(input, restored_sample); !r) {
        return 1;
    }
    if (!input.empty()) {
        return 1;
    }

    if (!xer::printf(u8"version: %@\n", restored_header.version).has_value()) {
        return 1;
    }
    if (!xer::printf(u8"kind: %@\n", restored_header.kind).has_value()) {
        return 1;
    }
    if (!xer::printf(u8"sample name: %@\n", restored_sample.name).has_value()) {
        return 1;
    }
    if (!xer::printf(u8"value count: %@\n", restored_sample.values.size()).has_value()) {
        return 1;
    }
    if (!xer::printf(u8"raw count: %@\n", restored_sample.raw.size()).has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: serialize_generated_multi
