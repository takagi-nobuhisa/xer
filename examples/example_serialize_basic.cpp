#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <xer/serialize.h>
#include <xer/stdio.h>

struct sample_record {
    std::uint32_t id;
    std::u8string name;
    std::array<std::uint32_t, 3> points;
    std::vector<std::uint16_t> flags;
    std::map<std::u8string, double> scores;
};

template<class Archive>
auto xfer(Archive& archive, sample_record& value) -> xer::result<void>
{
    if (auto r = archive(value.id); !r) {
        return std::unexpected(r.error());
    }
    if (auto r = archive(value.name); !r) {
        return std::unexpected(r.error());
    }
    if (auto r = archive(value.points); !r) {
        return std::unexpected(r.error());
    }
    if (auto r = archive(value.flags); !r) {
        return std::unexpected(r.error());
    }
    if (auto r = archive(value.scores); !r) {
        return std::unexpected(r.error());
    }

    return {};
}

// XER_EXAMPLE_BEGIN: serialize_basic
//
// This example transfers a fixed-schema structure to and from binary data.
//
// Expected output:
// id: 42
// name: xer
// byte size: 92
// score count: 2

auto main() -> int
{
    sample_record source{
        42u,
        u8"xer",
        {10u, 20u, 30u},
        {1u, 2u, 3u},
        {{u8"math", 98.5}, {u8"english", 87.25}},
    };

    xer::binary_output_archive output;
    if (auto r = xfer(output, source); !r) {
        return 1;
    }

    const auto bytes = output.release();

    sample_record restored{};
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
    if (!xer::printf(u8"byte size: %@\n", bytes.size()).has_value()) {
        return 1;
    }
    if (!xer::printf(u8"score count: %@\n", restored.scores.size()).has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: serialize_basic
