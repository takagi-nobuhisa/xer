/**
 * @file tests/test_version.cpp
 * @brief Tests for xer/version.h.
 */

#include <string>
#include <string_view>

#include <xer/version.h>

namespace {

void test_version_macro_values()
{
    static_assert(XER_VERSION_MAJOR >= 0);
    static_assert(XER_VERSION_MINOR >= 0);
    static_assert(XER_VERSION_PATCH >= 0);

    static_assert(XER_VERSION_MAJOR == xer::version_major);
    static_assert(XER_VERSION_MINOR == xer::version_minor);
    static_assert(XER_VERSION_PATCH == xer::version_patch);

    static_assert(std::string_view(XER_VERSION_SUFFIX) == xer::version_suffix);
    static_assert(std::string_view(XER_VERSION_STRING) == xer::version_string);
}

void test_version_string_consistency()
{
    const auto expected =
        std::to_string(xer::version_major) + "." +
        std::to_string(xer::version_minor) + "." +
        std::to_string(xer::version_patch) +
        std::string(xer::version_suffix);

    if (expected != std::string(xer::version_string)) {
        throw "test_version_string_consistency: version string mismatch";
    }
}

void test_version_suffix_consistency()
{
    const auto numeric_prefix =
        std::to_string(xer::version_major) + "." +
        std::to_string(xer::version_minor) + "." +
        std::to_string(xer::version_patch);

    const auto version = std::string(xer::version_string);

    if (!version.starts_with(numeric_prefix)) {
        throw "test_version_suffix_consistency: numeric prefix mismatch";
    }

    const auto suffix = version.substr(numeric_prefix.size());

    if (suffix != std::string(xer::version_suffix)) {
        throw "test_version_suffix_consistency: suffix mismatch";
    }
}

} // namespace

auto main() -> int
{
    test_version_macro_values();
    test_version_string_consistency();
    test_version_suffix_consistency();

    return 0;
}
