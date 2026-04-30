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
    // XER_EXAMPLE_BEGIN: toml_keys_tables
    //
    // This example decodes quoted keys, dotted keys, and nested tables.
    //
    // Expected output:
    // site.name = xer
    // port = 8080
    // package = core

    const auto decoded = xer::toml_decode(
        u8"\"site.name\" = \"xer\"\n"
        u8"server.port = 8080\n"
        u8"\n"
        u8"[project.package]\n"
        u8"name = \"core\"\n");

    if (!decoded.has_value()) {
        return 1;
    }

    const auto* root = decoded->as_table();
    if (root == nullptr) {
        return 1;
    }

    const auto* site_name = find_key(*root, u8"site.name");
    const auto* server = find_key(*root, u8"server");
    const auto* project = find_key(*root, u8"project");

    if (site_name == nullptr || server == nullptr || project == nullptr) {
        return 1;
    }

    const auto* server_table = server->as_table();
    const auto* project_table = project->as_table();
    if (server_table == nullptr || project_table == nullptr) {
        return 1;
    }

    const auto* port = find_key(*server_table, u8"port");
    const auto* package = find_key(*project_table, u8"package");
    if (port == nullptr || package == nullptr) {
        return 1;
    }

    const auto* package_table = package->as_table();
    if (package_table == nullptr) {
        return 1;
    }

    const auto* package_name = find_key(*package_table, u8"name");
    if (package_name == nullptr || !site_name->is_string() ||
        !port->is_integer() || !package_name->is_string()) {
        return 1;
    }

    if (!xer::printf(u8"site.name = %@\n", *site_name->as_string()).has_value()) {
        return 1;
    }

    if (!xer::printf(
            u8"port = %@\n",
            static_cast<long long>(*port->as_integer()))
             .has_value()) {
        return 1;
    }

    if (!xer::printf(u8"package = %@\n", *package_name->as_string()).has_value()) {
        return 1;
    }

    // XER_EXAMPLE_END: toml_keys_tables

    return 0;
}
