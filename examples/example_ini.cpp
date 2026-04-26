#include <xer/ini.h>
#include <xer/stdio.h>

#include <string>

auto main() -> int
{
    // XER_EXAMPLE_BEGIN: ini_basic
    //
    // This example decodes INI text, reads values from the decoded structure,
    // and encodes the structure back into INI text.
    //
    // Expected output:
    // title = sample
    // name = xer
    // version = 0.2.0a3
    // encoded:
    // title=sample
    //
    // [project]
    // name=xer
    // version=0.2.0a3

    const auto decoded = xer::ini_decode(
        u8"title = sample\n"
        u8"\n"
        u8"[project]\n"
        u8"name = xer\n"
        u8"version = 0.2.0a3\n");

    if (!decoded.has_value()) {
        return 1;
    }

    if (decoded->entries.empty()) {
        return 1;
    }

    if (decoded->sections.empty()) {
        return 1;
    }

    const auto& global_entry = decoded->entries[0];
    const auto& project = decoded->sections[0];

    if (project.entries.size() < 2) {
        return 1;
    }

    if (!xer::printf(u8"title = %@\n", global_entry.value).has_value()) {
        return 1;
    }

    if (!xer::printf(u8"name = %@\n", project.entries[0].value).has_value()) {
        return 1;
    }

    if (!xer::printf(u8"version = %@\n", project.entries[1].value).has_value()) {
        return 1;
    }

    const auto encoded = xer::ini_encode(*decoded);
    if (!encoded.has_value()) {
        return 1;
    }

    if (!xer::puts(u8"encoded:").has_value()) {
        return 1;
    }

    if (!xer::fputs(*encoded, xer_stdout).has_value()) {
        return 1;
    }

    // XER_EXAMPLE_END: ini_basic

    return 0;
}
