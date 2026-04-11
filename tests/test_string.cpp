/**
 * @file tests/test_string.cpp
 * @brief Minimal integration test for xer/string.h.
 */

#include <array>
#include <cstddef>
#include <expected>
#include <string_view>
#include <type_traits>

#include <xer/string.h>

namespace {

/**
 * @brief Verifies that the public API in xer/string.h can be referenced
 *        together in a single translation unit.
 */
void test_public_api_is_usable()
{
    std::array<std::byte, 4> bytes {
        std::byte {0x00},
        std::byte {0x00},
        std::byte {0x00},
        std::byte {0x00}
    };
    const std::array<std::byte, 2> source {
        std::byte {0x11},
        std::byte {0x22}
    };
    [[maybe_unused]] const auto memcpy_result = xer::memcpy(bytes, source);

    constexpr std::u8string_view text = u8"alpha";
    constexpr std::u8string_view same_text = u8"alpha";

    [[maybe_unused]] const auto strlen_result = xer::strlen(text);
    [[maybe_unused]] const auto strcmp_result = xer::strcmp(text, same_text);
    [[maybe_unused]] const auto strchr_result = xer::strchr(text, u8'a');

    std::array<char8_t, 16> buffer {};
    [[maybe_unused]] const auto strcpy_result = xer::strcpy(buffer, text);

    [[maybe_unused]] const auto strerror_result =
        xer::strerror(xer::error_t::invalid_argument);
}

/**
 * @brief Verifies representative public return types exposed through
 *        xer/string.h.
 */
void test_return_types()
{
    std::array<std::byte, 4> bytes {};
    const std::array<std::byte, 2> source {
        std::byte {0x11},
        std::byte {0x22}
    };
    constexpr std::u8string_view text = u8"alpha";
    std::array<char8_t, 16> buffer {};

    static_assert(std::same_as<
        decltype(xer::memcpy(bytes, source)),
        std::expected<decltype(bytes.begin()), xer::error<void>>>);

    static_assert(std::same_as<
        decltype(xer::strlen(text)),
        std::expected<std::size_t, xer::error<void>>>);

    static_assert(std::same_as<
        decltype(xer::strcmp(text, text)),
        std::expected<int, xer::error<void>>>);

    static_assert(std::same_as<
        decltype(xer::strchr(text, u8'a')),
        std::expected<decltype(text.begin()), xer::error<void>>>);

    static_assert(std::same_as<
        decltype(xer::strcpy(buffer, text)),
        std::expected<decltype(buffer.begin()), xer::error<void>>>);

    static_assert(std::same_as<
        decltype(xer::strerror(xer::error_t::invalid_argument)),
        std::expected<std::u8string_view, xer::error<void>>>);
}

} // namespace

int main()
{
    test_public_api_is_usable();
    test_return_types();
    return 0;
}
