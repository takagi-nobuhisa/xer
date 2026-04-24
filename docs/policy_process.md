# Policy for Child Process Handling

## Overview

XER provides child process handling through `<xer/process.h>`.

The initial goal is to provide direct process spawning and standard stream control without introducing a shell-oriented API.

---

## Basic Policy

- A child process is represented by a move-only `process` handle.
- Programs are executed directly, not through a command shell.
- Arguments are passed as separate values.
- Paths use `xer::path`.
- Arguments use UTF-8 strings.
- Ordinary failures are returned as `xer::result`.

---

## Standard Stream Policy

Each standard stream can be configured as:

- `inherit`: inherit the corresponding parent stream
- `null`: connect to the platform null device
- `pipe`: create a parent-side pipe

Pipe streams are represented as `binary_stream` objects.
This keeps the first implementation low-level and avoids prematurely deciding text encoding policy for child process pipes.

---

## Waiting Policy

`process_wait` waits for the process and returns a `process_result` containing an exit code.

On POSIX, signal termination is normalized to `128 + signal_number`, following a common shell convention.

The process destructor releases the handle, but it does not wait for termination.
Users should call `process_wait` when they need the result and when the platform requires waiting to avoid zombie processes.

---

## Deferred Items

At least the following are deferred:

- shell-style command execution
- environment variable customization
- working-directory customization
- asynchronous process management
- process termination APIs
- popen-style convenience wrappers

---

## Summary

- `<xer/process.h>` starts with a low-level direct-spawn API
- no command shell is used implicitly
- stdin/stdout/stderr can inherit, connect to null, or use pipes
- pipe endpoints are exposed as `binary_stream`
- higher-level popen-style APIs may be considered later
