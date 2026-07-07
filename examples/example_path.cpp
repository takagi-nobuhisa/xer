// XER_EXAMPLE_BEGIN: path_basic
//
// This example shows basic path handling with xer::path.
//
// Expected output:
// normalized: C:/work/xer/archive.tar.gz
// joined: C:/work/xer/docs/readme.md
// basename: archive.tar.gz
// extension: .tar.gz
// stem: archive
// parent: C:/work/xer
// absolute: true

#include <xer/path.h>
#include <xer/diagnostics.h>


auto main() -> int
{
    const xer::path source(u8"C:\\work\\xer\\archive.tar.gz");
    if (!xer_print(u8"normalized", source.str())) {
        return 1;
    }

    const xer::path base(u8"C:/work/xer");
    const auto joined = base / xer::path(u8"docs/readme.md");
    if (!joined) {
        return 1;
    }

    if (!xer_print(u8"joined", joined->str())) {
        return 1;
    }

    if (!xer_print(u8"basename", xer::basename(source))) {
        return 1;
    }

    if (!xer_print(u8"extension", xer::extension(source))) {
        return 1;
    }

    if (!xer_print(u8"stem", xer::stem(source))) {
        return 1;
    }

    const auto parent = xer::parent_path(source);
    if (!parent) {
        return 1;
    }

    if (!xer_print(u8"parent", parent->str())) {
        return 1;
    }

    if (!xer_print(u8"absolute", xer::is_absolute(source))) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: path_basic
