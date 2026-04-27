/**
 * @file tests/test_realpath.cpp
 * @brief Tests for realpath.
 */

#include <string_view>

#include <xer/assert.h>
#include <xer/stdio.h>

namespace {

void test_realpath_existing_directory()
{
    const xer::path temp_dir(u8"test_realpath_tmp");
    if (xer::is_dir(temp_dir)) {
        const auto removed = xer::rmdir(temp_dir);
        xer_assert(removed.has_value());
    }

    const auto created = xer::mkdir(temp_dir);
    xer_assert(created.has_value());

    const auto resolved = xer::realpath(temp_dir);
    xer_assert(resolved.has_value());
    xer_assert(xer::is_absolute(*resolved));
    xer_assert_eq(xer::basename(*resolved), std::u8string_view(u8"test_realpath_tmp"));

    const auto removed = xer::rmdir(temp_dir);
    xer_assert(removed.has_value());
}

void test_realpath_dot_returns_current_directory()
{
    const auto current = xer::getcwd();
    xer_assert(current.has_value());

    const auto resolved_current = xer::realpath(*current);
    xer_assert(resolved_current.has_value());

    const auto resolved_dot = xer::realpath(xer::path(u8"."));
    xer_assert(resolved_dot.has_value());

    xer_assert_eq(resolved_dot->str(), resolved_current->str());
}

void test_realpath_parent_component()
{
    const xer::path temp_dir(u8"test_realpath_tmp");

    const auto nested_dir_result = temp_dir / xer::path(u8"nested");
    xer_assert(nested_dir_result.has_value());
    const xer::path nested_dir = *nested_dir_result;

    if (xer::is_dir(nested_dir)) {
        const auto removed = xer::rmdir(nested_dir);
        xer_assert(removed.has_value());
    }

    if (xer::is_dir(temp_dir)) {
        const auto removed = xer::rmdir(temp_dir);
        xer_assert(removed.has_value());
    }

    const auto created_temp = xer::mkdir(temp_dir);
    xer_assert(created_temp.has_value());

    const auto created_nested = xer::mkdir(nested_dir);
    xer_assert(created_nested.has_value());

    const auto resolved_temp = xer::realpath(temp_dir);
    xer_assert(resolved_temp.has_value());

    const auto parent_path_result = temp_dir / xer::path(u8"nested/..");
    xer_assert(parent_path_result.has_value());

    const auto resolved_parent = xer::realpath(*parent_path_result);
    xer_assert(resolved_parent.has_value());

    xer_assert_eq(resolved_parent->str(), resolved_temp->str());

    const auto removed_nested = xer::rmdir(nested_dir);
    xer_assert(removed_nested.has_value());

    const auto removed_temp = xer::rmdir(temp_dir);
    xer_assert(removed_temp.has_value());
}

void test_realpath_missing_path_fails()
{
    const auto resolved = xer::realpath(xer::path(u8"test_realpath_missing"));

    xer_assert_not(resolved.has_value());
}

} // namespace

auto main() -> int
{
    test_realpath_existing_directory();
    test_realpath_dot_returns_current_directory();
    test_realpath_parent_component();
    test_realpath_missing_path_fails();

    return 0;
}
