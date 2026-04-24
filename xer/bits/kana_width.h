/**
 * @file xer/bits/kana_width.h
 * @brief Internal fullwidth and halfwidth Kana helpers.
 */

#pragma once

#ifndef XER_BITS_KANA_WIDTH_H_INCLUDED_
#define XER_BITS_KANA_WIDTH_H_INCLUDED_

#include <expected>

#include <xer/bits/common.h>
#include <xer/error.h>

namespace xer::detail {

struct halfwidth_kana_decomposition {
    char32_t base{};
    char32_t mark{};
};

[[nodiscard]] constexpr auto is_halfwidth_kana(char32_t c) noexcept -> bool
{
    return c >= U'\uff61' && c <= U'\uff9f';
}

[[nodiscard]] constexpr auto is_fullwidth_kana(char32_t c) noexcept -> bool
{
    return c == U'\u3001' || c == U'\u3002' || c == U'\u300c' ||
           c == U'\u300d' || c == U'\u309b' || c == U'\u309c' ||
           c == U'\u30fb' || c == U'\u30fc' ||
           (c >= U'\u30a1' && c <= U'\u30fa');
}

[[nodiscard]] constexpr auto is_halfwidth_voiced_sound_mark(char32_t c) noexcept
    -> bool
{
    return c == U'\uff9e';
}

[[nodiscard]] constexpr auto is_halfwidth_semivoiced_sound_mark(char32_t c) noexcept
    -> bool
{
    return c == U'\uff9f';
}

[[nodiscard]] constexpr auto halfwidth_kana_to_fullwidth_base(char32_t c) noexcept
    -> char32_t
{
    switch (c) {
        case U'\uff61': return U'\u3002';
        case U'\uff62': return U'\u300c';
        case U'\uff63': return U'\u300d';
        case U'\uff64': return U'\u3001';
        case U'\uff65': return U'\u30fb';
        case U'\uff66': return U'\u30f2';
        case U'\uff67': return U'\u30a1';
        case U'\uff68': return U'\u30a3';
        case U'\uff69': return U'\u30a5';
        case U'\uff6a': return U'\u30a7';
        case U'\uff6b': return U'\u30a9';
        case U'\uff6c': return U'\u30e3';
        case U'\uff6d': return U'\u30e5';
        case U'\uff6e': return U'\u30e7';
        case U'\uff6f': return U'\u30c3';
        case U'\uff70': return U'\u30fc';
        case U'\uff71': return U'\u30a2';
        case U'\uff72': return U'\u30a4';
        case U'\uff73': return U'\u30a6';
        case U'\uff74': return U'\u30a8';
        case U'\uff75': return U'\u30aa';
        case U'\uff76': return U'\u30ab';
        case U'\uff77': return U'\u30ad';
        case U'\uff78': return U'\u30af';
        case U'\uff79': return U'\u30b1';
        case U'\uff7a': return U'\u30b3';
        case U'\uff7b': return U'\u30b5';
        case U'\uff7c': return U'\u30b7';
        case U'\uff7d': return U'\u30b9';
        case U'\uff7e': return U'\u30bb';
        case U'\uff7f': return U'\u30bd';
        case U'\uff80': return U'\u30bf';
        case U'\uff81': return U'\u30c1';
        case U'\uff82': return U'\u30c4';
        case U'\uff83': return U'\u30c6';
        case U'\uff84': return U'\u30c8';
        case U'\uff85': return U'\u30ca';
        case U'\uff86': return U'\u30cb';
        case U'\uff87': return U'\u30cc';
        case U'\uff88': return U'\u30cd';
        case U'\uff89': return U'\u30ce';
        case U'\uff8a': return U'\u30cf';
        case U'\uff8b': return U'\u30d2';
        case U'\uff8c': return U'\u30d5';
        case U'\uff8d': return U'\u30d8';
        case U'\uff8e': return U'\u30db';
        case U'\uff8f': return U'\u30de';
        case U'\uff90': return U'\u30df';
        case U'\uff91': return U'\u30e0';
        case U'\uff92': return U'\u30e1';
        case U'\uff93': return U'\u30e2';
        case U'\uff94': return U'\u30e4';
        case U'\uff95': return U'\u30e6';
        case U'\uff96': return U'\u30e8';
        case U'\uff97': return U'\u30e9';
        case U'\uff98': return U'\u30ea';
        case U'\uff99': return U'\u30eb';
        case U'\uff9a': return U'\u30ec';
        case U'\uff9b': return U'\u30ed';
        case U'\uff9c': return U'\u30ef';
        case U'\uff9d': return U'\u30f3';
        case U'\uff9e': return U'\u309b';
        case U'\uff9f': return U'\u309c';
        default: return c;
    }
}

[[nodiscard]] constexpr auto compose_halfwidth_kana(char32_t base, char32_t mark)
    noexcept -> char32_t
{
    if (mark == U'\uff9e') {
        switch (base) {
            case U'\uff73': return U'\u30f4';
            case U'\uff76': return U'\u30ac';
            case U'\uff77': return U'\u30ae';
            case U'\uff78': return U'\u30b0';
            case U'\uff79': return U'\u30b2';
            case U'\uff7a': return U'\u30b4';
            case U'\uff7b': return U'\u30b6';
            case U'\uff7c': return U'\u30b8';
            case U'\uff7d': return U'\u30ba';
            case U'\uff7e': return U'\u30bc';
            case U'\uff7f': return U'\u30be';
            case U'\uff80': return U'\u30c0';
            case U'\uff81': return U'\u30c2';
            case U'\uff82': return U'\u30c5';
            case U'\uff83': return U'\u30c7';
            case U'\uff84': return U'\u30c9';
            case U'\uff8a': return U'\u30d0';
            case U'\uff8b': return U'\u30d3';
            case U'\uff8c': return U'\u30d6';
            case U'\uff8d': return U'\u30d9';
            case U'\uff8e': return U'\u30dc';
            case U'\uff9c': return U'\u30f7';
            case U'\uff66': return U'\u30fa';
            default: return U'\0';
        }
    }

    if (mark == U'\uff9f') {
        switch (base) {
            case U'\uff8a': return U'\u30d1';
            case U'\uff8b': return U'\u30d4';
            case U'\uff8c': return U'\u30d7';
            case U'\uff8d': return U'\u30da';
            case U'\uff8e': return U'\u30dd';
            default: return U'\0';
        }
    }

    return U'\0';
}

[[nodiscard]] constexpr auto fullwidth_kana_to_halfwidth(
    char32_t c) noexcept -> halfwidth_kana_decomposition
{
    switch (c) {
        case U'\u3002': return {U'\uff61', U'\0'};
        case U'\u300c': return {U'\uff62', U'\0'};
        case U'\u300d': return {U'\uff63', U'\0'};
        case U'\u3001': return {U'\uff64', U'\0'};
        case U'\u30fb': return {U'\uff65', U'\0'};
        case U'\u30f2': return {U'\uff66', U'\0'};
        case U'\u30a1': return {U'\uff67', U'\0'};
        case U'\u30a3': return {U'\uff68', U'\0'};
        case U'\u30a5': return {U'\uff69', U'\0'};
        case U'\u30a7': return {U'\uff6a', U'\0'};
        case U'\u30a9': return {U'\uff6b', U'\0'};
        case U'\u30e3': return {U'\uff6c', U'\0'};
        case U'\u30e5': return {U'\uff6d', U'\0'};
        case U'\u30e7': return {U'\uff6e', U'\0'};
        case U'\u30c3': return {U'\uff6f', U'\0'};
        case U'\u30fc': return {U'\uff70', U'\0'};
        case U'\u30a2': return {U'\uff71', U'\0'};
        case U'\u30a4': return {U'\uff72', U'\0'};
        case U'\u30a6': return {U'\uff73', U'\0'};
        case U'\u30a8': return {U'\uff74', U'\0'};
        case U'\u30aa': return {U'\uff75', U'\0'};
        case U'\u30ab': return {U'\uff76', U'\0'};
        case U'\u30ad': return {U'\uff77', U'\0'};
        case U'\u30af': return {U'\uff78', U'\0'};
        case U'\u30b1': return {U'\uff79', U'\0'};
        case U'\u30b3': return {U'\uff7a', U'\0'};
        case U'\u30b5': return {U'\uff7b', U'\0'};
        case U'\u30b7': return {U'\uff7c', U'\0'};
        case U'\u30b9': return {U'\uff7d', U'\0'};
        case U'\u30bb': return {U'\uff7e', U'\0'};
        case U'\u30bd': return {U'\uff7f', U'\0'};
        case U'\u30bf': return {U'\uff80', U'\0'};
        case U'\u30c1': return {U'\uff81', U'\0'};
        case U'\u30c4': return {U'\uff82', U'\0'};
        case U'\u30c6': return {U'\uff83', U'\0'};
        case U'\u30c8': return {U'\uff84', U'\0'};
        case U'\u30ca': return {U'\uff85', U'\0'};
        case U'\u30cb': return {U'\uff86', U'\0'};
        case U'\u30cc': return {U'\uff87', U'\0'};
        case U'\u30cd': return {U'\uff88', U'\0'};
        case U'\u30ce': return {U'\uff89', U'\0'};
        case U'\u30cf': return {U'\uff8a', U'\0'};
        case U'\u30d2': return {U'\uff8b', U'\0'};
        case U'\u30d5': return {U'\uff8c', U'\0'};
        case U'\u30d8': return {U'\uff8d', U'\0'};
        case U'\u30db': return {U'\uff8e', U'\0'};
        case U'\u30de': return {U'\uff8f', U'\0'};
        case U'\u30df': return {U'\uff90', U'\0'};
        case U'\u30e0': return {U'\uff91', U'\0'};
        case U'\u30e1': return {U'\uff92', U'\0'};
        case U'\u30e2': return {U'\uff93', U'\0'};
        case U'\u30e4': return {U'\uff94', U'\0'};
        case U'\u30e6': return {U'\uff95', U'\0'};
        case U'\u30e8': return {U'\uff96', U'\0'};
        case U'\u30e9': return {U'\uff97', U'\0'};
        case U'\u30ea': return {U'\uff98', U'\0'};
        case U'\u30eb': return {U'\uff99', U'\0'};
        case U'\u30ec': return {U'\uff9a', U'\0'};
        case U'\u30ed': return {U'\uff9b', U'\0'};
        case U'\u30ef': return {U'\uff9c', U'\0'};
        case U'\u30f3': return {U'\uff9d', U'\0'};
        case U'\u309b': return {U'\uff9e', U'\0'};
        case U'\u309c': return {U'\uff9f', U'\0'};
        case U'\u30f4': return {U'\uff73', U'\uff9e'};
        case U'\u30ac': return {U'\uff76', U'\uff9e'};
        case U'\u30ae': return {U'\uff77', U'\uff9e'};
        case U'\u30b0': return {U'\uff78', U'\uff9e'};
        case U'\u30b2': return {U'\uff79', U'\uff9e'};
        case U'\u30b4': return {U'\uff7a', U'\uff9e'};
        case U'\u30b6': return {U'\uff7b', U'\uff9e'};
        case U'\u30b8': return {U'\uff7c', U'\uff9e'};
        case U'\u30ba': return {U'\uff7d', U'\uff9e'};
        case U'\u30bc': return {U'\uff7e', U'\uff9e'};
        case U'\u30be': return {U'\uff7f', U'\uff9e'};
        case U'\u30c0': return {U'\uff80', U'\uff9e'};
        case U'\u30c2': return {U'\uff81', U'\uff9e'};
        case U'\u30c5': return {U'\uff82', U'\uff9e'};
        case U'\u30c7': return {U'\uff83', U'\uff9e'};
        case U'\u30c9': return {U'\uff84', U'\uff9e'};
        case U'\u30d0': return {U'\uff8a', U'\uff9e'};
        case U'\u30d3': return {U'\uff8b', U'\uff9e'};
        case U'\u30d6': return {U'\uff8c', U'\uff9e'};
        case U'\u30d9': return {U'\uff8d', U'\uff9e'};
        case U'\u30dc': return {U'\uff8e', U'\uff9e'};
        case U'\u30d1': return {U'\uff8a', U'\uff9f'};
        case U'\u30d4': return {U'\uff8b', U'\uff9f'};
        case U'\u30d7': return {U'\uff8c', U'\uff9f'};
        case U'\u30da': return {U'\uff8d', U'\uff9f'};
        case U'\u30dd': return {U'\uff8e', U'\uff9f'};
        case U'\u30f7': return {U'\uff9c', U'\uff9e'};
        case U'\u30fa': return {U'\uff66', U'\uff9e'};
        default: return {c, U'\0'};
    }
}

[[nodiscard]] constexpr auto to_fullwidth_kana(char32_t c) noexcept
    -> result<char32_t>
{
    return halfwidth_kana_to_fullwidth_base(c);
}

[[nodiscard]] constexpr auto to_halfwidth_kana(char32_t c) noexcept
    -> result<char32_t>
{
    return fullwidth_kana_to_halfwidth(c).base;
}

} // namespace xer::detail

#endif /* XER_BITS_KANA_WIDTH_H_INCLUDED_ */
