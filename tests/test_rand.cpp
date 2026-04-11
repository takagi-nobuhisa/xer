/**
 * @file tests/test_rand.cpp
 * @brief Tests for xer/bits/rand.h.
 */

#include <cstdint>

#include <xer/assert.h>
#include <xer/bits/rand.h>

namespace {

/**
 * @brief Tests deterministic generation with the same explicit seed.
 */
void test_seeded_context_determinism()
{
    xer::rand_context context1(UINT64_C(123456789));
    xer::rand_context context2(UINT64_C(123456789));

    for (int index = 0; index < 32; ++index) {
        xer_assert_eq(xer::rand(context1), xer::rand(context2));
    }
}

/**
 * @brief Tests that different seeds produce different initial sequences.
 */
void test_different_seeds()
{
    xer::rand_context context1(UINT64_C(123456789));
    xer::rand_context context2(UINT64_C(987654321));

    bool found_difference = false;

    for (int index = 0; index < 32; ++index) {
        if (xer::rand(context1) != xer::rand(context2)) {
            found_difference = true;
            break;
        }
    }

    xer_assert(found_difference);
}

/**
 * @brief Tests copy semantics of rand_context.
 */
void test_context_copy()
{
    xer::rand_context original(UINT64_C(0x1122334455667788));

    (void)xer::rand(original);
    (void)xer::rand(original);
    (void)xer::rand(original);

    xer::rand_context copied = original;

    for (int index = 0; index < 32; ++index) {
        xer_assert_eq(xer::rand(original), xer::rand(copied));
    }
}

/**
 * @brief Tests default-context reseeding through srand().
 */
void test_default_context_reseed()
{
    constexpr std::uint64_t seed_value = UINT64_C(0x123456789abcdef0);

    xer::srand(seed_value);

    const std::uint64_t value1 = xer::rand();
    const std::uint64_t value2 = xer::rand();
    const std::uint64_t value3 = xer::rand();
    const std::uint64_t value4 = xer::rand();

    xer::srand(seed_value);

    xer_assert_eq(xer::rand(), value1);
    xer_assert_eq(xer::rand(), value2);
    xer_assert_eq(xer::rand(), value3);
    xer_assert_eq(xer::rand(), value4);
}

/**
 * @brief Tests round-trip conversion through bytes_type.
 */
void test_to_bytes_from_bytes_round_trip()
{
    xer::rand_context original(UINT64_C(0x0fedcba987654321));

    (void)xer::rand(original);
    (void)xer::rand(original);
    (void)xer::rand(original);
    (void)xer::rand(original);
    (void)xer::rand(original);

    const xer::rand_context::bytes_type bytes = original.to_bytes();
    const auto restored_result = xer::rand_context::from_bytes(bytes);

    xer_assert(restored_result.has_value());

    xer::rand_context restored = *restored_result;

    for (int index = 0; index < 32; ++index) {
        xer_assert_eq(xer::rand(original), xer::rand(restored));
    }
}

/**
 * @brief Tests rejection of the all-zero serialized state.
 */
void test_from_bytes_rejects_all_zero_state()
{
    constexpr xer::rand_context::bytes_type zero_bytes{};

    const auto result = xer::rand_context::from_bytes(zero_bytes);

    xer_assert_not(result.has_value());
}

/**
 * @brief Tests that random-device construction yields a restorable state.
 */
void test_default_constructed_context_is_serializable()
{
    xer::rand_context context;

    const xer::rand_context::bytes_type bytes = context.to_bytes();
    const auto restored_result = xer::rand_context::from_bytes(bytes);

    xer_assert(restored_result.has_value());

    xer::rand_context restored = *restored_result;

    for (int index = 0; index < 16; ++index) {
        xer_assert_eq(xer::rand(context), xer::rand(restored));
    }
}

} // namespace

int main()
{
    test_seeded_context_determinism();
    test_different_seeds();
    test_context_copy();
    test_default_context_reseed();
    test_to_bytes_from_bytes_round_trip();
    test_from_bytes_rejects_all_zero_state();
    test_default_constructed_context_is_serializable();

    return 0;
}
