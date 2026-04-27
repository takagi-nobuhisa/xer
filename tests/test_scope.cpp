/**
 * @file tests/test_scope.cpp
 * @brief Tests for scope guard utilities.
 */

#include <utility>

#include <xer/assert.h>
#include <xer/scope.h>

namespace {

void test_scope_exit_runs_on_scope_exit()
{
    int value = 0;

    {
        auto guard = xer::scope_exit([&] {
            value = 42;
        });

        xer_assert(guard.active());
        xer_assert_eq(value, 0);
    }

    xer_assert_eq(value, 42);
}

void test_scope_exit_release()
{
    int value = 0;

    {
        auto guard = xer::scope_exit([&] {
            value = 42;
        });

        xer_assert(guard.active());

        guard.release();

        xer_assert_not(guard.active());
    }

    xer_assert_eq(value, 0);
}

void test_scope_exit_move_construct()
{
    int value = 0;

    {
        auto guard1 = xer::scope_exit([&] {
            ++value;
        });

        xer_assert(guard1.active());

        auto guard2 = std::move(guard1);

        xer_assert_not(guard1.active());
        xer_assert(guard2.active());
    }

    xer_assert_eq(value, 1);
}

void test_scope_exit_runs_on_early_return_impl(int& value)
{
    auto guard = xer::scope_exit([&] {
        value = 100;
    });

    if (value == 0) {
        return;
    }

    value = 1;
}

void test_scope_exit_runs_on_early_return()
{
    int value = 0;

    test_scope_exit_runs_on_early_return_impl(value);

    xer_assert_eq(value, 100);
}

void test_scope_exit_can_capture_by_value()
{
    int value = 0;

    {
        const int captured = 7;

        auto guard = xer::scope_exit([&, captured] {
            value = captured;
        });

        xer_assert(guard.active());
    }

    xer_assert_eq(value, 7);
}

void test_scope_exit_is_not_copyable()
{
    using guard_type = decltype(xer::scope_exit([] {}));

    static_assert(!std::is_copy_constructible_v<guard_type>);
    static_assert(!std::is_copy_assignable_v<guard_type>);
}

void test_scope_exit_is_move_constructible()
{
    using guard_type = decltype(xer::scope_exit([] {}));

    static_assert(std::is_move_constructible_v<guard_type>);
    static_assert(!std::is_move_assignable_v<guard_type>);
}

} // namespace

auto main() -> int
{
    test_scope_exit_runs_on_scope_exit();
    test_scope_exit_release();
    test_scope_exit_move_construct();
    test_scope_exit_runs_on_early_return();
    test_scope_exit_can_capture_by_value();
    test_scope_exit_is_not_copyable();
    test_scope_exit_is_move_constructible();

    return 0;
}
