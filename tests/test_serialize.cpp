#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <span>
#include <string>
#include <vector>

#include <xer/assert.h>
#include <xer/error.h>
#include <xer/serialize.h>

namespace {

[[nodiscard]] auto byte_value(std::byte value) -> std::uint8_t
{
    return static_cast<std::uint8_t>(value);
}

void test_scalar_little_endian()
{
    xer::binary_output_archive out;

    xer_assert(out(std::uint16_t{0x1234}).has_value());
    xer_assert(out(std::uint32_t{0x12345678}).has_value());
    xer_assert(out(std::uint64_t{0x0102030405060708ull}).has_value());
    xer_assert(out(std::int16_t{-2}).has_value());

    const auto bytes = out.bytes();
    xer_assert_eq(bytes.size(), 16u);

    xer_assert_eq(byte_value(bytes[0]), 0x34u);
    xer_assert_eq(byte_value(bytes[1]), 0x12u);

    xer_assert_eq(byte_value(bytes[2]), 0x78u);
    xer_assert_eq(byte_value(bytes[3]), 0x56u);
    xer_assert_eq(byte_value(bytes[4]), 0x34u);
    xer_assert_eq(byte_value(bytes[5]), 0x12u);

    xer_assert_eq(byte_value(bytes[6]), 0x08u);
    xer_assert_eq(byte_value(bytes[7]), 0x07u);
    xer_assert_eq(byte_value(bytes[8]), 0x06u);
    xer_assert_eq(byte_value(bytes[9]), 0x05u);
    xer_assert_eq(byte_value(bytes[10]), 0x04u);
    xer_assert_eq(byte_value(bytes[11]), 0x03u);
    xer_assert_eq(byte_value(bytes[12]), 0x02u);
    xer_assert_eq(byte_value(bytes[13]), 0x01u);

    xer_assert_eq(byte_value(bytes[14]), 0xfeu);
    xer_assert_eq(byte_value(bytes[15]), 0xffu);
}

void test_scalar_round_trip()
{
    xer::binary_output_archive out;

    bool b = true;
    std::uint8_t u8 = 0xa5u;
    std::uint16_t u16 = 0x1234u;
    std::uint32_t u32 = 0x12345678u;
    std::uint64_t u64 = 0x0102030405060708ull;
    std::int8_t i8 = -12;
    std::int16_t i16 = -1234;
    std::int32_t i32 = -12345678;
    std::int64_t i64 = -123456789012345ll;
    float f32 = 1.25f;
    double f64 = -2.5;

    xer_assert(out(b).has_value());
    xer_assert(out(u8).has_value());
    xer_assert(out(u16).has_value());
    xer_assert(out(u32).has_value());
    xer_assert(out(u64).has_value());
    xer_assert(out(i8).has_value());
    xer_assert(out(i16).has_value());
    xer_assert(out(i32).has_value());
    xer_assert(out(i64).has_value());
    xer_assert(out(f32).has_value());
    xer_assert(out(f64).has_value());

    xer::binary_input_archive in(out.bytes());

    bool rb = false;
    std::uint8_t ru8 = 0;
    std::uint16_t ru16 = 0;
    std::uint32_t ru32 = 0;
    std::uint64_t ru64 = 0;
    std::int8_t ri8 = 0;
    std::int16_t ri16 = 0;
    std::int32_t ri32 = 0;
    std::int64_t ri64 = 0;
    float rf32 = 0.0f;
    double rf64 = 0.0;

    xer_assert(in(rb).has_value());
    xer_assert(in(ru8).has_value());
    xer_assert(in(ru16).has_value());
    xer_assert(in(ru32).has_value());
    xer_assert(in(ru64).has_value());
    xer_assert(in(ri8).has_value());
    xer_assert(in(ri16).has_value());
    xer_assert(in(ri32).has_value());
    xer_assert(in(ri64).has_value());
    xer_assert(in(rf32).has_value());
    xer_assert(in(rf64).has_value());

    xer_assert_eq(rb, b);
    xer_assert_eq(ru8, u8);
    xer_assert_eq(ru16, u16);
    xer_assert_eq(ru32, u32);
    xer_assert_eq(ru64, u64);
    xer_assert_eq(ri8, i8);
    xer_assert_eq(ri16, i16);
    xer_assert_eq(ri32, i32);
    xer_assert_eq(ri64, i64);
    xer_assert(std::fabs(rf32 - f32) < 0.000001f);
    xer_assert(std::fabs(rf64 - f64) < 0.000001);
    xer_assert(in.empty());
}

void test_string_and_bytes_round_trip()
{
    xer::binary_output_archive out;

    const std::u8string text = u8"xer 日本語";
    const std::vector<std::byte> raw = {
        std::byte{0x00},
        std::byte{0x10},
        std::byte{0xff},
    };

    xer_assert(out(text).has_value());
    xer_assert(out(raw).has_value());

    xer::binary_input_archive in(out.bytes());

    std::u8string read_text;
    std::vector<std::byte> read_raw;

    xer_assert(in(read_text).has_value());
    xer_assert(in(read_raw).has_value());
    xer_assert_eq(read_text, text);
    xer_assert_eq(read_raw.size(), raw.size());
    for (std::size_t i = 0; i < raw.size(); ++i) {
        xer_assert(read_raw[i] == raw[i]);
    }
    xer_assert(in.empty());
}

void test_containers_round_trip()
{
    xer::binary_output_archive out;

    const std::array<std::uint32_t, 4> array_value = {1u, 2u, 3u, 4u};
    const std::vector<std::int16_t> vector_value = {-1, 2, -3};
    const std::map<std::u8string, double> map_value = {
        {u8"one", 1.0},
        {u8"two", 2.0},
    };

    xer_assert(out(array_value).has_value());
    xer_assert(out(vector_value).has_value());
    xer_assert(out(map_value).has_value());

    xer::binary_input_archive in(out.bytes());

    std::array<std::uint32_t, 4> read_array{};
    std::vector<std::int16_t> read_vector;
    std::map<std::u8string, double> read_map;

    xer_assert(in(read_array).has_value());
    xer_assert(in(read_vector).has_value());
    xer_assert(in(read_map).has_value());

    xer_assert_eq(read_array, array_value);
    xer_assert_eq(read_vector, vector_value);
    xer_assert_eq(read_map.size(), map_value.size());
    xer_assert_eq(read_map[u8"one"], 1.0);
    xer_assert_eq(read_map[u8"two"], 2.0);
    xer_assert(in.empty());
}

void test_nested_containers_round_trip()
{
    xer::binary_output_archive out;

    const std::vector<std::array<std::uint32_t, 3>> values = {
        std::array<std::uint32_t, 3>{1u, 2u, 3u},
        std::array<std::uint32_t, 3>{4u, 5u, 6u},
    };
    const std::map<std::u8string, std::vector<std::uint32_t>> table = {
        {u8"a", {1u, 2u}},
        {u8"b", {3u, 4u, 5u}},
    };

    xer_assert(out(values).has_value());
    xer_assert(out(table).has_value());

    xer::binary_input_archive in(out.bytes());

    std::vector<std::array<std::uint32_t, 3>> read_values;
    std::map<std::u8string, std::vector<std::uint32_t>> read_table;

    xer_assert(in(read_values).has_value());
    xer_assert(in(read_table).has_value());
    xer_assert_eq(read_values, values);
    xer_assert_eq(read_table, table);
    xer_assert(in.empty());
}

void test_input_errors()
{
    {
        const std::array bytes = {std::byte{0x02}};
        xer::binary_input_archive in(bytes);
        bool value = false;
        auto r = in(value);
        xer_assert(!r.has_value());
        xer_assert_eq(r.error().code, xer::error_t::invalid_argument);
    }

    {
        const std::array bytes = {std::byte{0x01}};
        xer::binary_input_archive in(bytes);
        std::uint32_t value = 0;
        auto r = in(value);
        xer_assert(!r.has_value());
        xer_assert_eq(r.error().code, xer::error_t::end_of_file);
    }

    {
        xer::binary_output_archive out;
        xer_assert(out(std::uint64_t{2}).has_value());
        xer_assert(out(std::uint32_t{1}).has_value());
        xer_assert(out(std::uint32_t{11}).has_value());
        xer_assert(out(std::uint32_t{1}).has_value());
        xer_assert(out(std::uint32_t{22}).has_value());

        xer::binary_input_archive in(out.bytes());
        std::map<std::uint32_t, std::uint32_t> value;
        auto r = in(value);
        xer_assert(!r.has_value());
        xer_assert_eq(r.error().code, xer::error_t::invalid_argument);
    }
}

} // namespace

auto main() -> int
{
    test_scalar_little_endian();
    test_scalar_round_trip();
    test_string_and_bytes_round_trip();
    test_containers_round_trip();
    test_nested_containers_round_trip();
    test_input_errors();
    return 0;
}
