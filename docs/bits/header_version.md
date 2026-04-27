# `<xer/version.h>`

## Purpose

`<xer/version.h>` provides compile-time version information for XER.

Its purpose is to make the library version available in a simple and explicit form for:

- source-level conditional handling
- diagnostics and reporting
- build-time checks
- generated documentation and example output when needed

This header is intentionally small and focused.

---

## Main Role

The main role of `<xer/version.h>` is to provide a stable public way to query the version of the XER library.

In particular, it exists to make the following easy:

- checking the major, minor, and patch version numbers
- checking the current suffix such as an alpha marker
- retrieving the complete version string
- keeping version information available both as macros and as inline constants

This makes the header useful both in preprocessor contexts and in ordinary C++ code.

---

## Main Entities

At minimum, `<xer/version.h>` provides entities such as the following:

```cpp
#define XER_VERSION_MAJOR <major>
#define XER_VERSION_MINOR <minor>
#define XER_VERSION_PATCH <patch>
#define XER_VERSION_SUFFIX "<suffix>"
#define XER_VERSION_STRING "<version>"

inline constexpr int version_major;
inline constexpr int version_minor;
inline constexpr int version_patch;
inline constexpr std::string_view version_suffix;
inline constexpr std::string_view version_string;
```

The exact values naturally change from release to release, but this is the intended public shape.

---

## Preprocessor Macros

`<xer/version.h>` provides preprocessor macros for version information.

### Purpose

The macro form exists for cases where version handling must happen before ordinary C++ constant evaluation or where preprocessor-based branching is desired.

### Main Macros

At minimum, the following macros are expected:

```cpp id="gkdm5v"
XER_VERSION_MAJOR
XER_VERSION_MINOR
XER_VERSION_PATCH
XER_VERSION_SUFFIX
XER_VERSION_STRING
```

### Meaning

* `XER_VERSION_MAJOR`: major version number
* `XER_VERSION_MINOR`: minor version number
* `XER_VERSION_PATCH`: patch version number
* `XER_VERSION_SUFFIX`: suffix such as `a3`
* `XER_VERSION_STRING`: full version string such as `0.2.0a3`

### Notes

These macros are useful in contexts such as:

* conditional compilation
* generated banners
* simple compile-time checks
* embedding version information into other generated artifacts

---

## Inline Constants

`<xer/version.h>` also provides inline C++ constants corresponding to the macro-level version information.

### Purpose

The inline constant form exists so that ordinary C++ code can use version information without needing to rely on macros.

### Main Constants

At minimum, the following constants are expected:

```cpp id="0t63o7"
inline constexpr int version_major;
inline constexpr int version_minor;
inline constexpr int version_patch;
inline constexpr std::string_view version_suffix;
inline constexpr std::string_view version_string;
```

### Meaning

These constants correspond directly to the macro values, but in a form more natural for C++ code.

### Notes

This dual provision of macros and constants is useful because:

* macros remain practical for preprocessing
* constants are better suited to type-safe ordinary code

---

## Version Components

The XER version is divided into several components.

### Major Version

The major version represents a large-scale compatibility or release-line change.

### Minor Version

The minor version represents smaller feature-line progression within the same major line.

### Patch Version

The patch version represents smaller corrective or maintenance progression.

### Suffix

The suffix represents additional release-state information such as alpha-stage notation.

For example:

```text id="uh0aq6"
0.2.0a3
```

may be interpreted as:

* major: `0`
* minor: `2`
* patch: `0`
* suffix: `a3`

### Full Version String

The full version string combines these elements into a single human-readable version identifier.

---

## Design Direction

`<xer/version.h>` is intentionally simple.

### Why It Is Small

Version information is important, but it should remain easy to understand and easy to consume.

This means the header should avoid:

* excessive abstraction
* overly clever compile-time metaprogramming
* unnecessarily complicated parsing facilities

The main goal is simply to expose the library version clearly.

---

## Intended Uses

Typical uses of `<xer/version.h>` include:

* displaying the current XER version in logs or diagnostics
* embedding the version into generated documentation
* checking the library version in test code
* simple conditional handling in source code

### Example Situations

It is especially natural to use this header in:

* documentation generation scripts or related examples
* test output
* release verification code
* feature-gating code when version distinctions matter

---

## Relationship to Documentation

`<xer/version.h>` is especially relevant to generated documentation.

### Why

The reference manual already records the target version explicitly.
For example, if the reference manual records a specific target version, `<xer/version.h>` is the natural source of truth for keeping that information consistent. 

### Design Direction

In the long term, generated documentation should ideally remain aligned with the actual contents of `<xer/version.h>` so that version drift is minimized.

---

## Relationship to Other Headers

`<xer/version.h>` is largely independent from the rest of the public headers.

However, it should be understood together with:

* `policy_project_outline.md`
* `public_headers.md`

The rough relationship is:

* `<xer/version.h>` provides version metadata
* other headers provide functional library facilities

This header does not define behavior of the rest of the library, but it identifies the release state of that library.

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that it provides compile-time version information
* that both macro and inline-constant forms are available
* that the suffix is part of the public version model
* that `version_string` / `XER_VERSION_STRING` provide the full human-readable version

Detailed release-policy interpretation belongs in release notes or higher-level project documentation rather than in the per-header summary.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* reading `version_string`
* checking `version_major`
* printing the current version
* comparing against a known release-line expectation in test code

These are good candidates for executable examples in `examples/`.

---

## Example

```cpp id="yrg8u0"
#include <xer/version.h>

auto main() -> int
{
    if (xer::version_major < 0) {
        return 1;
    }

    if (xer::version_string.empty()) {
        return 1;
    }

    return 0;
}
```

This example shows the normal style:

* include the version header directly
* use the inline constants in ordinary C++ code
* treat the header as a simple source of library metadata

---

## See Also

* `policy_project_outline.md`
* `public_headers.md`
