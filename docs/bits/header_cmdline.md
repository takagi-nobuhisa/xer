# `<xer/cmdline.h>`

## Purpose

`<xer/cmdline.h>` provides command-line argument handling facilities for the current process.

The purpose of this header is to make command-line arguments available as UTF-8 strings without requiring the caller to pass `main`'s `argc` and `argv` around manually.

This is useful in situations such as:

* code that runs outside `main`
* non-local object initialization
* code running on threads other than the main thread
* utility functions where carrying `argc` and `argv` explicitly would be awkward

---

## Main Entities

At minimum, `<xer/cmdline.h>` provides the following entities:

```cpp
using cmdline_arg =
    std::pair<std::u8string_view, std::u8string_view>;

class cmdline;

auto parse_arg(std::u8string_view value) noexcept -> cmdline_arg;

auto get_cmdline() -> xer::result<cmdline>;
````

---

## `cmdline`

`cmdline` owns an argv-like sequence of UTF-8 strings.

```cpp
class cmdline;
```

Internally, it stores command-line arguments as:

```cpp
std::vector<std::u8string>
```

The class itself does not interpret options.
It is responsible only for owning and exposing the argument sequence.

### Basic Operations

```cpp
auto size() const noexcept -> std::size_t;
auto empty() const noexcept -> bool;

auto args() const noexcept -> std::span<const std::u8string>;

auto at(std::size_t index) const -> xer::result<std::u8string_view>;
```

### `size`

`size()` returns the number of stored arguments.

### `empty`

`empty()` returns whether the argument list is empty.

In normal successful use of `get_cmdline`, the command-line list is expected to contain at least the program name, but callers should not rely on that in manually constructed `cmdline` objects.

### `args`

`args()` returns a span over the raw stored UTF-8 arguments.

The returned span and its string references are valid as long as the `cmdline` object remains alive and is not modified.

### `at`

`at(index)` returns one raw argument as `std::u8string_view`.

If `index` is out of range, it returns an error through `xer::result`.

---

## `parse_arg`

```cpp
auto parse_arg(std::u8string_view value) noexcept -> cmdline_arg;
```

`parse_arg` parses one raw command-line argument according to XER's simple command-line rule.

The return value is a pair:

```cpp
{ option_name, value }
```

The meaning is:

* if `first` is not empty, the argument is an option
* if `first` is empty, the argument is an ordinary value
* `second` contains the option value or the ordinary value

---

## Supported Argument Forms

XER recognizes only simple long-option forms.

Supported option forms are:

```text
--option
--option=value
```

Ordinary values are also accepted:

```text
value
```

A single-leading-hyphen form such as `-x` is not treated as an option.
It is treated as an ordinary value.

### Examples

```text
--name        -> { "name", "" }
--name=       -> { "name", "" }
--name=value  -> { "name", "value" }
value         -> { "", "value" }
-name         -> { "", "-name" }
--            -> { "", "--" }
--=value      -> { "", "--=value" }
```

`--name` and `--name=` are intentionally treated the same.

Distinguishing “no value” from “empty value” would require a more complex representation, and XER's initial command-line helper deliberately avoids that complexity.

---

## Why Short Options Are Not Special

`parse_arg` does not treat single-leading-hyphen arguments as options.

For example:

```text
-x
```

is parsed as:

```text
{ "", "-x" }
```

This is intentional.

The initial command-line model supports only:

* `--option`
* `--option=value`
* ordinary values

This keeps the rule simple and avoids introducing a larger command-line parser at this stage.

---

## `get_cmdline`

```cpp
auto get_cmdline() -> xer::result<cmdline>;
```

`get_cmdline` obtains the current process command-line arguments and returns them as a `cmdline` object.

The returned arguments are UTF-8 strings.

### Windows Behavior

On Windows, the implementation obtains the raw command line through:

```cpp
GetCommandLineW
```

and splits it through:

```cpp
CommandLineToArgvW
```

This avoids relying on CRT-specific globals such as `__wargv`.

That choice is intentional because command-line access should not depend on details of how the C runtime library is linked.

The resulting UTF-16 strings are converted to UTF-8.

### Linux Behavior

On Linux, the implementation reads:

```text
/proc/self/cmdline
```

This file contains the current process command-line arguments as NUL-separated byte strings.

The byte strings are interpreted as UTF-8 according to XER's Linux text assumptions.
If an argument is not valid UTF-8, `get_cmdline` fails.

Reading `/proc/self/cmdline` can theoretically fail in unusual environments.
In that case, `get_cmdline` returns an error through `xer::result`.

---

## Lifetime of Views

`cmdline::at` and `parse_arg` return `std::u8string_view` values.

These views do not own the underlying text.

For views obtained from a `cmdline` object, the referenced data remains valid only while the `cmdline` object remains alive and unchanged.

Example:

```cpp
const auto line = xer::get_cmdline();
if (!line.has_value()) {
    return 1;
}

const auto raw = line->at(1);
if (!raw.has_value()) {
    return 1;
}

const auto parsed = xer::parse_arg(*raw);
```

Here, `parsed.first` and `parsed.second` refer to the string owned by `line`.

---

## Error Handling

`<xer/cmdline.h>` follows XER's ordinary failure model.

`parse_arg` itself does not fail.
It is a simple view-based parser and returns an ordinary `cmdline_arg`.

`cmdline::at` can fail when the requested index is out of range.

`get_cmdline` can fail when the platform-specific command-line retrieval fails or when command-line data cannot be converted to XER's UTF-8 representation.

Typical failure conditions include:

* out-of-range argument access
* failure to retrieve the platform command line
* failure to read `/proc/self/cmdline`
* invalid UTF-8 in Linux command-line byte strings
* failure to convert Windows UTF-16 command-line strings to UTF-8

---

## Relationship to `main`

The ordinary C and C++ way to receive command-line arguments is through `main`.

```cpp
auto main(int argc, char** argv) -> int;
```

XER does not reject that approach.

However, `<xer/cmdline.h>` exists for cases where explicit `argc` / `argv` propagation is inconvenient or unavailable.

This means `get_cmdline` is a convenience facility for the current process, not a replacement for all uses of `main` arguments.

---

## Relationship to Process Handling

`<xer/cmdline.h>` handles the current process command line.

`<xer/process.h>` handles child process creation and management.

These are related topics, but they are intentionally separate:

* `cmdline.h` observes how the current process was launched
* `process.h` launches and controls child processes

This separation keeps each header focused.

---

## Design Role

`<xer/cmdline.h>` is intentionally small.

It is not a full command-line option parser.

In particular, it does not currently provide:

* short option parsing such as `-x`
* grouped short options such as `-abc`
* option terminator handling such as `--`
* automatic type conversion
* required-option validation
* help text generation
* subcommand handling

The initial feature is only a small argv retrieval and simple long-option parsing facility.

---

## Example

```cpp
#include <xer/cmdline.h>
#include <xer/stdio.h>

auto main() -> int
{
    const auto line = xer::get_cmdline();
    if (!line.has_value()) {
        return 1;
    }

    for (std::size_t i = 1; i < line->size(); ++i) {
        const auto raw = line->at(i);
        if (!raw.has_value()) {
            return 1;
        }

        const auto parsed = xer::parse_arg(*raw);

        if (!parsed.first.empty()) {
            if (!xer::printf(
                    u8"option %@ = %@\n",
                    parsed.first,
                    parsed.second)
                     .has_value()) {
                return 1;
            }
        } else {
            if (!xer::printf(u8"value %@\n", parsed.second).has_value()) {
                return 1;
            }
        }
    }

    return 0;
}
```

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that `cmdline` owns UTF-8 command-line arguments
* that `get_cmdline` obtains the current process arguments without using `main` parameters
* that Windows uses `GetCommandLineW` and `CommandLineToArgvW`
* that Linux reads `/proc/self/cmdline`
* that `parse_arg` recognizes only simple `--option` and `--option=value` forms
* that single-leading-hyphen arguments are treated as ordinary values

Detailed command-line parser behavior should not be implied.
This header is intentionally not a full option parsing framework.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* listing raw command-line arguments
* parsing `--option` and `--option=value`
* treating ordinary values separately from options
* showing that `-x` is treated as a value

These are good candidates for executable examples under `examples/`.

---

## See Also

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `policy_process.md`
* `header_process.md`
