/**
 * @file xer/bits/typeinfo.h
 * @brief Defines type information helpers for XER.
 */

#pragma once

#ifndef XER_BITS_TYPEINFO_H_INCLUDED_
#define XER_BITS_TYPEINFO_H_INCLUDED_

#include <cstdlib>
#include <string>
#include <string_view>
#include <typeindex>
#include <typeinfo>

#include <cxxabi.h>

#include <xer/bits/common.h>

namespace xer::detail {

/**
 * @brief Converts an ordinary byte string to a UTF-8 string.
 *
 * Demangled type names returned by the GCC C++ ABI are ordinary narrow strings.
 * XER source files are assumed to use UTF-8, and GCC is the current supported
 * compiler, so this helper copies the byte sequence into a char8_t string
 * without locale-dependent conversion.
 *
 * @param value Byte string view.
 * @return UTF-8 string containing the same bytes.
 */
[[nodiscard]] inline auto type_name_bytes_to_u8string(std::string_view value) -> std::u8string
{
    return std::u8string(reinterpret_cast<const char8_t*>(value.data()), value.size());
}

/**
 * @brief Demangles a GCC C++ ABI type name when possible.
 *
 * If demangling fails, the raw name is returned. Type names are mainly used
 * for diagnostics and tracing, so falling back to the implementation-provided
 * spelling is more useful than making type-name retrieval itself fail.
 *
 * @param raw_name Raw name returned from std::type_info or std::type_index.
 * @return Demangled UTF-8 type name, or the raw name on failure.
 */
[[nodiscard]] inline auto demangle_type_name(const char* raw_name) -> std::u8string
{
    if (raw_name == nullptr) {
        return {};
    }

    int status = 0;
    char* const demangled = abi::__cxa_demangle(raw_name, nullptr, nullptr, &status);

    if (status != 0 || demangled == nullptr) {
        return type_name_bytes_to_u8string(raw_name);
    }

    std::u8string result = type_name_bytes_to_u8string(demangled);
    std::free(demangled);
    return result;
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Lightweight type information wrapper with demangled name support.
 *
 * type_info stores std::type_index so that it can be used as an ordered key in
 * tracing tables and similar containers. The name() member returns a
 * demangled UTF-8 type name when the GCC C++ ABI demangler succeeds.
 */
class type_info {
public:
    /**
     * @brief Constructs a type information object from std::type_info.
     * @param value Standard type information object.
     */
    explicit type_info(const std::type_info& value) noexcept
        : index_(value)
    {
    }

    /**
     * @brief Returns the implementation-provided raw type name.
     * @return Raw type name from std::type_index.
     */
    [[nodiscard]] auto raw_name() const noexcept -> const char*
    {
        return index_.name();
    }

    /**
     * @brief Returns a demangled UTF-8 type name.
     * @return Demangled type name, or the raw type name if demangling fails.
     */
    [[nodiscard]] auto name() const -> std::u8string
    {
        return detail::demangle_type_name(raw_name());
    }

    /**
     * @brief Returns the wrapped std::type_index.
     * @return Wrapped type index.
     */
    [[nodiscard]] auto index() const noexcept -> std::type_index
    {
        return index_;
    }

    /**
     * @brief Tests whether two type information objects refer to the same type.
     * @param rhs Right-hand side.
     * @return true if both objects refer to the same type.
     */
    [[nodiscard]] auto operator==(const type_info& rhs) const noexcept -> bool
    {
        return index_ == rhs.index_;
    }

    /**
     * @brief Tests whether two type information objects refer to different types.
     * @param rhs Right-hand side.
     * @return true if the objects refer to different types.
     */
    [[nodiscard]] auto operator!=(const type_info& rhs) const noexcept -> bool
    {
        return index_ != rhs.index_;
    }

    /**
     * @brief Provides a stable ordering compatible with std::type_index.
     * @param rhs Right-hand side.
     * @return true if this type precedes rhs in the implementation-defined ordering.
     */
    [[nodiscard]] auto operator<(const type_info& rhs) const noexcept -> bool
    {
        return index_ < rhs.index_;
    }

private:
    std::type_index index_;
};

} // namespace xer

#endif /* XER_BITS_TYPEINFO_H_INCLUDED_ */
