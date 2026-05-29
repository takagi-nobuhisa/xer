# `<xer/statistics.h>`

## Purpose

`<xer/statistics.h>` provides small descriptive statistical utility functions for arithmetic ranges.

The initial scope is intentionally limited to common descriptive statistics:

- sum
- product
- arithmetic mean
- geometric mean
- harmonic mean
- median
- quantile
- percentile
- mode
- population variance
- sample variance
- population standard deviation
- sample standard deviation

The scalar functions return `xer::result<double>` so that invalid inputs are reported explicitly.
`mode` returns `xer::result<std::vector<double>>` because multiple values can share the highest frequency.

---

## Provided Functions

```cpp
template<class Range>
auto mean(Range&& range) -> xer::result<double>;

template<class T>
auto mean(std::initializer_list<T> values) -> xer::result<double>;

template<class Range>
auto sum(Range&& range) -> xer::result<double>;

template<class T>
auto sum(std::initializer_list<T> values) -> xer::result<double>;

template<class Range>
auto product(Range&& range) -> xer::result<double>;

template<class T>
auto product(std::initializer_list<T> values) -> xer::result<double>;

template<class Range>
auto geometric_mean(Range&& range) -> xer::result<double>;

template<class T>
auto geometric_mean(std::initializer_list<T> values) -> xer::result<double>;

template<class Range>
auto harmonic_mean(Range&& range) -> xer::result<double>;

template<class T>
auto harmonic_mean(std::initializer_list<T> values) -> xer::result<double>;

template<class Range>
auto median(Range&& range) -> xer::result<double>;

template<class T>
auto median(std::initializer_list<T> values) -> xer::result<double>;

template<class Range>
auto quantile(Range&& range, double q) -> xer::result<double>;

template<class T>
auto quantile(std::initializer_list<T> values, double q) -> xer::result<double>;

template<class Range>
auto percentile(Range&& range, double p) -> xer::result<double>;

template<class T>
auto percentile(std::initializer_list<T> values, double p) -> xer::result<double>;

template<class Range>
auto mode(Range&& range, double tolerance = 0.0) -> xer::result<std::vector<double>>;

template<class T>
auto mode(std::initializer_list<T> values, double tolerance = 0.0) -> xer::result<std::vector<double>>;

template<class Range>
auto variance(Range&& range) -> xer::result<double>;

template<class T>
auto variance(std::initializer_list<T> values) -> xer::result<double>;

template<class Range>
auto sample_variance(Range&& range) -> xer::result<double>;

template<class T>
auto sample_variance(std::initializer_list<T> values) -> xer::result<double>;

template<class Range>
auto stddev(Range&& range) -> xer::result<double>;

template<class T>
auto stddev(std::initializer_list<T> values) -> xer::result<double>;

template<class Range>
auto sample_stddev(Range&& range) -> xer::result<double>;

template<class T>
auto sample_stddev(std::initializer_list<T> values) -> xer::result<double>;
```

Each range overload accepts an input range whose reference type is a non-`bool` arithmetic type.
The initializer-list overloads are provided so that calls such as the following work naturally:

```cpp
auto value = xer::mean({1.0, 2.0, 3.0});
```

---

## Mean

```cpp
template<class Range>
auto mean(Range&& range) -> xer::result<double>;
```

Computes the arithmetic mean of the input values.

The range must contain at least one value.
If the range is empty, the function returns `error_t::invalid_argument`.

---


## Sum

```cpp
template<class Range>
auto sum(Range&& range) -> xer::result<double>;
```

Computes the sum of the input values.

The range must contain at least one value.
If the range is empty, the function returns `error_t::invalid_argument`.

---

## Product

```cpp
template<class Range>
auto product(Range&& range) -> xer::result<double>;
```

Computes the product of the input values.

The range must contain at least one value.
If the range is empty, the function returns `error_t::invalid_argument`.

---

## Geometric Mean

```cpp
template<class Range>
auto geometric_mean(Range&& range) -> xer::result<double>;
```

Computes the geometric mean of the input values.

All input values must be finite and non-negative.
If any input value is zero, the result is zero.
If any input value is negative, NaN, or infinity, the function returns `error_t::invalid_argument`.

The range must contain at least one value.
If the range is empty, the function returns `error_t::invalid_argument`.

---

## Harmonic Mean

```cpp
template<class Range>
auto harmonic_mean(Range&& range) -> xer::result<double>;
```

Computes the harmonic mean of the input values.

All input values must be finite and positive.
If any input value is zero, negative, NaN, or infinity, the function returns `error_t::invalid_argument`.

The range must contain at least one value.
If the range is empty, the function returns `error_t::invalid_argument`.

---

## Median

```cpp
template<class Range>
auto median(Range&& range) -> xer::result<double>;
```

Computes the median of the input values.

The input values are copied and sorted internally.
For an odd number of values, the middle value is returned.
For an even number of values, the arithmetic mean of the two middle values is returned.

The range must contain at least one value.
If the range is empty, the function returns `error_t::invalid_argument`.

---

## Quantile

```cpp
template<class Range>
auto quantile(Range&& range, double q) -> xer::result<double>;
```

Computes a quantile of the input values.

The input values are copied and sorted internally.
The quantile fraction `q` must be finite and in the range `[0.0, 1.0]`.

The interpolation rule is linear interpolation on the sorted sequence:

```text
position = q * (n - 1)
result = values[floor(position)] * (1 - fraction)
       + values[ceil(position)]  * fraction
```

where `fraction` is the fractional part of `position`.

This means:

```text
quantile(values, 0.0) == minimum value
quantile(values, 0.5) == median value
quantile(values, 1.0) == maximum value
```

The range must contain at least one value.
If the range is empty, the function returns `error_t::invalid_argument`.

If `q` is outside `[0.0, 1.0]`, NaN, or infinity, the function returns `error_t::invalid_argument`.

---

## Percentile

```cpp
template<class Range>
auto percentile(Range&& range, double p) -> xer::result<double>;
```

Computes a percentile of the input values.

The percentile value `p` must be finite and in the range `[0.0, 100.0]`.
This function uses the same interpolation rule as `quantile`.

```text
percentile(values, p) == quantile(values, p / 100.0)
```

This means:

```text
percentile(values, 0.0)   == minimum value
percentile(values, 50.0)  == median value
percentile(values, 100.0) == maximum value
```

The range must contain at least one value.
If the range is empty, the function returns `error_t::invalid_argument`.

If `p` is outside `[0.0, 100.0]`, NaN, or infinity, the function returns `error_t::invalid_argument`.

---

## Mode

```cpp
template<class Range>
auto mode(Range&& range, double tolerance = 0.0) -> xer::result<std::vector<double>>;
```

Computes the mode values of the input values.

The result is a vector because multiple values can share the highest frequency.
If no value appears at least twice, the result is an empty vector.
This means that `mode({1, 2, 3})` succeeds and returns an empty vector rather than treating every value as a mode.

When `tolerance` is `0.0`, values are grouped by exact equality after sorting.
When `tolerance` is positive, sorted values whose distance from the first value of the current group is at most `tolerance` are treated as the same group.
The representative value returned for a tolerant group is the arithmetic mean of the values in that group.

For example:

```cpp
auto values = std::vector<double>{1.00, 1.02, 1.04, 2.00, 2.04};
auto modes = xer::mode(values, 0.05);
```

The first three values are grouped together, and the returned mode is approximately `1.02`.

`tolerance` must be finite and non-negative.
If `tolerance` is negative, NaN, or infinity, the function returns `error_t::invalid_argument`.

---

## Population Variance

```cpp
template<class Range>
auto variance(Range&& range) -> xer::result<double>;
```

Computes the population variance.

The population variance divides the sum of squared deviations by `n`:

```text
sum((x - mean)^2) / n
```

This function requires at least one value.
For a single value, the population variance is `0`.

---

## Sample Variance

```cpp
template<class Range>
auto sample_variance(Range&& range) -> xer::result<double>;
```

Computes the sample variance.

The sample variance divides the sum of squared deviations by `n - 1`:

```text
sum((x - mean)^2) / (n - 1)
```

This function requires at least two values.
If fewer than two values are provided, the function returns `error_t::invalid_argument`.

---

## Standard Deviation

```cpp
template<class Range>
auto stddev(Range&& range) -> xer::result<double>;

template<class Range>
auto sample_stddev(Range&& range) -> xer::result<double>;
```

`stddev` computes the population standard deviation.
It is the square root of `variance`.

`sample_stddev` computes the sample standard deviation.
It is the square root of `sample_variance`.

---

## Error Handling

The statistical functions report invalid inputs through `xer::result`.

| Condition | Error |
|---|---|
| empty range | `error_t::invalid_argument` |
| fewer than two values for sample variance / sample standard deviation | `error_t::invalid_argument` |
| NaN or infinity in the input | `error_t::invalid_argument` |
| negative input for `geometric_mean` | `error_t::invalid_argument` |
| zero or negative input for `harmonic_mean` | `error_t::invalid_argument` |
| `quantile` fraction outside `[0.0, 1.0]`, NaN, or infinity | `error_t::invalid_argument` |
| `percentile` value outside `[0.0, 100.0]`, NaN, or infinity | `error_t::invalid_argument` |
| negative, NaN, or infinite `mode` tolerance | `error_t::invalid_argument` |
| intermediate or final value outside the representable `double` range | `error_t::range_error` |

---

## Numeric Behavior

`sum`, `product`, `mean`, `geometric_mean`, `harmonic_mean`, `variance`, `sample_variance`, `stddev`, and `sample_stddev` use one-pass accumulation internally.
This allows them to work with input ranges and avoids requiring a second traversal of the input.

`geometric_mean` uses logarithmic accumulation for positive values.
A zero input value makes the result zero without taking `log(0)`.

`median`, `quantile`, `percentile`, and `mode` copy the input values because they need sorted data.
They still accept input ranges, but they require memory proportional to the number of input values.

Accumulation is performed internally using `long double`, and the public scalar result type is `double`.
This keeps the initial API simple and predictable while providing better intermediate precision than accumulating directly in `double`.

---

## Example

```cpp
#include <iostream>
#include <vector>

#include <xer/statistics.h>

void print_modes(const std::vector<double>& values)
{
    for (const double value : values) {
        std::cout << ' ' << value;
    }
    std::cout << '\n';
}

auto main() -> int
{
    const std::vector<double> values{2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0};

    const auto mean = xer::mean(values);
    const auto sum = xer::sum(values);
    const auto product = xer::product(values);
    const auto median = xer::median(values);
    const auto quantile = xer::quantile(values, 0.25);
    const auto percentile = xer::percentile(values, 75.0);
    const auto variance = xer::variance(values);
    const auto stddev = xer::stddev(values);
    const auto modes = xer::mode(values);

    if (!mean || !sum || !product || !median || !quantile || !percentile ||
        !variance || !stddev || !modes) {
        return 1;
    }

    std::cout << *mean << '\n';
    std::cout << *sum << '\n';
    std::cout << *product << '\n';
    std::cout << *median << '\n';
    std::cout << *quantile << '\n';
    std::cout << *percentile << '\n';
    std::cout << *variance << '\n';
    std::cout << *stddev << '\n';
    print_modes(*modes);

    return 0;
}
```

---

## Notes

`variance` and `stddev` intentionally mean population variance and population standard deviation.

Use `sample_variance` and `sample_stddev` when the sample formulas with `n - 1` are required.
This naming keeps the denominator choice explicit and avoids relying on ambiguous default terminology.

`<xer/statistics.h>` does not provide range minimum or range maximum helpers.
Use `std::ranges::min_element`, `std::ranges::max_element`, `std::min_element`, or `std::max_element` for that purpose.
