#include <array>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <xer/assert.h>
#include <xer/bytes.h>

namespace {

void assert_bytes_eq(
    std::span<const std::byte> lhs,
    std::span<const std::byte> rhs)
{
    xer_assert_eq(lhs.size(), rhs.size());

    for (std::size_t i = 0; i < lhs.size(); ++i) {
        xer_assert(lhs[i] == rhs[i]);
    }
}

void test_to_bytes_view_string_view()
{
    constexpr std::string_view text = "ABC";
    const auto view = xer::to_bytes_view(text);

    xer_assert_eq(view.size(), text.size());
    xer_assert(view.data() == reinterpret_cast<const std::byte*>(text.data()));
    xer_assert_eq(std::to_integer<unsigned char>(view[0]), 'A');
    xer_assert_eq(std::to_integer<unsigned char>(view[1]), 'B');
    xer_assert_eq(std::to_integer<unsigned char>(view[2]), 'C');
}

void test_to_bytes_view_u8string_view()
{
    constexpr std::u8string_view text = u8"xer";
    const auto view = xer::to_bytes_view(text);

    xer_assert_eq(view.size(), text.size());
    xer_assert(view.data() == reinterpret_cast<const std::byte*>(text.data()));
    xer_assert_eq(std::to_integer<unsigned char>(view[0]), 'x');
    xer_assert_eq(std::to_integer<unsigned char>(view[1]), 'e');
    xer_assert_eq(std::to_integer<unsigned char>(view[2]), 'r');
}

void test_to_bytes_view_char_span()
{
    const std::array<char, 4> data = {'t', 'e', 's', 't'};
    const auto view = xer::to_bytes_view(std::span<const char>{data});

    xer_assert_eq(view.size(), data.size());
    xer_assert(view.data() == reinterpret_cast<const std::byte*>(data.data()));
    xer_assert_eq(std::to_integer<unsigned char>(view[0]), 't');
    xer_assert_eq(std::to_integer<unsigned char>(view[3]), 't');
}

void test_to_bytes_view_char8_span()
{
    const std::array<char8_t, 4> data = {u8'u', u8't', u8'f', u8'8'};
    const auto view = xer::to_bytes_view(std::span<const char8_t>{data});

    xer_assert_eq(view.size(), data.size());
    xer_assert(view.data() == reinterpret_cast<const std::byte*>(data.data()));
    xer_assert_eq(std::to_integer<unsigned char>(view[0]), 'u');
    xer_assert_eq(std::to_integer<unsigned char>(view[3]), '8');
}

void test_to_bytes_view_unsigned_char_span()
{
    const std::array<unsigned char, 3> data = {0x00u, 0x7Fu, 0xFFu};
    const auto view = xer::to_bytes_view(std::span<const unsigned char>{data});

    xer_assert_eq(view.size(), data.size());
    xer_assert(view.data() == reinterpret_cast<const std::byte*>(data.data()));
    xer_assert_eq(std::to_integer<unsigned char>(view[0]), 0x00u);
    xer_assert_eq(std::to_integer<unsigned char>(view[1]), 0x7Fu);
    xer_assert_eq(std::to_integer<unsigned char>(view[2]), 0xFFu);
}

void test_to_bytes_view_byte_span()
{
    const std::array<std::byte, 2> data = {std::byte{0x12}, std::byte{0x34}};
    const auto input = std::span<const std::byte>{data};
    const auto view = xer::to_bytes_view(input);

    xer_assert_eq(view.size(), input.size());
    xer_assert(view.data() == input.data());
    assert_bytes_eq(view, input);
}

void test_to_bytes_copies_u8string_view()
{
    std::u8string text = u8"abc";
    const auto copied = xer::to_bytes(std::u8string_view{text});

    text[0] = u8'z';

    xer_assert_eq(copied.size(), 3u);
    xer_assert_eq(std::to_integer<unsigned char>(copied[0]), 'a');
    xer_assert_eq(std::to_integer<unsigned char>(copied[1]), 'b');
    xer_assert_eq(std::to_integer<unsigned char>(copied[2]), 'c');
}

void test_to_bytes_copies_spans()
{
    const std::array<unsigned char, 3> data = {1u, 2u, 3u};
    const auto copied = xer::to_bytes(std::span<const unsigned char>{data});

    xer_assert_eq(copied.size(), data.size());
    xer_assert_eq(std::to_integer<unsigned char>(copied[0]), 1u);
    xer_assert_eq(std::to_integer<unsigned char>(copied[1]), 2u);
    xer_assert_eq(std::to_integer<unsigned char>(copied[2]), 3u);
}

} // namespace

auto main() -> int
{
    test_to_bytes_view_string_view();
    test_to_bytes_view_u8string_view();
    test_to_bytes_view_char_span();
    test_to_bytes_view_char8_span();
    test_to_bytes_view_unsigned_char_span();
    test_to_bytes_view_byte_span();
    test_to_bytes_copies_u8string_view();
    test_to_bytes_copies_spans();

    return 0;
}
