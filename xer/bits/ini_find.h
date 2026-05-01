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
