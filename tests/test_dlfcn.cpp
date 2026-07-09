/**
 * @file tests/test_dlfcn.cpp
 * @brief Tests for dynamic shared object loading facilities.
 */

#include <string_view>

#include <xer/assert.h>
#include <xer/dlfcn.h>
#include <xer/error.h>

namespace {

#if defined(_WIN32)

using get_current_process_id_type = unsigned long();

constexpr auto test_library_name = "kernel32.dll";
constexpr auto test_symbol_name = "GetCurrentProcessId";

#else

using puts_type = int(const char*);

constexpr auto test_library_name = "libc.so.6";
constexpr auto test_symbol_name = "puts";

#endif

} // namespace

int main()
{
    auto opened = xer::dlopen(test_library_name);
    xer_assert(opened.has_value());

    auto library = *opened;
    xer_assert(xer::is_open(library));
    xer_assert(xer::native_handle(library) != nullptr);

    auto copied = library;
    xer_assert(xer::is_open(copied));
    xer_assert(xer::native_handle(copied) == xer::native_handle(library));

#if defined(_WIN32)
    get_current_process_id_type* function = nullptr;
#else
    puts_type* function = nullptr;
#endif

    const auto symbol_result = xer::dlsym(library, test_symbol_name, function);
    xer_assert(symbol_result.has_value());
    xer_assert(function != nullptr);

    void* raw_symbol = nullptr;
    const auto raw_symbol_result = xer::dlsym(library, test_symbol_name, raw_symbol);
    xer_assert(raw_symbol_result.has_value());
    xer_assert(raw_symbol != nullptr);

    void* missing_symbol = nullptr;
    const auto missing_symbol_result = xer::dlsym(
        library,
        "xer_missing_symbol_for_test",
        missing_symbol);
    xer_assert(!missing_symbol_result.has_value());
    xer_assert(missing_symbol == nullptr);

    xer::dlclose(library);
    xer_assert(!xer::is_open(library));
    xer_assert(xer::native_handle(library) == nullptr);
    xer_assert(xer::is_open(copied));

    xer::dlclose(copied);
    xer_assert(!xer::is_open(copied));

    auto invalid = xer::dlopen(std::string_view());
    xer_assert(!invalid.has_value());
    xer_assert(invalid.error().code == xer::error_t::invalid_argument);

    return 0;
}
