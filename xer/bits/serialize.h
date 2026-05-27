/**
 * @file xer/bits/serialize.h
 * @brief Low-level binary transfer archives.
 */

#pragma once

#ifndef XER_BITS_SERIALIZE_H_INCLUDED_
#define XER_BITS_SERIALIZE_H_INCLUDED_

#include <array>
#include <concepts>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <xer/error.h>

namespace xer::detail {

[[nodiscard]] constexpr auto serialize_byte(std::uint8_t value) noexcept -> std::byte
{
    return static_cast<std::byte>(value);
}

[[nodiscard]] constexpr auto deserialize_byte(std::byte value) noexcept -> std::uint8_t
{
    return static_cast<std::uint8_t>(value);
}

inline auto serialize_append_u8(std::vector<std::byte>& out, std::uint8_t value)
    -> result<void>
{
    try {
        out.push_back(serialize_byte(value));
    } catch (...) {
        return std::unexpected(make_error(error_t::length_error));
    }

    return {};
}

inline auto serialize_append_bytes(
    std::vector<std::byte>& out,
    std::span<const std::byte> bytes) -> result<void>
{
    if (bytes.size() > out.max_size() - out.size()) {
        return std::unexpected(make_error(error_t::length_error));
    }

    try {
        out.insert(out.end(), bytes.begin(), bytes.end());
    } catch (...) {
        return std::unexpected(make_error(error_t::length_error));
    }

    return {};
}

inline auto serialize_read_bytes(
    std::span<const std::byte> input,
    std::size_t& offset,
    std::size_t count) -> result<std::span<const std::byte>>
{
    if (offset > input.size() || count > input.size() - offset) {
        return std::unexpected(make_error(error_t::end_of_file));
    }

    const auto bytes = input.subspan(offset, count);
    offset += count;
    return bytes;
}

template<class UInt>
    requires(std::unsigned_integral<UInt>)
inline auto serialize_write_unsigned(std::vector<std::byte>& out, UInt value)
    -> result<void>
{
    for (std::size_t i = 0; i < sizeof(UInt); ++i) {
        const auto byte = static_cast<std::uint8_t>((value >> (i * 8u)) & UInt{0xffu});
        if (auto r = serialize_append_u8(out, byte); !r) {
            return std::unexpected(r.error());
        }
    }

    return {};
}

template<class UInt>
    requires(std::unsigned_integral<UInt>)
inline auto serialize_read_unsigned(
    std::span<const std::byte> input,
    std::size_t& offset,
    UInt& value) -> result<void>
{
    auto bytes = serialize_read_bytes(input, offset, sizeof(UInt));
    if (!bytes) {
        return std::unexpected(bytes.error());
    }

    UInt result = 0;
    for (std::size_t i = 0; i < sizeof(UInt); ++i) {
        result |= static_cast<UInt>(deserialize_byte((*bytes)[i])) << (i * 8u);
    }

    value = result;
    return {};
}

template<class Int>
    requires(std::signed_integral<Int>)
inline auto serialize_write_signed(std::vector<std::byte>& out, Int value)
    -> result<void>
{
    using unsigned_type = std::make_unsigned_t<Int>;
    return serialize_write_unsigned(out, std::bit_cast<unsigned_type>(value));
}

template<class Int>
    requires(std::signed_integral<Int>)
inline auto serialize_read_signed(
    std::span<const std::byte> input,
    std::size_t& offset,
    Int& value) -> result<void>
{
    using unsigned_type = std::make_unsigned_t<Int>;

    unsigned_type raw = 0;
    if (auto r = serialize_read_unsigned(input, offset, raw); !r) {
        return std::unexpected(r.error());
    }

    value = std::bit_cast<Int>(raw);
    return {};
}

template<class Float>
    requires(std::floating_point<Float>)
inline auto serialize_write_float(std::vector<std::byte>& out, Float value)
    -> result<void>
{
    if constexpr (sizeof(Float) == sizeof(std::uint32_t)) {
        return serialize_write_unsigned(out, std::bit_cast<std::uint32_t>(value));
    } else if constexpr (sizeof(Float) == sizeof(std::uint64_t)) {
        return serialize_write_unsigned(out, std::bit_cast<std::uint64_t>(value));
    } else {
        static_assert(sizeof(Float) == sizeof(std::uint32_t) || sizeof(Float) == sizeof(std::uint64_t));
    }
}

template<class Float>
    requires(std::floating_point<Float>)
inline auto serialize_read_float(
    std::span<const std::byte> input,
    std::size_t& offset,
    Float& value) -> result<void>
{
    if constexpr (sizeof(Float) == sizeof(std::uint32_t)) {
        std::uint32_t raw = 0;
        if (auto r = serialize_read_unsigned(input, offset, raw); !r) {
            return std::unexpected(r.error());
        }
        value = std::bit_cast<Float>(raw);
        return {};
    } else if constexpr (sizeof(Float) == sizeof(std::uint64_t)) {
        std::uint64_t raw = 0;
        if (auto r = serialize_read_unsigned(input, offset, raw); !r) {
            return std::unexpected(r.error());
        }
        value = std::bit_cast<Float>(raw);
        return {};
    } else {
        static_assert(sizeof(Float) == sizeof(std::uint32_t) || sizeof(Float) == sizeof(std::uint64_t));
    }
}

[[nodiscard]] constexpr auto serialize_u64_to_size(std::uint64_t value)
    -> result<std::size_t>
{
    if (value > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
        return std::unexpected(make_error(error_t::length_error));
    }

    return static_cast<std::size_t>(value);
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Binary output archive for low-level XER serialization.
 *
 * `binary_output_archive` writes scalar values and supported standard
 * containers to a compact binary buffer. Multi-byte scalar values are always
 * written in little-endian order. Type names, field names, and schema
 * information are not stored.
 */
class binary_output_archive {
public:
    binary_output_archive() = default;

    /**
     * @brief Returns the currently accumulated byte sequence.
     *
     * The returned span is invalidated by later writes or by `release`.
     */
    [[nodiscard]] auto bytes() const noexcept -> std::span<const std::byte>
    {
        return data_;
    }

    /**
     * @brief Moves out the accumulated byte sequence and clears this archive.
     */
    [[nodiscard]] auto release() noexcept -> std::vector<std::byte>
    {
        return std::exchange(data_, {});
    }

    auto operator()(bool value) -> result<void>
    {
        return detail::serialize_append_u8(data_, value ? 1u : 0u);
    }

    auto operator()(std::uint8_t value) -> result<void>
    {
        return detail::serialize_append_u8(data_, value);
    }

    auto operator()(std::uint16_t value) -> result<void>
    {
        return detail::serialize_write_unsigned(data_, value);
    }

    auto operator()(std::uint32_t value) -> result<void>
    {
        return detail::serialize_write_unsigned(data_, value);
    }

    auto operator()(std::uint64_t value) -> result<void>
    {
        return detail::serialize_write_unsigned(data_, value);
    }

    auto operator()(std::int8_t value) -> result<void>
    {
        return detail::serialize_append_u8(data_, std::bit_cast<std::uint8_t>(value));
    }

    auto operator()(std::int16_t value) -> result<void>
    {
        return detail::serialize_write_signed(data_, value);
    }

    auto operator()(std::int32_t value) -> result<void>
    {
        return detail::serialize_write_signed(data_, value);
    }

    auto operator()(std::int64_t value) -> result<void>
    {
        return detail::serialize_write_signed(data_, value);
    }

    auto operator()(float value) -> result<void>
    {
        return detail::serialize_write_float(data_, value);
    }

    auto operator()(double value) -> result<void>
    {
        return detail::serialize_write_float(data_, value);
    }

    auto operator()(std::u8string_view value) -> result<void>
    {
        if (auto r = (*this)(static_cast<std::uint64_t>(value.size())); !r) {
            return std::unexpected(r.error());
        }

        const auto bytes = std::as_bytes(std::span(value.data(), value.size()));
        return detail::serialize_append_bytes(data_, bytes);
    }

    auto operator()(const std::u8string& value) -> result<void>
    {
        return (*this)(std::u8string_view(value));
    }

    auto operator()(std::span<const std::byte> value) -> result<void>
    {
        if (auto r = (*this)(static_cast<std::uint64_t>(value.size())); !r) {
            return std::unexpected(r.error());
        }

        return detail::serialize_append_bytes(data_, value);
    }

    auto operator()(const std::vector<std::byte>& value) -> result<void>
    {
        return (*this)(std::span<const std::byte>(value.data(), value.size()));
    }

    template<class T, std::size_t N>
    auto operator()(const std::array<T, N>& value) -> result<void>
    {
        for (const auto& element : value) {
            if (auto r = (*this)(element); !r) {
                return std::unexpected(r.error());
            }
        }

        return {};
    }

    template<class T, class Allocator>
        requires(!std::same_as<T, std::byte>)
    auto operator()(const std::vector<T, Allocator>& value) -> result<void>
    {
        if (auto r = (*this)(static_cast<std::uint64_t>(value.size())); !r) {
            return std::unexpected(r.error());
        }

        for (const auto& element : value) {
            if (auto r = (*this)(element); !r) {
                return std::unexpected(r.error());
            }
        }

        return {};
    }

    template<class Key, class T, class Compare, class Allocator>
    auto operator()(const std::map<Key, T, Compare, Allocator>& value) -> result<void>
    {
        if (auto r = (*this)(static_cast<std::uint64_t>(value.size())); !r) {
            return std::unexpected(r.error());
        }

        for (const auto& [key, mapped] : value) {
            if (auto r = (*this)(key); !r) {
                return std::unexpected(r.error());
            }
            if (auto r = (*this)(mapped); !r) {
                return std::unexpected(r.error());
            }
        }

        return {};
    }

private:
    std::vector<std::byte> data_;
};

/**
 * @brief Binary input archive for low-level XER serialization.
 *
 * `binary_input_archive` reads values from a byte span produced according to
 * the low-level XER binary serialization rules. Multi-byte scalar values are
 * always interpreted as little-endian data.
 */
class binary_input_archive {
public:
    explicit binary_input_archive(std::span<const std::byte> data) noexcept
        : data_(data) {}

    [[nodiscard]] auto remaining_size() const noexcept -> std::size_t
    {
        if (offset_ > data_.size()) {
            return 0;
        }

        return data_.size() - offset_;
    }

    [[nodiscard]] auto empty() const noexcept -> bool
    {
        return remaining_size() == 0;
    }

    auto operator()(bool& value) -> result<void>
    {
        std::uint8_t raw = 0;
        if (auto r = (*this)(raw); !r) {
            return std::unexpected(r.error());
        }

        if (raw > 1u) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        value = raw != 0u;
        return {};
    }

    auto operator()(std::uint8_t& value) -> result<void>
    {
        auto bytes = detail::serialize_read_bytes(data_, offset_, 1u);
        if (!bytes) {
            return std::unexpected(bytes.error());
        }

        value = detail::deserialize_byte((*bytes)[0]);
        return {};
    }

    auto operator()(std::uint16_t& value) -> result<void>
    {
        return detail::serialize_read_unsigned(data_, offset_, value);
    }

    auto operator()(std::uint32_t& value) -> result<void>
    {
        return detail::serialize_read_unsigned(data_, offset_, value);
    }

    auto operator()(std::uint64_t& value) -> result<void>
    {
        return detail::serialize_read_unsigned(data_, offset_, value);
    }

    auto operator()(std::int8_t& value) -> result<void>
    {
        std::uint8_t raw = 0;
        if (auto r = (*this)(raw); !r) {
            return std::unexpected(r.error());
        }

        value = std::bit_cast<std::int8_t>(raw);
        return {};
    }

    auto operator()(std::int16_t& value) -> result<void>
    {
        return detail::serialize_read_signed(data_, offset_, value);
    }

    auto operator()(std::int32_t& value) -> result<void>
    {
        return detail::serialize_read_signed(data_, offset_, value);
    }

    auto operator()(std::int64_t& value) -> result<void>
    {
        return detail::serialize_read_signed(data_, offset_, value);
    }

    auto operator()(float& value) -> result<void>
    {
        return detail::serialize_read_float(data_, offset_, value);
    }

    auto operator()(double& value) -> result<void>
    {
        return detail::serialize_read_float(data_, offset_, value);
    }

    auto operator()(std::u8string& value) -> result<void>
    {
        std::uint64_t size64 = 0;
        if (auto r = (*this)(size64); !r) {
            return std::unexpected(r.error());
        }

        auto size = detail::serialize_u64_to_size(size64);
        if (!size) {
            return std::unexpected(size.error());
        }

        auto bytes = detail::serialize_read_bytes(data_, offset_, *size);
        if (!bytes) {
            return std::unexpected(bytes.error());
        }

        if (*size > value.max_size()) {
            return std::unexpected(make_error(error_t::length_error));
        }

        try {
            value.assign(
                reinterpret_cast<const char8_t*>(bytes->data()),
                reinterpret_cast<const char8_t*>(bytes->data()) + bytes->size());
        } catch (...) {
            return std::unexpected(make_error(error_t::length_error));
        }

        return {};
    }

    auto operator()(std::vector<std::byte>& value) -> result<void>
    {
        std::uint64_t size64 = 0;
        if (auto r = (*this)(size64); !r) {
            return std::unexpected(r.error());
        }

        auto size = detail::serialize_u64_to_size(size64);
        if (!size) {
            return std::unexpected(size.error());
        }

        auto bytes = detail::serialize_read_bytes(data_, offset_, *size);
        if (!bytes) {
            return std::unexpected(bytes.error());
        }

        if (*size > value.max_size()) {
            return std::unexpected(make_error(error_t::length_error));
        }

        try {
            value.assign(bytes->begin(), bytes->end());
        } catch (...) {
            return std::unexpected(make_error(error_t::length_error));
        }

        return {};
    }

    template<class T, std::size_t N>
    auto operator()(std::array<T, N>& value) -> result<void>
    {
        for (auto& element : value) {
            if (auto r = (*this)(element); !r) {
                return std::unexpected(r.error());
            }
        }

        return {};
    }

    template<class T, class Allocator>
        requires(!std::same_as<T, std::byte>)
    auto operator()(std::vector<T, Allocator>& value) -> result<void>
    {
        std::uint64_t count64 = 0;
        if (auto r = (*this)(count64); !r) {
            return std::unexpected(r.error());
        }

        auto count = detail::serialize_u64_to_size(count64);
        if (!count) {
            return std::unexpected(count.error());
        }

        if (*count > value.max_size()) {
            return std::unexpected(make_error(error_t::length_error));
        }

        try {
            value.clear();
            value.reserve(*count);
        } catch (...) {
            return std::unexpected(make_error(error_t::length_error));
        }

        for (std::size_t i = 0; i < *count; ++i) {
            T element{};
            if (auto r = (*this)(element); !r) {
                return std::unexpected(r.error());
            }

            try {
                value.push_back(std::move(element));
            } catch (...) {
                return std::unexpected(make_error(error_t::length_error));
            }
        }

        return {};
    }

    template<class Key, class T, class Compare, class Allocator>
    auto operator()(std::map<Key, T, Compare, Allocator>& value) -> result<void>
    {
        std::uint64_t count64 = 0;
        if (auto r = (*this)(count64); !r) {
            return std::unexpected(r.error());
        }

        auto count = detail::serialize_u64_to_size(count64);
        if (!count) {
            return std::unexpected(count.error());
        }

        if (*count > value.max_size()) {
            return std::unexpected(make_error(error_t::length_error));
        }

        value.clear();

        for (std::size_t i = 0; i < *count; ++i) {
            Key key{};
            T mapped{};

            if (auto r = (*this)(key); !r) {
                return std::unexpected(r.error());
            }
            if (auto r = (*this)(mapped); !r) {
                return std::unexpected(r.error());
            }

            try {
                auto [it, inserted] = value.emplace(std::move(key), std::move(mapped));
                static_cast<void>(it);
                if (!inserted) {
                    return std::unexpected(make_error(error_t::invalid_argument));
                }
            } catch (...) {
                return std::unexpected(make_error(error_t::length_error));
            }
        }

        return {};
    }

private:
    std::span<const std::byte> data_;
    std::size_t offset_ = 0;
};

} // namespace xer

#endif /* XER_BITS_SERIALIZE_H_INCLUDED_ */
