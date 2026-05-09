#include <sstream>
#include <string>

#include <xer/assert.h>
#include <xer/color.h>
#include <xer/cyclic.h>
#include <xer/error.h>
#include <xer/image.h>
#include <xer/interval.h>
#include <xer/iostream.h>
#include <xer/matrix.h>
#include <xer/quantity.h>
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
void test_quantity_output()
{
    using namespace xer::units;

    std::ostringstream out;
    out << 1.5 * km;
    xer_assert_eq(out.str(), "1500");
}

void test_quantity_input()
{
    std::istringstream in("2.5");
    xer::quantity<double, xer::units::length_dim> value;

    in >> value;

    xer_assert(in);
    xer_assert_eq(value.value(), 2.5);
}

void test_matrix_output()
{
    const xer::matrix<double, 2, 3> value(1.0, 2.0, 3.0, 4.0, 5.0, 6.0);

    std::ostringstream out;
    out << value;
    xer_assert_eq(out.str(), "[[1, 2, 3], [4, 5, 6]]");
}


void test_image_point_output()
{
    std::ostringstream out;
    out << xer::image::point(10, 20);
    xer_assert_eq(out.str(), "(10, 20)");
}

void test_image_size_output()
{
    std::ostringstream out;
    out << xer::image::size(320, 240);
    xer_assert_eq(out.str(), "{320, 240}");
}

void test_image_rect_output()
{
    std::ostringstream out;
    out << xer::image::rect(
        xer::image::point(10, 20),
        xer::image::size(320, 240));
    xer_assert_eq(out.str(), "(10, 20) {320, 240}");
}

void test_image_point_input()
{
    std::istringstream in("(10, 20)");
    xer::image::point value;

    in >> value;

    xer_assert(in);
    xer_assert(value == xer::image::point(10, 20));
}

void test_image_size_input()
{
    std::istringstream in("{320, 240}");
    xer::image::size value;

    in >> value;

    xer_assert(in);
    xer_assert(value == xer::image::size(320, 240));
}

void test_image_rect_input()
{
    std::istringstream in("(10, 20) {320, 240}");
    xer::image::rect value;

    in >> value;

    xer_assert(in);
    xer_assert(value == xer::image::rect(
                            xer::image::point(10, 20),
                            xer::image::size(320, 240)));
}

void test_image_rect_input_compact_spacing()
{
    std::istringstream in("(10,20){320,240}");
    xer::image::rect value;

    in >> value;

    xer_assert(in);
    xer_assert(value == xer::image::rect(10, 20, 320, 240));
}

void test_image_rectf_output()
{
    std::ostringstream out;
    out << xer::image::rectf(
        xer::image::pointf(1.5f, 2.5f),
        xer::image::sizef(3.5f, 4.5f));
    xer_assert_eq(out.str(), "(1.5, 2.5) {3.5, 4.5}");
}

void test_image_rectf_input()
{
    std::istringstream in("(1.5, 2.5) {3.5, 4.5}");
    xer::image::rectf value;

    in >> value;

    xer_assert(in);
    xer_assert(value == xer::image::rectf(
                            xer::image::pointf(1.5f, 2.5f),
                            xer::image::sizef(3.5f, 4.5f)));
}

void test_image_point_input_rejects_braces()
{
    std::istringstream in("{10, 20}");
    auto value = xer::image::point(1, 2);

    in >> value;

    xer_assert_not(in);
    xer_assert(value == xer::image::point(1, 2));
}

void test_rgb_output()
{
    std::ostringstream out;
    out << xer::rgb(1.0f, 0.5f, 0.0f);
    xer_assert_eq(out.str(), "rgb(1, 0.5, 0)");
}

void test_gray_output()
{
    std::ostringstream out;
    out << xer::gray(0.25f);
    xer_assert_eq(out.str(), "gray(0.25)");
}

void test_cmy_output()
{
    std::ostringstream out;
    out << xer::cmy(0.0f, 0.5f, 1.0f);
    xer_assert_eq(out.str(), "cmy(0, 0.5, 1)");
}

void test_hsv_output()
{
    std::ostringstream out;
    out << xer::hsv(1.25f, 0.5f, 1.0f);
    xer_assert_eq(out.str(), "hsv(0.25, 0.5, 1)");
}

void test_xyz_output()
{
    std::ostringstream out;
    out << xer::xyz(0.1f, 0.2f, 0.3f);
    xer_assert_eq(out.str(), "xyz(0.1, 0.2, 0.3)");
}

void test_lab_output()
{
    std::ostringstream out;
    out << xer::lab(50.0f, 10.0f, -20.0f);
    xer_assert_eq(out.str(), "lab(50, 10, -20)");
}

void test_luv_output()
{
    std::ostringstream out;
    out << xer::luv(50.0f, 10.0f, -20.0f);
    xer_assert_eq(out.str(), "luv(50, 10, -20)");
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
    test_quantity_output();
    test_quantity_input();
    test_matrix_output();
    test_image_point_output();
    test_image_size_output();
    test_image_rect_output();
    test_image_point_input();
    test_image_size_input();
    test_image_rect_input();
    test_image_rect_input_compact_spacing();
    test_image_rectf_output();
    test_image_rectf_input();
    test_image_point_input_rejects_braces();
    test_rgb_output();
    test_gray_output();
    test_cmy_output();
    test_hsv_output();
    test_xyz_output();
    test_lab_output();
    test_luv_output();

    return 0;
}
