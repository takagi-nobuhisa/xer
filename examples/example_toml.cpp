#include <xer/stdio.h>
#include <xer/toml.h>

#include <cstdint>
#include <string_view>

namespace {

auto find_key(const xer::toml_table& table, std::u8string_view key)
    -> const xer::toml_value*
{
    for (const auto& entry : table) {
        if (entry.first == key) {
            return &entry.second;
        }
    }

    return nullptr;
}

} // namespace

auto main() -> int
{
    // XER_EXAMPLE_BEGIN: toml_basic
    //
    // This example decodes TOML text, reads values from the decoded structure,
    // and encodes the structure back into TOML text.
    //
    // Expected output:
    // title = sample
    // name = xer
    // version = 0.2.0a3
    // encoded:
    // title = "sample"
    // enabled = true
    // count = 123
    // ports = [8000, 8001]
    //
    // [project]
    // name = "xer"
    // version = "0.2.0a3"

    const auto decoded = xer::toml_decode(
        u8"title = \"sample\"\n"
        u8"enabled = true\n"
        u8"count = 123\n"
        u8"ports = [8000, 8001]\n"
        u8"\n"
        u8"[project]\n"
        u8"name = \"xer\"\n"
        u8"version = \"0.2.0a3\"\n");

    if (!decoded.has_value()) {
        return 1;
    }

    const auto* root = decoded->as_table();
    if (root == nullptr) {
        return 1;
    }

    const auto* title = find_key(*root, u8"title");
    const auto* project = find_key(*root, u8"project");
    if (title == nullptr || project == nullptr || !title->is_string() ||
        !project->is_table()) {
        return 1;
    }

    const auto* project_table = project->as_table();
    if (project_table == nullptr) {
        return 1;
    }

    const auto* name = find_key(*project_table, u8"name");
    const auto* version = find_key(*project_table, u8"version");
    if (name == nullptr || version == nullptr || !name->is_string() ||
        !version->is_string()) {
        return 1;
    }

    if (!xer::printf(u8"title = %@\n", *title->as_string()).has_value()) {
        return 1;
    }

    if (!xer::printf(u8"name = %@\n", *name->as_string()).has_value()) {
        return 1;
    }

    if (!xer::printf(u8"version = %@\n", *version->as_string()).has_value()) {
        return 1;
    }

    const auto encoded = xer::toml_encode(*decoded);
    if (!encoded.has_value()) {
        return 1;
    }

    if (!xer::puts(u8"encoded:").has_value()) {
        return 1;
    }

    if (!xer::fputs(*encoded, xer_stdout).has_value()) {
        return 1;
    }

    // XER_EXAMPLE_END: toml_basic

    return 0;
}
