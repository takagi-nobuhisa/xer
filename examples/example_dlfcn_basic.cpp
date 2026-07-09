/**
 * @file examples/example_dlfcn_basic.cpp
 * @brief Basic dynamic shared object loading example.
 */

#include <iostream>

#include <xer/assert.h>
#include <xer/dlfcn.h>

#if defined(_WIN32)
using function_type = unsigned long();
constexpr auto library_name = "kernel32.dll";
constexpr auto symbol_name = "GetCurrentProcessId";
#else
using function_type = int(const char*);
constexpr auto library_name = "libc.so.6";
constexpr auto symbol_name = "puts";
#endif

int main()
{
    auto opened = xer::dlopen(library_name);
    xer_assert(opened.has_value());

    function_type* function = nullptr;
    auto symbol = xer::dlsym(*opened, symbol_name, function);
    xer_assert(symbol.has_value());
    xer_assert(function != nullptr);

    std::cout << "loaded: " << library_name << '\n';
    std::cout << "symbol: " << symbol_name << '\n';

    return 0;
}
