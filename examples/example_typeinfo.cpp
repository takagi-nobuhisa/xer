/**
 * @file examples/example_typeinfo.cpp
 * @brief Example of XER type information helpers.
 */

// XER_EXAMPLE_BEGIN: typeinfo_basic
//
// This example gets type information with xer_typeid and prints demangled
// type names.
//
// Expected output:
// int: int
// std::pair<int, long>: std::pair<int, long>
// object: sample_object

#include <utility>

#include <xer/diagnostics.h>
#include <xer/typeinfo.h>

namespace {

struct sample_object {
};

} // namespace

auto main() -> int
{
    if (!xer_print(u8"int", xer_typeid(int).name())) {
        return 1;
    }

    if (!xer_print(u8"std::pair<int, long>", xer_typeid(std::pair<int, long>).name())) {
        return 1;
    }

    const sample_object object;
    if (!xer_print(u8"object", xer_typeid(object).name())) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: typeinfo_basic
