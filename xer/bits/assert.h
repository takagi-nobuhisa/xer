/**
 * @file xer/bits/assert.h
 * @brief Internal helpers for xer assertion macros.
 */

#pragma once

#ifndef XER_BITS_ASSERT_H_INCLUDED_
#define XER_BITS_ASSERT_H_INCLUDED_

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <sstream>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <xer/bits/advanced_encoding.h>
#include <xer/bits/common.h>

namespace xer {

/**
 * @brief Exception thrown when a xer assert macro fails.
 */
class assertion_error : public std::runtime_error {
public:
    /**
     * @brief Initializes the exception.
     * @param message Diagnostic message.
     */
    explicit assertion_error(std::string message)
        : std::runtime_error(std::move(message))
    {
    }
};

} // namespace xer

namespace xer::detail {

/**
 * @brief Tests whether a value can be written to an output stream.
 */
template<typename t>
concept ostream_writable = requires(std::ostringstream& os, const t& value) {
    os << value;
};

/**
 * @brief Converts a byte string view to a UTF-8 string.
 * @param value Byte string view.
 * @return UTF-8 string.
 */
[[nodiscard]] inline std::u8string bytes_to_u8string(std::string_view value)
{
    return std::u8string(reinterpret_cast<const char8_t*>(value.data()), value.size());
}

/**
 * @brief Converts a UTF-8 string to a byte string.
 * @param value UTF-8 string.
 * @return Byte string.
 */
[[nodiscard]] inline std::string u8string_to_bytes(const std::u8string& value)
{
    return std::string(reinterpret_cast<const char*>(value.data()), value.size());
}

/**
 * @brief Appends a byte string view to a UTF-8 string.
 * @param out Output string.
 * @param value Byte string view.
 */
inline void append_bytes(std::u8string& out, std::string_view value)
{
    out.append(reinterpret_cast<const char8_t*>(value.data()), value.size());
}

/**
 * @brief Appends an unsigned integer in decimal form.
 * @param out Output string.
 * @param value Value to append.
 */
inline void append_uint(std::u8string& out, std::uint_least32_t value)
{
    char buffer[32];
    std::size_t length = 0;

    do {
        buffer[length++] = static_cast<char>('0' + (value % 10));
        value /= 10;
    } while (value != 0);

    while (length > 0) {
        --length;
        out.push_back(static_cast<char8_t>(buffer[length]));
    }
}

/**
 * @brief Appends a UTF-32 code point as UTF-8.
 * @param out Output string.
 * @param code_point Code point.
 */
inline void append_utf8(std::u8string& out, char32_t code_point)
{
    constexpr char32_t replacement = U'\uFFFD';

    std::uint32_t packed = xer::advanced::utf32_to_packed_utf8(code_point);
    if (packed == xer::advanced::detail::invalid_packed_utf8) {
        packed = xer::advanced::utf32_to_packed_utf8(replacement);
    }

    const std::uint8_t b1 = static_cast<std::uint8_t>(packed & 0xFFu);
    const std::uint8_t b2 = static_cast<std::uint8_t>((packed >> 8) & 0xFFu);
    const std::uint8_t b3 = static_cast<std::uint8_t>((packed >> 16) & 0xFFu);
    const std::uint8_t b4 = static_cast<std::uint8_t>((packed >> 24) & 0xFFu);

    if (b1 != 0) {
        out.push_back(static_cast<char8_t>(b1));
    }

    if (b2 != 0) {
        out.push_back(static_cast<char8_t>(b2));
    }

    if (b3 != 0) {
        out.push_back(static_cast<char8_t>(b3));
    }

    if (b4 != 0) {
        out.push_back(static_cast<char8_t>(b4));
    }
}

/**
 * @brief Converts a UTF-16 string view to UTF-8.
 * @param value UTF-16 string view.
 * @return UTF-8 string.
 */
[[nodiscard]] inline std::u8string utf16_to_utf8(std::u16string_view value)
{
    constexpr char32_t replacement = U'\uFFFD';

    std::u8string out;

    for (std::size_t i = 0; i < value.size(); ++i) {
        const char16_t w1 = value[i];

        if (w1 < 0xD800u || w1 > 0xDFFFu) {
            append_utf8(out, static_cast<char32_t>(w1));
            continue;
        }

        if (w1 >= 0xDC00u) {
            append_utf8(out, replacement);
            continue;
        }

        if (i + 1 >= value.size()) {
            append_utf8(out, replacement);
            continue;
        }

        const char16_t w2 = value[i + 1];
        if (w2 < 0xDC00u || w2 > 0xDFFFu) {
            append_utf8(out, replacement);
            continue;
        }

        const std::uint32_t high = static_cast<std::uint32_t>(w1) - 0xD800u;
        const std::uint32_t low = static_cast<std::uint32_t>(w2) - 0xDC00u;
        const char32_t code_point =
            static_cast<char32_t>(0x10000u + ((high << 10) | low));

        append_utf8(out, code_point);
        ++i;
    }

    return out;
}

/**
 * @brief Converts a UTF-32 string view to UTF-8.
 * @param value UTF-32 string view.
 * @return UTF-8 string.
 */
[[nodiscard]] inline std::u8string utf32_to_utf8(std::u32string_view value)
{
    std::u8string out;

    for (char32_t ch : value) {
        append_utf8(out, ch);
    }

    return out;
}

/**
 * @brief Converts a wide string view to UTF-8.
 * @param value Wide string view.
 * @return UTF-8 string.
 */
[[nodiscard]] inline std::u8string wide_to_utf8(std::wstring_view value)
{
#if __SIZEOF_WCHAR_T__ == 2
    return utf16_to_utf8(
        std::u16string_view(
            reinterpret_cast<const char16_t*>(value.data()),
            value.size()));
#elif __SIZEOF_WCHAR_T__ == 4
    return utf32_to_utf8(
        std::u32string_view(
            reinterpret_cast<const char32_t*>(value.data()),
            value.size()));
#else
# error unsupported wchar_t size
#endif
}

/**
 * @brief Converts a CP932 null-terminated string to UTF-8.
 * @param value CP932 string.
 * @return UTF-8 string.
 */
[[nodiscard]] inline std::u8string cp932_to_utf8(const unsigned char* value)
{
    constexpr char32_t replacement = U'\uFFFD';

    std::u8string out;

    while (*value != 0) {
        const std::uint8_t b1 = *value++;

        char32_t code_point = xer::advanced::detail::invalid_utf32;

        if (xer::advanced::detail::is_cp932_single_byte(b1)) {
            code_point = xer::advanced::packed_cp932_to_utf32(b1);
        } else if (xer::advanced::detail::is_cp932_lead_byte(b1)) {
            if (*value == 0) {
                code_point = replacement;
            } else {
                const std::uint8_t b2 = *value++;
                const std::uint16_t packed =
                    static_cast<std::uint16_t>(b1) |
                    (static_cast<std::uint16_t>(b2) << 8);
                code_point = xer::advanced::packed_cp932_to_utf32(packed);
            }
        }

        if (code_point == xer::advanced::detail::invalid_utf32) {
            code_point = replacement;
        }

        append_utf8(out, code_point);
    }

    return out;
}

/**
 * @brief Converts a bool value to a diagnostic string.
 * @param value Value to convert.
 * @return Diagnostic string.
 */
[[nodiscard]] inline std::u8string value_to_string(bool value)
{
    return value ? u8"true" : u8"false";
}

/**
 * @brief Converts a char value to a diagnostic string.
 * @param value Value to convert.
 * @return Diagnostic string.
 */
[[nodiscard]] inline std::u8string value_to_string(char value)
{
    return std::u8string(1, static_cast<char8_t>(static_cast<unsigned char>(value)));
}

/**
 * @brief Converts a signed char value to a diagnostic string.
 * @param value Value to convert.
 * @return Diagnostic string.
 */
[[nodiscard]] inline std::u8string value_to_string(signed char value)
{
    return std::u8string(1, static_cast<char8_t>(static_cast<unsigned char>(value)));
}

/**
 * @brief Converts an unsigned char value to a diagnostic string.
 * @param value Value to convert.
 * @return Diagnostic string.
 */
[[nodiscard]] inline std::u8string value_to_string(unsigned char value)
{
    std::ostringstream os;
    os << static_cast<unsigned int>(value);
    return bytes_to_u8string(os.str());
}

/**
 * @brief Converts a wchar_t value to a diagnostic string.
 * @param value Value to convert.
 * @return Diagnostic string.
 */
[[nodiscard]] inline std::u8string value_to_string(wchar_t value)
{
    return wide_to_utf8(std::wstring_view(&value, 1));
}

/**
 * @brief Converts a char8_t value to a diagnostic string.
 * @param value Value to convert.
 * @return Diagnostic string.
 */
[[nodiscard]] inline std::u8string value_to_string(char8_t value)
{
    return std::u8string(1, value);
}

/**
 * @brief Converts a char16_t value to a diagnostic string.
 * @param value Value to convert.
 * @return Diagnostic string.
 */
[[nodiscard]] inline std::u8string value_to_string(char16_t value)
{
    return utf16_to_utf8(std::u16string_view(&value, 1));
}

/**
 * @brief Converts a char32_t value to a diagnostic string.
 * @param value Value to convert.
 * @return Diagnostic string.
 */
[[nodiscard]] inline std::u8string value_to_string(char32_t value)
{
    return utf32_to_utf8(std::u32string_view(&value, 1));
}

/**
 * @brief Converts a null narrow string pointer to a diagnostic string.
 * @param value Pointer value.
 * @return Diagnostic string.
 */
[[nodiscard]] inline std::u8string value_to_string(const char* value)
{
    if (value == nullptr) {
        return u8"null";
    }

    return bytes_to_u8string(value);
}

/**
 * @brief Converts a null signed char string pointer to a diagnostic string.
 * @param value Pointer value.
 * @return Diagnostic string.
 */
[[nodiscard]] inline std::u8string value_to_string(const signed char* value)
{
    if (value == nullptr) {
        return u8"null";
    }

    return bytes_to_u8string(reinterpret_cast<const char*>(value));
}

/**
 * @brief Converts a null unsigned char string pointer to a diagnostic string.
 * @param value Pointer value.
 * @return Diagnostic string.
 */
[[nodiscard]] inline std::u8string value_to_string(const unsigned char* value)
{
    if (value == nullptr) {
        return u8"null";
    }

    return cp932_to_utf8(value);
}

/**
 * @brief Converts a null wide string pointer to a diagnostic string.
 * @param value Pointer value.
 * @return Diagnostic string.
 */
[[nodiscard]] inline std::u8string value_to_string(const wchar_t* value)
{
    if (value == nullptr) {
        return u8"null";
    }

    return wide_to_utf8(value);
}

/**
 * @brief Converts a null UTF-8 string pointer to a diagnostic string.
 * @param value Pointer value.
 * @return Diagnostic string.
 */
[[nodiscard]] inline std::u8string value_to_string(const char8_t* value)
{
    if (value == nullptr) {
        return u8"null";
    }

    return std::u8string(value);
}

/**
 * @brief Converts a null UTF-16 string pointer to a diagnostic string.
 * @param value Pointer value.
 * @return Diagnostic string.
 */
[[nodiscard]] inline std::u8string value_to_string(const char16_t* value)
{
    if (value == nullptr) {
        return u8"null";
    }

    return utf16_to_utf8(value);
}

/**
 * @brief Converts a null UTF-32 string pointer to a diagnostic string.
 * @param value Pointer value.
 * @return Diagnostic string.
 */
[[nodiscard]] inline std::u8string value_to_string(const char32_t* value)
{
    if (value == nullptr) {
        return u8"null";
    }

    return utf32_to_utf8(value);
}

/**
 * @brief Converts a narrow string to a diagnostic string.
 * @param value Value to convert.
 * @return Diagnostic string.
 */
[[nodiscard]] inline std::u8string value_to_string(const std::string& value)
{
    return bytes_to_u8string(value);
}

/**
 * @brief Converts a narrow string view to a diagnostic string.
 * @param value Value to convert.
 * @return Diagnostic string.
 */
[[nodiscard]] inline std::u8string value_to_string(std::string_view value)
{
    return bytes_to_u8string(value);
}

/**
 * @brief Converts a wide string to a diagnostic string.
 * @param value Value to convert.
 * @return Diagnostic string.
 */
[[nodiscard]] inline std::u8string value_to_string(const std::wstring& value)
{
    return wide_to_utf8(value);
}

/**
 * @brief Converts a wide string view to a diagnostic string.
 * @param value Value to convert.
 * @return Diagnostic string.
 */
[[nodiscard]] inline std::u8string value_to_string(std::wstring_view value)
{
    return wide_to_utf8(value);
}

/**
 * @brief Converts a UTF-8 string to a diagnostic string.
 * @param value Value to convert.
 * @return Diagnostic string.
 */
[[nodiscard]] inline std::u8string value_to_string(const std::u8string& value)
{
    return value;
}

/**
 * @brief Converts a UTF-8 string view to a diagnostic string.
 * @param value Value to convert.
 * @return Diagnostic string.
 */
[[nodiscard]] inline std::u8string value_to_string(std::u8string_view value)
{
    return std::u8string(value);
}

/**
 * @brief Converts a UTF-16 string to a diagnostic string.
 * @param value Value to convert.
 * @return Diagnostic string.
 */
[[nodiscard]] inline std::u8string value_to_string(const std::u16string& value)
{
    return utf16_to_utf8(value);
}

/**
 * @brief Converts a UTF-16 string view to a diagnostic string.
 * @param value Value to convert.
 * @return Diagnostic string.
 */
[[nodiscard]] inline std::u8string value_to_string(std::u16string_view value)
{
    return utf16_to_utf8(value);
}

/**
 * @brief Converts a UTF-32 string to a diagnostic string.
 * @param value Value to convert.
 * @return Diagnostic string.
 */
[[nodiscard]] inline std::u8string value_to_string(const std::u32string& value)
{
    return utf32_to_utf8(value);
}

/**
 * @brief Converts a UTF-32 string view to a diagnostic string.
 * @param value Value to convert.
 * @return Diagnostic string.
 */
[[nodiscard]] inline std::u8string value_to_string(std::u32string_view value)
{
    return utf32_to_utf8(value);
}

/**
 * @brief Converts a value to a diagnostic string when possible.
 * @tparam t Value type.
 * @param value Value to convert.
 * @return Diagnostic string.
 */
template<typename t>
[[nodiscard]] inline std::u8string value_to_string(const t& value)
{
    if constexpr (ostream_writable<t>) {
        std::ostringstream os;
        os << value;
        return bytes_to_u8string(os.str());
    } else {
        return u8"<unprintable>";
    }
}

/**
 * @brief Builds a common assertion failure message.
 * @param kind Assertion kind.
 * @param expression_text Text of the failed expression.
 * @param detail_text Additional detail.
 * @param location Source location.
 * @return Formatted message.
 */
[[nodiscard]] inline std::u8string build_assert_message(
    std::string_view kind,
    std::string_view expression_text,
    std::u8string_view detail_text,
    const std::source_location& location)
{
    std::u8string out;

    append_bytes(out, location.file_name());
    out += u8":";
    append_uint(out, location.line());
    out += u8":";
    append_uint(out, location.column());
    out += u8": ";
    append_bytes(out, kind);
    out += u8" failed: ";
    append_bytes(out, expression_text);

    if (!detail_text.empty()) {
        out += u8" (";
        out.append(detail_text);
        out += u8")";
    }

    out += u8" [";
    append_bytes(out, location.function_name());
    out += u8"]";

    return out;
}

/**
 * @brief Throws an assertion error.
 * @param kind Assertion kind.
 * @param expression_text Text of the failed expression.
 * @param detail_text Additional detail.
 * @param location Source location.
 */
[[noreturn]] inline void throw_assertion_failure(
    std::string_view kind,
    std::string_view expression_text,
    std::u8string_view detail_text,
    const std::source_location& location)
{
    throw assertion_error(
        u8string_to_bytes(build_assert_message(kind, expression_text, detail_text, location)));
}

[[noreturn]] inline void throw_assertion_failure(
    std::string_view kind,
    std::string_view expression_text,
    std::string_view detail_text,
    const std::source_location& location)
{
    throw_assertion_failure(
        kind,
        expression_text,
        bytes_to_u8string(detail_text),
        location);
}

/**
 * @brief Checks a boolean condition.
 * @param condition Condition result.
 * @param expression_text Text of the checked expression.
 * @param location Source location.
 */
inline void assert_true(
    bool condition,
    std::string_view expression_text,
    const std::source_location& location)
{
    if (!condition) {
        throw_assertion_failure("xer_assert", expression_text, u8"expected true", location);
    }
}

/**
 * @brief Checks that a boolean condition is false.
 * @param condition Condition result.
 * @param expression_text Text of the checked expression.
 * @param location Source location.
 */
inline void assert_false(
    bool condition,
    std::string_view expression_text,
    const std::source_location& location)
{
    if (condition) {
        throw_assertion_failure("xer_assert_not", expression_text, u8"expected false", location);
    }
}

/**
 * @brief Checks equality.
 * @tparam lhs_t Left operand type.
 * @tparam rhs_t Right operand type.
 * @param lhs Left operand.
 * @param rhs Right operand.
 * @param lhs_text Left operand text.
 * @param rhs_text Right operand text.
 * @param location Source location.
 */
template<typename lhs_t, typename rhs_t>
inline void assert_eq(
    const lhs_t& lhs,
    const rhs_t& rhs,
    std::string_view lhs_text,
    std::string_view rhs_text,
    const std::source_location& location)
{
    if (!(lhs == rhs)) {
        std::u8string detail;

        append_bytes(detail, lhs_text);
        detail += u8" == ";
        append_bytes(detail, rhs_text);
        detail += u8", lhs=";
        detail += value_to_string(lhs);
        detail += u8", rhs=";
        detail += value_to_string(rhs);

        throw_assertion_failure("xer_assert_eq", lhs_text, detail, location);
    }
}

/**
 * @brief Checks inequality.
 * @tparam lhs_t Left operand type.
 * @tparam rhs_t Right operand type.
 * @param lhs Left operand.
 * @param rhs Right operand.
 * @param lhs_text Left operand text.
 * @param rhs_text Right operand text.
 * @param location Source location.
 */
template<typename lhs_t, typename rhs_t>
inline void assert_ne(
    const lhs_t& lhs,
    const rhs_t& rhs,
    std::string_view lhs_text,
    std::string_view rhs_text,
    const std::source_location& location)
{
    if (!(lhs != rhs)) {
        std::u8string detail;

        append_bytes(detail, lhs_text);
        detail += u8" != ";
        append_bytes(detail, rhs_text);
        detail += u8", lhs=";
        detail += value_to_string(lhs);
        detail += u8", rhs=";
        detail += value_to_string(rhs);

        throw_assertion_failure("xer_assert_ne", lhs_text, detail, location);
    }
}

/**
 * @brief Checks less-than relation.
 * @tparam lhs_t Left operand type.
 * @tparam rhs_t Right operand type.
 * @param lhs Left operand.
 * @param rhs Right operand.
 * @param lhs_text Left operand text.
 * @param rhs_text Right operand text.
 * @param location Source location.
 */
template<typename lhs_t, typename rhs_t>
inline void assert_lt(
    const lhs_t& lhs,
    const rhs_t& rhs,
    std::string_view lhs_text,
    std::string_view rhs_text,
    const std::source_location& location)
{
    if (!(lhs < rhs)) {
        std::u8string detail;

        append_bytes(detail, lhs_text);
        detail += u8" < ";
        append_bytes(detail, rhs_text);
        detail += u8", lhs=";
        detail += value_to_string(lhs);
        detail += u8", rhs=";
        detail += value_to_string(rhs);

        throw_assertion_failure("xer_assert_lt", lhs_text, detail, location);
    }
}

} // namespace xer::detail

#endif /* XER_BITS_ASSERT_H_INCLUDED_ */
