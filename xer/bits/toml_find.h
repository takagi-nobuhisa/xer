/**
 * @file xer/bits/toml_find.h
 * @brief TOML value search helpers.
 */

#pragma once

#ifndef XER_BITS_TOML_FIND_H_INCLUDED_
#define XER_BITS_TOML_FIND_H_INCLUDED_

#include <string_view>

#include <xer/bits/toml.h>

namespace xer::detail {

[[nodiscard]] inline auto toml_find_direct(
    toml_table& table,
    std::u8string_view key) noexcept -> toml_value*
{
    for (auto& entry : table) {
        if (entry.first == key) {
            return &entry.second;
        }
    }

    return nullptr;
}

[[nodiscard]] inline auto toml_find_direct(
    const toml_table& table,
    std::u8string_view key) noexcept -> const toml_value*
{
    for (const auto& entry : table) {
        if (entry.first == key) {
            return &entry.second;
        }
    }

    return nullptr;
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Finds a TOML value through a simple dot-separated path.
 *
 * The path is split on literal dot characters and each component is looked up
 * as a direct key in the current table. This helper is intentionally simpler
 * than the TOML key grammar: quoted-key syntax is not parsed by this function.
 *
 * @param value Root TOML value to inspect.
 * @param path Dot-separated path such as @c project.name.
 * @return Pointer to the existing value, or @c nullptr.
 */
[[nodiscard]] inline auto toml_find(
    toml_value& value,
    std::u8string_view path) noexcept -> toml_value*
{
    if (path.empty()) {
        return nullptr;
    }

    auto* current = &value;
    std::size_t pos = 0;

    for (;;) {
        const std::size_t dot = path.find(u8'.', pos);
        const auto key = path.substr(
            pos,
            dot == std::u8string_view::npos ? std::u8string_view::npos : dot - pos);
        if (key.empty()) {
            return nullptr;
        }

        auto* table = current->as_table();
        if (table == nullptr) {
            return nullptr;
        }

        current = detail::toml_find_direct(*table, key);
        if (current == nullptr) {
            return nullptr;
        }

        if (dot == std::u8string_view::npos) {
            return current;
        }

        pos = dot + 1;
    }
}

/**
 * @brief Finds a TOML value through a simple dot-separated path.
 *
 * This const overload has the same lookup rules as the non-const overload.
 *
 * @param value Root TOML value to inspect.
 * @param path Dot-separated path such as @c project.name.
 * @return Pointer to the existing value, or @c nullptr.
 */
[[nodiscard]] inline auto toml_find(
    const toml_value& value,
    std::u8string_view path) noexcept -> const toml_value*
{
    if (path.empty()) {
        return nullptr;
    }

    const auto* current = &value;
    std::size_t pos = 0;

    for (;;) {
        const std::size_t dot = path.find(u8'.', pos);
        const auto key = path.substr(
            pos,
            dot == std::u8string_view::npos ? std::u8string_view::npos : dot - pos);
        if (key.empty()) {
            return nullptr;
        }

        const auto* table = current->as_table();
        if (table == nullptr) {
            return nullptr;
        }

        current = detail::toml_find_direct(*table, key);
        if (current == nullptr) {
            return nullptr;
        }

        if (dot == std::u8string_view::npos) {
            return current;
        }

        pos = dot + 1;
    }
}

} // namespace xer

#endif /* XER_BITS_TOML_FIND_H_INCLUDED_ */
