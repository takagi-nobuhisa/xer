#include <sstream>
#include <string>

#include <xer/assert.h>
#include <xer/cyclic.h>
#include <xer/error.h>
#include <xer/interval.h>
#include <xer/iostream.h>
#include <xer/path.h>
#include <xer/typeinfo.h>

namespace {

void test_error_t_output()
{
    std::ostringstream out;
    out << xer::error_t::invalid_argument;
    xer_assert_eq(out.str(), "invalid_argument");
}

void test_error_t_input()
{
    std::istringstream in("not_found");
    auto code = xer::error_t::runtime_error;

    in >> code;

    xer_assert(in);
    xer_assert(code == xer::error_t::not_found);
}

void test_error_t_input_unknown_fails()
{
    std::istringstream in("unknown_name");
    auto code = xer::error_t::runtime_error;

    in >> code;

    xer_assert_not(in);
    xer_assert(code == xer::error_t::runtime_error);
}

void test_error_output()
{
    std::ostringstream out;
    out << xer::make_error(xer::error_t::invalid_argument);
    xer_assert_eq(out.str(), "xer::error{code=invalid_argument}");
}

void test_type_info_output()
{
    std::ostringstream out;
    out << xer_typeid(int);
    xer_assert_eq(out.str(), "int");
}

void test_path_output()
{
    std::ostringstream out;
    out << xer::path(u8"foo\\bar.txt");
    xer_assert_eq(out.str(), "foo/bar.txt");
}

void test_path_input()
{
    std::istringstream in("foo\\bar.txt");
    xer::path value;

    in >> value;

    xer_assert(in);
    xer_assert(value.str() == u8"foo/bar.txt");
}

void test_cyclic_output()
{
    std::ostringstream out;
    out << xer::cyclic<double>(1.25);
    xer_assert_eq(out.str(), "0.25");
}

void test_cyclic_input()
{
    std::istringstream in("-0.25");
    xer::cyclic<double> value;

    in >> value;

    xer_assert(in);
    xer_assert_eq(value.value(), 0.75);
}

void test_interval_output()
{
    std::ostringstream out;
    out << xer::interval<double>(1.25);
    xer_assert_eq(out.str(), "1");
}

void test_interval_input()
{
    std::istringstream in("0.25");
    xer::interval<double> value;

    in >> value;

    xer_assert(in);
    xer_assert_eq(value.value(), 0.25);
}

void test_interval_input_clamps()
{
    std::istringstream in("2.5");
    xer::interval<double> value;

    in >> value;

    xer_assert(in);
    xer_assert_eq(value.value(), 1.0);
}

} // namespace

auto main() -> int
{
    test_error_t_output();
    test_error_t_input();
    test_error_t_input_unknown_fails();
    test_error_output();
    test_type_info_output();
    test_path_output();
    test_path_input();
    test_cyclic_output();
    test_cyclic_input();
    test_interval_output();
    test_interval_input();
    test_interval_input_clamps();

    return 0;
}
