#include <type_traits>

#include <xer/assert.h>
#include <xer/quantity.h>

namespace {

void test_scalar_times_unit_creates_base_normalized_quantity()
{
    using namespace xer::units;

    const auto x = 1.5 * km;

    xer_assert_eq(x.value(), 1500.0);
    xer_assert_eq(x.value(m), 1500.0);
    xer_assert_eq(x.value(km), 1.5);
    xer_assert_eq(xer::value_in(x, km), 1.5);
}

void test_addition_uses_base_units()
{
    using namespace xer::units;

    const auto x = 1.0 * m + 20.0 * cm;

    xer_assert_eq(x.value(m), 1.2);
}

void test_unit_composition_for_speed()
{
    using namespace xer::units;

    const auto speed = 10.0 * m / sec;
    const auto time = 2.5 * sec;
    const auto distance = speed * time;

    xer_assert_eq(distance.value(m), 25.0);
}

void test_dimensionless_conversion_is_explicit()
{
    using namespace xer::units;

    const auto ratio = (10.0 * m) / (2.0 * m);
    const double value = static_cast<double>(ratio);

    xer_assert_eq(value, 5.0);
}

void test_force_energy_power_voltage_units()
{
    using namespace xer::units;

    const auto force = 2.0 * kg * (3.0 * m / (sec * sec));
    const auto energy = force * (4.0 * m);
    const auto power = energy / (2.0 * sec);
    const auto voltage = power / (3.0 * A);

    xer_assert_eq(force.value(N), 6.0);
    xer_assert_eq(energy.value(J), 24.0);
    xer_assert_eq(power.value(W), 12.0);
    xer_assert_eq(voltage.value(V), 4.0);
}

void test_area_volume_and_alias_units()
{
    using namespace xer::units;

    const auto field = 2.0 * ha;
    const auto liquid1 = 500.0 * mL;
    const auto liquid2 = 250.0 * cc;

    xer_assert_eq(field.value(m * m), 20000.0);
    xer_assert_eq(liquid1.value(L), 0.5);
    xer_assert(liquid2.value(mL) > 249.999999999);
    xer_assert(liquid2.value(mL) < 250.000000001);
}

void test_angle_units()
{
    using namespace xer::units;

    const auto turn = 1.0 * taurad;
    const auto quarter_turn = xer::pi_v<double> / 2.0 * rad;

    xer_assert_eq(turn.value(), 1.0);
    xer_assert(quarter_turn.value(taurad) > 0.249999999999);
    xer_assert(quarter_turn.value(taurad) < 0.250000000001);
}

void test_type_properties()
{
    using namespace xer::units;

    static_assert(std::is_empty_v<decltype(m)>);
    static_assert(std::is_same_v<decltype(μm), const xer::unit<xer::units::length_dim, std::micro>>);
}

} // namespace

int main()
{
    test_scalar_times_unit_creates_base_normalized_quantity();
    test_addition_uses_base_units();
    test_unit_composition_for_speed();
    test_dimensionless_conversion_is_explicit();
    test_force_energy_power_voltage_units();
    test_area_volume_and_alias_units();
    test_angle_units();
    test_type_properties();

    return 0;
}
