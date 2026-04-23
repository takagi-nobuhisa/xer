// XER_EXAMPLE_BEGIN: rename_remove_basic
//
// This example creates a temporary file, renames it,
// and then removes it.
//
// Expected output:
// renamed and removed

#include <xer/path.h>
#include <xer/stdio.h>

auto main() -> int
{
    const xer::path original(u8"example_rename_remove_original.tmp");
    const xer::path renamed(u8"example_rename_remove_renamed.tmp");

    {
        auto file = xer::fopen(original, "wb");
        if (!file) {
            return 1;
        }

        if (!xer::fclose(*file)) {
            return 1;
        }
    }

    if (!xer::rename(original, renamed)) {
        return 1;
    }

    if (!xer::remove(renamed)) {
        return 1;
    }

    if (!xer::puts(u8"renamed and removed")) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: rename_remove_basic
