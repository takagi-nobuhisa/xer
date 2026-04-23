# Policy for Path Handling

## Basic Policy

In XER, `std::filesystem::path` is not used.
Instead, XER provides its own `path` class.

Path handling is unified on a UTF-8 basis, with priority given to simplicity and consistency of the internal representation.

## The `path` Class

### Internal Representation

- `path` stores a `std::u8string` internally.
- The directory separator in the internal representation is always `/`.
- In the constructor, `\` in the input is normalized to `/`.
- The meaning of leading components on Windows is preserved.

### Handling of Leading Components

On Windows, separators may be unified to `/`, but the meaning of the leading component must be preserved.

For example, the following must remain distinguishable from one another:

- `C:foo`
- `C:/foo`
- `/foo`
- `//server/share/foo`

### Retrieving the String

- `path` provides `str()`.
- The return type of `str()` is `std::u8string_view`.
- `str()` returns the string in the internal normalized form.

### Display-Oriented String

If necessary, a function may be provided that generates a human-oriented display string by replacing `/` with `\`.

However, the internal representation itself remains normalized with `/`.

## Member Functions and Free Functions

In XER, anything that does not need to be a member function should be provided as a free function.

This limits the responsibility of `path` to storing the internal representation and maintaining invariants, and strengthens encapsulation.

### Member Functions

- constructors
- `str()`
- `operator/=`

### Free Functions

- `operator/`
- `basename`
- `parent_path`
- `extension`
- `stem`
- `is_absolute`
- `is_relative`

## Path Joining

### `operator/=`

- `operator/=` is a member function of `path`
- it is treated as the fundamental operation that mutates the left-hand side

### `operator/`

- `operator/` is a free function
- it may be implemented in terms of `operator/=`

## Absolute Path Determination on Windows

On Windows, paths are treated as follows:

- `X:foo` is not an absolute path
- `X:/foo` is an absolute path
- `/foo` is an absolute path
- `//server/share/foo` is an absolute path

## Path Joining Rules on Windows

### Basic Rules

- if the right-hand side is an ordinary relative path, join it
- if the right-hand side is a drive-relative path on the same drive, join it
- if the right-hand side is a drive-relative path on a different drive, the join is impossible and results in an error
- as a rule, if the right-hand side is an absolute path, the join is impossible and results in an error

### Exceptional Rules

However, the following two cases are allowed as exceptions:

- `/` + `/a` = `/a`
- `X:` + `/a` = `X:/a`

### Concrete Examples

- `C:/a` + `b` = `C:/a/b`
- `C:/a` + `C:b` = `C:/a/b`
- `C:/a` + `D:b` = error

## Lexical Parent Path

`parent_path` is a free function that returns the lexical parent path.

This is purely a lexical operation and does not depend on the actual filesystem.

### Rules

- remove one trailing component while preserving the leading component
- if no further lexical parent can be taken, return an error

### Examples

- `a/b/c` -> `a/b`
- `a/b` -> `a`
- `a` -> error

- `/a/b` -> `/a`
- `/a` -> `/`
- `/` -> error

- `X:a/b` -> `X:a`
- `X:a` -> `X:`
- `X:` -> error

- `X:/a/b` -> `X:/a`
- `X:/a` -> `X:/`
- `X:/` -> error

## Processing That Depends on the Actual Filesystem

The following kinds of processing are separated from the lexical operations of `path` and are provided as separate functions:

- resolution of relative paths
- conversion to absolute paths
- resolution of symbolic links
- normalization that refers to the actual filesystem

## `basename`

### Basic Policy

- `basename` is a free function
- it accepts only the `path` type as its argument
- its return type is `std::u8string_view`
- handling of a trailing `/` follows the behavior of PHP's `basename`

### Incompatibilities with PHP

The following points are intentionally incompatible with PHP's `basename`:

- it is locale-independent
- it does not treat `\` as a separator
- it does not accept an ordinary string and accepts only `path`

### Separator

- `basename` treats only `/` as a separator
- however, `\` is normalized to `/` in the constructor of `path`

## `extension`

### Basic Policy

- `extension` is a free function
- its return type is `std::u8string_view`
- it operates on `basename(path)`

### Definition

- return the part of `basename(path)` beginning with the first `.`
- if no `.` exists, return an empty string
- a leading `.` is not treated specially

### Examples

- `c.txt` -> `.txt`
- `archive.tar.gz` -> `.tar.gz`
- `.foo` -> `.foo`
- `foo.` -> `.`
- `foo` -> empty string
- `.foo.bar` -> `.foo.bar`

### Start Position Argument

- `extension` may optionally take an additional start position argument
- this position is relative to `basename`
- return the substring from the first `.` found at or after that position through the end
- if no `.` is found, return an empty string

## `stem`

### Basic Policy

- `stem` is a free function
- its return type is `std::u8string_view`

### Definition

- `stem(path)` returns the leading part of `basename(path)` after removing `extension(path)`

### Examples

- `c.txt` -> `c`
- `archive.tar.gz` -> `archive`
- `.foo` -> empty string
- `foo.` -> `foo`
- `foo` -> `foo`

## Summary

- `path` stores a UTF-8 `std::u8string` internally
- the internal separator is unified to `/`
- the constructor normalizes `\` to `/`
- the Windows-specific meaning of leading components is preserved
- `str()` returns `std::u8string_view`
- `/=` is a member function, while `/` is a free function
- decomposition and classification operations are generally free functions
- `parent_path` handles only lexical parent paths
- resolution that depends on the actual filesystem is separated into other functions
- `basename` is close to PHP compatibility, but remains locale-independent, does not treat `\` as a separator, and accepts only `path`
- `extension` returns the part beginning with the first `.` in the basename
- `stem` returns the leading part of the basename after removing the extension

## Error Policy

When conversion to or from native paths fails because XER's own UTF-8 / UTF-16 validation or conversion fails, it may use `error_t::encoding_error`.

On the other hand, if a failure directly reflects a platform-side conversion failure, it may continue to use an error code that reflects that lower-level cause as appropriate.

The distinction should follow the same general idea as in XER's encoding policy:

- use `encoding_error` for failures detected by XER's own encoding logic
- use a more direct system-side error representation when the failure directly reflects external behavior
