# `<xer/base64.h>`

## Purpose

`<xer/base64.h>` provides Base64 encode and decode facilities in XER.

Base64 is treated as a small binary-to-text conversion facility. It is not a structured data format like JSON, INI, or TOML, and it is not ordinary string processing either. Its role is to convert binary byte sequences into UTF-8 text that can be embedded in text-based data and to convert that text representation back into bytes.

The initial implementation deliberately supports only the standard Base64 form. Variants such as URL-safe Base64 and unpadded Base64 are deferred.

---

## Main Role

The main role of `<xer/base64.h>` is to make it possible to:

- encode binary data into standard Base64 text
- decode standard Base64 text back into binary data
- handle invalid encoded text through XER's ordinary `xer::result` error model
- provide a compact public API that can be extended later without changing its basic shape

This makes the header useful for simple binary payload handling, text-based interchange, configuration data, diagnostics, and small utility programs.

---

## Main Entities

At minimum, `<xer/base64.h>` provides the following functions:

```cpp
auto base64_encode(std::span<const std::byte> data)
    -> xer::result<std::u8string>;

auto base64_decode(std::u8string_view text)
    -> xer::result<std::vector<std::byte>>;
```

The current API is intentionally small. Additional overloads or options may be added later.

---

## `base64_encode`

```cpp
auto base64_encode(std::span<const std::byte> data)
    -> xer::result<std::u8string>;
```

### Purpose

`base64_encode` converts a binary byte sequence into standard Base64 text.

### Input Model

The input is provided as:

```cpp
std::span<const std::byte>
```

This makes the function explicitly byte-oriented. The input is not interpreted as text and is not validated as UTF-8.

### Output Model

On success, the function returns:

```cpp
std::u8string
```

The output contains only ASCII Base64 characters and is therefore valid UTF-8.

### Return Model

The return type is `xer::result<std::u8string>`.

The current minimal encoder normally has no content-based failure condition. However, the function still returns `xer::result` so that future variants can report ordinary failures without changing the public API shape.

Possible future failure cases include output-size limits, formatting options, stream-oriented output failures, or invalid option combinations.

---

## `base64_decode`

```cpp
auto base64_decode(std::u8string_view text)
    -> xer::result<std::vector<std::byte>>;
```

### Purpose

`base64_decode` converts standard Base64 text back into binary data.

### Input Model

The input is provided as:

```cpp
std::u8string_view
```

Only ASCII Base64 characters, `=`, and ignored ASCII whitespace are meaningful. Non-ASCII input is rejected because it is not part of the supported Base64 alphabet.

### Output Model

On success, the function returns:

```cpp
std::vector<std::byte>
```

The output is binary data. It is not interpreted as text.

---

## Supported Base64 Form

The initial implementation supports standard Base64 with the following alphabet:

```text
ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/
```

Padding uses:

```text
=
```

### Encoding Behavior

`base64_encode` currently behaves as follows:

- emits standard Base64 text
- always emits padding when padding is needed
- does not insert line breaks
- does not insert spaces
- does not support URL-safe output
- does not support unpadded output

Examples:

```text
f      -> Zg==
fo     -> Zm8=
foo    -> Zm9v
foobar -> Zm9vYmFy
```

### Decoding Behavior

`base64_decode` currently behaves as follows:

- accepts the standard Base64 alphabet
- accepts `=` padding only in valid final positions
- ignores ASCII whitespace
- rejects non-Base64 characters
- rejects malformed padding
- rejects inputs whose effective length is not a multiple of four
- rejects non-canonical padding bits

ASCII whitespace means the following characters:

```text
space, tab, LF, CR, FF, VT
```

Whitespace is ignored only while decoding. The encoder does not generate whitespace.

---

## Error Handling

`<xer/base64.h>` follows XER's ordinary failure model.

That means:

- normal failure is reported through `xer::result`
- invalid encoded text is not handled by exceptions
- callers explicitly check whether the returned `xer::result` has a value

The initial decoder reports malformed Base64 input as:

```cpp
error_t::invalid_argument
```

This includes at least the following cases:

- invalid Base64 character
- invalid effective input length
- padding before the final quartet
- malformed padding pattern
- non-canonical unused padding bits

At this stage, detailed error positions are not reported. If position-aware diagnostics become useful later, the error detail type can be extended separately.

---

## Deferred Items and Limitations

The following items are intentionally deferred from the initial implementation.

### URL-Safe Base64

URL-safe Base64, which uses `-` and `_` instead of `+` and `/`, is not supported yet.

A future API may add an option or separate variant for URL-safe Base64.

### Unpadded Base64

Unpadded Base64 is not supported yet.

The current decoder requires the effective input length after whitespace removal to be a multiple of four. Therefore, input that relies on omitted `=` padding is rejected.

A future API may add an option to accept or emit unpadded Base64.

### MIME-Style Line Wrapping on Encode

The encoder does not insert line breaks.

The decoder ignores ASCII whitespace, so it can read many line-wrapped Base64 strings. However, line-wrapped output generation is not provided yet.

A future API may add a line-width option.

### Streaming Encode and Decode

The initial API operates on complete input data and returns complete output data.

Streaming Base64 processing is deferred. This includes direct integration with `binary_stream` and `text_stream`.

### Custom Alphabets

Custom Base64 alphabets are not supported.

Only the standard alphabet is accepted in the initial implementation.

### Detailed Error Positions

The initial decoder reports invalid input as `error_t::invalid_argument`, but it does not report the exact position of the invalid character or malformed padding.

Detailed position information may be added later through `xer::error<Detail>`.

### Text Encoding Conversion

Base64 text itself is ASCII and therefore valid UTF-8, but `<xer/base64.h>` does not perform character encoding conversion.

The input to `base64_decode` is treated as UTF-8-oriented text storage, but only the ASCII Base64 subset is meaningful. Binary output is returned as bytes and is not interpreted as UTF-8.

---

## Relationship to Other Headers

`<xer/base64.h>` is related to the following headers and policies:

- `<xer/error.h>`
- `<xer/stdio.h>`
- `policy_project_outline.md`
- `policy_result_arguments.md`
- `policy_encoding.md`

The rough boundary is:

- `<xer/string.h>` handles general string and memory utilities
- `<xer/stdio.h>` handles stream I/O
- `<xer/json.h>`, `<xer/ini.h>`, and `<xer/toml.h>` handle structured or semi-structured data formats
- `<xer/base64.h>` handles byte-oriented binary-to-text conversion

This separation is intentional. Base64 is useful together with text formats, but it is not itself a structured data format.

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

- that Base64 encoding is byte-oriented
- that encoded output is UTF-8 text containing only ASCII characters
- that decoding returns binary bytes
- that both encode and decode return `xer::result`
- that the initial implementation supports only standard padded Base64
- that URL-safe, unpadded, wrapped-output, and streaming variants are deferred

Detailed option behavior should be added when such options are actually introduced.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

- encoding a small byte sequence into Base64
- decoding the Base64 result back into bytes
- showing explicit `xer::result` checking
- printing encoded text with `<xer/stdio.h>`

These are good candidates for executable examples in `examples/`.

---

## Example

```cpp
#include <array>
#include <cstddef>

#include <xer/base64.h>
#include <xer/stdio.h>

auto main() -> int
{
    const std::array<std::byte, 5> data = {
        std::byte{'h'},
        std::byte{'e'},
        std::byte{'l'},
        std::byte{'l'},
        std::byte{'o'},
    };

    const auto encoded = xer::base64_encode(data);
    if (!encoded.has_value()) {
        return 1;
    }

    if (!xer::printf(u8"Encoded: %@\n", *encoded).has_value()) {
        return 1;
    }

    const auto decoded = xer::base64_decode(*encoded);
    if (!decoded.has_value()) {
        return 1;
    }

    return 0;
}
```

This example shows the general style:

- pass ordinary byte data to `base64_encode`
- check `xer::result` explicitly
- pass the encoded text to `base64_decode`
- treat decoded data as bytes rather than as text unless the caller knows its content

---

## See Also

- `policy_project_outline.md`
- `policy_result_arguments.md`
- `policy_encoding.md`
- `header_error.md`
- `header_stdio.md`
