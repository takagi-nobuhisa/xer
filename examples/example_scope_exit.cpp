/**
 * @file examples/example_scope_exit.cpp
 * @brief Example for xer::scope_exit.
 */

// XER_EXAMPLE_BEGIN: scope_exit_basic
//
// This example uses scope_exit to restore the current working directory when
// leaving a block.
//
// Expected output:
// restored: yes

#include <xer/scope.h>
#include <xer/stdio.h>

auto main() -> int
{
    const auto original = xer::getcwd();
    if (!original.has_value()) {
        return 1;
    }

    const xer::path temp_dir(u8"example_scope_exit_tmp");

    if (xer::is_dir(temp_dir)) {
        if (!xer::rmdir(temp_dir).has_value()) {
            return 1;
        }
    }

    if (!xer::mkdir(temp_dir).has_value()) {
        return 1;
    }

    {
        auto restore_current_directory = xer::scope_exit([&] noexcept {
            static_cast<void>(xer::chdir(*original));
        });

        if (!xer::chdir(temp_dir).has_value()) {
            return 1;
        }

        const auto current = xer::getcwd();
        if (!current.has_value()) {
            return 1;
        }
    }

    const auto restored = xer::getcwd();
    if (!restored.has_value()) {
        static_cast<void>(xer::rmdir(temp_dir));
        return 1;
    }

    const bool is_restored = restored->str() == original->str();

    if (!xer::printf(u8"restored: %@\n", is_restored ? u8"yes" : u8"no").has_value()) {
        static_cast<void>(xer::rmdir(temp_dir));
        return 1;
    }

    if (!xer::rmdir(temp_dir).has_value()) {
        return 1;
    }

    return is_restored ? 0 : 1;
}

// XER_EXAMPLE_END: scope_exit_basic
