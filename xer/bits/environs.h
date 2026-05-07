/**
 * @file xer/bits/environs.h
 * @brief Environment variables enumeration implementation.
 */

#pragma once

#ifndef XER_BITS_ENVIRONS_H_INCLUDED_
#define XER_BITS_ENVIRONS_H_INCLUDED_

#include <cstddef>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <xer/bits/common.h>
#include <xer/bits/text_encoding_common.h>
#include <xer/error.h>

#ifdef _WIN32
extern "C" {
extern wchar_t** __wenvp;
extern char** __envp;
}
#else
extern "C" {
extern char** environ;
}
#endif

namespace xer {

/**
 * @brief One environment variable entry stored as UTF-8 strings.
 */
struct environ_entry {
    std::u8string name;
    std::u8string value;
};

using environ_arg = std::pair<std::u8string_view, std::u8string_view>;

/**
 * @brief Holds process environment variables as UTF-8 strings.
 *
 * `environs` owns a sequence of environment-variable entries. Each entry is
 * split into a name and a value. It represents a snapshot of the process
 * environment at the time it is obtained.
 */
class environs {
public:
    environs() = default;

    /**
     * @brief Constructs an environment snapshot from UTF-8 entries.
     *
     * @param entries Environment variable entries.
     */
    explicit environs(std::vector<environ_entry> entries)
        : entries_(std::move(entries))
    {
    }

    /**
     * @brief Returns the number of stored environment variables.
     *
     * @return Number of stored entries.
     */
    [[nodiscard]] auto size() const noexcept -> std::size_t
    {
        return entries_.size();
    }

    /**
     * @brief Returns whether no environment variables are stored.
     *
     * @return true if no entries are stored.
     */
    [[nodiscard]] auto empty() const noexcept -> bool
    {
        return entries_.empty();
    }

    /**
     * @brief Returns all environment variable entries.
     *
     * @return Span over stored UTF-8 entries.
     */
    [[nodiscard]] auto entries() const noexcept -> std::span<const environ_entry>
    {
        return std::span<const environ_entry>(entries_.data(), entries_.size());
    }

    /**
     * @brief Returns one environment variable entry.
     *
     * @param index Entry index.
     * @return Name and value views on success.
     */
    [[nodiscard]] auto at(std::size_t index) const -> result<environ_arg>
    {
        if (index >= entries_.size()) {
            return std::unexpected(make_error(error_t::out_of_range));
        }

        const auto& entry = entries_[index];
        return environ_arg{
            std::u8string_view(entry.name),
            std::u8string_view(entry.value),
        };
    }

    /**
     * @brief Finds an environment variable by name.
     *
     * If multiple entries with the same name exist, the first one in the
     * snapshot is returned.
     *
     * @param name Environment variable name.
     * @return Environment variable value view on success.
     */
    [[nodiscard]] auto find(std::u8string_view name) const -> result<std::u8string_view>
    {
        if (name.empty()) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        for (const auto& entry : entries_) {
            if (entry.name == name) {
                return std::u8string_view(entry.value);
            }
        }

        return std::unexpected(make_error(error_t::not_found));
    }

private:
    std::vector<environ_entry> entries_;
};

namespace detail {

[[nodiscard]] inline auto append_environment_entry(
    std::vector<environ_entry>& entries,
    std::u8string name,
    std::u8string value) -> result<void>
{
    if (name.empty()) {
        return {};
    }

    entries.push_back(environ_entry{std::move(name), std::move(value)});
    return {};
}

#ifndef _WIN32

[[nodiscard]] inline auto append_environment_entry_from_utf8_bytes(
    std::vector<environ_entry>& entries,
    std::string_view entry) -> result<void>
{
    const std::size_t separator = entry.find('=');
    if (separator == std::string_view::npos || separator == 0) {
        return {};
    }

    const std::string_view name(entry.data(), separator);
    const std::string_view value(
        entry.data() + separator + 1,
        entry.size() - separator - 1);

    if (!is_valid_utf8(name) || !is_valid_utf8(value)) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    return append_environment_entry(entries, to_u8string(name), to_u8string(value));
}

#else

[[nodiscard]] inline auto append_environment_entry_from_utf8_bytes(
    std::vector<environ_entry>& entries,
    std::string_view entry) -> result<void>
{
    const std::size_t separator = entry.find('=');
    if (separator == std::string_view::npos || separator == 0) {
        return {};
    }

    const std::string_view name(entry.data(), separator);
    const std::string_view value(
        entry.data() + separator + 1,
        entry.size() - separator - 1);

    if (!is_valid_utf8(name) || !is_valid_utf8(value)) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    return append_environment_entry(entries, to_u8string(name), to_u8string(value));
}

[[nodiscard]] inline auto append_environment_entry_from_wide(
    std::vector<environ_entry>& entries,
    std::wstring_view entry) -> result<void>
{
    const std::size_t separator = entry.find(L'=');
    if (separator == std::wstring_view::npos || separator == 0) {
        return {};
    }

    const auto name = wstring_to_utf8(entry.substr(0, separator));
    if (!name.has_value()) {
        return std::unexpected(name.error());
    }

    const auto value = wstring_to_utf8(entry.substr(separator + 1));
    if (!value.has_value()) {
        return std::unexpected(value.error());
    }

    return append_environment_entry(entries, *name, *value);
}

#endif

} // namespace detail

/**
 * @brief Gets all current process environment variables.
 *
 * On Windows, this function primarily uses `__wenvp` so that environment
 * strings are obtained as UTF-16 and converted to UTF-8. If `__wenvp` is
 * `nullptr`, it falls back to `__envp` and assumes that the byte strings are
 * UTF-8.
 *
 * On Linux, this function reads the process environment array and requires each
 * name and value to be valid UTF-8.
 *
 * Entries without `=` or entries with an empty name are ignored.
 *
 * @return Environment-variable snapshot on success.
 */
[[nodiscard]] inline auto get_environs() -> result<environs>
{
    std::vector<environ_entry> entries;

#ifdef _WIN32
    if (__wenvp != nullptr) {
        for (wchar_t** current = __wenvp; *current != nullptr; ++current) {
            const auto appended = detail::append_environment_entry_from_wide(
                entries,
                std::wstring_view(*current));
            if (!appended.has_value()) {
                return std::unexpected(appended.error());
            }
        }

        return environs(std::move(entries));
    }

    if (__envp == nullptr) {
        return environs(std::move(entries));
    }

    for (char** current = __envp; *current != nullptr; ++current) {
        const auto appended = detail::append_environment_entry_from_utf8_bytes(
            entries,
            std::string_view(*current));
        if (!appended.has_value()) {
            return std::unexpected(appended.error());
        }
    }
#else
    if (::environ == nullptr) {
        return environs(std::move(entries));
    }

    for (char** current = ::environ; *current != nullptr; ++current) {
        const auto appended = detail::append_environment_entry_from_utf8_bytes(
            entries,
            std::string_view(*current));
        if (!appended.has_value()) {
            return std::unexpected(appended.error());
        }
    }
#endif

    return environs(std::move(entries));
}

} // namespace xer

#endif /* XER_BITS_ENVIRONS_H_INCLUDED_ */
