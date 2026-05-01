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
