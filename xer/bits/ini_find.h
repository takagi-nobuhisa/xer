/**
 * @file xer/bits/ini_find.h
 * @brief INI search helpers.
 */

#pragma once

#ifndef XER_BITS_INI_FIND_H_INCLUDED_
#define XER_BITS_INI_FIND_H_INCLUDED_

#include <string_view>

#include <xer/bits/ini.h>

namespace xer {

/**
 * @brief Finds a global INI entry by key.
 *
 * The first global entry whose key equals @p key is returned. Because the INI
 * representation preserves duplicates, later matching entries are not returned
 * by this helper.
 *
 * @param file INI file to inspect.
 * @param key Global entry key.
 * @return Pointer to the existing entry, or @c nullptr.
 */
[[nodiscard]] inline auto ini_find(
    ini_file& file,
    std::u8string_view key) noexcept -> ini_entry*
{
    for (auto& entry : file.entries) {
        if (entry.key == key) {
            return &entry;
        }
    }

    return nullptr;
}

/**
 * @brief Finds a global INI entry by key in a const INI file.
 *
 * @param file INI file to inspect.
 * @param key Global entry key.
 * @return Pointer to the existing entry, or @c nullptr.
 */
[[nodiscard]] inline auto ini_find(
    const ini_file& file,
    std::u8string_view key) noexcept -> const ini_entry*
{
    for (const auto& entry : file.entries) {
        if (entry.key == key) {
            return &entry;
        }
    }

    return nullptr;
}

/**
 * @brief Finds an INI entry by section name and key.
 *
 * The first section whose name equals @p section is searched, and the first
 * entry in that section whose key equals @p key is returned. Duplicate sections
 * and keys are preserved by the data model, so this helper intentionally
 * returns only the first match.
 *
 * @param file INI file to inspect.
 * @param section Section name to search.
 * @param key Entry key to search within the section.
 * @return Pointer to the existing entry, or @c nullptr.
 */
[[nodiscard]] inline auto ini_find(
    ini_file& file,
    std::u8string_view section,
    std::u8string_view key) noexcept -> ini_entry*
{
    for (auto& current_section : file.sections) {
        if (current_section.name != section) {
            continue;
        }

        for (auto& entry : current_section.entries) {
            if (entry.key == key) {
                return &entry;
            }
        }

        return nullptr;
    }

    return nullptr;
}

/**
 * @brief Finds an INI entry by section name and key in a const INI file.
 *
 * @param file INI file to inspect.
 * @param section Section name to search.
 * @param key Entry key to search within the section.
 * @return Pointer to the existing entry, or @c nullptr.
 */
[[nodiscard]] inline auto ini_find(
    const ini_file& file,
    std::u8string_view section,
    std::u8string_view key) noexcept -> const ini_entry*
{
    for (const auto& current_section : file.sections) {
        if (current_section.name != section) {
            continue;
        }

        for (const auto& entry : current_section.entries) {
            if (entry.key == key) {
                return &entry;
            }
        }

        return nullptr;
    }

    return nullptr;
}

} // namespace xer

#endif /* XER_BITS_INI_FIND_H_INCLUDED_ */
