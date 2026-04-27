/**
 * @file examples/example_cmdline.cpp
 * @brief Example for xer::cmdline and xer::parse_arg.
 */

// XER_EXAMPLE_BEGIN: cmdline_basic
//
// This example reads the current process command-line arguments and parses
// each argument as either a simple long option or an ordinary value.
//
// Example:
//   ./example_cmdline --name=xer --verbose input.txt -x
//
// Example output:
//   arg[1]: option name = xer
//   arg[2]: option verbose =
//   arg[3]: value input.txt
//   arg[4]: value -x

#include <cstddef>

#include <xer/cmdline.h>
#include <xer/stdio.h>

auto main() -> int
{
    const auto line = xer::get_cmdline();
    if (!line.has_value()) {
        return 1;
    }

    for (std::size_t i = 1; i < line->size(); ++i) {
        const auto raw = line->at(i);
        if (!raw.has_value()) {
            return 1;
        }

        const auto parsed = xer::parse_arg(*raw);

        if (!parsed.first.empty()) {
            if (!xer::printf(
                    u8"arg[%@]: option %@ = %@\n",
                    i,
                    parsed.first,
                    parsed.second)
                     .has_value()) {
                return 1;
            }

            continue;
        }

        if (!xer::printf(
                u8"arg[%@]: value %@\n",
                i,
                parsed.second)
                 .has_value()) {
            return 1;
        }
    }

    return 0;
}

// XER_EXAMPLE_END: cmdline_basic
