# XER scanf Format Specifiers

## Scope

This document describes the format strings used by the XER scanf family.

Target functions:

```cpp
scanf
fscanf
sscanf
```

The printf family is documented separately in `stdio_printf_format.md`.

---

## Basic Policy

XER scanf-style functions are inspired by C scanf, but they are not strict source-compatible reimplementations.

- format strings are UTF-8 strings
- input text is read as XER text and is processed as Unicode scalar values where appropriate
- ordinary fixed text in the format string must match the input
- ASCII whitespace in the format string matches zero or more ASCII whitespace characters in the input
- conversion specifications start with `%`
- ordinary failure is reported through `xer::result`
- match failure returns the number of successful assignments already completed
- XER-specific extensions may exist

A format string may contain ordinary UTF-8 text, whitespace, control tokens, and conversion specifications.

---

## Function Result

The scanf family returns the number of successful assignments.

```cpp
auto result = xer::sscanf(input, format, &a, &b);
```

On success, the returned value is the number of output arguments that were assigned.

If input matching fails after some assignments have already succeeded, the function returns the partial assignment count as a successful result.
This follows the general scanf-style model where an ordinary mismatch is not necessarily a format error.

If the format string is invalid, a type mismatch is detected, input decoding fails, or another ordinary runtime error occurs, the function returns failure through `xer::result`.

---

## Format String Structure

A scanf format string consists of the following kinds of items:

```text
ordinary UTF-8 literal text
ASCII whitespace
conversion specifications beginning with %
XER control tokens such as %@
```

Ordinary literal text must match the input exactly.

ASCII whitespace in the format string consumes zero or more ASCII whitespace characters in the input.
Consecutive whitespace in the format string is treated as a single whitespace-matching item.

---

## Conversion Specification Syntax

A conversion specification begins with `%`.

The currently supported structure is:

```text
%[position$][*][width][length]conversion
```

The positional form is optional.
When it is used, the first output argument is numbered `1`.

Examples:

```cpp
xer::sscanf(u8"10 abc", u8"%d %s", &value, &text);
xer::sscanf(u8"10 abc", u8"%2$s %1$d", &value, &text);
```

---

## Positional Arguments

A conversion may specify an output argument position:

```text
%1$d
%2$s
```

Argument positions are one-based.

When positional arguments are used, the format string is treated as positional.
Sequential and positional argument selection must not be mixed in the same format string, except through the XER `%@` control token rules described below.

Examples:

```cpp
int number = 0;
std::u8string text;

xer::sscanf(u8"hello 123", u8"%2$s %1$d", &number, &text);
```

Here `%2$s` stores into the second output argument and `%1$d` stores into the first output argument.

---

## Assignment Suppression

A conversion may suppress assignment by using `*` after `%` or after the optional positional prefix.

```text
%*d
%*s
```

The input is still matched and consumed, but no output argument is assigned and the assignment count is not incremented.

Example:

```cpp
int value = 0;
xer::sscanf(u8"10 20", u8"%*d %d", &value);
```

This stores `20` in `value`.

---

## Field Width

A field width may be specified as a positive decimal integer.

```text
%3s
%2d
%1c
```

The width limits the number of input characters considered by the conversion.

For `%s` and `%[...]`, the width limits the number of Unicode scalar values collected, not the number of UTF-8 code units.

A width of `0` is not accepted as an explicit field width.
When no field width is present, the conversion reads as much as its own matching rule allows.

---

## Length Modifiers

The following length modifiers are parsed:

```text
hh h l ll j z t L
```

They are accepted as part of the scanf-style grammar.
Their effect is applied to the intermediate value used by numeric conversions.
The actual output type is still determined by the pointer type passed by the caller.

For `%[...]`, length modifiers are currently invalid.

---

## Whitespace Handling Around Conversions

For most conversions, leading ASCII whitespace in the input is skipped before matching.

The following conversions skip leading ASCII whitespace:

```text
%d %u %o %x %X
%f %F %e %E %g %G
%s
```

The following conversions do not skip leading whitespace automatically:

```text
%c
%[...]
%%
```

This follows the usual scanf-style distinction: `%c` and scansets read the next input character according to their own matching rule.

---

## Supported Conversions

The following conversion specifiers are supported:

```text
%d
%u
%o
%x %X
%f %F
%e %E
%g %G
%c
%s
%[...]
%%
```

The `%@` token is also supported as an XER-specific control token.
It is described separately below.

---

## Integer Conversions

### `%d`

`%d` reads a signed decimal integer.

It accepts an optional leading sign followed by decimal digits.

### `%u`

`%u` reads an unsigned decimal integer.

### `%o`

`%o` reads an unsigned octal integer.

### `%x` and `%X`

`%x` and `%X` read an unsigned hexadecimal integer.

The implementation accepts hexadecimal digits using either lowercase or uppercase letters.

### Output Targets

Integer conversions can be stored into ordinary integer scalar targets when the target type is compatible with the intermediate value.

The implementation first parses into an intermediate integer value and then stores into the caller-provided output object.
If the destination pointer type is not compatible with the conversion result, the scan operation reports an error.

---

## Floating-Point Conversions

The following floating-point conversions are supported:

```text
%f %F
%e %E
%g %G
```

They read a floating-point lexeme and store the value into a floating-point target.

The accepted input form follows the implementation's current floating parser.
It includes ordinary decimal forms and exponent forms used by typical scanf-style input.

---

## Character Conversion: `%c`

`%c` reads one input character and stores it as a character-like value.

Unlike `%s`, `%c` does not skip leading whitespace automatically.

In the current implementation, `%c` accepts a field width only when the width is `1` or omitted.
A larger width is treated as invalid.

Typical output targets include:

```cpp
char32_t
char16_t
wchar_t
char8_t
char
signed char
unsigned char
```

The input is read as a Unicode scalar value and then stored into the destination character type.
When the destination is a single-byte character type, the caller is responsible for using it only for values that make sense for that type.

---

## String Conversion: `%s`

`%s` reads a non-empty sequence of non-whitespace characters.

Before matching, leading ASCII whitespace is skipped.
The conversion then collects characters until one of the following occurs:

- end of input
- ASCII whitespace
- field width is reached

The collected text is stored as UTF-8 internally and can be assigned to supported string targets.

Supported string targets include:

```cpp
std::u8string
std::u16string
std::u32string
std::wstring
```

The input text is UTF-8 in the XER text model.
When the destination is `std::u16string`, `std::u32string`, or `std::wstring`, the collected UTF-8 text is converted to the corresponding character-string representation.

For `std::wstring`, conversion follows the width of `wchar_t`:

- when `wchar_t` is effectively UTF-16, UTF-16 code units are produced
- when `wchar_t` is effectively UTF-32, UTF-32 code units are produced

---

## Scanset Conversion: `%[...]`

`%[...]` reads a non-empty sequence of characters that match a scanset.

Unlike `%s`, a scanset does not skip leading whitespace automatically.
The first input character must match the scanset for the conversion to succeed.

The collected text is stored as UTF-8 internally and can be assigned to the same string targets as `%s`:

```cpp
std::u8string
std::u16string
std::u32string
std::wstring
```

### Basic Form

```text
%[abc]
```

This matches one or more characters from the set `a`, `b`, and `c`.

### Negated Form

```text
%[^,]
```

A leading `^` negates the scanset.
This example reads characters until a comma is encountered.

### Including `]`

If `]` appears immediately after `[` or after `[^`, it is treated as a member of the scanset.

Examples:

```text
%[]x]
%[^]x]
```

### Ranges

ASCII ranges are supported.

```text
%[a-z]
%[0-9]
```

Ranges are interpreted over ASCII byte values.
For non-ASCII characters, each UTF-8 code point is handled as an individual scanset item rather than as part of a range.

---

## Literal Percent: `%%`

`%%` matches one literal percent sign in the input.

It does not assign to an output argument and does not increment the assignment count.

---

## XER Control Token: `%@`

`%@` is an XER-specific scanf control token.

It does not read input by itself.
Instead, it controls argument selection for the following conversion specification.

The main purpose is to make a following conversion use a specific output argument while keeping the conversion itself written in the ordinary form.

### Sequential Form

```text
%@ %d
```

In sequential mode, `%@` marks the following conversion as controlled by the current argument-selection flow.
This form is mainly useful as a building block for the same mechanism that also supports positional control.

### Positional Form

```text
%1$@ %d
```

The positional form applies the specified argument position to the following conversion.

Example:

```cpp
int a = 0;
int b = 0;

xer::sscanf(u8"10 20", u8"%2$d %1$@ %d", &a, &b);
```

The behavior is:

```text
%2$d   reads 10 into the second output argument
%1$@   selects the first output argument for the next conversion
%d     reads 20 into the first output argument
```

After the call:

```text
a == 20
b == 10
```

### Restrictions

A `%@` control token must be followed by a conversion specification.
A format string ending with pending `%@` is invalid.

Two consecutive control tokens are invalid.

When positional control is used, the format's argument-selection mode rules still apply.
Mixing incompatible sequential and positional forms is treated as an invalid format.

---

## Generic Stream-Extraction Storage

When a destination type is not one of the explicitly supported scalar, character, or string target categories, the implementation may store through a generic stream-extraction route.

Conceptually, the intermediate scanned value is first converted to UTF-8 text, then to a narrow byte string, and then read through:

```cpp
std::istringstream stream(text);
stream >> value;
```

This route is intended for types that naturally support `operator>>`.

Special string and wide-string targets such as `std::u16string`, `std::u32string`, and `std::wstring` are not handled through this generic route; they are handled explicitly from UTF-8 text.

---

## Assignment Count

The returned assignment count is incremented only when a conversion succeeds and actually assigns to a non-null output pointer.

The count is not incremented for:

- `%%`
- suppressed assignments such as `%*d`
- output arguments passed as `nullptr`
- control tokens such as `%@`

---

## Null Output Pointers

If an output pointer is `nullptr`, the conversion still reads and consumes input normally, but the value is discarded.

A successful conversion with a null output pointer does not increment the assignment count.

This allows callers to ignore selected values without changing the input-matching behavior.

---

## Match Failure vs Error

XER scanf-style functions distinguish ordinary match failure from errors.

### Match Failure

A match failure occurs when the input does not match the next literal or conversion.
In this case, the function returns the assignment count already completed.

Example:

```cpp
int a = 0;
int b = 0;

const auto result = xer::sscanf(u8"10 xx", u8"%d %d", &a, &b);
```

The first conversion succeeds, the second conversion fails to match, and the returned count is `1`.

### Error

An error is reported through `xer::result` failure.

Typical error cases include:

- invalid format syntax
- unsupported conversion syntax
- incompatible argument-selection mode
- invalid UTF-8 input where decoding is required
- type mismatch between a conversion and an output target
- invalid use of field width or length modifier

---

## Encoding Notes

XER scanf-style input works in XER's text model.

For `sscanf`, the input is a UTF-8 string.
For `fscanf` and `scanf`, the source is a `text_stream`, whose external encoding is handled by the stream layer and whose characters are read as XER text characters.

Collected string values are stored internally as UTF-8 before being assigned to the destination string type.

---

## Examples

### Basic scanning

```cpp
int value = 0;
std::u8string text;

const auto result = xer::sscanf(u8"123 hello", u8"%d %s", &value, &text);
```

After success:

```text
value == 123
text == u8"hello"
```

### Reading UTF-8 text into wide string targets

```cpp
std::u16string a;
std::u32string b;
std::wstring c;

xer::sscanf(u8"猫 犬 鳥", u8"%s %s %s", &a, &b, &c);
```

Each `%s` reads UTF-8 input and stores it in the destination string type.

### Reading a scanset

```cpp
std::u8string field;

xer::sscanf(u8"abc,rest", u8"%[^,]", &field);
```

This stores `u8"abc"` in `field`.

---

## Implementation Notes

This document is intended to describe the user-visible scanf-family behavior.
When implementation details in `xer/bits/scanf_format.h` or `xer/bits/scanf.h` change, this document should be kept in sync.
