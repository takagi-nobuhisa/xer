// XER_EXAMPLE_BEGIN: mkdir_rmdir_basic
//
// This example creates a directory and removes it.
//
// Expected output:
// created and removed

#include <xer/path.h>
#include <xer/stdio.h>

auto main() -> int
{
    const xer::path directory(u8"example_mkdir_rmdir_dir");

    if (!xer::mkdir(directory)) {
        return 1;
    }

    if (!xer::rmdir(directory)) {
        return 1;
    }

    if (!xer::puts(u8"created and removed")) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: mkdir_rmdir_basic
