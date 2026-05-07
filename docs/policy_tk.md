# Policy for Tcl/Tk GUI Support

## Overview

XER provides Tcl/Tk support through `<xer/tk.h>` in order to add a lightweight GUI facility that fits the rest of the library.

The purpose of this facility is not to wrap every Tcl/Tk feature with a large C++ framework.
Instead, XER uses Tcl/Tk as a practical GUI backend and exposes a small C++ interface that handles initialization, interpreter lifetime, command evaluation, command registration, variable access, and the event loop.

The initial goal is to make Tcl/Tk usable from XER programs while preserving the ability to fall back to the native Tcl/Tk C API when necessary.

---

## Public Header

Tcl/Tk support is provided through the public header:

```text
xer/tk.h
```

The implementation may be split internally into:

```text
xer/bits/tcl.h
xer/bits/tk.h
```

The internal split does not imply that both `xer::tcl` and `xer::tk` are public namespaces.
In the initial public API, Tcl/Tk support is exposed through the single namespace:

```cpp
namespace xer::tk;
```

This is because the feature is introduced as GUI support based on Tcl/Tk, not as a separate general-purpose Tcl library.
If a future version needs to expose Tcl-only functionality independently, a separate `<xer/tcl.h>` and `xer::tcl` namespace may be considered then.

---

## Namespace Policy

The public API uses `xer::tk` rather than `tcl_` or `tk_` prefixes on individual names.

For example, the intended style is:

```cpp
auto interp = xer::tk::create_interpreter();
if (!interp.has_value()) {
    return 1;
}

if (!xer::tk::init(*interp).has_value()) {
    return 1;
}

if (!xer::tk::eval(*interp, u8"wm title . XER").has_value()) {
    return 1;
}
```

Names such as `tcl_eval`, `tk_init`, or `tcl_create_command` are not used in the public API.
The namespace already provides the Tcl/Tk context.

---

## Relationship Between Tcl and Tk

Although Tcl and Tk are separate technologies, XER initially treats them as one GUI facility.

The public initialization function:

```cpp
auto init(interpreter& interp) -> xer::result<void, error_detail>;
```

performs both Tcl initialization and Tk initialization.
Conceptually, it performs:

```text
Tcl_Init
Tk_Init
```

in that order.

A separate public `init_tcl` function is not provided in the initial API.
If a user truly needs to call Tcl-only initialization or lower-level Tcl functions directly, the native interpreter handle can be obtained through `to_native_handle` and the Tcl C API can be called explicitly.

This keeps the ordinary public API focused on Tcl/Tk GUI usage.

---

## Avoiding `Tk_Main`

XER does not use `Tk_Main` as the foundation of the public API.

`Tk_Main` is unsuitable because it owns too much of the program startup and shutdown flow.
In particular, returning from the callback passed to `Tk_Main` can terminate the process.
That behavior does not fit a library that should be usable from ordinary functions, tests, and user-managed threads.

Instead, XER constructs and initializes the Tcl/Tk interpreter explicitly and exposes event-loop related operations separately.

At minimum, XER may provide:

```cpp
auto main_loop() -> void;
auto do_one_event(int flags = 0) -> int;
```

The exact naming and behavior should remain close to Tcl/Tk while fitting XER naming style.

---

## Thread Policy

XER aims to allow Tcl/Tk to run on a user-chosen thread.

This does not mean that one interpreter may be freely used from arbitrary threads.
A Tcl/Tk interpreter is treated as a thread-affine object.

The basic rule is:

```text
An interpreter should be created, initialized, used, and destroyed on the same thread.
```

XER may allow the thread to be a non-main thread, but it does not make a single interpreter safely callable from any thread.

Cross-thread notification, event posting, and thread-safe dispatch may be considered later as separate features.
They are not part of the initial Tcl/Tk API.

---

## `Tcl_FindExecutable`

Tcl requires `Tcl_FindExecutable` to be called in ordinary embedding scenarios.

XER provides an initialization helper such as:

```cpp
auto find_executable() -> xer::result<void>;
```

This function should use XER's command-line support to obtain the executable path.
The intended source is `xer::cmdline`, because it already provides XER's platform-specific command-line handling.

The function should be safe to call multiple times.
If necessary, XER may internally guard the call so that the Tcl executable path is initialized once per process.

---

## Interpreter Type

A Tcl/Tk interpreter is represented by a move-only RAII type:

```cpp
namespace xer::tk {

class interpreter;

}
```

The interpreter owns a `Tcl_Interp*` and releases it by calling the appropriate Tcl deletion function in its destructor.

The type is:

- non-copyable
- movable
- RAII-based
- lightweight as a handle type

The ordinary way to create an interpreter is:

```cpp
auto create_interpreter() -> xer::result<interpreter>;
```

The returned interpreter is not automatically initialized for Tcl/Tk use.
The caller should call `xer::tk::init` before evaluating Tk scripts or creating Tk widgets.

---

## Native Handle Access

The public `interpreter` type does not provide a casual `get()` member function.

Retrieving the raw `Tcl_Interp*` breaks the normal abstraction boundary and should look like an explicit escape hatch.
For that reason, XER follows the same general style as native-handle access for stream-like facilities and provides a free function:

```cpp
auto to_native_handle(interpreter& interp) noexcept -> Tcl_Interp*;
```

A `const interpreter&` overload is not provided.
The Tcl C API generally uses `Tcl_Interp*` to mutate interpreter state, and a const overload would not provide meaningful const-safety.

Users who call `to_native_handle` are responsible for respecting Tcl/Tk lifetime, threading, and reference-counting rules.

---

## Evaluation Policy

Script evaluation is based on Tcl object APIs rather than string-only APIs.

XER should use:

```text
Tcl_EvalObjEx
```

rather than:

```text
Tcl_EvalEx
```

The input script is converted from `std::u8string_view` to a `Tcl_Obj*`, and that object is evaluated with `Tcl_EvalObjEx`.

The result of evaluation is obtained through:

```text
Tcl_GetObjResult
```

and converted to `std::u8string` using:

```text
Tcl_GetStringFromObj
```

`Tcl_GetStringResult` is not used as the primary result path.

The basic public API is:

```cpp
auto eval(interpreter& interp, std::u8string_view script)
    -> xer::result<std::u8string, error_detail>;
```

The initial implementation may use `TCL_EVAL_DIRECT` by default.
Bytecode behavior and evaluation options may be exposed later if a real need appears.

---

## Error Detail

Tcl/Tk failures often produce useful interpreter result text.
XER should preserve that information in a dedicated detail type.

At minimum, Tcl/Tk errors may use:

```cpp
namespace xer::tk {

struct error_detail {
    std::u8string message;
};

}
```

Fallible Tcl/Tk APIs then return `xer::result<T, error_detail>` where Tcl/Tk diagnostic text is useful.

Examples include:

```cpp
auto init(interpreter& interp) -> xer::result<void, error_detail>;
auto eval(interpreter& interp, std::u8string_view script)
    -> xer::result<std::u8string, error_detail>;
auto set_var(...) -> xer::result<void, error_detail>;
auto get_var(...) -> xer::result<std::u8string, error_detail>;
```

The message should normally be obtained from `Tcl_GetObjResult`.

---

## Tcl Object Reference Handling

Tcl objects are represented by `Tcl_Obj*` and managed by Tcl reference counts.

XER may use a small internal RAII helper:

```cpp
namespace xer::tk::detail {

class obj_ref;

}
```

This type owns one reference to a `Tcl_Obj*`.

Its copy operations increment the Tcl reference count:

```text
Tcl_IncrRefCount
```

Its destructor decrements the Tcl reference count:

```text
Tcl_DecrRefCount
```

Move operations transfer the owned reference.

This helper is initially an implementation detail and is not exposed as `xer::tk::obj_ref` in the public API.
If users later need first-class Tcl object handling, the type may be promoted or a public object wrapper may be designed separately.

---

## String Conversion Policy

XER's public text model uses UTF-8 strings based on `char8_t`.
Tcl/Tk C APIs use `char*` strings that are conventionally UTF-8 in modern Tcl.

XER converts between these representations at API boundaries.

The ordinary public string forms are:

```cpp
std::u8string
std::u8string_view
const char8_t*
```

When passing text to Tcl, XER converts from UTF-8 code units to the `const char*` pointer expected by Tcl.
When receiving text from Tcl, XER copies the returned bytes into `std::u8string`.

XER should validate sizes before passing lengths to Tcl APIs that require `int` lengths.
If a string is too large to be represented by Tcl's `int` length parameter, the function should fail rather than truncate.

---

## Variable Access

Tcl variable access should use Tcl object APIs.

XER uses:

```text
Tcl_ObjSetVar2
Tcl_ObjGetVar2
```

rather than older string-only variable APIs.

The public API should provide ordinary string-oriented helpers such as:

```cpp
auto set_var(
    interpreter& interp,
    std::u8string_view name,
    std::u8string_view value,
    int flags = TCL_GLOBAL_ONLY) -> xer::result<void, error_detail>;

auto set_var(
    interpreter& interp,
    std::u8string_view name1,
    std::u8string_view name2,
    std::u8string_view value,
    int flags = TCL_GLOBAL_ONLY) -> xer::result<void, error_detail>;

auto get_var(
    interpreter& interp,
    std::u8string_view name,
    int flags = TCL_GLOBAL_ONLY) -> xer::result<std::u8string, error_detail>;

auto get_var(
    interpreter& interp,
    std::u8string_view name1,
    std::u8string_view name2,
    int flags = TCL_GLOBAL_ONLY) -> xer::result<std::u8string, error_detail>;
```

The two-name overloads correspond to Tcl array-style variable access through `Tcl_ObjSetVar2` and `Tcl_ObjGetVar2`.

---

## Command Registration

XER provides a wrapper around Tcl command registration.

The underlying Tcl function is:

```text
Tcl_CreateObjCommand
```

The public API should allow ordinary C++ callables, including lambdas, to be registered as Tcl commands.

A typical intended use is:

```cpp
xer::tk::create_command(interp, u8"add", [](int a, int b) {
    return a + b;
});
```

Tcl command arguments are received as a `Tcl_Obj*` array and converted into the callable's C++ parameter types.

The initial supported argument types may include:

- `Tcl_Obj*`
- `bool`
- signed integer types
- unsigned integer types where safe conversion is defined
- floating-point types
- `std::u8string`
- `std::u8string_view` where lifetime can be handled safely

The initial supported return types may include:

- `void`
- `bool`
- integer types
- floating-point types
- `std::u8string`
- `std::u8string_view`
- `const char8_t*`
- `Tcl_Obj*`
- `xer::result<T, error_detail>` or an equivalent XER result form

The exact supported type set may grow incrementally.
Unsupported parameter or return types should fail at compile time where practical.

---

## Command Callback Scope

Tcl command callbacks are the first priority.

Widget callbacks that use Tk substitution parameters such as `%w` are deferred.

For example, support for command strings that pass widget path names, event coordinates, or other Tk substitution values should be considered later as part of widget and event handling.
It should not be mixed into the first command-registration implementation.

---

## Callback Lifetime Management

When a C++ callable is registered as a Tcl command, XER owns the stored callable object until Tcl deletes the command.

The callable may be allocated dynamically and passed to Tcl through `ClientData`.
The delete callback supplied to `Tcl_CreateObjCommand` releases that stored callable.

This ownership model should be hidden from users.
Users should not need to manually manage the lifetime of the registered callable as long as the interpreter remains valid.

---

## Event Loop Policy

The event-loop API should avoid taking over the entire process.

At minimum, XER may expose:

```cpp
auto main_loop() -> void;
auto do_one_event(int flags = 0) -> int;
```

`main_loop` may wrap `Tk_MainLoop` or an equivalent loop, but it must not rely on `Tk_Main`.

`do_one_event` is important because it allows user-managed loops, tests, and non-main-thread GUI execution patterns.

More advanced event integration, timers, idle handlers, cross-thread event posting, and custom dispatch helpers are deferred.

---

## Initial Scope

The initial Tcl/Tk support should focus on the following:

- public header `<xer/tk.h>`
- namespace `xer::tk`
- `interpreter` RAII wrapper
- explicit executable initialization through `find_executable`
- interpreter creation through `create_interpreter`
- combined `Tcl_Init` and `Tk_Init` through `init`
- script evaluation through `Tcl_EvalObjEx`
- result retrieval through `Tcl_GetObjResult`
- Tcl variable access through `Tcl_ObjSetVar2` and `Tcl_ObjGetVar2`
- command registration through `Tcl_CreateObjCommand`
- conversion between `Tcl_Obj*` arguments and C++ callable parameters
- conversion from C++ callable return values to Tcl results
- basic event-loop helpers

---

## Deferred Items

The following are intentionally deferred:

- full widget wrapper classes
- layout helper APIs
- Tk event substitution values such as `%w`
- high-level callback binding helpers
- cross-thread event dispatch
- TomMath integration
- complete Tcl object public wrapper API
- full Tcl/Tk command coverage
- theme and style abstraction
- image handling
- menu and dialog convenience APIs
- asynchronous integration with sockets or process handling

These should be added only when the lower-level Tcl/Tk foundation has stabilized.

---

## Testing Policy

Tcl/Tk tests are environment-dependent.
Some environments may have Tcl installed without Tk, or may lack the required development headers.

The PHP test runner should detect Tcl/Tk availability and skip Tcl/Tk tests when the required environment is unavailable.
When Tcl/Tk is available, the tests should compile and run normally.

Header combination tests must also understand that `<xer/tk.h>` requires Tcl/Tk development headers.
If Tcl/Tk is unavailable, pairs involving `<xer/tk.h>` may be skipped rather than reported as ordinary header collisions.

MSYS2 and Debian-family Linux systems may require different include-path handling.
Debian-family systems may need paths such as:

```text
/usr/include/tcl
/usr/include/tk
```

MSYS2 should normally rely on its ordinary compiler include paths or `pkg-config` rather than Debian-specific include directories.

---

## Documentation and Examples

Tcl/Tk examples should be placed under `examples/` like other XER examples.

Initial examples should be small and should avoid depending on complex user interaction.
Good initial examples include:

- creating an interpreter
- initializing Tcl/Tk
- evaluating a simple script
- setting and getting Tcl variables
- registering a simple C++ command and calling it from Tcl

Graphical widget examples may be added after the lower-level foundation is stable.

---

## Summary

- Tcl/Tk support is exposed through `<xer/tk.h>` and `xer::tk`
- `Tk_Main` is not used
- `Tcl_Init` and `Tk_Init` are combined in `xer::tk::init`
- interpreters are RAII handle objects and are thread-affine
- raw `Tcl_Interp*` access is available only through `to_native_handle(interpreter&)`
- evaluation uses `Tcl_EvalObjEx`
- command results are obtained through `Tcl_GetObjResult`
- variable access uses `Tcl_ObjSetVar2` and `Tcl_ObjGetVar2`
- command registration supports C++ callables and `Tcl_Obj*` argument conversion
- widget callbacks, `%w` substitution handling, and TomMath are deferred
