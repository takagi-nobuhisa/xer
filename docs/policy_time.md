# Policy for Reimplementing `time.h`

## 1. Basic Policy

`xer/time.h` is not intended to mimic the C standard library `time.h` as it is.
Instead, it is redesigned as a C++ library in a way that preserves the simplicity associated with C while aligning with XER's overall policy.

C++ `std::chrono` is powerful, but its expressions often become heavy, and it can be awkward in situations where simple C-style time handling is desired.
For that reason, XER does not place `chrono` at the center of the public API, and instead reconstructs `time.h` as a simple time library derived from C.

Error handling follows XER's general policy and is expressed with `xer::result` and `error<void>` rather than with special values.

---

## 2. Policy for `time_t`

### 2.1 Type

`xer::time_t` is an arithmetic type and uses `double`.

```cpp
using time_t = double;
````

### 2.2 Meaning

The unit of `xer::time_t` is seconds.

* integer part: seconds
* fractional part: fractions of a second

This makes it easy to use ordinary arithmetic in a C-like way.

### 2.3 Resolution

The practical resolution is assumed to be on the order of microseconds.
However, the internal representation is still `double`, and strict fixed-point semantics or guaranteed nanosecond precision are not intended.

---

## 3. Epoch Policy

In the C standard, the epoch of `time_t` is implementation-defined.
In XER, the epoch is fixed to the POSIX epoch:

* 1970-01-01 00:00:00 UTC

That is, the value `0.0` of `xer::time_t` represents this date and time.

---

## 4. Supported Range

In the initial implementation, times before the epoch are not supported.

* `time_t < 0` is unsupported
* dates and times before 1970-01-01 00:00:00 UTC are unsupported

Accordingly, passing a negative `time_t` to `gmtime` or `localtime` results in an error.
Likewise, passing a pre-epoch date and time to `mktime` also results in an error.

---

## 5. Policy for `xer::tm`

### 5.1 Basic Structure

`xer::tm` is based on C's `struct tm`, but adds `tm_microsec` in order to store sub-second information.

```cpp
struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
    int tm_microsec;
};
```

### 5.2 `tm_microsec`

`tm_microsec` represents the fractional microseconds of a second.

* range: from `0` to `999999`

`tm_sec` continues to represent whole seconds in the usual way, and `tm_microsec` complements its fractional part.

---

## 6. Function Categories

Functions in `time.h` are classified as follows according to their nature.

### 6.1 Time Retrieval

* `time`
* `clock`

### 6.2 Breakdown and Reverse Conversion

* `gmtime`
* `localtime`
* `mktime`

### 6.3 Difference Calculation

* `difftime`

### 6.4 String Conversion and Formatting

* `ctime`
* `strftime`

### 6.5 Deferred or Simplified in the Initial Stage

* `asctime` (merged into `ctime`)
* XER-specific high-precision formatting such as `%f`
* advanced locale-dependent functionality
* timezone extension features

---

## 7. Policy for Individual Functions

## 7.1 `time`

Returns the current calendar time.

```cpp
auto time() -> xer::result<time_t>;
```

In practice failure is rare, but when it occurs it is represented with `xer::result`.
`error_t::runtime_error` is sufficient as the error kind.

---

## 7.2 `gmtime`

Converts to a broken-down UTC time.

```cpp
auto gmtime(time_t value) -> xer::result<tm>;
```

* if `value < 0`, returns an error
* the fractional part is stored in `tm_microsec`

---

## 7.3 `localtime`

Converts to a broken-down local time.

```cpp
auto localtime(time_t value) -> xer::result<tm>;
```

* if `value < 0`, returns an error
* the fractional part is stored in `tm_microsec`
* if local time conversion fails, use `error_t::runtime_error`

---

## 7.4 `mktime`

Converts from broken-down time to calendar time.

```cpp
auto mktime(const tm& value) -> xer::result<time_t>;
```

* `tm_microsec` is reflected in the fractional part
* if the result would be a pre-epoch date/time, returns an error
* if `tm_microsec` is out of range, returns an error

---

## 7.5 `ctime`

In XER, `asctime` and `ctime` are unified as `ctime`.
Differences in input type are represented by overloading.

```cpp
auto ctime(time_t value) -> std::u8string;
auto ctime(const tm& value) -> std::u8string;
```

### Policy

* provide one overload that takes `time_t` and another that takes `tm`
* take `tm` as `const tm&`
* return `std::u8string`
* do not use a static internal buffer as in C

There is no need to take `tm` by pointer.
Because XER redesigns this as a C++ library, `const tm&` is adopted to make it explicit that the argument is input-only.

---

## 7.6 `strftime`

Generates a formatted time string.

```cpp
auto strftime(std::u8string_view format, const tm& value) -> xer::result<std::u8string>;
```

### Policy

* the format string uses UTF-8
* the return type is `std::u8string`
* on failure, return `xer::result`

### About the Format String

The format string is not restricted to ASCII.
It may contain fixed UTF-8 text.

For example, the following format should be accepted:

```text
%Y年%m月%d日 %H時%M分%S秒
```

In this case, only `%` and the specifier immediately following it are interpreted specially.
All other UTF-8 text is treated as fixed text and copied as is.

### Initial Implementation

In the initial implementation, the internal implementation may use the implementation's `std::strftime` or `wcsftime`.

However, the XER-specific extension `%f` is not supported in the initial stage.

### Locale Dependence

For specifiers that depend on `LC_TIME` (such as weekday names, month names, and customary representations), implementation-dependent results are acceptable in the initial stage.

In practice, it is uncommon to modify `LC_TIME` explicitly, so excessive strictness is not necessary at the beginning.

---

## 8. Handling of `asctime`

C has `asctime`, but XER does not provide it as an independent function and instead integrates it into `ctime(const tm&)`.

That is, the function that converts broken-down time to a string is handled by `ctime(const tm&)`.

---

## 9. Handling of `error_t`

In the initial implementation of `time.h`, error kinds should be kept only as detailed as necessary.

### Mainly Used Kinds

* `error_t::runtime_error`
* `error_t::invalid_argument`

### Uses

Use `runtime_error` for cases such as:

* failure to retrieve the current time at runtime
* failure of time conversion

Use `invalid_argument` for cases such as:

* negative `time_t`
* out-of-range `tm_microsec`
* pre-epoch date/time
* invalid format specifiers

---

## 10. Functions Included in the Initial Implementation

The following functions are included in the initial implementation:

* `time`
* `clock`
* `gmtime`
* `localtime`
* `mktime`
* `difftime`
* `ctime`
* `strftime`

---

## 11. Items Deferred in the Initial Stage

At least the following are unsupported or simplified in the initial stage:

* times before the epoch
* `%f` in `strftime`
* advanced locale control in `strftime`
* advanced timezone features
* C-compatible static internal buffer behavior
* separate provision of `asctime`

---

## 12. Tentative API List

The current tentative API is as follows:

```cpp
using time_t = double;

struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
    int tm_microsec;
};

auto time() -> xer::result<time_t>;
auto clock() -> xer::result<clock_t>;

auto gmtime(time_t value) -> xer::result<tm>;
auto localtime(time_t value) -> xer::result<tm>;
auto mktime(const tm& value) -> xer::result<time_t>;

auto difftime(time_t left, time_t right) -> double;

auto ctime(time_t value) -> std::u8string;
auto ctime(const tm& value) -> std::u8string;

auto strftime(std::u8string_view format, const tm& value) -> xer::result<std::u8string>;
```

---

## 13. Candidates for Future Extensions

The following extensions may be considered in the future:

* adding `%f` to `strftime`
* better ISO 8601 support including microseconds
* extended timezone information
* a stricter internal representation for high-precision time

---

## Summary

* `xer/time.h` is redesigned as a simple C-style time library rather than centering the public API on `std::chrono`
* normal failure is represented with `xer::result`
* `xer::time_t` uses `double` and represents seconds
* the epoch is fixed to 1970-01-01 00:00:00 UTC
* times before the epoch are unsupported in the initial implementation
* `xer::tm` extends C's `struct tm` with `tm_microsec`
* `ctime` unifies the roles of C's `ctime` and `asctime`
* `strftime` accepts UTF-8 format strings and returns `std::u8string`
* `%f` and advanced locale/timezone features are deferred in the initial stage
