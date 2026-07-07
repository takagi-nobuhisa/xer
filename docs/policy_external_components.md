# Policy for External Components

## Overview

xer is header-only, but some practical facilities intentionally depend on external components.
Examples include Tcl/Tk, MeCab, ICU, and zlib.

This policy defines how such dependencies are introduced, detected, tested, and documented.
The goal is to keep ordinary headers lightweight while allowing mature external components to provide capabilities that would be unrealistic or undesirable to reimplement inside xer.

---

## Basic Policy

External components may be used when they are:

- mature and widely used
- reasonably stable in C or command-line interfaces
- available on the primary target environments
- more practical than maintaining a large in-house implementation
- clearly isolated behind a dedicated public header or feature boundary

xer should not depend on fast-moving ecosystems for core functionality.
External integration should be added only when it provides clear practical value.

---

## Public Header Boundary

A header that depends on an external component should be separated from unrelated lightweight headers.

For example:

```text
xer/tk.h
xer/mecab.h
xer/unicode.h
```

should not make ordinary string, character, arithmetic, or I/O headers depend on Tcl/Tk, MeCab, or ICU.

This keeps programs that do not use those facilities free from unnecessary compile-time and link-time dependencies.

---

## Compile-Time Detection

When a public header requires external development headers, the header should detect the required headers with `__has_include` when available.

If the required headers are not found, the header should fail at compile time with `#error` and a useful diagnostic message.

This is intentional.
If a user explicitly includes a feature header that requires an external component, missing headers should be detected statically instead of being hidden behind runtime fallback behavior.

Typical pattern:

```cpp
#if defined(__has_include)
#    if __has_include(<some_external_header.h>)
#        include <some_external_header.h>
#    else
#        error "xer/example.h requires some_external_header.h."
#    endif
#else
#    include <some_external_header.h>
#endif
```

The fallback branch for compilers without `__has_include` should include the required headers directly.
xer's primary compiler target supports `__has_include`, but the fallback keeps the code simple and conventional.

---

## Link-Time Responsibility

xer does not try to manage user build-system settings.

Users are responsible for providing appropriate link options in their own build system, such as CMake, Makefiles, shell scripts, IDE settings, or manual compiler commands.

A header can detect missing include files, but it cannot reliably prove that the final link command contains the correct libraries.
Therefore, link errors are treated as build-configuration errors in the user's environment.

Documentation for each external-component header should mention typical link options when doing so is helpful.

---

## Test Runner Responsibility

Although xer does not manage the user's build system, xer's own PHP test runner should know how to build and run tests for supported environments.

The primary supported and tested environments are:

- Ubuntu with GCC
- Ubuntu with Clang and libc++
- MSYS2 UCRT64 with GCC
- MSYS2 CLANG64 with Clang
- Visual Studio 2026 with clang-cl
- Visual Studio 2026 with MSVC cl.exe

MSYS2 MSYS and MSYS2 MINGW64 are not supported targets and are not part of the current or planned test matrix.

For known supported environments, the test runner may add include paths, link options, and runtime DLL search paths needed for external-component tests.

Examples:

- Tcl/Tk tests may need Tcl/Tk include paths, libraries, and runtime DLL paths.
- ICU tests may need ICU libraries such as `icuuc`, `icuin`, and the platform-specific ICU data library.
- zlib tests may need the zlib header and library.
- MeCab tests may need the MeCab executable to be available at runtime rather than link-time libraries.

On Visual Studio 2026 with clang-cl or MSVC cl.exe, ICU and zlib tests use vcpkg manifest mode through the repository's `vcpkg.json`. The generated `vcpkg_installed/x64-windows` directory is used for include paths, library paths, and runtime DLL lookup by the test runner, and it must not be committed or packaged.

Tcl/Tk on Windows is detected from common installation roots, preferring Tcl/Tk 9.0 over 8.6 when both are available. A custom Tcl/Tk root can be specified with `XER_TEST_TCLTK_ROOT`.

If a required component is not available in the test environment, component-specific tests may be skipped.
A skipped component test should mean "this optional external component is not available here", not "the feature silently falls back at runtime".

---

## Command-Line External Components

Some integrations use an external executable instead of directly linking to a library.

MeCab integration is the main example.
In that case, compile-time header detection is not applicable.
The API should instead validate runtime process execution and report ordinary `xer::result` errors when the external command cannot be started or returns unusable output.

This distinction is important:

- library-header dependencies should fail at include/compile time when missing
- command-line tool dependencies should fail at runtime when execution fails

---

## C API Preference

When an external component provides both C and C++ APIs, xer should prefer the C API unless there is a strong reason not to.

Reasons include:

- C APIs are usually more stable across C++ standard library boundaries.
- C APIs fit xer's C-oriented design style.
- Platform-provided integrations may expose only C APIs.

ICU is an important example: Unicode normalization should use ICU's C API, not ICU's C++ API.

---

## Documentation Requirements

A public header that depends on an external component should document:

- the external component name
- whether the dependency is compile-time, link-time, runtime, or a combination of these
- the required headers or executable names when useful
- typical link options when useful
- what happens when the dependency is missing
- which tests or examples depend on the component

A separate feature policy document should be added when the dependency affects API design or future expansion.

---

## Current External Components

### Tcl/Tk

Tcl/Tk support is provided by `<xer/tk.h>`.
It requires Tcl/Tk development headers at compile time and Tcl/Tk libraries at link time.
The header uses `__has_include` and `#error` to diagnose missing headers.

### MeCab

MeCab support is provided by `<xer/mecab.h>`.
It invokes a MeCab executable and therefore depends mainly on runtime availability of the command and dictionary environment.

### ICU

ICU-based Unicode normalization is provided by `<xer/unicode.h>`.
It requires ICU C API headers at compile time and ICU libraries at link time.
The ICU-dependent public scope is NFC normalization and NFC status checking. The code point traversal APIs in `<xer/unicode.h>` are table-free, but the public header currently also includes ICU-based normalization.

### zlib

ZIP archive support is provided by `<xer/zip.h>`.
It requires the zlib development header at compile time and the zlib library at link time.
On Visual Studio 2026 with clang-cl or MSVC cl.exe, zlib is supplied by vcpkg manifest mode for xer's own tests and examples.

---

## Summary

- External components are allowed when they are mature, stable, and practically valuable.
- External-component APIs should be isolated behind dedicated public headers.
- Missing library headers should be detected with `__has_include` and reported with `#error`.
- User build-system link settings remain the user's responsibility.
- xer's own test runner should handle known environments for tests and examples.
- Command-line integrations, such as MeCab, are runtime dependencies and should report runtime errors through `xer::result`.
- C APIs are preferred for external libraries when available.


## zlib / ZIP

`xer/zip.h` uses zlib for deflate compression and expansion. The public header checks for `<zlib.h>` with `__has_include` when available. Tests and examples that include `xer/zip.h` are marked as the `zip` feature and should be linked with zlib, typically `-lz`.

The current ZIP implementation supports ordinary non-ZIP64 single-disk archives with stored or deflated entries. It provides sequential reading, exact-name lookup, whole-entry reads, archive creation, deflated entry writing, explicit commit, and extraction helpers. ZIP64, encrypted entries, archive comments, multi-disk archives, and streaming large-entry I/O are deferred.
