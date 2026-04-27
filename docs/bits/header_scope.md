# `<xer/scope.h>`

## Purpose

`<xer/scope.h>` provides small scope-based utility facilities for XER.

At present, this header provides `xer::scope_exit`, a lightweight scope guard that invokes a registered callable when the guard object is destroyed.

This is useful for cleanup actions that should be performed automatically when leaving a block.

---

## Main Entity

At minimum, `<xer/scope.h>` provides the following entity:

```cpp
template <class F>
class scope_exit;
```

It also provides a deduction guide so that ordinary lambdas and function objects can be passed naturally:

```cpp
template <class F>
scope_exit(F&&) -> scope_exit<std::decay_t<F>>;
```

---

## `scope_exit`

`scope_exit` is a move-only RAII helper.

It stores a callable object and invokes it from the destructor while the guard is active.

```cpp
auto guard = xer::scope_exit([&] noexcept {
    cleanup();
});
```

### Basic Behavior

A `scope_exit` object is active immediately after construction.

When the object is destroyed:

* if it is active, the registered callable is invoked
* if it has been released, nothing is invoked

This makes it suitable for registering cleanup logic close to the resource or state change that needs cleanup.

---

## Typical Use Cases

`scope_exit` is useful for actions such as:

* restoring the current working directory after `chdir`
* removing a temporary file or temporary directory
* unlocking a resource
* restoring a global or process-wide setting
* rolling back a local state change
* closing or releasing a non-RAII resource when no better wrapper exists

It is especially useful when cleanup must happen along multiple return paths.

---

## Move-Only Semantics

`scope_exit` is copy-disabled and move-constructible.

Copying is not allowed because copying a guard would make it unclear which object owns the cleanup action.

Move construction transfers the cleanup responsibility to the new object.
The moved-from guard is released so that the registered callable is not invoked twice.

Move assignment is not provided.

---

## Releasing a Guard

A guard can be released explicitly:

```cpp
guard.release();
```

After `release()` is called, the registered callable will not be invoked when the guard is destroyed.

This is useful when the cleanup action is only needed if a later operation does not complete successfully.

---

## Checking Whether a Guard Is Active

The current active state can be checked:

```cpp
auto active() const noexcept -> bool;
```

This returns `true` if the callable will be invoked on destruction.

---

## Exception Policy

The destructor of `scope_exit` is `noexcept`.

The registered callable is expected not to throw.

If the callable throws while the `scope_exit` destructor is running, `std::terminate` is called.

This policy follows the usual expectation for cleanup code executed from destructors.
A scope guard is not a good place to report ordinary recoverable failure, because destructors cannot return `xer::result`.

If the cleanup action can fail in a meaningful way, callers should normally handle that failure explicitly before leaving the scope, or intentionally ignore it when failure is not actionable.

---

## Relationship to Standard and Experimental Facilities

XER provides its own `scope_exit`.

This is not a wrapper around a standard C++ `<scope>` header.
Standard C++ does not currently provide such a header as part of the ordinary standard library.

Some similar facilities exist in experimental or library-extension contexts, but XER keeps this utility small and self-contained so that it fits the library's header-only and GCC-oriented portability policy.

---

## Design Role

`scope_exit` is intentionally small.

It does not attempt to provide a complete scope-guard framework.

In particular, this header does not currently provide:

* `scope_success`
* `scope_fail`
* `unique_resource`

The initial purpose is only to support the common “run this cleanup action when leaving the current scope” pattern.

---

## Relationship to XER's Error Model

XER generally represents ordinary fallible operations through `xer::result`.

However, `scope_exit` itself does not return a result from its destructor.

For that reason, cleanup actions registered with `scope_exit` should normally be actions whose failure can be safely ignored or actions that are known not to fail in the intended context.

A common pattern is to explicitly discard the result of a cleanup operation:

```cpp
auto guard = xer::scope_exit([&] noexcept {
    static_cast<void>(xer::chdir(original));
});
```

This makes the decision to ignore cleanup failure visible.

---

## Example

```cpp
#include <xer/scope.h>
#include <xer/stdio.h>

auto main() -> int
{
    const auto original = xer::getcwd();
    if (!original.has_value()) {
        return 1;
    }

    {
        auto restore = xer::scope_exit([&] noexcept {
            static_cast<void>(xer::chdir(*original));
        });

        if (!xer::chdir(xer::path(u8"work")).has_value()) {
            return 1;
        }

        // Work inside another current directory.
    }

    return 0;
}
```

This example shows the typical XER style:

* acquire or record state explicitly
* register cleanup with `scope_exit`
* use `noexcept` for the cleanup lambda when practical
* explicitly discard cleanup results when failure is not actionable

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that `scope_exit` is a move-only scope guard
* that it calls the registered callable from its destructor
* that `release()` disables the cleanup action
* that the registered callable should not throw
* that `scope_success` and `scope_fail` are intentionally not provided at this stage

Detailed examples should focus on practical cleanup patterns rather than abstract RAII theory.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* restoring the current working directory after `chdir`
* deleting a temporary file on scope exit
* releasing a guard after successful completion
* moving a guard to transfer cleanup responsibility

These are good candidates for executable examples under `examples/`.

---

## See Also

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `header_stdio.md`
