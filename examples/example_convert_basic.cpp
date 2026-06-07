// XER_EXAMPLE_BEGIN: convert_basic
//
// This example converts values by using xer::to.
//
// Expected output:
// count = 42
// path = data/out.txt
// text = 3.5

#include <string>

#include <xer/convert.h>
#include <xer/path.h>
#include <xer/stdio.h>

namespace {

template<class T>
auto fail_if_error(const xer::result<T>& result) -> bool
{
    return !result.has_value();
}

} // namespace

auto main() -> int
{
    const auto count = xer::to<int>(u8"42");
    const auto path = xer::to<xer::path>(u8"data\\out.txt");
    const auto text = xer::to<std::u8string>(3.5);

    if (fail_if_error(count) || fail_if_error(path) || fail_if_error(text)) {
        return 1;
    }

    if (!xer::printf(u8"count = %@\n", *count)) {
        return 1;
    }

    if (!xer::printf(u8"path = %s\n", path->str().data())) {
        return 1;
    }

    if (!xer::printf(u8"text = %s\n", text->c_str())) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: convert_basic
