# `<xer/process.h>`

## Purpose

`<xer/process.h>` provides child process management facilities.

The initial API is deliberately small and focuses on direct process spawning, waiting, and standard stream wiring.

---

## Main Role

This header provides:

- a move-only process handle
- direct child process spawning without a command shell
- standard input, standard output, and standard error configuration
- optional pipes exposed as `binary_stream` objects
- process waiting and exit-code retrieval

---

## Main Types

```cpp
enum class process_stdio;
struct process_options;
struct process_result;
class process;
struct process_spawn_result;
```

### `process_stdio`

```cpp
inherit
null
pipe
```

- `inherit` connects the child stream to the corresponding parent stream.
- `null` connects the child stream to the platform null device.
- `pipe` creates a parent-side pipe represented as a `binary_stream`.

### `process_options`

```cpp
path program;
std::vector<std::u8string> arguments;
process_stdio stdin_mode;
process_stdio stdout_mode;
process_stdio stderr_mode;
```

`arguments` excludes `argv[0]`; the program path is supplied separately.

### `process_result`

```cpp
int exit_code;
```

On POSIX, signal termination is represented as `128 + signal_number`.

### `process_spawn_result`

```cpp
process proc;
std::optional<binary_stream> stdin_stream;
std::optional<binary_stream> stdout_stream;
std::optional<binary_stream> stderr_stream;
```

The optional streams are present only when the corresponding `process_stdio::pipe` mode is requested.

---

## Main Functions

```cpp
auto process_spawn(const process_options& options) noexcept -> xer::result<process_spawn_result>;
auto process_wait(process& value) noexcept -> xer::result<process_result>;
```

`process_spawn` executes the target program directly and passes arguments as separate command-line arguments.
It does not invoke a command shell.

`process_wait` waits for the child process and returns its exit status.

---

## Process Handle

`process` is a move-only handle type.

```cpp
auto is_open() const noexcept -> bool;
```

The destructor releases the native handle owned by the object, but it does not wait for process termination.
Call `process_wait` explicitly when the exit code is needed.

---

## Notes

- Paths use `xer::path` and native path conversion internally.
- Arguments use UTF-8 `std::u8string` values.
- On Windows, command-line quoting is performed internally for direct process creation.
- On POSIX, the child process is created using the platform process facilities and then executed directly.
- Pipe streams are binary streams. Higher-level text handling can be layered separately if needed.
