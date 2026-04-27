/**
 * @file xer/bits/scope_exit.h
 * @brief Scope-exit guard implementation.
 */

#pragma once

#ifndef XER_BITS_SCOPE_EXIT_H_INCLUDED_
#define XER_BITS_SCOPE_EXIT_H_INCLUDED_

#include <exception>
#include <type_traits>
#include <utility>

namespace xer {

/**
 * @brief Calls a registered function when leaving the current scope.
 *
 * `scope_exit` is a small move-only scope guard. It stores a callable object
 * and invokes it from the destructor while the guard is active.
 *
 * This class is useful for cleanup actions that should happen when a block is
 * left, regardless of whether the block is left normally or by exception.
 *
 * The registered callable is expected not to throw. The destructor is
 * `noexcept`; if the callable throws, `std::terminate` is called. This matches
 * the normal expectation for cleanup code executed from destructors.
 *
 * @tparam F Callable type.
 */
template <class F>
class scope_exit {
public:
    /**
     * @brief Constructs an active scope guard.
     *
     * @tparam Fn Source callable type.
     * @param function Callable to invoke on scope exit.
     */
    template <class Fn>
        requires std::constructible_from<F, Fn&&>
    explicit scope_exit(Fn&& function) noexcept(
        std::is_nothrow_constructible_v<F, Fn&&>)
        : function_(std::forward<Fn>(function))
        , active_(true)
    {
    }

    scope_exit(const scope_exit&) = delete;
    auto operator=(const scope_exit&) -> scope_exit& = delete;

    /**
     * @brief Move-constructs a scope guard.
     *
     * The moved-from guard is released so that the registered function is not
     * called twice.
     *
     * @param other Source guard.
     */
    scope_exit(scope_exit&& other) noexcept(
        std::is_nothrow_move_constructible_v<F>)
        : function_(std::move(other.function_))
        , active_(other.active_)
    {
        other.release();
    }

    auto operator=(scope_exit&&) -> scope_exit& = delete;

    /**
     * @brief Destroys the scope guard.
     *
     * If the guard is active, the registered callable is invoked. The callable
     * should not throw. If it throws, this destructor calls `std::terminate`.
     */
    ~scope_exit() noexcept
    {
        if (!active_) {
            return;
        }

        if constexpr (std::is_nothrow_invocable_v<F&>) {
            function_();
        } else {
            try {
                function_();
            } catch (...) {
                std::terminate();
            }
        }
    }

    /**
     * @brief Releases the guard without invoking the registered callable.
     */
    auto release() noexcept -> void
    {
        active_ = false;
    }

    /**
     * @brief Returns whether the guard is active.
     *
     * @return true if the registered callable will be invoked on destruction.
     */
    [[nodiscard]] auto active() const noexcept -> bool
    {
        return active_;
    }

private:
    F function_;
    bool active_;
};

/**
 * @brief Deduction guide for `scope_exit`.
 *
 * The callable is stored by value after decay. This allows lambdas and function
 * objects to be passed naturally while avoiding dangling references caused by
 * storing a temporary callable by reference.
 */
template <class F>
scope_exit(F&&) -> scope_exit<std::decay_t<F>>;

} // namespace xer

#endif /* XER_BITS_SCOPE_EXIT_H_INCLUDED_ */
