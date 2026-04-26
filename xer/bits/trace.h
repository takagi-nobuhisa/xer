/**
 * @file xer/bits/trace.h
 * @brief Trace output support for XER diagnostics.
 */

#pragma once

#ifndef XER_BITS_TRACE_H_INCLUDED_
#define XER_BITS_TRACE_H_INCLUDED_

#ifndef XER_ENABLE_TRACE
#    define XER_ENABLE_TRACE 1
#endif

#include <string_view>
#include <utility>

#include <xer/bits/diag_common.h>
#include <xer/bits/printf.h>
#include <xer/bits/standard_streams.h>
#include <xer/bits/text_stream.h>
#include <xer/bits/typeinfo.h>

namespace xer::detail {

inline text_stream* trace_output_stream = nullptr;
inline diag_level_t current_trace_level = diag_info;

} // namespace xer::detail

namespace xer {

/**
 * @brief Sets the destination stream for trace output.
 *
 * The stream is borrowed and must outlive trace use. Passing a temporary stream
 * object is therefore not appropriate.
 *
 * @param stream New trace output stream.
 */
inline auto set_trace_stream(text_stream& stream) noexcept -> void
{
    detail::trace_output_stream = &stream;
}

/**
 * @brief Resets trace output to the default standard error stream.
 */
inline auto reset_trace_stream() noexcept -> void
{
    detail::trace_output_stream = nullptr;
}

/**
 * @brief Returns the current trace output stream.
 *
 * @return Trace output stream, or standard error when no override is set.
 */
[[nodiscard]] inline auto get_trace_stream() noexcept -> text_stream&
{
    if (detail::trace_output_stream != nullptr) {
        return *detail::trace_output_stream;
    }

    return xer_stderr;
}

/**
 * @brief Sets the current trace level.
 *
 * Trace messages whose level is numerically larger than this value are omitted.
 *
 * @param level New trace level.
 */
inline auto set_trace_level(diag_level_t level) noexcept -> void
{
    detail::current_trace_level = level;
}

/**
 * @brief Returns the current trace level.
 * @return Current trace level.
 */
[[nodiscard]] inline auto get_trace_level() noexcept -> diag_level_t
{
    return detail::current_trace_level;
}

/**
 * @brief Tests whether a trace message should be emitted.
 *
 * The category is currently used for output and future filtering. The initial
 * implementation filters by level only.
 *
 * @param category Trace category.
 * @param level Requested trace level.
 * @return true if the trace message is enabled.
 */
[[nodiscard]] inline auto is_trace_enabled(
    diag_category category,
    diag_level_t level) noexcept -> bool
{
    static_cast<void>(category);
    return is_diag_level_enabled(get_trace_level(), level);
}

/**
 * @brief Emits one trace line.
 *
 * The value is formatted through XER's generic `%@` printf conversion. The type
 * name is obtained through `type_name<T>()` to avoid repeated demangling for
 * common traced types.
 *
 * @tparam T Traced value type.
 * @param category Trace category.
 * @param level Trace level.
 * @param name Source expression text.
 * @param value Traced value.
 */
template<class T>
inline auto trace(
    diag_category category,
    diag_level_t level,
    std::u8string_view name,
    const T& value) -> void
{
    static_cast<void>(fprintf(
        get_trace_stream(),
        u8"[%@][%@] %@ (%@) = %@\n",
        get_diag_category_name(category),
        level,
        name,
        type_name<T>(),
        value));
}

} // namespace xer

#if defined(NDEBUG) || !XER_ENABLE_TRACE
#    define xer_trace(category, level, object) static_cast<void>(0)
#else
#    define xer_trace(category, level, object)                                  \
        [&]() -> void {                                                         \
            if (::xer::is_trace_enabled((category), (level))) {                 \
                ::xer::trace((category), (level), u8"" #object, (object));       \
            }                                                                   \
        }()
#endif

#endif /* XER_BITS_TRACE_H_INCLUDED_ */
