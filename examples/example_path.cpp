// XER_EXAMPLE_BEGIN: path_basic
//
// This example shows basic path handling with xer::path.
//
// Expected output:
// normalized = C:/work/xer/archive.tar.gz
// joined     = C:/work/xer/docs/readme.md
// basename   = archive.tar.gz
// extension  = .tar.gz
// stem       = archive
// parent     = C:/work/xer
// absolute   = true

#include <xer/path.h>
#include <xer/stdio.h>

namespace {

auto print_line(std::u8string_view label, std::u8string_view value) -> int
{
    if (!xer::fputs(label, xer_stdout)) {
        return 1;
    }

    if (!xer::fputs(value, xer_stdout)) {
        return 1;
    }

    if (!xer::puts(u8"")) {
        return 1;
    }

    return 0;
}

auto print_bool(std::u8string_view label, bool value) -> int
{
    return print_line(label, value ? u8"true" : u8"false");
}

} // namespace

auto main() -> int
{
    const xer::path source(u8"C:\\work\\xer\\archive.tar.gz");
    if (print_line(u8"normalized = ", source.str()) != 0) {
        return 1;
    }

    const xer::path base(u8"C:/work/xer");
    const auto joined = base / xer::path(u8"docs/readme.md");
    if (!joined) {
        return 1;
    }

    if (print_line(u8"joined     = ", joined->str()) != 0) {
        return 1;
    }

    if (print_line(u8"basename   = ", xer::basename(source)) != 0) {
        return 1;
    }

    if (print_line(u8"extension  = ", xer::extension(source)) != 0) {
        return 1;
    }

    if (print_line(u8"stem       = ", xer::stem(source)) != 0) {
        return 1;
    }

    const auto parent = xer::parent_path(source);
    if (!parent) {
        return 1;
    }

    if (print_line(u8"parent     = ", parent->str()) != 0) {
        return 1;
    }

    if (print_bool(u8"absolute   = ", xer::is_absolute(source)) != 0) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: path_basic
