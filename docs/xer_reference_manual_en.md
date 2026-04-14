# xer Reference Manual

## `<xer/arithmetic.h>`

### Functions

#### `xer::add`

Signature 1:

```cpp
decltype(add(lhs, std::declval< U >())) add(T lhs, const result< U > &rhs) noexcept
```
Signature 2:

```cpp
decltype(add(static_cast< detail::forwarded_integer_t< A > >(lhs), static_cast< detail::forwarded_integer_t< B > >(rhs))) add(A lhs, B rhs) noexcept
```
Forwards narrow integer operands to the canonical overload set.

Parameters:

- `lhs`: Left-hand side.
- `rhs`: Right-hand side.

Returns:

- Result of add.

Signature 3:

```cpp
decltype(add(std::declval< T >(), rhs)) add(const result< T > &lhs, U rhs) noexcept
```
Propagates an error from the left operand if present.

Parameters:

- `lhs`: Left-hand side expected value.
- `rhs`: Right-hand side operand.

Returns:

- Result of add.

Signature 4:

```cpp
decltype(add(std::declval< T >(), std::declval< U >())) add(const result< T > &lhs, const result< U > &rhs) noexcept
```
Signature 5:

```cpp
result< long double > add(A lhs, B rhs) noexcept
```
Adds two operands using floating-point arithmetic rules.

Parameters:

- `lhs`: Left-hand side.
- `rhs`: Right-hand side.

Returns:

- Sum as long double.

#### `xer::div`

Signature 1:

```cpp
decltype(div(lhs, std::declval< U >())) div(T lhs, const result< U > &rhs) noexcept
```
Signature 2:

```cpp
decltype(div(lhs, std::declval< U >(), rem)) div(T lhs, const result< U > &rhs, long double *rem) noexcept
```
Signature 3:

```cpp
decltype(div(std::declval< T >(), rhs)) div(const result< T > &lhs, U rhs) noexcept
```
Signature 4:

```cpp
decltype(div(std::declval< T >(), rhs, rem)) div(const result< T > &lhs, U rhs, long double *rem) noexcept
```
Signature 5:

```cpp
decltype(div(std::declval< T >(), std::declval< U >())) div(const result< T > &lhs, const result< U > &rhs) noexcept
```
Signature 6:

```cpp
decltype(div(std::declval< T >(), std::declval< U >(), rem)) div(const result< T > &lhs, const result< U > &rhs, long double *rem) noexcept
```
Signature 7:

```cpp
result< long double > div(A lhs, B rhs) noexcept
```
Divides two operands using floating-point arithmetic rules.

Parameters:

- `lhs`: Left-hand side.
- `rhs`: Right-hand side.

Returns:

- Quotient as long double.

Signature 8:

```cpp
result< long double > div(A lhs, B rhs, long double *rem) noexcept
```
Divides two operands using floating-point arithmetic rules and stores the remainder.

Parameters:

- `lhs`: Left-hand side.
- `rhs`: Right-hand side.
- `rem`: Remainder output pointer. May be null.

Returns:

- Quotient as long double.

#### `xer::eq`

Signature 1:

```cpp
decltype(eq(static_cast< detail::forwarded_compare_integer_t< A > >(lhs), static_cast< detail::forwarded_compare_integer_t< B > >(rhs))) eq(A lhs, B rhs) noexcept
```
Signature 2:

```cpp
result< bool > eq(T lhs, const result< U > &rhs) noexcept
```
Signature 3:

```cpp
result< bool > eq(const result< T > &lhs, U rhs) noexcept
```
Signature 4:

```cpp
result< bool > eq(const result< T > &lhs, const result< U > &rhs) noexcept
```
#### `xer::ge`

Signature 1:

```cpp
decltype(ge(static_cast< detail::forwarded_compare_integer_t< A > >(lhs), static_cast< detail::forwarded_compare_integer_t< B > >(rhs))) ge(A lhs, B rhs) noexcept
```
Signature 2:

```cpp
result< bool > ge(T lhs, const result< U > &rhs) noexcept
```
Signature 3:

```cpp
result< bool > ge(const result< T > &lhs, U rhs) noexcept
```
Signature 4:

```cpp
result< bool > ge(const result< T > &lhs, const result< U > &rhs) noexcept
```
#### `xer::gt`

Signature 1:

```cpp
decltype(gt(static_cast< detail::forwarded_compare_integer_t< A > >(lhs), static_cast< detail::forwarded_compare_integer_t< B > >(rhs))) gt(A lhs, B rhs) noexcept
```
Signature 2:

```cpp
result< bool > gt(T lhs, const result< U > &rhs) noexcept
```
Signature 3:

```cpp
result< bool > gt(const result< T > &lhs, U rhs) noexcept
```
Signature 4:

```cpp
result< bool > gt(const result< T > &lhs, const result< U > &rhs) noexcept
```
#### `xer::le`

Signature 1:

```cpp
decltype(le(static_cast< detail::forwarded_compare_integer_t< A > >(lhs), static_cast< detail::forwarded_compare_integer_t< B > >(rhs))) le(A lhs, B rhs) noexcept
```
Signature 2:

```cpp
result< bool > le(T lhs, const result< U > &rhs) noexcept
```
Signature 3:

```cpp
result< bool > le(const result< T > &lhs, U rhs) noexcept
```
Signature 4:

```cpp
result< bool > le(const result< T > &lhs, const result< U > &rhs) noexcept
```
#### `xer::lt`

Signature 1:

```cpp
decltype(lt(static_cast< detail::forwarded_compare_integer_t< A > >(lhs), static_cast< detail::forwarded_compare_integer_t< B > >(rhs))) lt(A lhs, B rhs) noexcept
```
Signature 2:

```cpp
result< bool > lt(T lhs, const result< U > &rhs) noexcept
```
Signature 3:

```cpp
result< bool > lt(const result< T > &lhs, U rhs) noexcept
```
Signature 4:

```cpp
result< bool > lt(const result< T > &lhs, const result< U > &rhs) noexcept
```
#### `xer::mod`

Signature 1:

```cpp
decltype(mod(lhs, std::declval< U >())) mod(T lhs, const result< U > &rhs) noexcept
```
Signature 2:

```cpp
decltype(mod(std::declval< T >(), rhs)) mod(const result< T > &lhs, U rhs) noexcept
```
Signature 3:

```cpp
decltype(mod(std::declval< T >(), std::declval< U >())) mod(const result< T > &lhs, const result< U > &rhs) noexcept
```
Signature 4:

```cpp
result< long double > mod(A lhs, B rhs) noexcept
```
Returns the floating-point remainder.

Parameters:

- `lhs`: Left-hand side.
- `rhs`: Right-hand side.

Returns:

- Remainder as long double.

#### `xer::mul`

Signature 1:

```cpp
decltype(mul(lhs, std::declval< U >())) mul(T lhs, const result< U > &rhs) noexcept
```
Signature 2:

```cpp
decltype(mul(static_cast< detail::forwarded_integer_t< A > >(lhs), static_cast< detail::forwarded_integer_t< B > >(rhs))) mul(A lhs, B rhs) noexcept
```
Signature 3:

```cpp
decltype(mul(std::declval< T >(), rhs)) mul(const result< T > &lhs, U rhs) noexcept
```
Signature 4:

```cpp
decltype(mul(std::declval< T >(), std::declval< U >())) mul(const result< T > &lhs, const result< U > &rhs) noexcept
```
Signature 5:

```cpp
result< long double > mul(A lhs, B rhs) noexcept
```
Multiplies two operands using floating-point arithmetic rules.

Parameters:

- `lhs`: Left-hand side.
- `rhs`: Right-hand side.

Returns:

- Product as long double.

#### `xer::ne`

Signature 1:

```cpp
decltype(ne(static_cast< detail::forwarded_compare_integer_t< A > >(lhs), static_cast< detail::forwarded_compare_integer_t< B > >(rhs))) ne(A lhs, B rhs) noexcept
```
Signature 2:

```cpp
result< bool > ne(T lhs, const result< U > &rhs) noexcept
```
Signature 3:

```cpp
result< bool > ne(const result< T > &lhs, U rhs) noexcept
```
Signature 4:

```cpp
result< bool > ne(const result< T > &lhs, const result< U > &rhs) noexcept
```
#### `xer::sub`

Signature 1:

```cpp
decltype(sub(lhs, std::declval< U >())) sub(T lhs, const result< U > &rhs) noexcept
```
Signature 2:

```cpp
decltype(sub(static_cast< detail::forwarded_integer_t< A > >(lhs), static_cast< detail::forwarded_integer_t< B > >(rhs))) sub(A lhs, B rhs) noexcept
```
Signature 3:

```cpp
decltype(sub(std::declval< T >(), rhs)) sub(const result< T > &lhs, U rhs) noexcept
```
Signature 4:

```cpp
decltype(sub(std::declval< T >(), std::declval< U >())) sub(const result< T > &lhs, const result< U > &rhs) noexcept
```
Signature 5:

```cpp
result< long double > sub(A lhs, B rhs) noexcept
```
Subtracts two operands using floating-point arithmetic rules.

Parameters:

- `lhs`: Left-hand side.
- `rhs`: Right-hand side.

Returns:

- Difference as long double.

#### `xer::uadd`

Signature 1:

```cpp
decltype(uadd(lhs, std::declval< U >())) uadd(T lhs, const result< U > &rhs) noexcept
```
Signature 2:

```cpp
decltype(uadd(static_cast< detail::forwarded_integer_t< A > >(lhs), static_cast< detail::forwarded_integer_t< B > >(rhs))) uadd(A lhs, B rhs) noexcept
```
Signature 3:

```cpp
decltype(uadd(std::declval< T >(), rhs)) uadd(const result< T > &lhs, U rhs) noexcept
```
Signature 4:

```cpp
decltype(uadd(std::declval< T >(), std::declval< U >())) uadd(const result< T > &lhs, const result< U > &rhs) noexcept
```
#### `xer::umul`

Signature 1:

```cpp
decltype(umul(lhs, std::declval< U >())) umul(T lhs, const result< U > &rhs) noexcept
```
Signature 2:

```cpp
decltype(umul(static_cast< detail::forwarded_integer_t< A > >(lhs), static_cast< detail::forwarded_integer_t< B > >(rhs))) umul(A lhs, B rhs) noexcept
```
Signature 3:

```cpp
decltype(umul(std::declval< T >(), rhs)) umul(const result< T > &lhs, U rhs) noexcept
```
Signature 4:

```cpp
decltype(umul(std::declval< T >(), std::declval< U >())) umul(const result< T > &lhs, const result< U > &rhs) noexcept
```
#### `xer::usub`

Signature 1:

```cpp
decltype(usub(lhs, std::declval< U >())) usub(T lhs, const result< U > &rhs) noexcept
```
Signature 2:

```cpp
decltype(usub(static_cast< detail::forwarded_integer_t< A > >(lhs), static_cast< detail::forwarded_integer_t< B > >(rhs))) usub(A lhs, B rhs) noexcept
```
Signature 3:

```cpp
decltype(usub(std::declval< T >(), rhs)) usub(const result< T > &lhs, U rhs) noexcept
```
Signature 4:

```cpp
decltype(usub(std::declval< T >(), std::declval< U >())) usub(const result< T > &lhs, const result< U > &rhs) noexcept
```

## `<xer/assert.h>`

### Macros

#### `xer_assert()`

Asserts that the expression is true.

#### `xer_assert_eq(, )`

Asserts that two values are equal.

#### `xer_assert_lt(, )`

Asserts that the left value is less than the right value.

#### `xer_assert_ne(, )`

Asserts that two values are not equal.

#### `xer_assert_not()`

Asserts that the expression is false.

#### `xer_assert_nothrow()`

Asserts that the expression does not throw.

#### `xer_assert_throw(, )`

Asserts that the expression throws the specified exception type.

### Types

#### `xer::assertion_error`

Exception thrown when a xer assert macro fails.

Member functions:

- `assertion_error`
  - Overview: Initializes the exception.
  - Signature 1:

    ```cpp
    assertion_error(std::string message)
    ```
    Initializes the exception.

    Parameters:
    - `message`: Diagnostic message.

### Functions

#### `xer::advanced::packed_cp932_to_utf32`

Converts packed CP932 to a UTF-32 code point.

Signature 1:

```cpp
char32_t packed_cp932_to_utf32(std::uint16_t packed)
```
Converts packed CP932 to a UTF-32 code point.

Parameters:

- `packed`: Packed CP932.

Returns:

- UTF-32 code point, or static_cast<char32_t>(-1) on failure.

#### `xer::advanced::packed_utf16_to_utf32`

Converts packed UTF-16 to a UTF-32 code point.

Signature 1:

```cpp
char32_t packed_utf16_to_utf32(std::uint32_t packed)
```
Converts packed UTF-16 to a UTF-32 code point.

Parameters:

- `packed`: Packed UTF-16.

Returns:

- UTF-32 code point, or static_cast<char32_t>(-1) on failure.

#### `xer::advanced::packed_utf8_to_utf32`

Converts packed UTF-8 to a UTF-32 code point.

Signature 1:

```cpp
char32_t packed_utf8_to_utf32(std::uint32_t packed)
```
Converts packed UTF-8 to a UTF-32 code point.

Parameters:

- `packed`: Packed UTF-8.

Returns:

- UTF-32 code point, or static_cast<char32_t>(-1) on failure.

#### `xer::advanced::utf32_to_packed_cp932`

Converts a UTF-32 code point to packed CP932.

Signature 1:

```cpp
std::int32_t utf32_to_packed_cp932(char32_t code_point)
```
Converts a UTF-32 code point to packed CP932.

Parameters:

- `code_point`: UTF-32 code point.

Returns:

- Packed CP932, or -1 on failure.

#### `xer::advanced::utf32_to_packed_utf16`

Converts a UTF-32 code point to packed UTF-16.

Signature 1:

```cpp
std::uint32_t utf32_to_packed_utf16(char32_t code_point)
```
Converts a UTF-32 code point to packed UTF-16.

Parameters:

- `code_point`: UTF-32 code point.

Returns:

- Packed UTF-16, or static_cast<std::uint32_t>(-1) on failure.

#### `xer::advanced::utf32_to_packed_utf8`

Converts a UTF-32 code point to packed UTF-8.

Signature 1:

```cpp
std::uint32_t utf32_to_packed_utf8(char32_t code_point)
```
Converts a UTF-32 code point to packed UTF-8.

Parameters:

- `code_point`: UTF-32 code point.

Returns:

- Packed UTF-8, or static_cast<std::uint32_t>(-1) on failure.

## `<xer/ctype.h>`

### Types

#### `xer::ctrans_id`

Identifiers for character transformations.

Enumerators:

- `lower`
- `upper`
- `latin1_lowercase`
- `latin1_uppercase`

#### `xer::ctype_id`

Identifiers for character classes.

Enumerators:

- `alpha`
- `digit`
- `alnum`
- `lower`
- `upper`
- `space`
- `blank`
- `cntrl`
- `print`
- `graph`
- `punct`
- `xdigit`
- `ascii`
- `octal`
- `binary`
- `latin1_alpha`
- `latin1_upper`
- `latin1_lower`
- `latin1_alnum`
- `latin1_graph`
- `latin1_print`

### Functions

#### `xer::isalnum`

Returns whether the code point is an ASCII alphanumeric character.

Signature 1:

```cpp
bool isalnum(char32_t c) noexcept
```
Returns whether the code point is an ASCII alphanumeric character.

Parameters:

- `c`: Code point to test.

Returns:

- True if the code point is an ASCII letter or digit.

#### `xer::isalpha`

Returns whether the code point is an ASCII alphabetic letter.

Signature 1:

```cpp
bool isalpha(char32_t c) noexcept
```
Returns whether the code point is an ASCII alphabetic letter.

Parameters:

- `c`: Code point to test.

Returns:

- True if the code point is an ASCII letter.

#### `xer::isascii`

Returns whether the code point is an ASCII character.

Signature 1:

```cpp
bool isascii(char32_t c) noexcept
```
Returns whether the code point is an ASCII character.

Parameters:

- `c`: Code point to test.

Returns:

- True if the code point is in the ASCII range.

#### `xer::isbinary`

Returns whether the code point is an ASCII binary digit.

Signature 1:

```cpp
bool isbinary(char32_t c) noexcept
```
Returns whether the code point is an ASCII binary digit.

Parameters:

- `c`: Code point to test.

Returns:

- True if the code point is '0' or '1'.

#### `xer::isblank`

Returns whether the code point is an ASCII horizontal blank.

Signature 1:

```cpp
bool isblank(char32_t c) noexcept
```
Returns whether the code point is an ASCII horizontal blank.

Parameters:

- `c`: Code point to test.

Returns:

- True if the code point is space or horizontal tab.

#### `xer::iscntrl`

Returns whether the code point is an ASCII control character.

Signature 1:

```cpp
bool iscntrl(char32_t c) noexcept
```
Returns whether the code point is an ASCII control character.

Parameters:

- `c`: Code point to test.

Returns:

- True if the code point is in the ASCII control range.

#### `xer::isctype`

Classifies a code point by the specified character class identifier.

Signature 1:

```cpp
bool isctype(char32_t c, ctype_id id) noexcept
```
Classifies a code point by the specified character class identifier.

Parameters:

- `c`: Code point to test.
- `id`: Character class identifier.

Returns:

- Classification result.

#### `xer::isdigit`

Returns whether the code point is an ASCII decimal digit.

Signature 1:

```cpp
bool isdigit(char32_t c) noexcept
```
Returns whether the code point is an ASCII decimal digit.

Parameters:

- `c`: Code point to test.

Returns:

- True if the code point is in the range '0'..'9'.

#### `xer::isgraph`

Returns whether the code point is an ASCII graphic character.

Signature 1:

```cpp
bool isgraph(char32_t c) noexcept
```
Returns whether the code point is an ASCII graphic character.

Parameters:

- `c`: Code point to test.

Returns:

- True if the code point is in the range 0x21..0x7E.

#### `xer::islatin1_alnum`

Returns whether the code point is a Latin-1 alphanumeric character.

Signature 1:

```cpp
bool islatin1_alnum(char32_t c) noexcept
```
Returns whether the code point is a Latin-1 alphanumeric character.

Parameters:

- `c`: Code point to test.

Returns:

- True if the code point is a Latin-1 letter or an ASCII digit.

#### `xer::islatin1_alpha`

Returns whether the code point is a Latin-1 alphabetic letter.

Signature 1:

```cpp
bool islatin1_alpha(char32_t c) noexcept
```
Returns whether the code point is a Latin-1 alphabetic letter.

Parameters:

- `c`: Code point to test.

Returns:

- True if the code point is an alphabetic Latin-1 character.

#### `xer::islatin1_graph`

Returns whether the code point is a graphic Latin-1 character.

Signature 1:

```cpp
bool islatin1_graph(char32_t c) noexcept
```
Returns whether the code point is a graphic Latin-1 character.

Parameters:

- `c`: Code point to test.

Returns:

- True if the code point is graphic in Latin-1.

#### `xer::islatin1_lower`

Returns whether the code point is a Latin-1 lowercase letter.

Signature 1:

```cpp
bool islatin1_lower(char32_t c) noexcept
```
Returns whether the code point is a Latin-1 lowercase letter.

Parameters:

- `c`: Code point to test.

Returns:

- True if the code point is a lowercase Latin-1 letter.

#### `xer::islatin1_print`

Returns whether the code point is a printable Latin-1 character.

Signature 1:

```cpp
bool islatin1_print(char32_t c) noexcept
```
Returns whether the code point is a printable Latin-1 character.

Parameters:

- `c`: Code point to test.

Returns:

- True if the code point is printable in Latin-1.

#### `xer::islatin1_upper`

Returns whether the code point is a Latin-1 uppercase letter.

Signature 1:

```cpp
bool islatin1_upper(char32_t c) noexcept
```
Returns whether the code point is a Latin-1 uppercase letter.

Parameters:

- `c`: Code point to test.

Returns:

- True if the code point is an uppercase Latin-1 letter.

#### `xer::islower`

Returns whether the code point is an ASCII lowercase letter.

Signature 1:

```cpp
bool islower(char32_t c) noexcept
```
Returns whether the code point is an ASCII lowercase letter.

Parameters:

- `c`: Code point to test.

Returns:

- True if the code point is in the range 'a'..'z'.

#### `xer::isoctal`

Returns whether the code point is an ASCII octal digit.

Signature 1:

```cpp
bool isoctal(char32_t c) noexcept
```
Returns whether the code point is an ASCII octal digit.

Parameters:

- `c`: Code point to test.

Returns:

- True if the code point is in the range '0'..'7'.

#### `xer::isprint`

Returns whether the code point is an ASCII printable character.

Signature 1:

```cpp
bool isprint(char32_t c) noexcept
```
Returns whether the code point is an ASCII printable character.

Parameters:

- `c`: Code point to test.

Returns:

- True if the code point is in the range 0x20..0x7E.

#### `xer::ispunct`

Returns whether the code point is an ASCII punctuation character.

Signature 1:

```cpp
bool ispunct(char32_t c) noexcept
```
Returns whether the code point is an ASCII punctuation character.

Parameters:

- `c`: Code point to test.

Returns:

- True if the code point is printable and neither alnum nor space.

#### `xer::isspace`

Returns whether the code point is an ASCII whitespace character.

Signature 1:

```cpp
bool isspace(char32_t c) noexcept
```
Returns whether the code point is an ASCII whitespace character.

Parameters:

- `c`: Code point to test.

Returns:

- True if the code point is one of the C locale whitespace characters.

#### `xer::isupper`

Returns whether the code point is an ASCII uppercase letter.

Signature 1:

```cpp
bool isupper(char32_t c) noexcept
```
Returns whether the code point is an ASCII uppercase letter.

Parameters:

- `c`: Code point to test.

Returns:

- True if the code point is in the range 'A'..'Z'.

#### `xer::isxdigit`

Returns whether the code point is an ASCII hexadecimal digit.

Signature 1:

```cpp
bool isxdigit(char32_t c) noexcept
```
Returns whether the code point is an ASCII hexadecimal digit.

Parameters:

- `c`: Code point to test.

Returns:

- True if the code point is 0-9, A-F, or a-f.

#### `xer::toctrans`

Converts a code point according to the specified transformation.

Signature 1:

```cpp
result< char32_t > toctrans(char32_t c, ctrans_id id)
```
Converts a code point according to the specified transformation.

Parameters:

- `c`: Code point to convert.
- `id`: Transformation identifier.

Returns:

- Converted character on success.

#### `xer::tolower`

Converts an ASCII character to lowercase.

Signature 1:

```cpp
result< char32_t > tolower(char32_t c)
```
Converts an ASCII character to lowercase.

Parameters:

- `c`: Code point to convert.

Returns:

- Converted character on success.

#### `xer::toupper`

Converts an ASCII character to uppercase.

Signature 1:

```cpp
result< char32_t > toupper(char32_t c)
```
Converts an ASCII character to uppercase.

Parameters:

- `c`: Code point to convert.

Returns:

- Converted character on success.

## `<xer/error.h>`

### Types

#### `xer::error`

Represents an error with additional detail.

Member functions:

- `error`
  - Overview: Constructs an error object.
  - Signature 1:

    ```cpp
    constexpr error(error_t code_, T &&value_, std::source_location location_)
    ```
    Constructs an error object.

    Parameters:
    - `code_`: Error code.
    - `value_`: Value forwarded to Detail construction.
    - `location_`: Source location where the error was created.

#### `xer::error< void >`

Specialization for errors without detail.

Member functions:

- `error`
  - Overview: Constructs an error object.
  - Signature 1:

    ```cpp
    constexpr error(error_t code_, std::source_location location_) noexcept
    ```
    Constructs an error object.

    Parameters:
    - `code_`: Error code.
    - `location_`: Source location where the error was created.

#### `xer::error_t`

Represents XER error codes.

Underlying type: `std::int32_t`

Enumerators:

- `perm` = EPERM
- `noent` = ENOENT
- `srch` = ESRCH
- `intr` = EINTR
- `io` = EIO
- `nxio` = ENXIO
- `toobig` = E2BIG
- `noexec` = ENOEXEC
- `badf` = EBADF
- `child` = ECHILD
- `again` = EAGAIN
- `nomem` = ENOMEM
- `acces` = EACCES
- `fault` = EFAULT
- `busy` = EBUSY
- `exist` = EEXIST
- `xdev` = EXDEV
- `nodev` = ENODEV
- `notdir` = ENOTDIR
- `isdir` = EISDIR
- `inval` = EINVAL
- `nfile` = ENFILE
- `mfile` = EMFILE
- `notty` = ENOTTY
- `fbig` = EFBIG
- `nospc` = ENOSPC
- `spipe` = ESPIPE
- `rofs` = EROFS
- `mlink` = EMLINK
- `pipe` = EPIPE
- `dom` = EDOM
- `range` = ERANGE
- `logic_error` = -1
- `domain_error` = -2
- `invalid_argument` = -3
- `length_error` = -4
- `out_of_range` = -5
- `runtime_error` = -6
- `range_error` = -7
- `overflow_error` = -8
- `underflow_error` = -9
- `io_error` = -10
- `encoding_error` = -11
- `not_found` = -12
- `divide_by_zero` = -13
- `user_error` = -1000

#### `xer::result`

Alias of `std::expected< T, error< Detail > >`.

Standard XER result type.

### Functions

#### `xer::make_error`

Creates an error with additional detail.

Signature 1:

```cpp
error< Detail > make_error(error_t code, T &&value, std::source_location location=std::source_location::current())
```
Creates an error with additional detail.

Parameters:

- `code`: Error code.
- `value`: Value forwarded to Detail construction.
- `location`: Source location where the error is created.

Returns:

- Created error object.

Signature 2:

```cpp
error< void > make_error(error_t code, std::source_location location=std::source_location::current()) noexcept
```
Creates an error without additional detail.

Parameters:

- `code`: Error code.
- `location`: Source location where the error is created.

Returns:

- Created error object.

## `<xer/path.h>`

### Macros

#### `XER_PRETTY_FUNCTION`

Common name for the current function signature string.

#### `XER_STDCPP_VERSION`

Common name for the active C++ language version.

### Types

#### `xer::native_path_char_t`

Alias of `char`.

Native path character type for the current platform.

#### `xer::native_path_string`

Alias of `std::string`.

Native path string type for the current platform.

#### `xer::native_path_view`

Alias of `std::string_view`.

Native path string view type for the current platform.

#### `xer::path`

UTF-8 based lexical path.

Member functions:

- `path`
  - Overview: Constructs an empty path.
  - Signature 1:

    ```cpp
    path()=default
    ```
    Constructs an empty path.

  - Signature 2:

    ```cpp
    path(const char8_t *value)
    ```
    Constructs a path from a UTF-8 null-terminated string.

    Parameters:
    - `value`: Source UTF-8 path string.

  - Signature 3:

    ```cpp
    path(std::u8string_view value)
    ```
    Constructs a path from a UTF-8 string view.

    Parameters:
    - `value`: Source UTF-8 path string.

- `str`
  - Overview: Returns the normalized internal UTF-8 representation.
  - Signature 1:

    ```cpp
    view_type str() const noexcept
    ```
    Returns the normalized internal UTF-8 representation.

    Returns:
    - Normalized UTF-8 path string.

### Functions

#### `xer::advanced::packed_cp932_to_utf32`

Converts packed CP932 to a UTF-32 code point.

Signature 1:

```cpp
char32_t packed_cp932_to_utf32(std::uint16_t packed)
```
Converts packed CP932 to a UTF-32 code point.

Parameters:

- `packed`: Packed CP932.

Returns:

- UTF-32 code point, or static_cast<char32_t>(-1) on failure.

#### `xer::advanced::packed_utf16_to_utf32`

Converts packed UTF-16 to a UTF-32 code point.

Signature 1:

```cpp
char32_t packed_utf16_to_utf32(std::uint32_t packed)
```
Converts packed UTF-16 to a UTF-32 code point.

Parameters:

- `packed`: Packed UTF-16.

Returns:

- UTF-32 code point, or static_cast<char32_t>(-1) on failure.

#### `xer::advanced::packed_utf8_to_utf32`

Converts packed UTF-8 to a UTF-32 code point.

Signature 1:

```cpp
char32_t packed_utf8_to_utf32(std::uint32_t packed)
```
Converts packed UTF-8 to a UTF-32 code point.

Parameters:

- `packed`: Packed UTF-8.

Returns:

- UTF-32 code point, or static_cast<char32_t>(-1) on failure.

#### `xer::advanced::utf32_to_packed_cp932`

Converts a UTF-32 code point to packed CP932.

Signature 1:

```cpp
std::int32_t utf32_to_packed_cp932(char32_t code_point)
```
Converts a UTF-32 code point to packed CP932.

Parameters:

- `code_point`: UTF-32 code point.

Returns:

- Packed CP932, or -1 on failure.

#### `xer::advanced::utf32_to_packed_utf16`

Converts a UTF-32 code point to packed UTF-16.

Signature 1:

```cpp
std::uint32_t utf32_to_packed_utf16(char32_t code_point)
```
Converts a UTF-32 code point to packed UTF-16.

Parameters:

- `code_point`: UTF-32 code point.

Returns:

- Packed UTF-16, or static_cast<std::uint32_t>(-1) on failure.

#### `xer::advanced::utf32_to_packed_utf8`

Converts a UTF-32 code point to packed UTF-8.

Signature 1:

```cpp
std::uint32_t utf32_to_packed_utf8(char32_t code_point)
```
Converts a UTF-32 code point to packed UTF-8.

Parameters:

- `code_point`: UTF-32 code point.

Returns:

- Packed UTF-8, or static_cast<std::uint32_t>(-1) on failure.

#### `xer::from_native_path`

Converts a native null-terminated path string to a XER path.

Signature 1:

```cpp
std::expected< path, error< void > > from_native_path(const native_path_char_t *value)
```
Converts a native null-terminated path string to a XER path.

Parameters:

- `value`: Source native path string.

Returns:

- Converted XER path on success.

Signature 2:

```cpp
std::expected< path, error< void > > from_native_path(native_path_view value)
```
Converts a native path string view to a XER path.

Parameters:

- `value`: Source native path string.

Returns:

- Converted XER path on success.

#### `xer::to_native_path`

Converts a XER path to the native path string type.

Signature 1:

```cpp
std::expected< native_path_string, error< void > > to_native_path(const path &value)
```
Converts a XER path to the native path string type.

Parameters:

- `value`: Source path.

Returns:

- Native path string on success.

## `<xer/stdint.h>`

### Objects and constants

#### `xer::bit_width_of`

Type: `int`

#### `xer::max_of`

Type: `T`

#### `xer::min_of`

Type: `T`

### Functions

#### `xer::literals::integer_literals::operator""_i16`

Signature 1:

```cpp
int16_t operator""_i16()
```
#### `xer::literals::integer_literals::operator""_i32`

Signature 1:

```cpp
int32_t operator""_i32()
```
#### `xer::literals::integer_literals::operator""_i64`

Signature 1:

```cpp
int64_t operator""_i64()
```
#### `xer::literals::integer_literals::operator""_i8`

Signature 1:

```cpp
int8_t operator""_i8()
```
#### `xer::literals::integer_literals::operator""_u16`

Signature 1:

```cpp
uint16_t operator""_u16()
```
#### `xer::literals::integer_literals::operator""_u32`

Signature 1:

```cpp
uint32_t operator""_u32()
```
#### `xer::literals::integer_literals::operator""_u64`

Signature 1:

```cpp
uint64_t operator""_u64()
```
#### `xer::literals::integer_literals::operator""_u8`

Signature 1:

```cpp
uint8_t operator""_u8()
```

## `<xer/stdio.h>`

### Macros

#### `xer_stderr`

#### `xer_stdin`

#### `xer_stdout`

### Types

#### `xer::binary_stream`

Move-only binary stream object.

#### `xer::encoding_t`

Text encoding selector.

Enumerators:

- `utf8`
- `cp932`
- `auto_detect`

#### `xer::fpos_t`

Alias of `std::uint64_t`.

Stream position type for fgetpos/fsetpos.

#### `xer::seek_origin_t`

Stream seek origin constants.

Enumerators:

- `seek_set` = 0 — Seek relative to the beginning of the stream.
- `seek_cur` = 1 — Seek relative to the current stream position.
- `seek_end` = 2 — Seek relative to the end of the stream.

#### `xer::text_stream`

Move-only text stream object.

#### `xer::text_stream_pos_t`

Alias of `std::int64_t`.

Opaque text stream position type.

### Objects and constants

#### `xer::standard_error`

Type: `text_stream`

Standard error text stream.

#### `xer::standard_input`

Type: `text_stream`

Standard input text stream.

#### `xer::standard_output`

Type: `text_stream`

Standard output text stream.

### Functions

#### `xer::advanced::packed_cp932_to_utf32`

Converts packed CP932 to a UTF-32 code point.

Signature 1:

```cpp
char32_t packed_cp932_to_utf32(std::uint16_t packed)
```
Converts packed CP932 to a UTF-32 code point.

Parameters:

- `packed`: Packed CP932.

Returns:

- UTF-32 code point, or static_cast<char32_t>(-1) on failure.

#### `xer::advanced::packed_utf16_to_utf32`

Converts packed UTF-16 to a UTF-32 code point.

Signature 1:

```cpp
char32_t packed_utf16_to_utf32(std::uint32_t packed)
```
Converts packed UTF-16 to a UTF-32 code point.

Parameters:

- `packed`: Packed UTF-16.

Returns:

- UTF-32 code point, or static_cast<char32_t>(-1) on failure.

#### `xer::advanced::packed_utf8_to_utf32`

Converts packed UTF-8 to a UTF-32 code point.

Signature 1:

```cpp
char32_t packed_utf8_to_utf32(std::uint32_t packed)
```
Converts packed UTF-8 to a UTF-32 code point.

Parameters:

- `packed`: Packed UTF-8.

Returns:

- UTF-32 code point, or static_cast<char32_t>(-1) on failure.

#### `xer::advanced::utf32_to_packed_cp932`

Converts a UTF-32 code point to packed CP932.

Signature 1:

```cpp
std::int32_t utf32_to_packed_cp932(char32_t code_point)
```
Converts a UTF-32 code point to packed CP932.

Parameters:

- `code_point`: UTF-32 code point.

Returns:

- Packed CP932, or -1 on failure.

#### `xer::advanced::utf32_to_packed_utf16`

Converts a UTF-32 code point to packed UTF-16.

Signature 1:

```cpp
std::uint32_t utf32_to_packed_utf16(char32_t code_point)
```
Converts a UTF-32 code point to packed UTF-16.

Parameters:

- `code_point`: UTF-32 code point.

Returns:

- Packed UTF-16, or static_cast<std::uint32_t>(-1) on failure.

#### `xer::advanced::utf32_to_packed_utf8`

Converts a UTF-32 code point to packed UTF-8.

Signature 1:

```cpp
std::uint32_t utf32_to_packed_utf8(char32_t code_point)
```
Converts a UTF-32 code point to packed UTF-8.

Parameters:

- `code_point`: UTF-32 code point.

Returns:

- Packed UTF-8, or static_cast<std::uint32_t>(-1) on failure.

#### `xer::clearerr`

Clears the EOF and error indicators of the binary stream.

Signature 1:

```cpp
void clearerr(binary_stream &stream) noexcept
```
Clears the EOF and error indicators of the binary stream.

Parameters:

- `stream`: Target binary stream.

Signature 2:

```cpp
void clearerr(text_stream &stream) noexcept
```
Clears the EOF and error indicators of the text stream.

Parameters:

- `stream`: Target text stream.

#### `xer::fclose`

Closes a binary stream.

Signature 1:

```cpp
result< void > fclose(binary_stream &stream) noexcept
```
Closes a binary stream.

Parameters:

- `stream`: Target stream.

Returns:

- Empty expected on success.
- Unexpected error on failure.

Signature 2:

```cpp
result< void > fclose(text_stream &stream) noexcept
```
Closes a text stream.

Parameters:

- `stream`: Target stream.

Returns:

- Empty expected on success.
- Unexpected error on failure.

#### `xer::feof`

Returns whether the binary stream is at end-of-file state.

Signature 1:

```cpp
bool feof(const binary_stream &stream) noexcept
```
Returns whether the binary stream is at end-of-file state.

Parameters:

- `stream`: Target binary stream.

Returns:

- true if the EOF indicator is set.
- false otherwise.

Signature 2:

```cpp
bool feof(const text_stream &stream) noexcept
```
Returns whether the text stream is at end-of-file state.

Parameters:

- `stream`: Target text stream.

Returns:

- true if the EOF indicator is set.
- false otherwise.

#### `xer::ferror`

Returns whether the binary stream is at error state.

Signature 1:

```cpp
bool ferror(const binary_stream &stream) noexcept
```
Returns whether the binary stream is at error state.

Parameters:

- `stream`: Target binary stream.

Returns:

- true if the error indicator is set.
- false otherwise.

Signature 2:

```cpp
bool ferror(const text_stream &stream) noexcept
```
Returns whether the text stream is at error state.

Parameters:

- `stream`: Target text stream.

Returns:

- true if the error indicator is set.
- false otherwise.

#### `xer::fflush`

Flushes a binary stream.

Signature 1:

```cpp
result< void > fflush(binary_stream &stream) noexcept
```
Flushes a binary stream.

Parameters:

- `stream`: Target stream.

Returns:

- Empty expected on success.
- Unexpected error on failure.

Signature 2:

```cpp
result< void > fflush(text_stream &stream) noexcept
```
Flushes a text stream.

Parameters:

- `stream`: Target stream.

Returns:

- Empty expected on success.
- Unexpected error on failure.

#### `xer::fgetb`

Reads one byte from a binary stream.

Signature 1:

```cpp
result< std::byte > fgetb(binary_stream &stream) noexcept
```
Reads one byte from a binary stream.

Parameters:

- `stream`: Source stream.

Returns:

- Read byte on success.

#### `xer::fgetc`

Reads one character from a text stream.

Signature 1:

```cpp
result< char32_t > fgetc(text_stream &stream)
```
Reads one character from a text stream.

Parameters:

- `stream`: Target stream.

Returns:

- Read code point on success.

#### `xer::fgetpos`

Obtains a binary stream position for later restoration.

Signature 1:

```cpp
result< std::uint64_t > fgetpos(binary_stream &stream) noexcept
```
Obtains a binary stream position for later restoration.

Parameters:

- `stream`: Target stream.

Returns:

- Current position on success.

Signature 2:

```cpp
result< std::uint64_t > fgetpos(text_stream &stream) noexcept
```
Obtains a text stream position for later restoration.

Parameters:

- `stream`: Target stream.

Returns:

- Current position on success.

#### `xer::fgets`

Reads one line from a text stream.

Signature 1:

```cpp
result< std::u8string > fgets(text_stream &stream, bool keep_newline=true)
```
Reads one line from a text stream.

Parameters:

- `stream`: Target stream.
- `keep_newline`: Whether to keep the trailing newline.

Returns:

- UTF-8 line text on success.

#### `xer::fopen`

Opens a binary file stream.

Signature 1:

```cpp
result< binary_stream > fopen(const path &filename, const char *mode) noexcept
```
Opens a binary file stream.

Parameters:

- `filename`: Source file path.
- `mode`: Open mode string.

Returns:

- Opened binary stream on success.
- Unexpected error on failure.

Signature 2:

```cpp
result< text_stream > fopen(const path &filename, const char *mode, encoding_t encoding) noexcept
```
Opens a text file stream.

Parameters:

- `filename`: Source file path.
- `mode`: Open mode string.
- `encoding`: Text encoding selector.

Returns:

- Opened text stream on success.
- Unexpected error on failure.

#### `xer::fprintf`

Writes formatted text to a text stream.

Signature 1:

```cpp
result< std::size_t > fprintf(text_stream &stream, std::u8string_view format, Args &&... args)
```
Writes formatted text to a text stream.

Parameters:

- `stream`: Destination stream.
- `format`: UTF-8 format string.
- `args`: Format arguments.

Returns:

- Written byte count on success.

#### `xer::fputb`

Writes one byte to a binary stream.

Signature 1:

```cpp
result< std::byte > fputb(std::byte value, binary_stream &stream) noexcept
```
Writes one byte to a binary stream.

Parameters:

- `value`: Source byte.
- `stream`: Destination stream.

Returns:

- Success or error.

#### `xer::fputc`

Writes one character to a text stream.

Signature 1:

```cpp
result< char32_t > fputc(char32_t ch, text_stream &stream)
```
Writes one character to a text stream.

Parameters:

- `ch`: Source code point.
- `stream`: Target stream.

Returns:

- The written code point on success.

#### `xer::fputs`

Writes UTF-8 text to a text stream.

Signature 1:

```cpp
result< std::size_t > fputs(std::u8string_view text, text_stream &stream, bool append_newline=false)
```
Writes UTF-8 text to a text stream.

Parameters:

- `text`: Source UTF-8 text.
- `stream`: Target stream.
- `append_newline`: Whether to append a trailing newline.

Returns:

- Number of UTF-8 code units requested for output on success.

#### `xer::fread`

Reads bytes from a binary stream.

Signature 1:

```cpp
result< std::size_t > fread(std::span< std::byte > buffer, binary_stream &stream) noexcept
```
Reads bytes from a binary stream.

Parameters:

- `buffer`: Destination buffer.
- `stream`: Source stream.

Returns:

- Number of bytes read on success.

#### `xer::fscanf`

Scans values from a text stream.

Signature 1:

```cpp
result< std::size_t > fscanf(text_stream &input, std::u8string_view format, Args *... args)
```
Scans values from a text stream.

Parameters:

- `input`: Source text stream.
- `format`: Source UTF-8 format string.
- `args`: Output target pointers.

Returns:

- Assignment count on success.

#### `xer::fseek`

Repositions a binary stream.

Signature 1:

```cpp
result< void > fseek(binary_stream &stream, std::int64_t offset, seek_origin_t origin) noexcept
```
Repositions a binary stream.

Parameters:

- `stream`: Target stream.
- `offset`: Signed byte offset.
- `origin`: Seek origin.

Returns:

- Success or error.

Signature 2:

```cpp
result< void > fseek(text_stream &stream, std::int64_t offset, seek_origin_t origin) noexcept
```
Repositions a text stream.

Parameters:

- `stream`: Target stream.
- `offset`: Signed offset.
- `origin`: Seek origin.

Returns:

- Success or error.

#### `xer::fsetpos`

Restores a previously obtained binary stream position.

Signature 1:

```cpp
result< void > fsetpos(binary_stream &stream, fpos_t position) noexcept
```
Restores a previously obtained binary stream position.

Parameters:

- `stream`: Target stream.
- `position`: Position value obtained from fgetpos.

Returns:

- Success or error.

Signature 2:

```cpp
result< void > fsetpos(text_stream &stream, fpos_t position) noexcept
```
Restores a previously obtained text stream position.

Parameters:

- `stream`: Target stream.
- `position`: Position value obtained from fgetpos.

Returns:

- Success or error.

#### `xer::ftell`

Returns the current binary stream position.

Signature 1:

```cpp
result< std::uint64_t > ftell(binary_stream &stream) noexcept
```
Returns the current binary stream position.

Parameters:

- `stream`: Target stream.

Returns:

- Current byte offset on success.

Signature 2:

```cpp
result< std::uint64_t > ftell(text_stream &stream) noexcept
```
Returns the current text stream position value.

Parameters:

- `stream`: Target stream.

Returns:

- Current position value on success.

#### `xer::fwrite`

Writes bytes to a binary stream.

Signature 1:

```cpp
result< std::size_t > fwrite(std::span< const std::byte > buffer, binary_stream &stream) noexcept
```
Writes bytes to a binary stream.

Parameters:

- `buffer`: Source buffer.
- `stream`: Destination stream.

Returns:

- Number of bytes written on success.

#### `xer::getchar`

Reads one character from the standard input text stream.

Signature 1:

```cpp
result< char32_t > getchar()
```
Reads one character from the standard input text stream.

Returns:

- Read code point on success.

#### `xer::gets`

Reads one line from the standard input text stream.

Signature 1:

```cpp
result< std::u8string > gets(bool keep_newline=false)
```
Reads one line from the standard input text stream.

Parameters:

- `keep_newline`: Whether to keep the trailing newline.

Returns:

- UTF-8 line text on success.

#### `xer::memopen`

Opens a binary memory stream.

Signature 1:

```cpp
result< binary_stream > memopen(std::span< std::byte > mem, const char *mode) noexcept
```
Opens a binary memory stream.

Parameters:

- `mem`: Target memory region.
- `mode`: Open mode string.

Returns:

- Opened binary stream on success.
- Unexpected error on failure.

#### `xer::printf`

Writes formatted text to the standard output stream.

Signature 1:

```cpp
result< std::size_t > printf(std::u8string_view format, Args &&... args)
```
Writes formatted text to the standard output stream.

Parameters:

- `format`: UTF-8 format string.
- `args`: Format arguments.

Returns:

- Written byte count on success.

#### `xer::putchar`

Writes one character to the standard output text stream.

Signature 1:

```cpp
result< char32_t > putchar(char32_t ch)
```
Writes one character to the standard output text stream.

Parameters:

- `ch`: Source code point.

Returns:

- The written code point on success.

#### `xer::puts`

Writes UTF-8 text and a trailing newline to the standard output text stream.

Signature 1:

```cpp
result< std::size_t > puts(std::u8string_view text)
```
Writes UTF-8 text and a trailing newline to the standard output text stream.

Parameters:

- `text`: Source UTF-8 text.

Returns:

- Number of UTF-8 code units requested for output on success.

#### `xer::scanf`

Scans values from standard input.

Signature 1:

```cpp
result< std::size_t > scanf(std::u8string_view format, Args *... args)
```
Scans values from standard input.

Parameters:

- `format`: Source UTF-8 format string.
- `args`: Output target pointers.

Returns:

- Assignment count on success.

#### `xer::sprintf`

Writes formatted text to a UTF-8 string.

Signature 1:

```cpp
result< std::size_t > sprintf(std::u8string &out, std::u8string_view format, Args &&... args)
```
Writes formatted text to a UTF-8 string.

Parameters:

- `out`: Destination string.
- `format`: UTF-8 format string.
- `args`: Format arguments.

Returns:

- Written byte count on success.

#### `xer::sscanf`

Scans values from a UTF-8 string.

Signature 1:

```cpp
result< std::size_t > sscanf(std::u8string_view input, std::u8string_view format, Args *... args)
```
Scans values from a UTF-8 string.

Parameters:

- `input`: Source UTF-8 input string.
- `format`: Source UTF-8 format string.
- `args`: Output target pointers.

Returns:

- Assignment count on success.

#### `xer::stropen`

Opens a writable text string stream.

Signature 1:

```cpp
result< text_stream > stropen(std::u8string &str, const char *mode) noexcept
```
Opens a writable text string stream.

Parameters:

- `str`: Target string.
- `mode`: Open mode string.

Returns:

- Opened text stream on success.
- Unexpected error on failure.

Signature 2:

```cpp
result< text_stream > stropen(std::u8string_view str, const char *mode) noexcept
```
Opens a read-only text string stream.

Parameters:

- `str`: Source string view.
- `mode`: Open mode string.

Returns:

- Opened text stream on success.
- Unexpected error on failure.

#### `xer::swap`

Swaps two binary streams.

Signature 1:

```cpp
void swap(binary_stream &lhs, binary_stream &rhs) noexcept
```
Swaps two binary streams.

Parameters:

- `lhs`: Left stream.
- `rhs`: Right stream.

Signature 2:

```cpp
void swap(text_stream &lhs, text_stream &rhs) noexcept
```
Swaps two text streams.

Parameters:

- `lhs`: Left stream.
- `rhs`: Right stream.

#### `xer::tmpfile`

Opens a temporary binary file stream.

Signature 1:

```cpp
result< binary_stream > tmpfile() noexcept
```
Opens a temporary binary file stream.

Returns:

- Opened binary stream on success.
- Unexpected error on failure.

Signature 2:

```cpp
result< text_stream > tmpfile(encoding_t encoding) noexcept
```
Opens a temporary text file stream.

Parameters:

- `encoding`: Text encoding selector.

Returns:

- Opened text stream on success.
- Unexpected error on failure.

#### `xer::to_native_handle`

Returns the native FILE handle of a binary stream.

Signature 1:

```cpp
result< std::FILE * > to_native_handle(binary_stream &stream) noexcept
```
Returns the native FILE handle of a binary stream.

Parameters:

- `stream`: Target stream.

Returns:

- Native FILE handle on success.
- Unexpected error on failure.

Signature 2:

```cpp
result< std::FILE * > to_native_handle(text_stream &stream) noexcept
```
Returns the native FILE handle of a text stream.

Parameters:

- `stream`: Target stream.

Returns:

- Native FILE handle on success.
- Unexpected error on failure.

Notes:

- If the returned FILE object is manipulated directly, especially by reading, writing, or repositioning, the subsequent behavior of the text_stream may become unspecified because its internal buffering and encoding state can diverge from the underlying FILE state.

#### `xer::ungetc`

Pushes one character back to a text stream.

Signature 1:

```cpp
result< char32_t > ungetc(char32_t ch, text_stream &stream)
```
Pushes one character back to a text stream.

Parameters:

- `ch`: Character to push back.
- `stream`: Target stream.

Returns:

- The pushed-back character on success.

## `<xer/stdlib.h>`

### Macros

#### `XER_DEFINE_MBSTOTCS_FOR_TC()`

#### `XER_DEFINE_MBTOTC_FOR_TC()`

#### `XER_DEFINE_TCSTOMBS_FOR_TC()`

#### `XER_DEFINE_TCTOMB_FOR_TC()`

### Types

#### `xer::div_t`

Alias of `rem_quot< int >`.

#### `xer::i16div_t`

Alias of `rem_quot< xer::int16_t >`.

#### `xer::i32div_t`

Alias of `rem_quot< xer::int32_t >`.

#### `xer::i64div_t`

Alias of `rem_quot< xer::int64_t >`.

#### `xer::i8div_t`

Alias of `rem_quot< xer::int8_t >`.

#### `xer::ldiv_t`

Alias of `rem_quot< long >`.

#### `xer::lldiv_t`

Alias of `rem_quot< long long >`.

#### `xer::mbstate_t`

State object for restartable multibyte conversion.

Member functions:

- `empty`
  - Overview: Returns whether this state object currently holds no buffered bytes.
  - Signature 1:

    ```cpp
    bool empty() const noexcept
    ```
    Returns whether this state object currently holds no buffered bytes.

    Returns:
    - true if no incomplete sequence is buffered, otherwise false.

- `mbstate_t`
  - Overview: Constructs an unused state object.
  - Signature 1:

    ```cpp
    constexpr mbstate_t() noexcept
    ```
    Constructs an unused state object.

#### `xer::rand_context`

Holds the internal state of a pseudo-random number generator.

Member functions:

- `from_bytes`
  - Overview: Restores a context from a serialized byte representation.
  - Signature 1:

    ```cpp
    result< rand_context > from_bytes(const bytes_type &bytes) noexcept
    ```
    Restores a context from a serialized byte representation.

    Parameters:
    - `bytes`: Serialized byte representation.

    Returns:
    - Restored context on success, or an error on failure.

- `rand_context`
  - Overview: Constructs a context seeded from std::random_device.
  - Signature 1:

    ```cpp
    rand_context()
    ```
    Constructs a context seeded from std::random_device.

  - Signature 2:

    ```cpp
    rand_context(std::uint64_t seed_value) noexcept
    ```
    Constructs a context from an explicit seed value.

    Parameters:
    - `seed_value`: Seed value.

- `to_bytes`
  - Overview: Serializes the current state into a byte array.
  - Signature 1:

    ```cpp
    bytes_type to_bytes() const noexcept
    ```
    Serializes the current state into a byte array.

    Returns:
    - Serialized byte representation.

#### `xer::rem_quot`

Quotient and remainder pair.

#### `xer::u16div_t`

Alias of `rem_quot< xer::uint16_t >`.

#### `xer::u32div_t`

Alias of `rem_quot< xer::uint32_t >`.

#### `xer::u64div_t`

Alias of `rem_quot< xer::uint64_t >`.

#### `xer::u8div_t`

Alias of `rem_quot< xer::uint8_t >`.

### Functions

#### `xer::abs`

Returns the absolute value of an expected int.

Signature 1:

```cpp
result< int > abs(const std::expected< int, E > &value) noexcept
```
Returns the absolute value of an expected int.

Parameters:

- `value`: Source expected object.

Returns:

- Absolute value on success, or normalized error on failure.

Signature 2:

```cpp
result< int > abs(int value) noexcept
```
Returns the absolute value of an int.

Parameters:

- `value`: Source value.

Returns:

- Absolute value on success, or overflow_error on failure.

Signature 3:

```cpp
result< long > abs(const std::expected< long, E > &value) noexcept
```
Returns the absolute value of an expected long.

Parameters:

- `value`: Source expected object.

Returns:

- Absolute value on success, or normalized error on failure.

Signature 4:

```cpp
result< long > abs(long value) noexcept
```
Returns the absolute value of a long.

Parameters:

- `value`: Source value.

Returns:

- Absolute value on success, or overflow_error on failure.

Signature 5:

```cpp
result< long long > abs(const std::expected< long long, E > &value) noexcept
```
Returns the absolute value of an expected long long.

Parameters:

- `value`: Source expected object.

Returns:

- Absolute value on success, or normalized error on failure.

Signature 6:

```cpp
result< long long > abs(long long value) noexcept
```
Returns the absolute value of a long long.

Parameters:

- `value`: Source value.

Returns:

- Absolute value on success, or overflow_error on failure.

#### `xer::advanced::packed_cp932_to_utf32`

Converts packed CP932 to a UTF-32 code point.

Signature 1:

```cpp
char32_t packed_cp932_to_utf32(std::uint16_t packed)
```
Converts packed CP932 to a UTF-32 code point.

Parameters:

- `packed`: Packed CP932.

Returns:

- UTF-32 code point, or static_cast<char32_t>(-1) on failure.

#### `xer::advanced::packed_utf16_to_utf32`

Converts packed UTF-16 to a UTF-32 code point.

Signature 1:

```cpp
char32_t packed_utf16_to_utf32(std::uint32_t packed)
```
Converts packed UTF-16 to a UTF-32 code point.

Parameters:

- `packed`: Packed UTF-16.

Returns:

- UTF-32 code point, or static_cast<char32_t>(-1) on failure.

#### `xer::advanced::packed_utf8_to_utf32`

Converts packed UTF-8 to a UTF-32 code point.

Signature 1:

```cpp
char32_t packed_utf8_to_utf32(std::uint32_t packed)
```
Converts packed UTF-8 to a UTF-32 code point.

Parameters:

- `packed`: Packed UTF-8.

Returns:

- UTF-32 code point, or static_cast<char32_t>(-1) on failure.

#### `xer::advanced::utf32_to_packed_cp932`

Converts a UTF-32 code point to packed CP932.

Signature 1:

```cpp
std::int32_t utf32_to_packed_cp932(char32_t code_point)
```
Converts a UTF-32 code point to packed CP932.

Parameters:

- `code_point`: UTF-32 code point.

Returns:

- Packed CP932, or -1 on failure.

#### `xer::advanced::utf32_to_packed_utf16`

Converts a UTF-32 code point to packed UTF-16.

Signature 1:

```cpp
std::uint32_t utf32_to_packed_utf16(char32_t code_point)
```
Converts a UTF-32 code point to packed UTF-16.

Parameters:

- `code_point`: UTF-32 code point.

Returns:

- Packed UTF-16, or static_cast<std::uint32_t>(-1) on failure.

#### `xer::advanced::utf32_to_packed_utf8`

Converts a UTF-32 code point to packed UTF-8.

Signature 1:

```cpp
std::uint32_t utf32_to_packed_utf8(char32_t code_point)
```
Converts a UTF-32 code point to packed UTF-8.

Parameters:

- `code_point`: UTF-32 code point.

Returns:

- Packed UTF-8, or static_cast<std::uint32_t>(-1) on failure.

#### `xer::ato`

Signature 1:

```cpp
result< T > ato(char8_t *str)
```
Signature 2:

```cpp
result< T > ato(const char8_t *str)
```
Signature 3:

```cpp
result< T > ato(const std::u8string &str)
```
Signature 4:

```cpp
result< T > ato(std::u8string_view str)
```
#### `xer::atoi`

Signature 1:

```cpp
result< int > atoi(char8_t *str)
```
Signature 2:

```cpp
result< int > atoi(const char8_t *str)
```
Signature 3:

```cpp
result< int > atoi(const std::u8string &str)
```
Signature 4:

```cpp
result< int > atoi(std::u8string_view str)
```
#### `xer::atol`

Signature 1:

```cpp
result< long > atol(char8_t *str)
```
Signature 2:

```cpp
result< long > atol(const char8_t *str)
```
Signature 3:

```cpp
result< long > atol(const std::u8string &str)
```
Signature 4:

```cpp
result< long > atol(std::u8string_view str)
```
#### `xer::atoll`

Signature 1:

```cpp
result< long long > atoll(char8_t *str)
```
Signature 2:

```cpp
result< long long > atoll(const char8_t *str)
```
Signature 3:

```cpp
result< long long > atoll(const std::u8string &str)
```
Signature 4:

```cpp
result< long long > atoll(std::u8string_view str)
```
#### `xer::bsearch`

Searches a sorted contiguous sequence.

Signature 1:

```cpp
result< T * > bsearch(const T *key, T *base, std::size_t count, Compare comp)
```
Searches a sorted contiguous sequence.

Parameters:

- `key`: Pointer to the search key.
- `base`: Base pointer.
- `count`: Element count.
- `comp`: Comparator.

Returns:

- Found pointer on success, or not_found / invalid_argument on failure.

Signature 2:

```cpp
result< T * > bsearch(const T *key, T(&base)[N], Compare comp)
```
Searches a sorted built-in array.

Parameters:

- `key`: Pointer to the search key.
- `base`: Target array.
- `comp`: Comparator.

Returns:

- Found pointer on success, or not_found / invalid_argument on failure.

Signature 3:

```cpp
result< const T * > bsearch(const T *key, const T *base, std::size_t count, Compare comp)
```
Searches a sorted const contiguous sequence.

Parameters:

- `key`: Pointer to the search key.
- `base`: Base pointer.
- `count`: Element count.
- `comp`: Comparator.

Returns:

- Found pointer on success, or not_found / invalid_argument on failure.

Signature 4:

```cpp
result< const T * > bsearch(const T *key, const T(&base)[N], Compare comp)
```
Searches a sorted const built-in array.

Parameters:

- `key`: Pointer to the search key.
- `base`: Target array.
- `comp`: Comparator.

Returns:

- Found pointer on success, or not_found / invalid_argument on failure.

Signature 5:

```cpp
result< std::ranges::iterator_t< Range > > bsearch(const std::ranges::range_value_t< Range > *key, Range &base, Compare comp)
```
Searches a sorted mutable random-access range.

Parameters:

- `key`: Pointer to the search key.
- `base`: Target range.
- `comp`: Comparator.

Returns:

- Found iterator on success, or not_found / invalid_argument on failure.

Signature 6:

```cpp
result< std::ranges::iterator_t< const Range > > bsearch(const std::ranges::range_value_t< Range > *key, const Range &base, Compare comp)
```
Searches a sorted const random-access range.

Parameters:

- `key`: Pointer to the search key.
- `base`: Target range.
- `comp`: Comparator.

Returns:

- Found iterator on success, or not_found / invalid_argument on failure.

#### `xer::div`

Signature 1:

```cpp
decltype(div(lhs, std::declval< U >())) div(T lhs, const result< U > &rhs) noexcept
```
Signature 2:

```cpp
decltype(div(static_cast< detail::forwarded_div_integer_t< A > >(lhs), static_cast< detail::forwarded_div_integer_t< B > >(rhs))) div(A lhs, B rhs) noexcept
```
Signature 3:

```cpp
decltype(div(std::declval< T >(), rhs)) div(const result< T > &lhs, U rhs) noexcept
```
Signature 4:

```cpp
decltype(div(std::declval< T >(), std::declval< U >())) div(const result< T > &lhs, const result< U > &rhs) noexcept
```
#### `xer::getenv`

Gets the value of an environment variable.

Signature 1:

```cpp
result< std::u8string > getenv(std::u8string_view name)
```
Gets the value of an environment variable.

Parameters:

- `name`: Environment variable name.

Returns:

- Environment variable value in UTF-8 on success.
- error<void> on failure.

#### `xer::mblen`

Returns the length in bytes of the first multibyte character.

Signature 1:

```cpp
result< std::size_t > mblen(const char *s, std::size_t n)
```
Returns the length in bytes of the first multibyte character.

Parameters:

- `s`: Input string.
- `n`: Maximum number of input bytes.

Returns:

- Consumed byte count or error.

Signature 2:

```cpp
result< std::size_t > mblen(const char *s, std::size_t n, mbstate_t *ps)
```
Returns the length in bytes of the first multibyte character.

Parameters:

- `s`: Input string.
- `n`: Maximum number of input bytes.
- `ps`: State object.

Returns:

- Consumed byte count or error.

Signature 3:

```cpp
result< std::size_t > mblen(const char8_t *s, std::size_t n)
```
Returns the length in bytes of the first multibyte character.

Parameters:

- `s`: Input string.
- `n`: Maximum number of input bytes.

Returns:

- Consumed byte count or error.

Signature 4:

```cpp
result< std::size_t > mblen(const char8_t *s, std::size_t n, mbstate_t *ps)
```
Returns the length in bytes of the first multibyte character.

Parameters:

- `s`: Input string.
- `n`: Maximum number of input bytes.
- `ps`: State object.

Returns:

- Consumed byte count or error.

Signature 5:

```cpp
result< std::size_t > mblen(const unsigned char *s, std::size_t n)
```
Returns the length in bytes of the first multibyte character.

Parameters:

- `s`: Input string.
- `n`: Maximum number of input bytes.

Returns:

- Consumed byte count or error.

Signature 6:

```cpp
result< std::size_t > mblen(const unsigned char *s, std::size_t n, mbstate_t *ps)
```
Returns the length in bytes of the first multibyte character.

Parameters:

- `s`: Input string.
- `n`: Maximum number of input bytes.
- `ps`: State object.

Returns:

- Consumed byte count or error.

#### `xer::qsort`

Sorts a mutable random-access range in place.

Signature 1:

```cpp
void qsort(Range &base, Compare comp)
```
Sorts a mutable random-access range in place.

Parameters:

- `base`: Target range.
- `comp`: Comparator.

Signature 2:

```cpp
void qsort(T *base, std::size_t count, Compare comp)
```
Sorts a contiguous sequence in place.

Parameters:

- `base`: Base pointer.
- `count`: Element count.
- `comp`: Comparator.

Signature 3:

```cpp
void qsort(T(&base)[N], Compare comp)
```
Sorts a built-in array in place.

Parameters:

- `base`: Target array.
- `comp`: Comparator.

#### `xer::rand`

Generates one random value from the default context.

Signature 1:

```cpp
std::uint64_t rand()
```
Generates one random value from the default context.

Returns:

- Generated random value.

Signature 2:

```cpp
std::uint64_t rand(rand_context &context)
```
Generates one random value from the specified context.

Parameters:

- `context`: Random generator context.

Returns:

- Generated random value.

#### `xer::srand`

Reseeds the default context with an explicit seed value.

Signature 1:

```cpp
void srand(std::uint64_t seed_value)
```
Reseeds the default context with an explicit seed value.

Parameters:

- `seed_value`: Seed value.

#### `xer::strto`

Signature 1:

```cpp
result< T > strto(char8_t *str, char8_t **endptr=nullptr)
```
Signature 2:

```cpp
result< T > strto(char8_t *str, char8_t **endptr=nullptr, int base=0)
```
Signature 3:

```cpp
result< T > strto(const char8_t *str, const char8_t **endptr=nullptr)
```
Signature 4:

```cpp
result< T > strto(const char8_t *str, const char8_t **endptr=nullptr, int base=0)
```
Signature 5:

```cpp
result< T > strto(const std::u8string &str, std::u8string::const_iterator *endit=nullptr)
```
Signature 6:

```cpp
result< T > strto(const std::u8string &str, std::u8string::const_iterator *endit=nullptr, int base=0)
```
Signature 7:

```cpp
result< T > strto(std::u8string &str, std::u8string::iterator *endit=nullptr)
```
Signature 8:

```cpp
result< T > strto(std::u8string &str, std::u8string::iterator *endit=nullptr, int base=0)
```
Signature 9:

```cpp
result< T > strto(std::u8string_view str, std::u8string_view::const_iterator *endit=nullptr)
```
Signature 10:

```cpp
result< T > strto(std::u8string_view str, std::u8string_view::const_iterator *endit=nullptr, int base=0)
```
#### `xer::strtod`

Signature 1:

```cpp
result< double > strtod(char8_t *str, char8_t **endptr=nullptr)
```
Signature 2:

```cpp
result< double > strtod(const char8_t *str, const char8_t **endptr=nullptr)
```
Signature 3:

```cpp
result< double > strtod(const std::u8string &str, std::u8string::const_iterator *endit=nullptr)
```
Signature 4:

```cpp
result< double > strtod(std::u8string &str, std::u8string::iterator *endit=nullptr)
```
Signature 5:

```cpp
result< double > strtod(std::u8string_view str, std::u8string_view::const_iterator *endit=nullptr)
```
#### `xer::strtof`

Signature 1:

```cpp
result< float > strtof(char8_t *str, char8_t **endptr=nullptr)
```
Signature 2:

```cpp
result< float > strtof(const char8_t *str, const char8_t **endptr=nullptr)
```
Signature 3:

```cpp
result< float > strtof(const std::u8string &str, std::u8string::const_iterator *endit=nullptr)
```
Signature 4:

```cpp
result< float > strtof(std::u8string &str, std::u8string::iterator *endit=nullptr)
```
Signature 5:

```cpp
result< float > strtof(std::u8string_view str, std::u8string_view::const_iterator *endit=nullptr)
```
#### `xer::strtof32`

Signature 1:

```cpp
result< float > strtof32(std::u8string_view str, std::u8string_view::const_iterator *endit=nullptr)
```
#### `xer::strtof64`

Signature 1:

```cpp
result< double > strtof64(std::u8string_view str, std::u8string_view::const_iterator *endit=nullptr)
```
#### `xer::strtol`

Signature 1:

```cpp
result< long > strtol(char8_t *str, char8_t **endptr=nullptr, int base=0)
```
Signature 2:

```cpp
result< long > strtol(const char8_t *str, const char8_t **endptr=nullptr, int base=0)
```
Signature 3:

```cpp
result< long > strtol(const std::u8string &str, std::u8string::const_iterator *endit=nullptr, int base=0)
```
Signature 4:

```cpp
result< long > strtol(std::u8string &str, std::u8string::iterator *endit=nullptr, int base=0)
```
Signature 5:

```cpp
result< long > strtol(std::u8string_view str, std::u8string_view::const_iterator *endit=nullptr, int base=0)
```
#### `xer::strtold`

Signature 1:

```cpp
result< long double > strtold(char8_t *str, char8_t **endptr=nullptr)
```
Signature 2:

```cpp
result< long double > strtold(const char8_t *str, const char8_t **endptr=nullptr)
```
Signature 3:

```cpp
result< long double > strtold(const std::u8string &str, std::u8string::const_iterator *endit=nullptr)
```
Signature 4:

```cpp
result< long double > strtold(std::u8string &str, std::u8string::iterator *endit=nullptr)
```
Signature 5:

```cpp
result< long double > strtold(std::u8string_view str, std::u8string_view::const_iterator *endit=nullptr)
```
#### `xer::strtoll`

Signature 1:

```cpp
result< long > strtoll(char8_t *str, char8_t **endptr=nullptr, int base=0)
```
Signature 2:

```cpp
result< long > strtoll(const char8_t *str, const char8_t **endptr=nullptr, int base=0)
```
Signature 3:

```cpp
result< long > strtoll(const std::u8string &str, std::u8string::const_iterator *endit=nullptr, int base=0)
```
Signature 4:

```cpp
result< long > strtoll(std::u8string &str, std::u8string::iterator *endit=nullptr, int base=0)
```
Signature 5:

```cpp
result< long > strtoll(std::u8string_view str, std::u8string_view::const_iterator *endit=nullptr, int base=0)
```
#### `xer::strtoul`

Signature 1:

```cpp
result< long > strtoul(char8_t *str, char8_t **endptr=nullptr, int base=0)
```
Signature 2:

```cpp
result< long > strtoul(const char8_t *str, const char8_t **endptr=nullptr, int base=0)
```
Signature 3:

```cpp
result< long > strtoul(const std::u8string &str, std::u8string::const_iterator *endit=nullptr, int base=0)
```
Signature 4:

```cpp
result< long > strtoul(std::u8string &str, std::u8string::iterator *endit=nullptr, int base=0)
```
Signature 5:

```cpp
result< long > strtoul(std::u8string_view str, std::u8string_view::const_iterator *endit=nullptr, int base=0)
```
#### `xer::strtoull`

Signature 1:

```cpp
result< long > strtoull(char8_t *str, char8_t **endptr=nullptr, int base=0)
```
Signature 2:

```cpp
result< long > strtoull(const char8_t *str, const char8_t **endptr=nullptr, int base=0)
```
Signature 3:

```cpp
result< long > strtoull(const std::u8string &str, std::u8string::const_iterator *endit=nullptr, int base=0)
```
Signature 4:

```cpp
result< long > strtoull(std::u8string &str, std::u8string::iterator *endit=nullptr, int base=0)
```
Signature 5:

```cpp
result< long > strtoull(std::u8string_view str, std::u8string_view::const_iterator *endit=nullptr, int base=0)
```
#### `xer::uabs`

Returns the unsigned absolute value of an expected int.

Signature 1:

```cpp
result< unsigned int > uabs(const std::expected< int, E > &value) noexcept
```
Returns the unsigned absolute value of an expected int.

Parameters:

- `value`: Source expected object.

Returns:

- Unsigned absolute value on success, or normalized error on failure.

Signature 2:

```cpp
result< unsigned int > uabs(const std::expected< unsigned int, E > &value) noexcept
```
Returns the unsigned absolute value of an expected unsigned int.

Parameters:

- `value`: Source expected object.

Returns:

- Same value on success, or normalized error on failure.

Signature 3:

```cpp
result< unsigned int > uabs(int value) noexcept
```
Returns the unsigned absolute value of an int.

Parameters:

- `value`: Source value.

Returns:

- Unsigned absolute value.

Signature 4:

```cpp
result< unsigned int > uabs(unsigned int value) noexcept
```
Returns the unsigned absolute value of an unsigned int.

Parameters:

- `value`: Source value.

Returns:

- Same value.

Signature 5:

```cpp
result< unsigned long > uabs(const std::expected< long, E > &value) noexcept
```
Returns the unsigned absolute value of an expected long.

Parameters:

- `value`: Source expected object.

Returns:

- Unsigned absolute value on success, or normalized error on failure.

Signature 6:

```cpp
result< unsigned long > uabs(const std::expected< unsigned long, E > &value) noexcept
```
Returns the unsigned absolute value of an expected unsigned long.

Parameters:

- `value`: Source expected object.

Returns:

- Same value on success, or normalized error on failure.

Signature 7:

```cpp
result< unsigned long > uabs(long value) noexcept
```
Returns the unsigned absolute value of a long.

Parameters:

- `value`: Source value.

Returns:

- Unsigned absolute value.

Signature 8:

```cpp
result< unsigned long > uabs(unsigned long value) noexcept
```
Returns the unsigned absolute value of an unsigned long.

Parameters:

- `value`: Source value.

Returns:

- Same value.

Signature 9:

```cpp
result< unsigned long long > uabs(const std::expected< long long, E > &value) noexcept
```
Returns the unsigned absolute value of an expected long long.

Parameters:

- `value`: Source expected object.

Returns:

- Unsigned absolute value on success, or normalized error on failure.

Signature 10:

```cpp
result< unsigned long long > uabs(const std::expected< unsigned long long, E > &value) noexcept
```
Returns the unsigned absolute value of an expected unsigned long long.

Parameters:

- `value`: Source expected object.

Returns:

- Same value on success, or normalized error on failure.

Signature 11:

```cpp
result< unsigned long long > uabs(long long value) noexcept
```
Returns the unsigned absolute value of a long long.

Parameters:

- `value`: Source value.

Returns:

- Unsigned absolute value.

Signature 12:

```cpp
result< unsigned long long > uabs(unsigned long long value) noexcept
```
Returns the unsigned absolute value of an unsigned long long.

Parameters:

- `value`: Source value.

Returns:

- Same value.

#### `xer::udiv`

Signature 1:

```cpp
decltype(udiv(lhs, std::declval< U >())) udiv(T lhs, const result< U > &rhs) noexcept
```
Signature 2:

```cpp
decltype(udiv(static_cast< detail::forwarded_div_integer_t< A > >(lhs), static_cast< detail::forwarded_div_integer_t< B > >(rhs))) udiv(A lhs, B rhs) noexcept
```
Signature 3:

```cpp
decltype(udiv(std::declval< T >(), rhs)) udiv(const result< T > &lhs, U rhs) noexcept
```
Signature 4:

```cpp
decltype(udiv(std::declval< T >(), std::declval< U >())) udiv(const result< T > &lhs, const result< U > &rhs) noexcept
```

## `<xer/string.h>`

### Functions

#### `xer::advanced::packed_cp932_to_utf32`

Converts packed CP932 to a UTF-32 code point.

Signature 1:

```cpp
char32_t packed_cp932_to_utf32(std::uint16_t packed)
```
Converts packed CP932 to a UTF-32 code point.

Parameters:

- `packed`: Packed CP932.

Returns:

- UTF-32 code point, or static_cast<char32_t>(-1) on failure.

#### `xer::advanced::packed_utf16_to_utf32`

Converts packed UTF-16 to a UTF-32 code point.

Signature 1:

```cpp
char32_t packed_utf16_to_utf32(std::uint32_t packed)
```
Converts packed UTF-16 to a UTF-32 code point.

Parameters:

- `packed`: Packed UTF-16.

Returns:

- UTF-32 code point, or static_cast<char32_t>(-1) on failure.

#### `xer::advanced::packed_utf8_to_utf32`

Converts packed UTF-8 to a UTF-32 code point.

Signature 1:

```cpp
char32_t packed_utf8_to_utf32(std::uint32_t packed)
```
Converts packed UTF-8 to a UTF-32 code point.

Parameters:

- `packed`: Packed UTF-8.

Returns:

- UTF-32 code point, or static_cast<char32_t>(-1) on failure.

#### `xer::advanced::utf32_to_packed_cp932`

Converts a UTF-32 code point to packed CP932.

Signature 1:

```cpp
std::int32_t utf32_to_packed_cp932(char32_t code_point)
```
Converts a UTF-32 code point to packed CP932.

Parameters:

- `code_point`: UTF-32 code point.

Returns:

- Packed CP932, or -1 on failure.

#### `xer::advanced::utf32_to_packed_utf16`

Converts a UTF-32 code point to packed UTF-16.

Signature 1:

```cpp
std::uint32_t utf32_to_packed_utf16(char32_t code_point)
```
Converts a UTF-32 code point to packed UTF-16.

Parameters:

- `code_point`: UTF-32 code point.

Returns:

- Packed UTF-16, or static_cast<std::uint32_t>(-1) on failure.

#### `xer::advanced::utf32_to_packed_utf8`

Converts a UTF-32 code point to packed UTF-8.

Signature 1:

```cpp
std::uint32_t utf32_to_packed_utf8(char32_t code_point)
```
Converts a UTF-32 code point to packed UTF-8.

Parameters:

- `code_point`: UTF-32 code point.

Returns:

- Packed UTF-8, or static_cast<std::uint32_t>(-1) on failure.

#### `xer::explode`

Splits a UTF-8 string by a separator and returns owning strings.

Signature 1:

```cpp
result< std::vector< std::u8string > > explode(const std::u8string_view separator, const std::u8string_view source, const int limit=INT_MAX)
```
Splits a UTF-8 string by a separator and returns owning strings.

Parameters:

- `separator`: Separator string. Must not be empty.
- `source`: Source string to split.
- `limit`: Maximum number of elements to return.

Returns:

- Vector of owning strings on success.

#### `xer::explode_view`

Splits a UTF-8 string by a separator and returns string views.

Signature 1:

```cpp
result< std::vector< std::u8string_view > > explode_view(const std::u8string_view separator, const std::u8string_view source, const int limit=INT_MAX)
```
Splits a UTF-8 string by a separator and returns string views.

Parameters:

- `separator`: Separator string. Must not be empty.
- `source`: Source string to split.
- `limit`: Maximum number of elements to return.

Returns:

- Vector of string views on success.

#### `xer::get_errno_name`

Returns the errno macro name for an errno-compatible error code.

Signature 1:

```cpp
result< std::u8string_view > get_errno_name(error_t code) noexcept
```
Returns the errno macro name for an errno-compatible error code.

Parameters:

- `code`: Error code.

Returns:

- errno macro name on success.

#### `xer::get_error_name`

Returns the xer::error_t enumerator name for an error code.

Signature 1:

```cpp
result< std::u8string_view > get_error_name(error_t code) noexcept
```
Returns the xer::error_t enumerator name for an error code.

Parameters:

- `code`: Error code.

Returns:

- Enumerator name on success.

#### `xer::implode`

Joins UTF-8 string pieces with a separator.

Signature 1:

```cpp
result< std::u8string > implode(const std::u8string_view separator, Range &&pieces)
```
Joins UTF-8 string pieces with a separator.

Parameters:

- `separator`: Separator string inserted between pieces.
- `pieces`: Input pieces.

Returns:

- Concatenated string on success.

#### `xer::memchr`

Searches for the specified byte in an immutable buffer.

Signature 1:

```cpp
result< const std::byte * > memchr(const std::byte *source, const std::size_t source_size, const std::byte value) noexcept
```
Searches for the specified byte in an immutable buffer.

Parameters:

- `source`: Source buffer.
- `source_size`: Size of the source buffer.
- `value`: Byte value to search for.

Returns:

- Pointer to the matched byte on success.

Signature 2:

```cpp
result< std::byte * > memchr(std::byte *source, const std::size_t source_size, const std::byte value) noexcept
```
Searches for the specified byte in a mutable buffer.

Parameters:

- `source`: Source buffer.
- `source_size`: Size of the source buffer.
- `value`: Byte value to search for.

Returns:

- Pointer to the matched byte on success.

Signature 3:

```cpp
result< std::ranges::iterator_t< Range > > memchr(Range &source, const std::byte value) noexcept
```
Searches for the specified byte in a mutable contiguous range.

Parameters:

- `source`: Source range.
- `value`: Byte value to search for.

Returns:

- Iterator to the matched byte on success.

Signature 4:

```cpp
result< std::ranges::iterator_t< const Range > > memchr(const Range &source, const std::byte value) noexcept
```
Searches for the specified byte in an immutable contiguous range.

Parameters:

- `source`: Source range.
- `value`: Byte value to search for.

Returns:

- Iterator to the matched byte on success.

#### `xer::memcmp`

Compares two contiguous byte ranges lexicographically.

Signature 1:

```cpp
result< int > memcmp(const Left &lhs, const Right &rhs) noexcept
```
Compares two contiguous byte ranges lexicographically.

Parameters:

- `lhs`: Left-hand range.
- `rhs`: Right-hand range.

Returns:

- Comparison result.

Signature 2:

```cpp
result< int > memcmp(const std::byte *lhs, const std::size_t lhs_size, const std::byte *rhs, const std::size_t rhs_size) noexcept
```
Compares two byte buffers lexicographically.

Parameters:

- `lhs`: Left-hand buffer.
- `lhs_size`: Size of the left-hand buffer.
- `rhs`: Right-hand buffer.
- `rhs_size`: Size of the right-hand buffer.

Returns:

- Negative value if lhs < rhs.
- Positive value if lhs > rhs.
- Zero if lhs == rhs.

#### `xer::memcpy`

Copies bytes from source to destination.

Signature 1:

```cpp
result< std::byte * > memcpy(std::byte *destination, const std::size_t destination_size, const std::byte *source, const std::size_t source_size) noexcept
```
Copies bytes from source to destination.

Parameters:

- `destination`: Destination buffer.
- `destination_size`: Size of the destination buffer.
- `source`: Source buffer.
- `source_size`: Size of the source buffer.

Returns:

- Destination pointer on success.

Signature 2:

```cpp
result< std::ranges::iterator_t< Destination > > memcpy(Destination &destination, const Source &source) noexcept
```
Copies bytes from source to destination.

Parameters:

- `destination`: Destination range.
- `source`: Source range.

Returns:

- Iterator to the beginning of the destination range on success.

#### `xer::memmove`

Moves bytes from source to destination.

Signature 1:

```cpp
result< std::byte * > memmove(std::byte *destination, const std::size_t destination_size, const std::byte *source, const std::size_t source_size) noexcept
```
Moves bytes from source to destination.

Parameters:

- `destination`: Destination buffer.
- `destination_size`: Size of the destination buffer.
- `source`: Source buffer.
- `source_size`: Size of the source buffer.

Returns:

- Destination pointer on success.

Signature 2:

```cpp
result< std::ranges::iterator_t< Destination > > memmove(Destination &destination, const Source &source) noexcept
```
Moves bytes from source to destination.

Parameters:

- `destination`: Destination range.
- `source`: Source range.

Returns:

- Iterator to the beginning of the destination range on success.

#### `xer::memset`

Fills a mutable buffer with the specified byte value.

Signature 1:

```cpp
result< std::byte * > memset(std::byte *destination, const std::size_t destination_size, const std::byte value) noexcept
```
Fills a mutable buffer with the specified byte value.

Parameters:

- `destination`: Destination buffer.
- `destination_size`: Size of the destination buffer.
- `value`: Byte value to write.

Returns:

- Destination pointer on success.

Signature 2:

```cpp
result< std::ranges::iterator_t< Destination > > memset(Destination &destination, const std::byte value) noexcept
```
Fills a mutable contiguous range with the specified byte value.

Parameters:

- `destination`: Destination range.
- `value`: Byte value to write.

Returns:

- Iterator to the beginning of the destination range on success.

#### `xer::strcat`

Appends the source string to the destination buffer and writes a terminating NUL character.

Signature 1:

```cpp
result< CharT * > strcat(CharT *destination, const std::size_t destination_size, const std::basic_string_view< CharT > source) noexcept
```
Appends the source string to the destination buffer and writes a terminating NUL character.

Parameters:

- `destination`: Destination buffer.
- `destination_size`: Destination buffer size.
- `source`: Source string.

Returns:

- Destination pointer on success.

Signature 2:

```cpp
result< std::ranges::iterator_t< Destination > > strcat(Destination &destination, const std::basic_string_view< CharT > source) noexcept
```
Appends the source string to the destination range and writes a terminating NUL character.

Parameters:

- `destination`: Destination range.
- `source`: Source string.

Returns:

- Iterator to the beginning of the destination range on success.

#### `xer::strchr`

Searches for the first occurrence of a code point in a UTF-16 string.

Signature 1:

```cpp
result< std::u16string_view::const_iterator > strchr(const std::u16string_view source, const char32_t value)
```
Searches for the first occurrence of a code point in a UTF-16 string.

Parameters:

- `source`: Source UTF-16 string.
- `value`: Code point to search for.

Returns:

- Iterator pointing to the first code unit of the found code point.

Signature 2:

```cpp
result< std::u32string_view::const_iterator > strchr(const std::u32string_view source, const char32_t value)
```
Searches for the first occurrence of a code point in a UTF-32 string.

Parameters:

- `source`: Source UTF-32 string.
- `value`: Code point to search for.

Returns:

- Iterator pointing to the found code point.

Signature 3:

```cpp
result< std::u8string_view::const_iterator > strchr(const std::u8string_view source, const char32_t value)
```
Searches for the first occurrence of a code point in a UTF-8 string.

Parameters:

- `source`: Source UTF-8 string.
- `value`: Code point to search for.

Returns:

- Iterator pointing to the first code unit of the found code point.

Signature 4:

```cpp
result< typename std::basic_string_view< CharT >::const_iterator > strchr(const std::basic_string_view< CharT > source, const CharT value)
```
Searches for the first occurrence of a code unit.

Parameters:

- `source`: Source string.
- `value`: Code unit to search for.

Returns:

- Iterator pointing to the found code unit.

#### `xer::strcmp`

Compares two strings lexicographically.

Signature 1:

```cpp
result< int > strcmp(const std::basic_string_view< CharT > lhs, const std::basic_string_view< CharT > rhs)
```
Compares two strings lexicographically.

Parameters:

- `lhs`: Left-hand string.
- `rhs`: Right-hand string.

Returns:

- Negative value if lhs < rhs, zero if equal, positive value if lhs > rhs.

#### `xer::strcpy`

Copies the source string to the destination buffer including the terminating NUL character.

Signature 1:

```cpp
result< CharT * > strcpy(CharT *destination, const std::size_t destination_size, const std::basic_string_view< CharT > source) noexcept
```
Copies the source string to the destination buffer including the terminating NUL character.

Parameters:

- `destination`: Destination buffer.
- `destination_size`: Destination buffer size.
- `source`: Source string.

Returns:

- Destination pointer on success.

Signature 2:

```cpp
result< std::ranges::iterator_t< Destination > > strcpy(Destination &destination, const std::basic_string_view< CharT > source) noexcept
```
Copies the source string to the destination range including the terminating NUL character.

Parameters:

- `destination`: Destination range.
- `source`: Source string.

Returns:

- Iterator to the beginning of the destination range on success.

#### `xer::strcspn`

Returns the length of the maximum initial segment consisting only of rejected code units.

Signature 1:

```cpp
result< std::size_t > strcspn(const std::basic_string_view< CharT > source, const std::basic_string_view< CharT > reject)
```
Returns the length of the maximum initial segment consisting only of rejected code units.

Parameters:

- `source`: Source string.
- `reject`: Reject set.

Returns:

- Length of the initial rejected segment.

#### `xer::strerror`

Returns the English error message for an error code.

Signature 1:

```cpp
result< std::u8string_view > strerror(error_t code) noexcept
```
Returns the English error message for an error code.

Parameters:

- `code`: Error code.

Returns:

- English error message on success.

#### `xer::strlen`

Returns the length of a string view in code units.

Signature 1:

```cpp
result< std::size_t > strlen(const std::basic_string_view< CharT > source)
```
Returns the length of a string view in code units.

Parameters:

- `source`: Source string view.

Returns:

- Code-unit length of the source string.

#### `xer::strncat`

Appends at most count characters from the source string to the destination buffer and writes a terminating NUL character.

Signature 1:

```cpp
result< CharT * > strncat(CharT *destination, const std::size_t destination_size, const std::basic_string_view< CharT > source, const std::size_t count) noexcept
```
Appends at most count characters from the source string to the destination buffer and writes a terminating NUL character.

Parameters:

- `destination`: Destination buffer.
- `destination_size`: Destination buffer size.
- `source`: Source string.
- `count`: Maximum number of characters to append.

Returns:

- Destination pointer on success.

Signature 2:

```cpp
result< std::ranges::iterator_t< Destination > > strncat(Destination &destination, const std::basic_string_view< CharT > source, const std::size_t count) noexcept
```
Appends at most count characters from the source string to the destination range and writes a terminating NUL character.

Parameters:

- `destination`: Destination range.
- `source`: Source string.
- `count`: Maximum number of characters to append.

Returns:

- Iterator to the beginning of the destination range on success.

#### `xer::strncmp`

Compares up to the specified number of code units lexicographically.

Signature 1:

```cpp
result< int > strncmp(const std::basic_string_view< CharT > lhs, const std::basic_string_view< CharT > rhs, const std::size_t count)
```
Compares up to the specified number of code units lexicographically.

Parameters:

- `lhs`: Left-hand string.
- `rhs`: Right-hand string.
- `count`: Maximum number of code units to compare.

Returns:

- Negative value if lhs < rhs, zero if equal, positive value if lhs > rhs.

#### `xer::strncpy`

Copies at most count characters from the source string to the destination buffer.

Signature 1:

```cpp
result< CharT * > strncpy(CharT *destination, const std::size_t destination_size, const std::basic_string_view< CharT > source, const std::size_t count) noexcept
```
Copies at most count characters from the source string to the destination buffer.

Parameters:

- `destination`: Destination buffer.
- `destination_size`: Destination buffer size.
- `source`: Source string.
- `count`: Maximum number of characters to write.

Returns:

- Destination pointer on success.

Signature 2:

```cpp
result< std::ranges::iterator_t< Destination > > strncpy(Destination &destination, const std::basic_string_view< CharT > source, const std::size_t count) noexcept
```
Copies at most count characters from the source string to the destination range.

Parameters:

- `destination`: Destination range.
- `source`: Source string.
- `count`: Maximum number of characters to write.

Returns:

- Iterator to the beginning of the destination range on success.

#### `xer::strpbrk`

Searches for the first code unit that is contained in the accept set.

Signature 1:

```cpp
result< typename std::basic_string_view< CharT >::const_iterator > strpbrk(const std::basic_string_view< CharT > source, const std::basic_string_view< CharT > accept)
```
Searches for the first code unit that is contained in the accept set.

Parameters:

- `source`: Source string.
- `accept`: Accept set.

Returns:

- Iterator pointing to the found code unit.

#### `xer::strrchr`

Searches for the last occurrence of a code point in a UTF-16 string.

Signature 1:

```cpp
result< std::u16string_view::const_iterator > strrchr(const std::u16string_view source, const char32_t value)
```
Searches for the last occurrence of a code point in a UTF-16 string.

Parameters:

- `source`: Source UTF-16 string.
- `value`: Code point to search for.

Returns:

- Iterator pointing to the first code unit of the found code point.

Signature 2:

```cpp
result< std::u32string_view::const_iterator > strrchr(const std::u32string_view source, const char32_t value)
```
Searches for the last occurrence of a code point in a UTF-32 string.

Parameters:

- `source`: Source UTF-32 string.
- `value`: Code point to search for.

Returns:

- Iterator pointing to the found code point.

Signature 3:

```cpp
result< std::u8string_view::const_iterator > strrchr(const std::u8string_view source, const char32_t value)
```
Searches for the last occurrence of a code point in a UTF-8 string.

Parameters:

- `source`: Source UTF-8 string.
- `value`: Code point to search for.

Returns:

- Iterator pointing to the first code unit of the found code point.

Signature 4:

```cpp
result< typename std::basic_string_view< CharT >::const_iterator > strrchr(const std::basic_string_view< CharT > source, const CharT value)
```
Searches for the last occurrence of a code unit.

Parameters:

- `source`: Source string.
- `value`: Code unit to search for.

Returns:

- Iterator pointing to the found code unit.

#### `xer::strspn`

Returns the length of the maximum initial segment consisting only of accepted code units.

Signature 1:

```cpp
result< std::size_t > strspn(const std::basic_string_view< CharT > source, const std::basic_string_view< CharT > accept)
```
Returns the length of the maximum initial segment consisting only of accepted code units.

Parameters:

- `source`: Source string.
- `accept`: Accept set.

Returns:

- Length of the initial accepted segment.

#### `xer::strstr`

Searches for the first occurrence of a substring.

Signature 1:

```cpp
result< typename std::basic_string_view< CharT >::const_iterator > strstr(const std::basic_string_view< CharT > source, const std::basic_string_view< CharT > pattern)
```
Searches for the first occurrence of a substring.

Parameters:

- `source`: Source string.
- `pattern`: Pattern string.

Returns:

- Iterator pointing to the beginning of the found substring.

## `<xer/time.h>`

### Types

#### `xer::clock_t`

Alias of `std::clock_t`.

Processor time type.

#### `xer::time_t`

Alias of `double`.

Calendar time value measured in seconds since the Unix epoch.

#### `xer::tm`

Broken-down calendar time with microsecond precision.

Member functions:

- `tm`
  - Overview: Constructs a zero-initialized broken-down time.
  - Signature 1:

    ```cpp
    constexpr tm() noexcept
    ```
    Constructs a zero-initialized broken-down time.

### Functions

#### `xer::clock`

Gets the processor time consumed by the program.

Signature 1:

```cpp
result< clock_t > clock() noexcept
```
Gets the processor time consumed by the program.

Returns:

- The processor time on success.
- An error with error_t::runtime_error on failure.

#### `xer::ctime`

Formats a broken-down time as a UTF-8 string.

Signature 1:

```cpp
std::u8string ctime(const tm &value)
```
Formats a broken-down time as a UTF-8 string.

Parameters:

- `value`: Broken-down time.

Returns:

- Formatted UTF-8 string.

Signature 2:

```cpp
std::u8string ctime(time_t value)
```
Formats a calendar time as a UTF-8 string.

Parameters:

- `value`: Calendar time.

Returns:

- Formatted UTF-8 string. Returns an empty string on conversion failure.

#### `xer::difftime`

Computes the difference between two calendar times.

Signature 1:

```cpp
double difftime(time_t left, time_t right) noexcept
```
Computes the difference between two calendar times.

Parameters:

- `left`: Left operand.
- `right`: Right operand.

Returns:

- Difference in seconds.

#### `xer::gmtime`

Converts a calendar time to UTC broken-down time.

Signature 1:

```cpp
result< tm > gmtime(time_t value) noexcept
```
Converts a calendar time to UTC broken-down time.

Parameters:

- `value`: Calendar time in seconds since 1970-01-01 00:00:00 UTC.

Returns:

- UTC broken-down time on success.
- An error with error_t::invalid_argument if value is negative.
- An error with error_t::runtime_error on conversion failure.

#### `xer::localtime`

Converts a calendar time to local broken-down time.

Signature 1:

```cpp
result< tm > localtime(time_t value) noexcept
```
Converts a calendar time to local broken-down time.

Parameters:

- `value`: Calendar time in seconds since 1970-01-01 00:00:00 UTC.

Returns:

- Local broken-down time on success.
- An error with error_t::invalid_argument if value is negative.
- An error with error_t::runtime_error on conversion failure.

#### `xer::mktime`

Converts local broken-down time to a calendar time.

Signature 1:

```cpp
result< time_t > mktime(const tm &value) noexcept
```
Converts local broken-down time to a calendar time.

Parameters:

- `value`: Local broken-down time.

Returns:

- Calendar time on success.
- An error with error_t::invalid_argument if the input is invalid, represents a time before the Unix epoch, or has an out-of-range microsecond field.
- An error with error_t::runtime_error on conversion failure.

#### `xer::strftime`

Formats a broken-down time according to a UTF-8 format string.

Signature 1:

```cpp
result< std::u8string > strftime(std::u8string_view format, const tm &value) noexcept
```
Formats a broken-down time according to a UTF-8 format string.

Parameters:

- `format`: UTF-8 format string.
- `value`: Broken-down time.

Returns:

- Formatted UTF-8 string on success.
- An error with error_t::invalid_argument if the format string or tm_microsec is invalid.
- An error with error_t::runtime_error on formatting failure.

#### `xer::time`

Gets the current calendar time.

Signature 1:

```cpp
result< time_t > time() noexcept
```
Gets the current calendar time.

Returns:

- The current calendar time on success.
- An error with error_t::runtime_error on failure.

## `<xer/version.h>`

### Objects and constants

#### `xer::version_major`

Type: `int`

Major version number of XER.

#### `xer::version_minor`

Type: `int`

Minor version number of XER.

#### `xer::version_patch`

Type: `int`

Patch version number of XER.

#### `xer::version_string`

Type: `std::string_view`

Full version string of XER.

#### `xer::version_suffix`

Type: `std::string_view`

Version suffix string of XER.
