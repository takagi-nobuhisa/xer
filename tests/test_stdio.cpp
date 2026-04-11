/**
 * @file tests/test_stdio.cpp
 * @brief Compile test for xer/stdio.h.
 */

#include <type_traits>
#include <utility>

#include <xer/stdio.h>

namespace {

void compile_smoke_test() {
    static_assert(std::is_move_constructible_v<xer::binary_stream>);
    static_assert(std::is_move_constructible_v<xer::text_stream>);

    xer::path file_path(u8"dummy.txt");

    [[maybe_unused]] auto binary_open_result = xer::fopen(file_path, "rb");
    [[maybe_unused]] auto text_open_result = xer::fopen(file_path, "r", xer::encoding_t::utf8);

    [[maybe_unused]] auto binary_tmp_result = xer::tmpfile();
    [[maybe_unused]] auto text_tmp_result = xer::tmpfile(xer::encoding_t::utf8);

    [[maybe_unused]] auto* stdin_ptr = &xer_stdin;
    [[maybe_unused]] auto* stdout_ptr = &xer_stdout;
    [[maybe_unused]] auto* stderr_ptr = &xer_stderr;
}

} // namespace

int main() {
    compile_smoke_test();
    return 0;
}
