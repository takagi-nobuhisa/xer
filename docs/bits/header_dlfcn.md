# `<xer/dlfcn.h>`

## Purpose

`<xer/dlfcn.h>` provides a small cross-platform API for loading shared libraries and resolving symbols at run time.

The API follows the familiar POSIX-style naming pattern while staying inside the `xer` namespace. On POSIX platforms it wraps `dlopen`, `dlsym`, and `dlclose`; on Windows it wraps the corresponding `LoadLibraryW`, `GetProcAddress`, and `FreeLibrary` functionality.

---

## Main Role

This header provides:

- a copyable shared-library handle
- run-time loading of shared libraries
- typed symbol lookup without explicit casts at the call site
- raw symbol lookup for low-level use
- automatic unloading when the last shared handle is released

---

## Main Types

```cpp
class shared_library;
```

### `shared_library`

`shared_library` is a lightweight copyable shared handle.

Copies refer to the same native library handle. The underlying library is unloaded when the last `shared_library` object referring to that handle is released.

---

## Main Functions

```cpp
auto dlopen(std::string_view path) -> result<shared_library>;
auto dlclose(shared_library& library) noexcept -> void;
auto is_open(const shared_library& library) noexcept -> bool;
auto native_handle(const shared_library& library) noexcept -> void*;

auto dlsym(const shared_library& library, std::string_view name, void*& out) -> result<void>;

template<class Function>
auto dlsym(const shared_library& library, std::string_view name, Function*& out) -> result<void>;
```

### `dlopen`

```cpp
auto dlopen(std::string_view path) -> result<shared_library>;
```

Loads a shared library and returns a `shared_library` handle.

On POSIX platforms, the initial implementation uses:

```cpp
RTLD_NOW | RTLD_LOCAL
```

On Windows, the initial implementation uses `LoadLibraryW`.

The first version intentionally does not expose platform flags. If flag control becomes necessary, an overload can be added later without changing the existing API.

### `dlclose`

```cpp
auto dlclose(shared_library& library) noexcept -> void;
```

Releases the target handle's reference to the loaded library.

If other `shared_library` objects still refer to the same native handle, the library remains loaded.

### `is_open`

```cpp
auto is_open(const shared_library& library) noexcept -> bool;
```

Returns whether the handle currently refers to a loaded library.

### `native_handle`

```cpp
auto native_handle(const shared_library& library) noexcept -> void*;
```

Returns the native library handle as a raw pointer value, or `nullptr` if the handle is empty.

### `dlsym`

```cpp
auto dlsym(const shared_library& library, std::string_view name, void*& out) -> result<void>;
```

Looks up a raw symbol address and stores it in `out`.

```cpp
template<class Function>
auto dlsym(const shared_library& library, std::string_view name, Function*& out) -> result<void>;
```

Looks up a function symbol and stores it in `out`. The function type is inferred from the output pointer, so callers do not have to write an explicit cast.

Example:

```cpp
using function_type = int(const char*);

function_type* function = nullptr;
auto r = xer::dlsym(library, "puts", function);
```

---

## Design Notes

The public header name is `<xer/dlfcn.h>` because the API intentionally follows the POSIX `dlopen` / `dlsym` / `dlclose` vocabulary.

The internal implementation is placed in `<xer/bits/dlfcn.h>`.

The `xer` namespace distinguishes these functions from platform APIs, so using POSIX-style names does not conflict with the global platform functions.

---

## Limitations

The initial API does not expose `dlopen` flags or `LoadLibraryExW` flags.

The default behavior is intentionally conservative:

- POSIX: load with `RTLD_NOW | RTLD_LOCAL`
- Windows: load with `LoadLibraryW`

Platform-specific flag control can be added later through overloads if a concrete need appears.
