#include <xer/assert.h>
#include <xer/quantity.h>

#include <concepts>
#include <type_traits>

namespace {

template<class T>
using plain_t = std::remove_cvref_t<T>;

auto test_sq_base_units() -> void
{
    using namespace xer::units;

    static_assert(std::same_as<plain_t<decltype(sq(m))>, plain_t<decltype(m²)>>);
    static_assert(std::same_as<plain_t<decltype(sq(sec))>, plain_t<decltype(sec²)>>);

    const auto area = 10.0 * sq(m);
    xer_assert_eq(area.value(m²), 10.0);

    const auto duration_squared = 4.0 * sq(sec);
    xer_assert_eq(duration_squared.value(sec²), 4.0);
}

auto test_cb_base_units() -> void
{
    using namespace xer::units;

    static_assert(std::same_as<plain_t<decltype(cb(m))>, plain_t<decltype(m³)>>);
    static_assert(std::same_as<plain_t<decltype(cb(sec))>, plain_t<decltype(sec³)>>);

    const auto volume = 2.0 * cb(m);
    xer_assert_eq(volume.value(m³), 2.0);

    const auto duration_cubed = 8.0 * cb(sec);
    xer_assert_eq(duration_cubed.value(sec³), 8.0);
}

auto test_symbolic_unit_constants() -> void
{
    using namespace xer::units;

    const auto area = 25.0 * m²;
    xer_assert_eq(area.value(sq(m)), 25.0);

    const auto volume = 125.0 * m³;
    xer_assert_eq(volume.value(cb(m)), 125.0);

    const auto time2 = 9.0 * sec²;
    xer_assert_eq(time2.value(sq(sec)), 9.0);

    const auto time3 = 27.0 * sec³;
    xer_assert_eq(time3.value(cb(sec)), 27.0);
}

auto test_acceleration_notation() -> void
{
    using namespace xer::units;

    const auto acceleration1 = 9.8 * m / (sec * sec);
    const auto acceleration2 = 9.8 * m / sq(sec);
    const auto acceleration3 = 9.8 * m / sec²;

    xer_assert_eq(acceleration1.value(), acceleration2.value());
    xer_assert_eq(acceleration1.value(), acceleration3.value());
}

auto test_force_and_pressure_notation() -> void
{
    using namespace xer::units;

    const auto force1 = 2.0 * kg * m / (sec * sec);
    const auto force2 = 2.0 * kg * m / sq(sec);
    const auto force3 = 2.0 * N;

    xer_assert_eq(force1.value(), force2.value());
    xer_assert_eq(force1.value(), force3.value());

    const auto pressure1 = force3 / sq(m);
    const auto pressure2 = force3 / m²;
    const auto pressure3 = 2.0 * Pa;

    xer_assert_eq(pressure1.value(), pressure2.value());
    xer_assert_eq(pressure1.value(), pressure3.value());
}

auto test_sq_quantity() -> void
{
    using namespace xer::units;

    const auto length = 3.0 * m;
    const auto area = xer::sq(length);

    xer_assert_eq(area.value(m²), 9.0);

    const auto time = 2.0 * sec;
    const auto time2 = xer::sq(time);

    xer_assert_eq(time2.value(sec²), 4.0);
}

auto test_cb_quantity() -> void
{
    using namespace xer::units;

    const auto length = 3.0 * m;
    const auto volume = xer::cb(length);

    xer_assert_eq(volume.value(m³), 27.0);

    const auto time = 2.0 * sec;
    const auto time3 = xer::cb(time);

    xer_assert_eq(time3.value(sec³), 8.0);
}

auto test_prefixed_units() -> void
{
    using namespace xer::units;

    const auto square_centimeter = sq(cm);
    const auto area = 4.0 * square_centimeter;

    xer_assert_eq(area.value(m²), 0.0004);
    xer_assert_eq(area.value(square_centimeter), 4.0);

    const auto cubic_centimeter = cb(cm);
    const auto volume = 8.0 * cubic_centimeter;

    xer_assert_eq(volume.value(m³), 0.000008);
    xer_assert_eq(volume.value(cubic_centimeter), 8.0);
}

} // namespace

auto main() -> int
{
    test_sq_base_units();
    test_cb_base_units();
    test_symbolic_unit_constants();
    test_acceleration_notation();
    test_force_and_pressure_notation();
    test_sq_quantity();
    test_cb_quantity();
    test_prefixed_units();

    return 0;
}
