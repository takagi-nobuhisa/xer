// XER_EXAMPLE_BEGIN: environs_basic
//
// This example obtains a snapshot of all environment variables and prints
// the number of entries. It also looks up PATH in that snapshot.
//
// Expected output:
// The program prints the number of environment variables.
// If PATH exists, it also prints PATH.

#include <xer/stdio.h>
#include <xer/stdlib.h>

#include <cstddef>

namespace {

auto print_size(std::size_t size) -> bool
{
    return xer::printf(u8"environment entries: %@\n", size).has_value();
}

} // namespace

auto main() -> int
{
    const auto environment = xer::get_environs();
    if (!environment.has_value()) {
        return 1;
    }

    if (!print_size(environment->size())) {
        return 1;
    }

    const auto path = environment->find(u8"PATH");
    if (path.has_value()) {
        if (!xer::fputs(u8"PATH = ", xer_stdout)) {
            return 1;
        }

        if (!xer::puts(*path)) {
            return 1;
        }
    }

    return 0;
}

// XER_EXAMPLE_END: environs_basic
