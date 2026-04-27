/**
 * @file examples/example_realpath.cpp
 * @brief Example for xer::realpath.
 */

// XER_EXAMPLE_BEGIN: realpath_basic
//
// This example canonicalizes an existing relative path.
//
// Expected output:
// resolved: <absolute path to example_realpath_tmp>

#include <xer/stdio.h>

auto main() -> int
{
    const xer::path temp_dir(u8"example_realpath_tmp");

    if (xer::is_dir(temp_dir)) {
        if (!xer::rmdir(temp_dir).has_value()) {
            return 1;
        }
    }

    if (!xer::mkdir(temp_dir).has_value()) {
        return 1;
    }

    const auto resolved = xer::realpath(temp_dir);
    if (!resolved.has_value()) {
        static_cast<void>(xer::rmdir(temp_dir));
        return 1;
    }

    if (!xer::printf(u8"resolved: %@\n", resolved->str()).has_value()) {
        static_cast<void>(xer::rmdir(temp_dir));
        return 1;
    }

    if (!xer::rmdir(temp_dir).has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: realpath_basic
