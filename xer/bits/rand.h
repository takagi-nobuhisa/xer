/**
 * @file xer/bits/rand.h
 * @brief Pseudo-random number generation based on xoroshiro128**.
 */

#pragma once

#ifndef XER_BITS_RAND_H_INCLUDED_
#define XER_BITS_RAND_H_INCLUDED_

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <random>

#include <xer/bits/common.h>
#include <xer/bits/error.h>
#include <xer/bits/stdint.h>

namespace xer {

/**
 * @brief Holds the internal state of a pseudo-random number generator.
 */
class rand_context {
public:
    /**
     * @brief Serialized byte representation of the generator state.
     */
    using bytes_type = std::array<std::byte, 16>;

    /**
     * @brief Constructs a context seeded from `std::random_device`.
     */
    rand_context()
    {
        seed(detail_random_seed());
    }

    /**
     * @brief Constructs a context from an explicit seed value.
     *
     * @param seed_value Seed value.
     */
    explicit rand_context(std::uint64_t seed_value) noexcept
    {
        seed(seed_value);
    }

    /**
     * @brief Serializes the current state into a byte array.
     *
     * The serialized representation is little-endian and stable across
     * supported platforms.
     *
     * @return Serialized byte representation.
     */
    [[nodiscard]] bytes_type to_bytes() const noexcept
    {
        bytes_type bytes{};

        store_u64_le(bytes.data(), state0_);
        store_u64_le(bytes.data() + 8, state1_);

        return bytes;
    }

    /**
     * @brief Restores a context from a serialized byte representation.
     *
     * The input is interpreted as a little-endian serialized state.
     * The all-zero state is rejected because it is invalid for xoroshiro128**.
     *
     * @param bytes Serialized byte representation.
     * @return Restored context on success, or an error on failure.
     */
    [[nodiscard]] static auto from_bytes(
        const bytes_type& bytes) noexcept -> result<rand_context> {
        const std::uint64_t state0 = load_u64_le(bytes.data());
        const std::uint64_t state1 = load_u64_le(bytes.data() + 8);

        if (state0 == 0 && state1 == 0) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        return rand_context(state0, state1);
    }

private:
    std::uint64_t state0_ = 0;
    std::uint64_t state1_ = 0;

    /**
     * @brief Advances the generator state and returns one random value.
     *
     * @return Generated random value.
     */
    [[nodiscard]] auto next() noexcept -> std::uint64_t {
        const std::uint64_t result = std::rotl(state1_ * 5ull, 7) * 9ull;
        const std::uint64_t xored = state1_ ^ state0_;

        state0_ = std::rotl(state0_, 24) ^ xored ^ (xored << 16);
        state1_ = std::rotl(xored, 37);

        return result;
    }

    /**
     * @brief Initializes the state from a 64-bit seed.
     *
     * @param seed_value Seed value.
     */
    auto seed(std::uint64_t seed_value) noexcept -> void {
        std::uint64_t value = seed_value;
        state0_ = splitmix64(value);
        state1_ = splitmix64(value);

        if (state0_ == 0 && state1_ == 0) {
            state1_ = 1;
        }
    }

    /**
     * @brief Generates the next SplitMix64 value.
     *
     * @param value SplitMix64 state.
     * @return Generated value.
     */
    [[nodiscard]] static auto splitmix64(std::uint64_t& value) noexcept -> std::uint64_t {
        value += 0x9E3779B97F4A7C15ull;

        std::uint64_t z = value;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
        return z ^ (z >> 31);
    }

    /**
     * @brief Obtains a seed value from `std::random_device`.
     *
     * @return Randomly generated seed value.
     */
    [[nodiscard]] static auto detail_random_seed() -> std::uint64_t {
        std::random_device device;
        std::uint64_t seed_value = 0;

        for (int index = 0; index < 8; ++index) {
            seed_value = (seed_value << 8) |
                static_cast<std::uint64_t>(device() & 0xffu);
        }

        return seed_value;
    }

    /**
     * @brief Stores a 64-bit value in little-endian byte order.
     *
     * @param destination Destination byte pointer.
     * @param value Value to store.
     */
    static auto store_u64_le(std::byte* destination, std::uint64_t value) noexcept -> void {
        for (int index = 0; index < 8; ++index) {
            destination[index] =
                static_cast<std::byte>((value >> (index * 8)) & 0xffu);
        }
    }

    /**
     * @brief Loads a 64-bit value from little-endian byte order.
     *
     * @param source Source byte pointer.
     * @return Loaded value.
     */
    [[nodiscard]] static auto load_u64_le(const std::byte* source) noexcept -> std::uint64_t {
        std::uint64_t value = 0;

        for (int index = 0; index < 8; ++index) {
            value |= static_cast<std::uint64_t>(
                         std::to_integer<unsigned>(source[index]))
                << (index * 8);
        }

        return value;
    }

    /**
     * @brief Constructs a context from explicit internal state values.
     *
     * @param state0 Initial first state word.
     * @param state1 Initial second state word.
     */
    constexpr rand_context(std::uint64_t state0, std::uint64_t state1) noexcept
        : state0_(state0),
          state1_(state1)
    {
    }

    friend auto rand() noexcept -> std::uint64_t ;
    friend auto rand(rand_context& context) noexcept -> std::uint64_t ;
    friend auto srand(std::uint64_t seed_value) noexcept -> void ;
};

namespace detail {

/**
 * @brief Default context used by the context-free random functions.
 */
inline rand_context default_rand_context{};

} // namespace detail

/**
 * @brief Generates one random value from the default context.
 *
 * @return Generated random value.
 */
[[nodiscard]] inline auto rand() noexcept -> std::uint64_t {
    return detail::default_rand_context.next();
}

/**
 * @brief Generates one random value from the specified context.
 *
 * @param context Random generator context.
 * @return Generated random value.
 */
[[nodiscard]] inline auto rand(rand_context& context) noexcept -> std::uint64_t {
    return context.next();
}

/**
 * @brief Reseeds the default context with an explicit seed value.
 *
 * @param seed_value Seed value.
 */
inline auto srand(std::uint64_t seed_value) noexcept -> void {
    detail::default_rand_context.seed(seed_value);
}

} // namespace xer

#endif /* XER_BITS_RAND_H_INCLUDED_ */
