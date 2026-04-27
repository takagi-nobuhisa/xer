# `<xer/dirent.h>`

## Purpose

`<xer/dirent.h>` provides directory stream operations in XER.

This header covers PHP/POSIX-style directory traversal facilities such as opening a directory, reading entry names, rewinding the directory stream, and closing it.

The purpose is not to reproduce POSIX `dirent.h` exactly.
Instead, XER provides a small C++23-friendly directory stream API that uses:

- `xer::path` for path names
- UTF-8 strings for directory entry names
- `xer::result` for ordinary failure
- a move-only RAII handle for directory streams

---

## Main Entities

At minimum, `<xer/dirent.h>` provides the following entities:

```cpp
class xer::dir;

auto xer::opendir(const path& dirname) noexcept -> result<dir>;
auto xer::closedir(dir& directory) noexcept -> result<void>;
auto xer::readdir(dir& directory) noexcept -> result<std::u8string>;
auto xer::rewinddir(dir& directory) noexcept -> result<void>;
````

---

## Design Role

This header exists for directory stream traversal.

It is separated from `<xer/stdio.h>` because directory streams are stateful traversal handles rather than ordinary file streams.
Although the names are familiar from POSIX and PHP, the API is adapted to XER's own path, string, and error-handling model.

---

## `xer::dir`

`xer::dir` is a move-only directory stream handle.

It owns a native directory stream handle internally and closes it automatically when destroyed.

```cpp
class xer::dir;
```

### Basic Properties

* move-only
* non-copyable
* RAII-based
* represents either an open directory stream or an empty/closed state

### Explicit Close

The destructor closes the directory stream automatically, but failures from a destructor cannot be observed.

When the caller needs to observe close errors, `xer::closedir` should be called explicitly.

---

## `xer::opendir`

```cpp
auto opendir(const path& dirname) noexcept -> result<dir>;
```

`opendir` opens a directory stream for the specified path.

The path is converted from XER's UTF-8 `xer::path` representation to the platform-native path representation before the underlying directory API is called.

### Return Value

On success, it returns an open `xer::dir`.

On failure, it returns `xer::result` failure.

### Notes

The returned directory stream is a snapshot-like traversal handle.
If the directory contents are modified while the directory is being read, the observed behavior is platform- and filesystem-dependent.

---

## `xer::closedir`

```cpp
auto closedir(dir& directory) noexcept -> result<void>;
```

`closedir` closes a directory stream.

After this function is called, the `xer::dir` object is treated as closed.

### Return Value

On success, it returns an empty success value.

On failure, it returns `xer::result` failure.

### Notes

Calling `closedir` for an already closed or empty `xer::dir` is treated as a no-op success.

The destructor of `xer::dir` also closes the directory stream, but explicit `closedir` is useful when the caller wants to observe close errors.

---

## `xer::readdir`

```cpp
auto readdir(dir& directory) noexcept -> result<std::u8string>;
```

`readdir` reads the next entry name from a directory stream.

The returned string is a UTF-8 directory entry name.

### Return Value

On success, it returns the next entry name.

At the end of the directory stream, it returns failure with:

```cpp
error_t::not_found
```

Other failures are reported through `xer::result` in the usual way.

### Important Notes

`readdir` returns only the entry name.

It does not return a full path.

For example, if the directory contains:

```text
example.txt
```

then `readdir` returns:

```text
example.txt
```

not:

```text
directory/example.txt
```

The special entries `"."` and `".."` are not filtered out.
This follows the PHP/POSIX-style behavior more closely.
Callers that do not want these entries should skip them explicitly.

Entry order is platform- and filesystem-dependent.
Code must not rely on a specific order unless it sorts the entries itself.

---

## `xer::rewinddir`

```cpp
auto rewinddir(dir& directory) noexcept -> result<void>;
```

`rewinddir` rewinds a directory stream to the beginning.

After this function succeeds, subsequent calls to `readdir` read entries again from the beginning of the directory stream.

### Return Value

On success, it returns an empty success value.

On failure, it returns `xer::result` failure.

### Notes

The order after rewinding is still platform- and filesystem-dependent.
XER does not guarantee a stable ordering of directory entries.

---

## End-of-Directory Handling

In XER, reaching the end of a directory stream is represented as:

```cpp
error_t::not_found
```

Typical usage is:

```cpp
for (;;) {
    auto entry = xer::readdir(directory);
    if (!entry.has_value()) {
        if (entry.error().code == xer::error_t::not_found) {
            break;
        }

        return 1;
    }

    // Use *entry.
}
```

This keeps end-of-directory separate from ordinary successful reads while still using the normal `xer::result` failure channel.

---

## Relationship to Path Handling

`<xer/dirent.h>` uses `xer::path` for directory paths.

The `path` object stores a UTF-8 path in XER's normalized internal form.
When `opendir` is called, the path is converted to the platform-native representation before being passed to the underlying directory API.

The names returned by `readdir` are converted back into UTF-8 strings.

---

## Relationship to Other Headers

`<xer/dirent.h>` is related to:

* `<xer/path.h>`
* `<xer/stdio.h>`
* `<xer/error.h>`

The rough boundary is:

* `<xer/path.h>` handles lexical path representation and path utilities
* `<xer/stdio.h>` handles ordinary file streams and file-related operations
* `<xer/dirent.h>` handles directory stream traversal

---

## Documentation Notes

When documenting this header, the most important points are:

* `xer::dir` is a move-only RAII directory stream handle
* `readdir` returns entry names, not full paths
* `"."` and `".."` are not filtered out
* end-of-directory is represented by `error_t::not_found`
* entry order is filesystem-dependent
* modifications during traversal have platform-dependent results

---

## Example

```cpp
#include <xer/dirent.h>
#include <xer/error.h>
#include <xer/stdio.h>

auto main() -> int
{
    auto directory = xer::opendir(u8".");
    if (!directory.has_value()) {
        return 1;
    }

    for (;;) {
        auto entry = xer::readdir(*directory);
        if (!entry.has_value()) {
            if (entry.error().code == xer::error_t::not_found) {
                break;
            }

            return 1;
        }

        if (*entry == u8"." || *entry == u8"..") {
            continue;
        }

        if (!xer::puts(*entry).has_value()) {
            return 1;
        }
    }

    if (!xer::closedir(*directory).has_value()) {
        return 1;
    }

    return 0;
}
```

---

## See Also

* `<xer/path.h>`
* `<xer/stdio.h>`
* `<xer/error.h>`
* `policy_path.md`
* `policy_examples.md`
