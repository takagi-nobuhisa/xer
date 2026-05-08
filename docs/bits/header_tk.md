# `<xer/tk.h>`

## Purpose

`<xer/tk.h>` provides the initial Tcl/Tk integration layer for XER.

The first purpose of this header is to make it possible to create and control a Tcl interpreter from C++ code, register C++ callables as Tcl commands, and then use that foundation for Tk-based GUI facilities.

The design deliberately avoids `Tk_Main`. `Tk_Main` owns too much of the process lifetime and may terminate the whole process when the main function exits. XER instead exposes interpreter creation, initialization, script evaluation, command registration, variable access, and event-loop helpers as ordinary C++ APIs.

---

## Main Entities

At minimum, `<xer/tk.h>` provides the following entities:

```cpp
namespace xer::tk {

using result_code_t = int;
using eval_flag_t = int;
using var_flag_t = int;
using event_flag_t = int;

inline constexpr result_code_t result_ok;
inline constexpr result_code_t result_error;
inline constexpr result_code_t result_return;
inline constexpr result_code_t result_break;
inline constexpr result_code_t result_continue;

inline constexpr eval_flag_t eval_direct;

inline constexpr var_flag_t var_none;
inline constexpr var_flag_t var_global_only;
inline constexpr var_flag_t var_namespace_only;
inline constexpr var_flag_t var_leave_error_msg;
inline constexpr var_flag_t var_append_value;
inline constexpr var_flag_t var_list_element;

inline constexpr event_flag_t event_all;
inline constexpr event_flag_t event_window;
inline constexpr event_flag_t event_file;
inline constexpr event_flag_t event_timer;
inline constexpr event_flag_t event_idle;
inline constexpr event_flag_t event_dont_wait;

struct error_detail;
class interpreter;
class obj;
class photo_image;

using photo_composite_rule_t = int;
using photo_image_block = Tk_PhotoImageBlock;

inline constexpr photo_composite_rule_t photo_composite_overlay;
inline constexpr photo_composite_rule_t photo_composite_set;

struct photo_size {
    int width;
    int height;
};

auto find_executable() -> xer::result<void, error_detail>;
auto init(interpreter& interp) -> xer::result<void, error_detail>;
auto get_result(interpreter& interp) -> std::u8string;
auto reset_result(interpreter& interp) noexcept -> void;

auto eval(interpreter& interp, std::u8string_view script,
          eval_flag_t flags = eval_direct)
    -> xer::result<std::u8string, error_detail>;

auto make_string_obj(std::u8string_view value)
    -> xer::result<obj, error_detail>;
auto make_int_obj(int value) -> obj;
auto make_list_obj(std::span<const std::u8string_view> values)
    -> xer::result<obj, error_detail>;
auto make_list_obj(std::initializer_list<std::u8string_view> values)
    -> xer::result<obj, error_detail>;

auto set_var(interpreter& interp,
             std::u8string_view name,
             std::u8string_view value,
             var_flag_t flags = var_global_only)
    -> xer::result<void, error_detail>;

auto set_var(interpreter& interp,
             std::u8string_view name1,
             std::u8string_view name2,
             std::u8string_view value,
             var_flag_t flags = var_global_only)
    -> xer::result<void, error_detail>;

auto set_var(interpreter& interp,
             std::u8string_view name,
             obj& value,
             var_flag_t flags = var_global_only)
    -> xer::result<void, error_detail>;

auto set_var(interpreter& interp,
             std::u8string_view name1,
             std::u8string_view name2,
             obj& value,
             var_flag_t flags = var_global_only)
    -> xer::result<void, error_detail>;

auto get_var(interpreter& interp,
             std::u8string_view name,
             var_flag_t flags = var_global_only)
    -> xer::result<std::u8string, error_detail>;

auto get_var(interpreter& interp,
             std::u8string_view name1,
             std::u8string_view name2,
             var_flag_t flags = var_global_only)
    -> xer::result<std::u8string, error_detail>;

template <class F>
auto create_command(interpreter& interp,
                    std::u8string_view name,
                    F&& callable)
    -> xer::result<void, error_detail>;

auto to_native_handle(interpreter& interp) noexcept -> Tcl_Interp*;
auto to_native_handle(obj& value) noexcept -> Tcl_Obj*;
auto to_native_handle(photo_image image) noexcept -> Tk_PhotoHandle;

auto find_photo(interpreter& interp, const char8_t* name)
    -> xer::result<photo_image, error_detail>;

auto photo_get_size(photo_image image) noexcept -> photo_size;
auto photo_blank(photo_image image) noexcept -> void;

auto photo_expand(interpreter& interp, photo_image image, int width, int height)
    -> xer::result<void, error_detail>;

auto photo_set_size(interpreter& interp, photo_image image, int width, int height)
    -> xer::result<void, error_detail>;

auto photo_get_image(photo_image image, photo_image_block* block)
    -> xer::result<void, error_detail>;

auto photo_put_block(interpreter& interp,
                     photo_image image,
                     photo_image_block* block,
                     int x,
                     int y,
                     int width,
                     int height,
                     photo_composite_rule_t rule = photo_composite_set)
    -> xer::result<void, error_detail>;

auto photo_put_zoomed_block(interpreter& interp,
                            photo_image image,
                            photo_image_block* block,
                            int x,
                            int y,
                            int width,
                            int height,
                            int zoom_x,
                            int zoom_y,
                            int subsample_x,
                            int subsample_y,
                            photo_composite_rule_t rule = photo_composite_set)
    -> xer::result<void, error_detail>;

template <class F>
auto main(F&& callback) -> xer::result<void>;

auto main_loop() -> void;
auto do_one_event(event_flag_t flags = event_all) -> int;

}
```

The exact set of supported callable argument and return types may expand as the Tcl/Tk layer develops.

---

## Namespace

All public Tcl/Tk integration APIs are placed in:

```cpp
namespace xer::tk
```

The initial public API does not provide a separate `xer::tcl` namespace. Tcl is treated as the scripting foundation used by the Tk integration layer.

If a future version provides a public Tcl-only header, a separate namespace may be reconsidered at that time.

---

## Result Codes and Flags

Tcl exposes result codes and flags through C macros such as `TCL_OK`, `TCL_ERROR`, and `TCL_GLOBAL_ONLY`.

XER does not require ordinary users to refer to those macros directly. Instead, `<xer/tk.h>` provides XER-named constants such as:

```cpp
xer::tk::result_ok
xer::tk::result_error
xer::tk::result_break
xer::tk::var_global_only
xer::tk::eval_direct
xer::tk::event_all
```

These constants preserve the native integer values used by Tcl/Tk while keeping the public API vocabulary under `xer::tk`.

---

## Error Detail

```cpp
struct error_detail {
    result_code_t result_code;
};
```

`error_detail` stores the Tcl/Tk result code associated with a failed operation.

For `eval`, the stored code is the return value of `Tcl_EvalObjEx` when it is not `result_ok`. For operations that fail through a null Tcl return value, `result_error` is used.

The current Tcl result string is not stored inside `error_detail`. When the caller needs the current interpreter result text, it can call:

```cpp
auto get_result(interpreter& interp) -> std::u8string;
```

This keeps the error detail small and avoids copying Tcl result text unless it is actually needed.

---

## `interpreter`

```cpp
class interpreter;
```

`interpreter` is a move-only RAII wrapper around `Tcl_Interp*`.

### Creation

```cpp
auto interpreter::create() -> xer::result<interpreter, error_detail>;
```

`interpreter::create` initializes Tcl executable information if necessary and then creates a new Tcl interpreter.

Ordinary code should create interpreters through this static member function:

```cpp
auto interp = xer::tk::interpreter::create();
```

### Lifetime

The interpreter owns its native `Tcl_Interp*` handle.

When the `interpreter` object is destroyed, the native interpreter is deleted. The type is move-only so that ownership remains explicit.

### Validity

```cpp
auto valid() const noexcept -> bool;
```

`valid()` returns whether the wrapper currently owns a native interpreter handle.

---

## Initialization

```cpp
auto init(interpreter& interp) -> xer::result<void, error_detail>;
```

`init` initializes both Tcl and Tk for the specified interpreter.

It calls `Tcl_Init` and then `Tk_Init`. XER intentionally does not provide a public API that calls only `Tcl_Init` in the `xer::tk` layer.

If a caller needs direct Tcl-only initialization behavior, the native handle can be obtained through `to_native_handle` and the Tcl C API can be called explicitly.

---

## Native Handle Escape Hatch

```cpp
auto to_native_handle(interpreter& interp) noexcept -> Tcl_Interp*;
```

`to_native_handle` returns the underlying `Tcl_Interp*`.

This function is an escape hatch for direct Tcl/Tk C API use. It is intentionally not named like an ordinary accessor such as `get`, because using the native handle bypasses part of XER's abstraction.

There is no overload for `const interpreter&`. Tcl interpreter handles are commonly used for state-changing operations, and a const overload would not provide meaningful const safety.

---

## Photo Image Helpers

`<xer/tk.h>` provides thin wrappers for Tk photo image APIs such as `Tk_FindPhoto`, `Tk_PhotoGetImage`, `Tk_PhotoPutBlock`, and related size operations.

These helpers are intentionally part of the Tk integration layer rather than the general image-processing layer. Pure image processing and framebuffer manipulation belong in `<xer/image.h>`, while `<xer/tk.h>` only handles the bridge to Tk photo images.

### `photo_image`

```cpp
class photo_image;
```

`photo_image` is a non-owning handle for an existing Tk photo image.

A `photo_image` value cannot be default-constructed. It is created only by `find_photo`, so it does not represent a null photo handle in ordinary XER code.

The type does not own the underlying Tk image. The caller must ensure that the Tcl/Tk photo image outlives any `photo_image` handle referring to it. If the image is deleted on the Tcl/Tk side, an existing `photo_image` becomes invalid by the native Tk lifetime rules.

The native escape hatch is:

```cpp
auto to_native_handle(photo_image image) noexcept -> Tk_PhotoHandle;
```

### Finding a Photo Image

```cpp
auto find_photo(interpreter& interp, const char8_t* name)
    -> xer::result<photo_image, error_detail>;
```

`find_photo` searches for an existing Tk photo image by name.

The `name` argument is a null-terminated UTF-8 string. This follows the native Tk API, which accepts a `const char*` image name. Unlike `eval`, this function does not take `std::u8string_view`, because the native API requires a null-terminated string rather than an object with an explicit length.

If `name` is null, the function fails with `error_t::invalid_argument`. If the named image does not exist or is not a photo image, the function fails with `error_t::not_found`.

### Size Operations

```cpp
struct photo_size {
    int width;
    int height;
};

auto photo_get_size(photo_image image) noexcept -> photo_size;

auto photo_expand(interpreter& interp, photo_image image, int width, int height)
    -> xer::result<void, error_detail>;

auto photo_set_size(interpreter& interp, photo_image image, int width, int height)
    -> xer::result<void, error_detail>;
```

`photo_get_size` returns the current Tk photo size.

`photo_expand` expands the photo image to at least the requested size. If the photo image has an explicit `-width` or `-height` set on the Tcl/Tk side, Tk may preserve that explicit dimension instead of expanding it.

`photo_set_size` sets the explicit size of the photo image. Passing zero for both dimensions can be used to clear the explicit size in the same manner as the native Tk API.

Negative dimensions are rejected as `error_t::invalid_argument` before calling Tk.

### Blank and Block Operations

```cpp
using photo_image_block = Tk_PhotoImageBlock;
using photo_composite_rule_t = int;

inline constexpr photo_composite_rule_t photo_composite_overlay;
inline constexpr photo_composite_rule_t photo_composite_set;

auto photo_blank(photo_image image) noexcept -> void;

auto photo_get_image(photo_image image, photo_image_block* block)
    -> xer::result<void, error_detail>;

auto photo_put_block(interpreter& interp,
                     photo_image image,
                     photo_image_block* block,
                     int x,
                     int y,
                     int width,
                     int height,
                     photo_composite_rule_t rule = photo_composite_set)
    -> xer::result<void, error_detail>;

auto photo_put_zoomed_block(interpreter& interp,
                            photo_image image,
                            photo_image_block* block,
                            int x,
                            int y,
                            int width,
                            int height,
                            int zoom_x,
                            int zoom_y,
                            int subsample_x,
                            int subsample_y,
                            photo_composite_rule_t rule = photo_composite_set)
    -> xer::result<void, error_detail>;
```

`photo_blank` clears the photo image.

`photo_get_image` fills a `photo_image_block` descriptor for the current Tk photo image. The block points to Tk-managed memory and should be treated according to Tk's lifetime rules.

`photo_put_block` writes a block into the photo image.

`photo_put_zoomed_block` writes a block while applying integer zooming and subsampling.

The block pointer must not be null. Coordinates, width, and height must not be negative. Zoom and subsample factors must be positive. Invalid arguments are rejected before calling Tk.

### Relationship to XER Image Facilities

The photo block helpers are low-level Tk wrappers. They intentionally expose `Tk_PhotoImageBlock` through the alias `photo_image_block` so that code can use the native Tk block layout when needed.

Higher-level conversion between Tk photo images and XER image or framebuffer types should be built separately on top of these helpers. The ordinary image-processing algorithms themselves should not depend on Tcl/Tk.

---

## Tcl Objects

```cpp
class obj;

auto make_string_obj(std::u8string_view value)
    -> xer::result<obj, error_detail>;
auto make_int_obj(int value) -> obj;
auto make_list_obj(std::span<const std::u8string_view> values)
    -> xer::result<obj, error_detail>;
auto make_list_obj(std::initializer_list<std::u8string_view> values)
    -> xer::result<obj, error_detail>;

auto to_native_handle(obj& value) noexcept -> Tcl_Obj*;
```

`obj` is a RAII wrapper for `Tcl_Obj*`. It owns one Tcl reference to the wrapped object. Construction and copying increment the Tcl reference count, destruction decrements it, and moving transfers the owned reference without changing the Tcl reference count.

`obj` has no public constructor from `Tcl_Obj*`. Ordinary code should create objects through factory functions such as `make_string_obj`, `make_int_obj`, and `make_list_obj`.

`to_native_handle(obj&)` returns a borrowed `Tcl_Obj*` and does not change the Tcl reference count. The returned pointer should not be released by the caller.

`make_list_obj` creates a real Tcl list object. It is useful when a value must behave as a Tcl list rather than as a manually joined string.

---

## Tcl Result Handling

```cpp
auto get_result(interpreter& interp) -> std::u8string;
auto reset_result(interpreter& interp) noexcept -> void;
```

`get_result` returns the current Tcl interpreter result as UTF-8 text. Internally, it retrieves the result object with `Tcl_GetObjResult` and converts it to text with `Tcl_GetStringFromObj`.

`reset_result` clears the current interpreter result.

---

## Script Evaluation

```cpp
auto eval(interpreter& interp,
          std::u8string_view script,
          eval_flag_t flags = eval_direct)
    -> xer::result<std::u8string, error_detail>;
```

`eval` evaluates Tcl script text using `Tcl_EvalObjEx`.

The script is first converted into a Tcl object. On success, the current Tcl result is returned as a `std::u8string`. On failure, the returned error detail stores the Tcl result code.

The default evaluation flag is `eval_direct`.

---

## Variable Access

`<xer/tk.h>` provides variable access based on Tcl object APIs.

```cpp
auto set_var(interpreter& interp,
             std::u8string_view name,
             std::u8string_view value,
             var_flag_t flags = var_global_only)
    -> xer::result<void, error_detail>;

auto get_var(interpreter& interp,
             std::u8string_view name,
             var_flag_t flags = var_global_only)
    -> xer::result<std::u8string, error_detail>;
```

These functions use `Tcl_ObjSetVar2` and `Tcl_ObjGetVar2`.

In addition to UTF-8 text values, `set_var` can accept `xer::tk::obj`. This allows callers to assign Tcl objects such as list objects without converting them into manually joined strings.

Array-variable forms are also provided:

```cpp
auto set_var(interpreter& interp,
             std::u8string_view name1,
             std::u8string_view name2,
             std::u8string_view value,
             var_flag_t flags = var_global_only)
    -> xer::result<void, error_detail>;

auto get_var(interpreter& interp,
             std::u8string_view name1,
             std::u8string_view name2,
             var_flag_t flags = var_global_only)
    -> xer::result<std::u8string, error_detail>;
```

The default variable flag is `var_global_only`.

For example, a Tcl list can be assigned as follows:

```cpp
auto list = xer::tk::make_list_obj({u8"first value", u8"second"});
if (!list.has_value()) {
    return 1;
}

if (!xer::tk::set_var(interp, u8"argv", *list).has_value()) {
    return 1;
}
```

Tcl code can then use ordinary list operations such as `llength $argv` and `lindex $argv 0`.

---

## Command Registration

```cpp
template <class F>
auto create_command(interpreter& interp,
                    std::u8string_view name,
                    F&& callable)
    -> xer::result<void, error_detail>;
```

`create_command` registers a C++ callable as a Tcl object command.

The command name is UTF-8 text. The callable may be a function object, function pointer, or lambda expression.

A simple example is:

```cpp
auto created = xer::tk::create_command(
    interp,
    u8"add",
    [](int a, int b) -> int {
        return a + b;
    });
```

After registration, Tcl script can call:

```tcl
add 10 20
```

The initial command bridge supports a practical subset of argument and return types.

Supported callback argument types include:

- `Tcl_Obj*`
- `xer::tk::obj`
- `bool`
- `int`
- `long`
- `long long`
- `unsigned int`
- `unsigned long`
- `unsigned long long`
- `float`
- `double`
- `long double`
- `std::u8string`
- `std::u8string_view`

Supported callback return types include:

- `void`
- `bool`
- `int`
- `long`
- `long long`
- `unsigned int`
- `unsigned long`
- `unsigned long long`
- `float`
- `double`
- `long double`
- `std::u8string`
- `std::u8string_view`
- `const char8_t*`
- `Tcl_Obj*`
- `xer::tk::obj`
- `xer::result<T, xer::tk::error_detail>`
- `xer::result<T>` where supported by the implementation
- `xer::tk::result_code_t`

In unsupported cases, users can fall back to `Tcl_Obj*` or the native Tcl/Tk API.

### `std::u8string_view` callback arguments

When a C++ command callback takes `std::u8string_view`, the view refers to the string representation of the corresponding `Tcl_Obj`.
The view is valid only while the callback is running.

If the value must be stored, captured, returned later, or otherwise used after the callback returns, the callback must copy it into an owning object such as `std::u8string`.

Returning `std::u8string_view` from a callback is different: XER copies the referenced text into the Tcl interpreter result before the command handler returns.
The returned view only needs to remain valid until the command handler has finished setting the Tcl result.

---

## Tcl/Tk Main Helper

```cpp
template <class F>
auto main(F&& callback) -> xer::result<void>;
```

`xer::tk::main` is a Tcl/Tk execution block, not a replacement for the C++ global `main` function. It may be called from the program's main thread or from another user-managed thread.

The callback should have the following basic form:

```cpp
auto callback(xer::tk::interpreter& interp) -> xer::result<void>;
```

The helper performs the ordinary setup sequence: executable discovery, interpreter creation, Tcl/Tk initialization, setup of Tcl startup variables, callback invocation, and then `main_loop()` if the callback succeeds. The interpreter is destroyed by RAII after the event loop returns.

The helper sets Tcl variables so that Tcl scripts can use the ordinary names:

- `argc`
- `argv`
- `argv0`
- `env`

`argv` is set as a real Tcl list object. `env` is set as a Tcl array variable.

---

## Event Loop

```cpp
auto main_loop() -> void;
auto do_one_event(event_flag_t flags = event_all) -> int;
```

`main_loop` runs the Tk main event loop through `Tk_MainLoop`.

`do_one_event` processes one event through `Tcl_DoOneEvent`.

XER does not use `Tk_Main`, because `Tk_Main` controls too much of the process lifetime. Programs should create and initialize an interpreter explicitly, then run the event loop in the desired thread.

---

## Threading Policy

XER aims to allow Tcl/Tk to run on a thread chosen by the user.

However, an interpreter is treated as thread-affine. It should normally be created, initialized, used, and destroyed on the same thread.

This rule applies to ordinary operations such as evaluation, variable access, command registration, event-loop use related to that interpreter, and destruction.

This means XER supports the idea of running Tcl/Tk on a non-main thread, but it does not make one interpreter freely callable from arbitrary threads.

A safe pattern is to create an interpreter inside a worker thread, use it only inside that worker thread, and let it be destroyed before that thread exits.

XER does not currently provide cross-thread invocation helpers, event posting helpers, or a thread-safe dispatch queue for Tcl/Tk.

---

## Deferred Items

The following items are intentionally deferred from the initial Tcl/Tk layer:

* widget wrapper classes
* callback substitution values such as `%w`
* cross-thread invocation helpers
* full event-dispatch abstraction
* TomMath integration
* complete coverage of Tcl object types
* Tcl-only public header and namespace separation

---

## Example

```cpp
#include <xer/stdio.h>
#include <xer/tk.h>

auto main() -> int
{
    auto interp = xer::tk::interpreter::create();
    if (!interp.has_value()) {
        return 1;
    }

    const auto command = xer::tk::create_command(
        *interp,
        u8"add",
        [](int a, int b) -> int {
            return a + b;
        });
    if (!command.has_value()) {
        return 1;
    }

    const auto result = xer::tk::eval(*interp, u8"add 10 20");
    if (!result.has_value()) {
        return 1;
    }

    if (!xer::printf(u8"%@\n", *result).has_value()) {
        return 1;
    }

    return 0;
}
```

This example creates a Tcl interpreter and registers a C++ callable as a Tcl command. It does not initialize Tk because no GUI widget is used.

---

## See Also

* `policy_tk.md`
* `policy_project_outline.md`
* `policy_result_arguments.md`
* `header_cmdline.md`
