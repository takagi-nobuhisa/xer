// XER_EXAMPLE_BEGIN: dirent_basic
//
// This example opens a directory, reads entry names, and rewinds the directory
// stream.
//
// Expected output:
// entries:
// file_a.txt
// file_b.txt
// rewound

#include <algorithm>
#include <vector>

#include <xer/dirent.h>
#include <xer/stdio.h>

auto print_entry(std::u8string_view name) -> int
{
    if (name == u8"." || name == u8"..") {
        return 0;
    }

    if (!xer::puts(name)) {
        return 1;
    }

    return 0;
}

auto main() -> int
{
    const xer::path directory(u8"example_dirent_dir");
    const xer::path file_a(u8"example_dirent_dir/file_a.txt");
    const xer::path file_b(u8"example_dirent_dir/file_b.txt");

    if (xer::file_exists(file_a)) {
        if (!xer::remove(file_a)) {
            return 1;
        }
    }

    if (xer::file_exists(file_b)) {
        if (!xer::remove(file_b)) {
            return 1;
        }
    }

    if (xer::file_exists(directory)) {
        if (!xer::rmdir(directory)) {
            return 1;
        }
    }

    if (!xer::mkdir(directory)) {
        return 1;
    }

    {
        auto stream = xer::fopen(file_a, "w", xer::encoding_t::utf8);
        if (!stream) {
            return 1;
        }

        if (!xer::fputs(u8"a", *stream)) {
            return 1;
        }

        if (!xer::fclose(*stream)) {
            return 1;
        }
    }

    {
        auto stream = xer::fopen(file_b, "w", xer::encoding_t::utf8);
        if (!stream) {
            return 1;
        }

        if (!xer::fputs(u8"b", *stream)) {
            return 1;
        }

        if (!xer::fclose(*stream)) {
            return 1;
        }
    }

    auto dir = xer::opendir(directory);
    if (!dir) {
        return 1;
    }

    std::vector<std::u8string> entries;

    for (;;) {
        auto entry = xer::readdir(*dir);
        if (!entry) {
            if (entry.error().code != xer::error_t::not_found) {
                return 1;
            }

            break;
        }

        if (*entry != u8"." && *entry != u8"..") {
            entries.push_back(*entry);
        }
    }

    std::ranges::sort(entries);

    if (!xer::puts(u8"entries:")) {
        return 1;
    }

    for (const auto& entry : entries) {
        if (print_entry(entry) != 0) {
            return 1;
        }
    }

    if (!xer::rewinddir(*dir)) {
        return 1;
    }

    if (!xer::puts(u8"rewound")) {
        return 1;
    }

    if (!xer::closedir(*dir)) {
        return 1;
    }

    if (!xer::remove(file_a)) {
        return 1;
    }

    if (!xer::remove(file_b)) {
        return 1;
    }

    if (!xer::rmdir(directory)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: dirent_basic
