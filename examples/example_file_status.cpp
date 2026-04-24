// XER_EXAMPLE_BEGIN: file_status_basic
//
// This example creates a file and a directory, then checks them with
// PHP-style file status predicates.
//
// Expected output:
// file exists
// file is a regular file
// directory exists
// directory is a directory
// file is readable
// file is writable
// missing path does not exist

#include <xer/path.h>
#include <xer/stdio.h>

auto main() -> int
{
    const xer::path file(u8"example_file_status.txt");
    const xer::path directory(u8"example_file_status_dir");
    const xer::path missing(u8"example_file_status_missing.txt");

    if (xer::file_exists(file)) {
        if (!xer::remove(file)) {
            return 1;
        }
    }

    if (xer::file_exists(directory)) {
        if (!xer::rmdir(directory)) {
            return 1;
        }
    }

    {
        auto stream = xer::fopen(file, "w", xer::encoding_t::utf8);
        if (!stream) {
            return 1;
        }

        if (!xer::fputs(u8"hello", *stream)) {
            return 1;
        }

        if (!xer::fclose(*stream)) {
            return 1;
        }
    }

    if (!xer::mkdir(directory)) {
        return 1;
    }

    if (xer::file_exists(file)) {
        if (!xer::puts(u8"file exists")) {
            return 1;
        }
    }

    if (xer::is_file(file)) {
        if (!xer::puts(u8"file is a regular file")) {
            return 1;
        }
    }

    if (xer::file_exists(directory)) {
        if (!xer::puts(u8"directory exists")) {
            return 1;
        }
    }

    if (xer::is_dir(directory)) {
        if (!xer::puts(u8"directory is a directory")) {
            return 1;
        }
    }

    if (xer::is_readable(file)) {
        if (!xer::puts(u8"file is readable")) {
            return 1;
        }
    }

    if (xer::is_writable(file)) {
        if (!xer::puts(u8"file is writable")) {
            return 1;
        }
    }

    if (!xer::file_exists(missing)) {
        if (!xer::puts(u8"missing path does not exist")) {
            return 1;
        }
    }

    if (!xer::remove(file)) {
        return 1;
    }

    if (!xer::rmdir(directory)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: file_status_basic
