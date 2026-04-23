/**
 * @file tests/test_mem.cpp
 * @brief Tests for xer/bits/mem.h.
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <vector>

#include <xer/assert.h>
#include <xer/bits/mem.h>
#include <xer/error.h>

namespace {

[[nodiscard]] constexpr auto b(const unsigned int value) noexcept -> std::byte
{
    return static_cast<std::byte>(value);
}

void test_memcpy_pointer_success()
{
    std::array<std::byte, 8> destination {
        b(0x00), b(0x00), b(0x00), b(0x00),
        b(0x00), b(0x00), b(0x00), b(0x00)
    };
    const std::array<std::byte, 4> source {b(0x11), b(0x22), b(0x33), b(0x44)};

    const auto result =
        xer::memcpy(destination.data(), destination.size(), source.data(), source.size());

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), destination.data());
    xer_assert_eq(destination[0], b(0x11));
    xer_assert_eq(destination[1], b(0x22));
    xer_assert_eq(destination[2], b(0x33));
    xer_assert_eq(destination[3], b(0x44));
    xer_assert_eq(destination[4], b(0x00));
}

void test_memcpy_pointer_zero_length_success()
{
    std::array<std::byte, 1> destination {b(0x7f)};

    const auto result = xer::memcpy(
        destination.data(),
        destination.size(),
        static_cast<const std::byte*>(nullptr),
        0);

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), destination.data());
    xer_assert_eq(destination[0], b(0x7f));
}

void test_memcpy_pointer_invalid_argument()
{
    const std::array<std::byte, 4> source {b(0x11), b(0x22), b(0x33), b(0x44)};

    const auto result = xer::memcpy(
        static_cast<std::byte*>(nullptr),
        source.size(),
        source.data(),
        source.size());

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_memcpy_pointer_length_error()
{
    std::array<std::byte, 2> destination {b(0x00), b(0x00)};
    const std::array<std::byte, 4> source {b(0x11), b(0x22), b(0x33), b(0x44)};

    const auto result =
        xer::memcpy(destination.data(), destination.size(), source.data(), source.size());

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::length_error);
}

void test_memcpy_pointer_overlap_error()
{
    std::array<std::byte, 8> buffer {
        b(0x10), b(0x11), b(0x12), b(0x13),
        b(0x14), b(0x15), b(0x16), b(0x17)
    };

    const auto result = xer::memcpy(buffer.data() + 1, 4, buffer.data(), 4);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_memcpy_container_success()
{
    std::array<std::byte, 6> destination {b(0), b(0), b(0), b(0), b(0), b(0)};
    const std::array<std::byte, 3> source {b(1), b(2), b(3)};

    const auto result = xer::memcpy(destination, source);

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), destination.begin());
    xer_assert_eq(destination[0], b(1));
    xer_assert_eq(destination[1], b(2));
    xer_assert_eq(destination[2], b(3));
    xer_assert_eq(destination[3], b(0));
}

void test_memcpy_container_length_error()
{
    std::array<std::byte, 2> destination {b(0), b(0)};
    const std::vector<std::byte> source {b(1), b(2), b(3)};

    const auto result = xer::memcpy(destination, source);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::length_error);
}

void test_memmove_pointer_non_overlap()
{
    std::array<std::byte, 6> destination {b(0), b(0), b(0), b(0), b(0), b(0)};
    const std::array<std::byte, 3> source {b(0xa1), b(0xa2), b(0xa3)};

    const auto result =
        xer::memmove(destination.data(), destination.size(), source.data(), source.size());

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), destination.data());
    xer_assert_eq(destination[0], b(0xa1));
    xer_assert_eq(destination[1], b(0xa2));
    xer_assert_eq(destination[2], b(0xa3));
}

void test_memmove_pointer_overlap_forward()
{
    std::array<std::byte, 6> buffer {b(1), b(2), b(3), b(4), b(5), b(6)};

    const auto result = xer::memmove(buffer.data() + 1, 5, buffer.data(), 4);

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), buffer.data() + 1);
    xer_assert_eq(buffer[0], b(1));
    xer_assert_eq(buffer[1], b(1));
    xer_assert_eq(buffer[2], b(2));
    xer_assert_eq(buffer[3], b(3));
    xer_assert_eq(buffer[4], b(4));
}

void test_memmove_pointer_overlap_backward()
{
    std::array<std::byte, 6> buffer {b(1), b(2), b(3), b(4), b(5), b(6)};

    const auto result = xer::memmove(buffer.data(), 6, buffer.data() + 1, 4);

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), buffer.data());
    xer_assert_eq(buffer[0], b(2));
    xer_assert_eq(buffer[1], b(3));
    xer_assert_eq(buffer[2], b(4));
    xer_assert_eq(buffer[3], b(5));
}

void test_memmove_pointer_length_error()
{
    std::array<std::byte, 2> destination {b(0), b(0)};
    const std::array<std::byte, 3> source {b(1), b(2), b(3)};

    const auto result =
        xer::memmove(destination.data(), destination.size(), source.data(), source.size());

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::length_error);
}

void test_memmove_container_success()
{
    std::vector<std::byte> destination {b(0), b(0), b(0), b(0)};
    const std::array<std::byte, 4> source {b(0x21), b(0x22), b(0x23), b(0x24)};

    const auto result = xer::memmove(destination, source);

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), destination.begin());
    xer_assert_eq(destination[0], b(0x21));
    xer_assert_eq(destination[1], b(0x22));
    xer_assert_eq(destination[2], b(0x23));
    xer_assert_eq(destination[3], b(0x24));
}

void test_memchr_pointer_mutable_success()
{
    std::array<std::byte, 5> buffer {b(0x10), b(0x20), b(0x30), b(0x40), b(0x50)};

    const auto result = xer::memchr(buffer.data(), buffer.size(), b(0x30));

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), buffer.data() + 2);
}

void test_memchr_pointer_const_success()
{
    const std::array<std::byte, 5> buffer {b(0x10), b(0x20), b(0x30), b(0x40), b(0x50)};

    const auto result = xer::memchr(buffer.data(), buffer.size(), b(0x40));

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), buffer.data() + 3);
}

void test_memchr_pointer_not_found()
{
    const std::array<std::byte, 5> buffer {b(0x10), b(0x20), b(0x30), b(0x40), b(0x50)};

    const auto result = xer::memchr(buffer.data(), buffer.size(), b(0xff));

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::not_found);
}

void test_memchr_pointer_invalid_argument()
{
    const auto result = xer::memchr(static_cast<const std::byte*>(nullptr), 1, b(0x10));

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_memchr_container_mutable_success()
{
    std::vector<std::byte> buffer {b(5), b(6), b(7), b(8)};

    const auto result = xer::memchr(buffer, b(7));

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), buffer.begin() + 2);
}

void test_memchr_container_const_success()
{
    const std::array<std::byte, 4> buffer {b(5), b(6), b(7), b(8)};

    const auto result = xer::memchr(buffer, b(8));

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), buffer.begin() + 3);
}

void test_memrchr_pointer_mutable_success()
{
    std::array<std::byte, 6> buffer {b(0x10), b(0x20), b(0x30), b(0x20), b(0x50), b(0x20)};

    const auto result = xer::memrchr(buffer.data(), buffer.size(), b(0x20));

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), buffer.data() + 5);
}

void test_memrchr_pointer_const_success()
{
    const std::array<std::byte, 6> buffer {b(0x10), b(0x20), b(0x30), b(0x40), b(0x20), b(0x60)};

    const auto result = xer::memrchr(buffer.data(), buffer.size(), b(0x20));

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), buffer.data() + 4);
}

void test_memrchr_pointer_not_found()
{
    const std::array<std::byte, 5> buffer {b(0x10), b(0x20), b(0x30), b(0x40), b(0x50)};

    const auto result = xer::memrchr(buffer.data(), buffer.size(), b(0xff));

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::not_found);
}

void test_memrchr_pointer_invalid_argument()
{
    const auto result = xer::memrchr(static_cast<const std::byte*>(nullptr), 1, b(0x10));

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_memrchr_container_mutable_success()
{
    std::vector<std::byte> buffer {b(5), b(6), b(7), b(6), b(8)};

    const auto result = xer::memrchr(buffer, b(6));

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), buffer.begin() + 3);
}

void test_memrchr_container_const_success()
{
    const std::array<std::byte, 5> buffer {b(5), b(9), b(7), b(9), b(8)};

    const auto result = xer::memrchr(buffer, b(9));

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), buffer.begin() + 3);
}

void test_memcmp_equal()
{
    const std::array<std::byte, 4> lhs {b(1), b(2), b(3), b(4)};
    const std::array<std::byte, 4> rhs {b(1), b(2), b(3), b(4)};

    const auto result = xer::memcmp(lhs.data(), lhs.size(), rhs.data(), rhs.size());

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), 0);
}

void test_memcmp_less_by_content()
{
    const std::array<std::byte, 4> lhs {b(1), b(2), b(3), b(4)};
    const std::array<std::byte, 4> rhs {b(1), b(2), b(9), b(4)};

    const auto result = xer::memcmp(lhs.data(), lhs.size(), rhs.data(), rhs.size());

    xer_assert(result.has_value());
    xer_assert(result.value() < 0);
}

void test_memcmp_greater_by_length()
{
    const std::array<std::byte, 4> lhs {b(1), b(2), b(3), b(4)};
    const std::array<std::byte, 3> rhs {b(1), b(2), b(3)};

    const auto result = xer::memcmp(lhs.data(), lhs.size(), rhs.data(), rhs.size());

    xer_assert(result.has_value());
    xer_assert(result.value() > 0);
}

void test_memcmp_container_success()
{
    const std::vector<std::byte> lhs {b(0x11), b(0x12), b(0x13)};
    const std::array<std::byte, 3> rhs {b(0x11), b(0x12), b(0x13)};

    const auto result = xer::memcmp(lhs, rhs);

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), 0);
}

void test_memcmp_invalid_argument()
{
    const std::array<std::byte, 2> rhs {b(1), b(2)};

    const auto result = xer::memcmp(
        static_cast<const std::byte*>(nullptr),
        rhs.size(),
        rhs.data(),
        rhs.size());

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_memset_pointer_success()
{
    std::array<std::byte, 5> buffer {b(1), b(2), b(3), b(4), b(5)};

    const auto result = xer::memset(buffer.data(), buffer.size(), b(0xaa));

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), buffer.data());
    xer_assert_eq(buffer[0], b(0xaa));
    xer_assert_eq(buffer[1], b(0xaa));
    xer_assert_eq(buffer[2], b(0xaa));
    xer_assert_eq(buffer[3], b(0xaa));
    xer_assert_eq(buffer[4], b(0xaa));
}

void test_memset_pointer_invalid_argument()
{
    const auto result = xer::memset(static_cast<std::byte*>(nullptr), 1, b(0));

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_memset_container_success()
{
    std::vector<std::byte> buffer {b(1), b(2), b(3)};

    const auto result = xer::memset(buffer, b(0xee));

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), buffer.begin());
    xer_assert_eq(buffer[0], b(0xee));
    xer_assert_eq(buffer[1], b(0xee));
    xer_assert_eq(buffer[2], b(0xee));
}

} // namespace

auto main() -> int
{
    test_memcpy_pointer_success();
    test_memcpy_pointer_zero_length_success();
    test_memcpy_pointer_invalid_argument();
    test_memcpy_pointer_length_error();
    test_memcpy_pointer_overlap_error();
    test_memcpy_container_success();
    test_memcpy_container_length_error();

    test_memmove_pointer_non_overlap();
    test_memmove_pointer_overlap_forward();
    test_memmove_pointer_overlap_backward();
    test_memmove_pointer_length_error();
    test_memmove_container_success();

    test_memchr_pointer_mutable_success();
    test_memchr_pointer_const_success();
    test_memchr_pointer_not_found();
    test_memchr_pointer_invalid_argument();
    test_memchr_container_mutable_success();
    test_memchr_container_const_success();

    test_memrchr_pointer_mutable_success();
    test_memrchr_pointer_const_success();
    test_memrchr_pointer_not_found();
    test_memrchr_pointer_invalid_argument();
    test_memrchr_container_mutable_success();
    test_memrchr_container_const_success();

    test_memcmp_equal();
    test_memcmp_less_by_content();
    test_memcmp_greater_by_length();
    test_memcmp_container_success();
    test_memcmp_invalid_argument();

    test_memset_pointer_success();
    test_memset_pointer_invalid_argument();
    test_memset_container_success();

    return 0;
}
