// XER_EXAMPLE_BEGIN: sscanf_basic
//
// This example parses an integer and a UTF-8 word
// from a UTF-8 string by using xer::sscanf.
//
// Expected output:
// count = 2
// number = 42
// word = hello

#include <string>

#include <xer/stdio.h>

auto main() -> int
{
    int number = 0;
    std::u8string word;

    const auto count = xer::sscanf(u8"42 hello", u8"%d %s", &number, &word);
    if (!count) {
        return 1;
    }

    if (!xer::printf(u8"count = %d\n", static_cast<int>(*count))) {
        return 1;
    }

    if (!xer::printf(u8"number = %d\n", number)) {
        return 1;
    }

    if (!xer::printf(u8"word = %s\n", word.c_str())) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: sscanf_basic
