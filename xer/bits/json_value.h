/**
 * @file xer/bits/json_value.h
 * @brief Internal JSON value type definitions.
 */

#pragma once

#ifndef XER_BITS_JSON_VALUE_H_INCLUDED_
#define XER_BITS_JSON_VALUE_H_INCLUDED_

#include <cstddef>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <xer/bits/common.h>

namespace xer {

/**
 * @brief Represents a JSON value.
 *
 * The numeric representation is fixed to double.
 * Object members are stored in insertion order.
 */
struct json_value {
    using array_type = std::vector<json_value>;
    using object_entry_type = std::pair<std::u8string, json_value>;
    using object_type = std::vector<object_entry_type>;
    using variant_type = std::variant<
        std::nullptr_t,
        bool,
        double,
        std::u8string,
        array_type,
        object_type>;

    variant_type value;

    constexpr json_value() noexcept
        : value(nullptr)
    {
    }

    constexpr json_value(std::nullptr_t) noexcept
        : value(nullptr)
    {
    }

    constexpr json_value(bool value_) noexcept
        : value(value_)
    {
    }

    constexpr json_value(double value_) noexcept
        : value(value_)
    {
    }

    json_value(std::u8string value_)
        : value(std::move(value_))
    {
    }

    json_value(const char8_t* value_)
        : value(std::u8string(value_))
    {
    }

    json_value(array_type value_)
        : value(std::move(value_))
    {
    }

    json_value(object_type value_)
        : value(std::move(value_))
    {
    }

    [[nodiscard]] constexpr auto is_null() const noexcept -> bool
    {
        return std::holds_alternative<std::nullptr_t>(value);
    }

    [[nodiscard]] constexpr auto is_bool() const noexcept -> bool
    {
        return std::holds_alternative<bool>(value);
    }

    [[nodiscard]] constexpr auto is_number() const noexcept -> bool
    {
        return std::holds_alternative<double>(value);
    }

    [[nodiscard]] constexpr auto is_string() const noexcept -> bool
    {
        return std::holds_alternative<std::u8string>(value);
    }

    [[nodiscard]] constexpr auto is_array() const noexcept -> bool
    {
        return std::holds_alternative<array_type>(value);
    }

    [[nodiscard]] constexpr auto is_object() const noexcept -> bool
    {
        return std::holds_alternative<object_type>(value);
    }

    [[nodiscard]] constexpr auto as_bool() const noexcept -> const bool&
    {
        return std::get<bool>(value);
    }

    [[nodiscard]] constexpr auto as_number() const noexcept -> const double&
    {
        return std::get<double>(value);
    }

    [[nodiscard]] constexpr auto as_string() const noexcept -> const std::u8string&
    {
        return std::get<std::u8string>(value);
    }

    [[nodiscard]] constexpr auto as_array() const noexcept -> const array_type&
    {
        return std::get<array_type>(value);
    }

    [[nodiscard]] constexpr auto as_object() const noexcept -> const object_type&
    {
        return std::get<object_type>(value);
    }

    [[nodiscard]] constexpr auto as_bool() noexcept -> bool&
    {
        return std::get<bool>(value);
    }

    [[nodiscard]] constexpr auto as_number() noexcept -> double&
    {
        return std::get<double>(value);
    }

    [[nodiscard]] constexpr auto as_string() noexcept -> std::u8string&
    {
        return std::get<std::u8string>(value);
    }

    [[nodiscard]] constexpr auto as_array() noexcept -> array_type&
    {
        return std::get<array_type>(value);
    }

    [[nodiscard]] constexpr auto as_object() noexcept -> object_type&
    {
        return std::get<object_type>(value);
    }

    friend constexpr auto operator==(const json_value&, const json_value&) -> bool = default;
};

using json_array = json_value::array_type;
using json_object = json_value::object_type;

} // namespace xer

#endif /* XER_BITS_JSON_VALUE_H_INCLUDED_ */
