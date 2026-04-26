/**
 * @file examples/example_chdir_getcwd.cpp
 * @brief Example for xer::chdir and xer::getcwd.
 */

// XER_EXAMPLE_BEGIN: chdir_getcwd_basic
//
// This example gets the current working directory, changes it to a temporary
// directory, and then restores the original working directory.
//
// Expected output:
// original: <current working directory>
// current:  <current working directory>/example_chdir_getcwd_tmp

#include <xer/stdio.h>

auto main() -> int
{
    const auto original = xer::getcwd();
    if (!original.has_value()) {
        return 1;
    }

    const xer::path temp_dir(u8"example_chdir_getcwd_tmp");

    if (xer::is_dir(temp_dir)) {
        if (!xer::rmdir(temp_dir).has_value()) {
            return 1;
        }
    }

    if (!xer::mkdir(temp_dir).has_value()) {
        return 1;
    }

    if (!xer::printf(u8"original: %@\n", original->str()).has_value()) {
        return 1;
    }

    if (!xer::chdir(temp_dir).has_value()) {
        return 1;
    }

    const auto current = xer::getcwd();
    if (!current.has_value()) {
        static_cast<void>(xer::chdir(*original));
        return 1;
    }

    if (!xer::printf(u8"current:  %@\n", current->str()).has_value()) {
        static_cast<void>(xer::chdir(*original));
        return 1;
    }

    if (!xer::chdir(*original).has_value()) {
        return 1;
    }

    if (!xer::rmdir(temp_dir).has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: chdir_getcwd_basic
