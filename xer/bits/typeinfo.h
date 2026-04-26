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
#include <type_traits>

#include <cxxabi.h>

#include <xer/bits/common.h>
#include <xer/error.h>

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

namespace xer::detail {

/**
 * @brief Provides a display name for a statically known type.
 *
 * The primary template falls back to demangled runtime type information.
 * Specializations for frequently traced scalar and string types avoid the
 * demangling path and produce concise, user-facing names.
 *
 * @tparam T Type whose display name is requested.
 */
template<class T>
struct static_type_name {
    [[nodiscard]] static auto get() -> std::u8string
    {
        return type_info(typeid(T)).name();
    }
};

#define XER_DETAIL_DEFINE_STATIC_TYPE_NAME(type_, name_)                       \
    template<>                                                                 \
    struct static_type_name<type_> {                                           \
        [[nodiscard]] static auto get() -> std::u8string                       \
        {                                                                      \
            return std::u8string(name_);                                       \
        }                                                                      \
    }

XER_DETAIL_DEFINE_STATIC_TYPE_NAME(bool, u8"bool");

XER_DETAIL_DEFINE_STATIC_TYPE_NAME(char, u8"char");
XER_DETAIL_DEFINE_STATIC_TYPE_NAME(signed char, u8"signed char");
XER_DETAIL_DEFINE_STATIC_TYPE_NAME(unsigned char, u8"unsigned char");
XER_DETAIL_DEFINE_STATIC_TYPE_NAME(char8_t, u8"char8_t");
XER_DETAIL_DEFINE_STATIC_TYPE_NAME(char16_t, u8"char16_t");
XER_DETAIL_DEFINE_STATIC_TYPE_NAME(char32_t, u8"char32_t");
XER_DETAIL_DEFINE_STATIC_TYPE_NAME(wchar_t, u8"wchar_t");

XER_DETAIL_DEFINE_STATIC_TYPE_NAME(short, u8"short");
XER_DETAIL_DEFINE_STATIC_TYPE_NAME(unsigned short, u8"unsigned short");
XER_DETAIL_DEFINE_STATIC_TYPE_NAME(int, u8"int");
XER_DETAIL_DEFINE_STATIC_TYPE_NAME(unsigned int, u8"unsigned int");
XER_DETAIL_DEFINE_STATIC_TYPE_NAME(long, u8"long");
XER_DETAIL_DEFINE_STATIC_TYPE_NAME(unsigned long, u8"unsigned long");
XER_DETAIL_DEFINE_STATIC_TYPE_NAME(long long, u8"long long");
XER_DETAIL_DEFINE_STATIC_TYPE_NAME(unsigned long long, u8"unsigned long long");

XER_DETAIL_DEFINE_STATIC_TYPE_NAME(float, u8"float");
XER_DETAIL_DEFINE_STATIC_TYPE_NAME(double, u8"double");
XER_DETAIL_DEFINE_STATIC_TYPE_NAME(long double, u8"long double");

XER_DETAIL_DEFINE_STATIC_TYPE_NAME(std::string, u8"std::string");
XER_DETAIL_DEFINE_STATIC_TYPE_NAME(std::string_view, u8"std::string_view");
XER_DETAIL_DEFINE_STATIC_TYPE_NAME(std::u8string, u8"std::u8string");
XER_DETAIL_DEFINE_STATIC_TYPE_NAME(std::u8string_view, u8"std::u8string_view");

XER_DETAIL_DEFINE_STATIC_TYPE_NAME(error_t, u8"xer::error_t");

#undef XER_DETAIL_DEFINE_STATIC_TYPE_NAME

} // namespace xer::detail

namespace xer {

/**
 * @brief Returns a human-readable UTF-8 name for a statically known type.
 *
 * cv-qualification and references are removed before lookup. This function is
 * intended for diagnostics and tracing where concise, stable display names for
 * common types are more useful than repeatedly demangling runtime type names.
 *
 * @tparam T Source type.
 * @return UTF-8 display name.
 */
template<class T>
[[nodiscard]] auto type_name() -> std::u8string
{
    using value_type = std::remove_cvref_t<T>;
    return detail::static_type_name<value_type>::get();
}

} // namespace xer

#endif /* XER_BITS_TYPEINFO_H_INCLUDED_ */
