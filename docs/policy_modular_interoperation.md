# Policy for Modular Interoperation

## Overview

xer will gradually add facilities for connecting modules, processes, and external runtimes.
This includes dynamic shared libraries, delayed loading, shared memory, synchronization primitives, message queues, and eventually RPC-like interfaces.

The purpose is not to make RPC the center of the design.
RPC is only one possible transport and invocation style built on top of lower-level interoperation facilities.
The first priority is to provide small, reliable, and reusable building blocks that can later be combined into higher-level communication mechanisms.

This policy defines the design direction for those facilities.

---

## Basic Direction

The interoperation layer should be developed incrementally.
The initial focus is on low-level facilities that are useful by themselves and also useful as foundations for later RPC, helper processes, plugins, and language bindings.

Important building blocks include:

- dynamic shared library loading
- delayed loading of optional components
- symbol lookup
- shared memory
- named synchronization primitives
- lightweight interprocess communication
- message framing
- logical API definition data
- marshaling rules for C ABI, RPC, and language bindings

The preferred development order is:

1. `shared_library`
2. `shared_memory`
3. named synchronization primitives
4. message queues or lightweight communication channels
5. message framing
6. RPC or helper-process interfaces

This order keeps each step useful even if later layers are not implemented immediately.

---

## Dynamic Shared Library Loading

Dynamic shared library loading is the first target because it is independent, testable, and useful for several future features.

The C++ API should provide a natural RAII-style wrapper around platform APIs such as:

- `LoadLibraryW`, `GetProcAddress`, and `FreeLibrary` on Windows
- `dlopen`, `dlsym`, and `dlclose` on POSIX systems

The public C++ interface should avoid exposing unnecessary platform details.
However, a native handle accessor may be provided when practical interoperation requires it.

The facility should be named in a platform-neutral way.
The preferred concept name is `shared_library` rather than `dll`, because the feature targets both Windows DLLs and POSIX shared objects.

---

## C++ API and Public Interoperation API

xer should distinguish between the ordinary C++ API and the externally published interoperation API.

The ordinary C++ API should remain convenient for C++ users.
It may use xer types and C++ standard library types such as:

- `std::string_view`
- `std::span`
- `xer::result<T>`
- RAII classes
- templates where appropriate

The externally published interoperation API should not expose C++ ABI details.
It should be based on C linkage and C-compatible types.

Reasons include:

- C++ name mangling is compiler-dependent.
- C++ ABI compatibility is not portable across compilers and standard libraries.
- `std::string`, `std::string_view`, `std::span`, templates, exceptions, and `xer::result<T>` are not suitable as language-neutral ABI types.
- C ABI is much easier to use from PHP FFI, Python `ctypes`, other languages, and dynamically loaded modules.

The two layers do not need to have identical function shapes.
The C++ API should prioritize C++ usability.
The C ABI should prioritize ABI stability and marshaling simplicity.

---

## C ABI Boundary

Externally callable native APIs should use C linkage.

Typical form:

```cpp
extern "C" {

auto xer_some_function(...) noexcept -> int;

}
```

The C ABI should use only C-compatible types at the ABI boundary.
Suitable ABI-level types include:

- `void`
- `int`
- fixed-width integer types
- `std::size_t` when acceptable for the target ABI
- `char*` and `const char*`
- `void*` and `const void*`
- opaque handle pointer types
- output parameters

The C ABI should avoid:

- C++ classes
- C++ references
- C++ exceptions
- templates
- overloaded functions
- `std::string`
- `std::string_view`
- `std::span`
- `std::vector`
- `std::optional`
- `std::expected`
- `xer::result<T>`
- C++ `enum class` values as direct ABI commitments

Boolean values at the C ABI boundary should generally be represented as `int` or another explicitly defined integer convention, rather than relying on cross-language handling of `bool`.

Errors should generally be returned as integer status codes.
Actual return values should be returned through output parameters when needed.

---

## Opaque Handles

Stateful facilities should use opaque handles in the C ABI.

For example, a C++ class such as:

```cpp
namespace xer {

class shared_library;

}
```

may be exposed at the C ABI boundary as:

```cpp
struct xer_shared_library;
```

The implementation may internally wrap the C++ object:

```cpp
struct xer_shared_library {
    xer::shared_library impl;
};
```

The C ABI should provide explicit create and destroy functions when a handle owns resources.
A destroy function should accept a null handle if doing so makes the API easier and safer to use.

---

## Logical API Definition Data

For future code generation, documentation generation, RPC, and language bindings, xer should describe selected APIs as data.

The API definition data should describe a logical API, not merely the final C ABI spelling.

For example, the logical API may use abstract types such as:

- `status`
- `bool`
- `string`
- `path`
- `bytes`
- `mutable_buffer`
- `handle<T>`
- `symbol`
- `duration_ms`
- `timeout_ms`

These logical types can then be mapped to different target representations.

The initial data format may be a PHP array because xer already uses PHP for development tools.
JSON can be generated from that data when needed for external tools.

The API definition data should be treated as a design and generation aid.
It does not need to replace hand-written C++ implementation immediately.

---

## Marshaling Layer

A marshaling layer should translate logical API definitions into target-specific interfaces.

The intended structure is:

```text
logical API definition
  -> marshaling rules
  -> C ABI / RPC / PHP FFI / Python ctypes / other bindings
```

The marshaling layer allows xer to preserve a stable C ABI while keeping the logical API expressive enough for higher-level interfaces.

Typical mappings may include:

```text
logical type          C ABI representation
------------------------------------------
string                const char*
path                  const char* encoded as UTF-8
bytes                 const void* + size_t
mutable_buffer        void* + size_t + size_t* out_size
handle<T>             xer_T*
bool                  int
status                int
symbol                void*
```

For RPC-style interfaces, the same logical types may map differently:

```text
logical type          RPC representation
---------------------------------------
string                JSON string
path                  JSON string
bytes                 base64 or hex string
mutable_buffer        returned bytes value
handle<T>             handle id
bool                  JSON boolean
status                result or error object
symbol                usually not exported
```

This separation is important.
If the C ABI is used directly as the design model, RPC and language bindings become unnecessarily awkward.

---

## Export Classification

Not every C++ function should be exported through every interface.

API definition data should be able to classify whether a function is intended for:

- C++ documentation
- C ABI export
- RPC export
- language binding export
- test generation

Some APIs are useful in C++ and C ABI form but should not be exposed through RPC.
For example, raw symbol lookup from a shared library may be meaningful for native code, but it is usually unsafe or meaningless across an RPC boundary.

Export intent should therefore be explicit.

---

## Error Handling

The ordinary C++ API should continue to use `xer::result<T>` where appropriate.

The C ABI should not expose `xer::result<T>`.
It should use integer status codes and output parameters.

The logical API definition should distinguish between:

- failure information
- successful return values

This allows C ABI output parameters and RPC result objects to be generated from the same logical definition without forcing one interface style onto the other.

---

## Shared Memory and Synchronization

Shared memory and synchronization primitives should be developed as independent low-level facilities before RPC is introduced.

Shared memory should provide a named, platform-neutral abstraction over mechanisms such as:

- Windows file mappings
- POSIX shared memory and `mmap`

Synchronization primitives should eventually support practical interprocess coordination.
Candidates include:

- named semaphores
- named mutexes
- events
- lightweight message queues

The design should be practical rather than theoretically complete.
The goal is a small set of facilities that are easy to use in ordinary programs, similar in spirit to the approachable synchronization and communication facilities found in small real-time operating systems.

---

## RPC Positioning

RPC should be built after lower-level interoperation pieces are available.

RPC should be treated as a higher-level invocation layer over one or more transports, not as the foundation of all interoperation.
Possible transports include:

- in-process dispatch
- pipes
- sockets
- shared memory plus synchronization
- helper processes
- external runtimes

The API definition and marshaling design should make RPC easier to add later, but the initial implementation should not be forced to include RPC.

---

## Header-Only Policy

xer remains primarily header-only.

Interoperation facilities may need to call platform APIs or external C APIs, but they should be designed so that ordinary use remains as simple as including the relevant headers and linking to platform libraries when necessary.

When a feature requires special link options or platform libraries, the relevant feature document should state that clearly.

If a future C ABI wrapper requires generated implementation files, that should be treated as a separate export or binding layer rather than a requirement for ordinary C++ use.

---

## Initial Shared Library API Definition Scope

When `shared_library` is implemented, it should also be used as the first trial case for logical API definition data.

The first API definition does not need to generate all code.
It is sufficient to describe the intended logical API and generate JSON from PHP data as a proof of direction.

A minimal initial scope may include:

- create and destroy handle operations for C ABI use
- open and close operations
- open-state query
- symbol lookup for native use
- explicit export flags for C ABI, RPC, and bindings

This will provide a small and practical test case for the design before larger facilities such as shared memory and message queues are added.

---

## Summary

The interoperation design should follow these principles:

- build low-level facilities before RPC
- keep the normal C++ API convenient
- expose native public boundaries through C ABI when needed
- use logical API definition data rather than raw C ABI signatures as the design source
- insert a marshaling layer between logical APIs and target interfaces
- classify export targets explicitly
- preserve xer's header-only character for ordinary C++ use
- grow the design incrementally, starting with `shared_library`
