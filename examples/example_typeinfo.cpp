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
// int
// std::pair<int, long>
// sample_object

#include <utility>

#include <xer/stdio.h>
#include <xer/typeinfo.h>

namespace {

struct sample_object {
};

auto print_type_name(xer::type_info type) -> int
{
    const auto result = xer::puts(type.name());
    if (!result.has_value()) {
        return 1;
    }

    return 0;
}

} // namespace

auto main() -> int
{
    if (print_type_name(xer_typeid(int)) != 0) {
        return 1;
    }

    if (print_type_name(xer_typeid(std::pair<int, long>)) != 0) {
        return 1;
    }

    const sample_object object;
    if (print_type_name(xer_typeid(object)) != 0) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: typeinfo_basic
