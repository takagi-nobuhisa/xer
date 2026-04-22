// XER_EXAMPLE_BEGIN: trim_basic

#include <xer/stdio.h>
#include <xer/string.h>

auto main() -> int
{
    if (const auto trimmed = xer::trim_view(u8"  hello  ")) {
        if (!xer::puts(trimmed.value())) {
            return 1;
        }
    }

    return 0;
}

// XER_EXAMPLE_END: trim_basic
