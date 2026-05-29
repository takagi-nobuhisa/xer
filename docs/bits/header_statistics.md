# `<xer/statistics.h>`

## Purpose

`<xer/statistics.h>` provides small statistical utility functions for arithmetic ranges.

The initial scope is intentionally limited to common descriptive statistics:

- arithmetic mean
- population variance
- sample variance
- population standard deviation
- sample standard deviation

The functions return `xer::result<double>` so that invalid inputs are reported explicitly.

---

## Provided Functions

```cpp
template<class Range>
auto mean(Range&& range) -> xer::result<double>;

template<class T>
auto mean(std::initializer_list<T> values) -> xer::result<double>;

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

The statistical functions report invalid inputs through `xer::result<double>`.

| Condition | Error |
|---|---|
| empty range | `error_t::invalid_argument` |
| fewer than two values for sample variance / sample standard deviation | `error_t::invalid_argument` |
| NaN or infinity in the input | `error_t::invalid_argument` |
| intermediate or final value outside the representable `double` range | `error_t::range_error` |

---

## Numeric Behavior

The functions use a one-pass online update internally.
This allows them to work with input ranges and avoids requiring a second traversal of the input.

Accumulation is performed internally using `long double`, and the public result type is `double`.
This keeps the initial API simple and predictable while providing better intermediate precision than accumulating directly in `double`.

---

## Example

```cpp
#include <iostream>
#include <vector>

#include <xer/statistics.h>

auto main() -> int
{
    const std::vector<double> values{2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0};

    const auto mean = xer::mean(values);
    const auto variance = xer::variance(values);
    const auto stddev = xer::stddev(values);

    if (!mean || !variance || !stddev) {
        return 1;
    }

    std::cout << *mean << '\n';
    std::cout << *variance << '\n';
    std::cout << *stddev << '\n';

    return 0;
}
```

---

## Notes

`variance` and `stddev` intentionally mean population variance and population standard deviation.

Use `sample_variance` and `sample_stddev` when the sample formulas with `n - 1` are required.
This naming keeps the denominator choice explicit and avoids relying on ambiguous default terminology.
