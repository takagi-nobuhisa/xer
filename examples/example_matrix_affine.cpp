#include <xer/matrix.h>
#include <xer/stdio.h>

auto main() -> int
{
    // XER_EXAMPLE_BEGIN: matrix_affine
    //
    // This example shows 2D and 3D affine transforms with fixed-size matrices.
    //
    // Expected output:
    // 2D transformed point: (13, 24)
    // 2D restored point: (2, 3)
    // 3D transformed point: (11, 22, 33)
    // 3D restored point: (1, 2, 3)

    const xer::vector3<double> point2{2.0, 3.0, 1.0};

    const auto transform2 =
        xer::translate2(10.0, 20.0) *
        xer::scale2(1.5, 4.0);

    const auto transformed2 = transform2 * point2;
    const auto inverse2 = xer::inverse(transform2);
    if (!inverse2.has_value()) {
        return 1;
    }

    const auto restored2 = *inverse2 * transformed2;

    if (!xer::printf(
            u8"2D transformed point: (%g, %g)\n",
            transformed2(0, 0),
            transformed2(1, 0))
             .has_value()) {
        return 1;
    }

    if (!xer::printf(
            u8"2D restored point: (%g, %g)\n",
            restored2(0, 0),
            restored2(1, 0))
             .has_value()) {
        return 1;
    }

    const xer::vector4<double> point3{1.0, 2.0, 3.0, 1.0};

    const auto transform3 = xer::translate3(10.0, 20.0, 30.0);
    const auto transformed3 = transform3 * point3;
    const auto inverse3 = xer::inverse(transform3);
    if (!inverse3.has_value()) {
        return 1;
    }

    const auto restored3 = *inverse3 * transformed3;

    if (!xer::printf(
            u8"3D transformed point: (%g, %g, %g)\n",
            transformed3(0, 0),
            transformed3(1, 0),
            transformed3(2, 0))
             .has_value()) {
        return 1;
    }

    if (!xer::printf(
            u8"3D restored point: (%g, %g, %g)\n",
            restored3(0, 0),
            restored3(1, 0),
            restored3(2, 0))
             .has_value()) {
        return 1;
    }

    // XER_EXAMPLE_END: matrix_affine

    return 0;
}
