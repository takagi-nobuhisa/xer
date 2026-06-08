#include <iostream>

#include <xer/quantity.h>

int main()
{
    using namespace xer::units;

    const auto height = 5.0 * ft + 10.0 * inch;
    const auto distance = 1.0 * mile;
    const auto weight = 150.0 * lb;

    std::cout << "height: " << height.value(cm) << " cm\n";
    std::cout << "distance: " << distance.value(km) << " km\n";
    std::cout << "weight: " << weight.value(kg) << " kg\n";

    return 0;
}
