/**
 * @file tests/test_chdir_getcwd.cpp
 * @brief Tests for chdir and getcwd.
 */

#include <string>

#include <xer/assert.h>
#include <xer/stdio.h>

namespace {

class current_directory_guard {
public:
    current_directory_guard()
        : original_(xer::getcwd())
    {
        xer_assert(original_.has_value());
    }

    current_directory_guard(const current_directory_guard&) = delete;
    auto operator=(const current_directory_guard&) -> current_directory_guard& = delete;

    ~current_directory_guard()
    {
        if (original_.has_value()) {
            static_cast<void>(xer::chdir(*original_));
        }
    }

private:
    xer::result<xer::path> original_;
};

void test_getcwd_returns_absolute_path()
{
    const auto current = xer::getcwd();

    xer_assert(current.has_value());
    xer_assert(xer::is_absolute(*current));
}

void test_chdir_relative_directory()
{
    current_directory_guard guard;

    const xer::path temp_dir(u8"test_chdir_getcwd_tmp");
    if (xer::is_dir(temp_dir)) {
        const auto removed = xer::rmdir(temp_dir);
        xer_assert(removed.has_value());
    }

    const auto created = xer::mkdir(temp_dir);
    xer_assert(created.has_value());

    const auto changed = xer::chdir(temp_dir);
    xer_assert(changed.has_value());

    const auto current = xer::getcwd();
    xer_assert(current.has_value());
    xer_assert_eq(
        xer::basename(*current),
        std::u8string_view(u8"test_chdir_getcwd_tmp"));

    const auto restored = xer::chdir(xer::path(u8".."));
    xer_assert(restored.has_value());

    const auto removed = xer::rmdir(temp_dir);
    xer_assert(removed.has_value());
}

void test_chdir_missing_directory_fails()
{
    const auto result = xer::chdir(xer::path(u8"test_chdir_getcwd_missing"));

    xer_assert_not(result.has_value());
}

} // namespace

auto main() -> int
{
    test_getcwd_returns_absolute_path();
    test_chdir_relative_directory();
    test_chdir_missing_directory_fails();

    return 0;
}
