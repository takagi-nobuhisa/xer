// XER_EXAMPLE_BEGIN: fgetcsv_basic
//
// This example reads one CSV record from a UTF-8 string
// by using xer::fgetcsv and prints the parsed fields.
//
// Expected output:
// field count = 3
// field[0] = apple
// field[1] = red,green
// field[2] = 42

#include <xer/stdio.h>

auto main() -> int
{
    auto stream = xer::stropen(u8"apple,\"red,green\",42\n", "r");
    if (!stream) {
        return 1;
    }

    const auto fields = xer::fgetcsv(*stream);
    if (!fields) {
        return 1;
    }

    if (!xer::printf(u8"field count = %d\n", static_cast<int>(fields->size()))) {
        return 1;
    }

    for (std::size_t i = 0; i < fields->size(); ++i) {
        if (!xer::printf(u8"field[%d] = %s\n",
                         static_cast<int>(i),
                         (*fields)[i].c_str())) {
            return 1;
        }
    }

    return 0;
}

// XER_EXAMPLE_END: fgetcsv_basic
