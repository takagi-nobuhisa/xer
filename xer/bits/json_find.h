/**
 * @file xer/bits/json_find.h
 * @brief JSON value search helpers.
 */

#pragma once

#ifndef XER_BITS_JSON_FIND_H_INCLUDED_
#define XER_BITS_JSON_FIND_H_INCLUDED_

#include <string_view>

#include <xer/bits/json_value.h>

namespace xer {

/**
 * @brief Finds a direct child value in a JSON object.
 *
 * If @p value is an object, this function searches its direct key/value
 * entries in source order and returns a pointer to the first value whose key
 * equals @p key. If @p value is not an object or the key is absent, it returns
 * @c nullptr.
 *
 * @param value JSON value to inspect.
 * @param key Object key to search for.
 * @return Pointer to the existing child value, or @c nullptr.
 */
[[nodiscard]] inline auto json_find(
    json_value& value,
    std::u8string_view key) noexcept -> json_value*
{
    auto* object = value.is_object() ? &value.as_object() : nullptr;
    if (object == nullptr) {
        return nullptr;
    }

    for (auto& entry : *object) {
        if (entry.first == key) {
            return &entry.second;
        }
    }

    return nullptr;
}

/**
 * @brief Finds a direct child value in a const JSON object.
 *
 * This overload has the same lookup rules as the non-const overload and returns
 * a const pointer to the existing child value.
 *
 * @param value JSON value to inspect.
 * @param key Object key to search for.
 * @return Pointer to the existing child value, or @c nullptr.
 */
[[nodiscard]] inline auto json_find(
    const json_value& value,
    std::u8string_view key) noexcept -> const json_value*
{
    const auto* object = value.is_object() ? &value.as_object() : nullptr;
    if (object == nullptr) {
        return nullptr;
    }

    for (const auto& entry : *object) {
        if (entry.first == key) {
            return &entry.second;
        }
    }

    return nullptr;
}

} // namespace xer

#endif /* XER_BITS_JSON_FIND_H_INCLUDED_ */
