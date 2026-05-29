<!-- xer-reference-source-sha256: 32944071115bbfed6463e366131fa0ac3db6e27c90c3589c4ec803b0609a8ac9 -->

# `<xer/statistics.h>`

## 目的

`<xer/statistics.h>` は、算術値の範囲に対する小さな記述統計ユーティリティ関数を提供します。

初期の対象範囲は、一般的な記述統計に意図的に限定されています。

- 合計
- 積
- 算術平均
- 幾何平均
- 調和平均
- 中央値
- 分位数
- パーセンタイル
- 最頻値
- 母分散
- 標本分散
- 母標準偏差
- 標本標準偏差

スカラー関数は `xer::result<double>` を返すため、不正な入力は明示的に報告されます。
`mode` は、複数の値が最高頻度を共有できるため、`xer::result<std::vector<double>>` を返します。

---

## 提供される関数

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

各範囲版は、参照型が `bool` ではない算術型である入力範囲を受け取ります。
初期化子リスト版は、次のような呼び出しが自然に動作するように提供されています。

```cpp
auto value = xer::mean({1.0, 2.0, 3.0});
```

---

## 平均

```cpp
template<class Range>
auto mean(Range&& range) -> xer::result<double>;
```

入力値の算術平均を計算します。

範囲には少なくとも 1 個の値が含まれていなければなりません。
範囲が空の場合、この関数は `error_t::invalid_argument` を返します。

---

## 合計

```cpp
template<class Range>
auto sum(Range&& range) -> xer::result<double>;
```

入力値の合計を計算します。

範囲には少なくとも 1 個の値が含まれていなければなりません。
範囲が空の場合、この関数は `error_t::invalid_argument` を返します。

---

## 積

```cpp
template<class Range>
auto product(Range&& range) -> xer::result<double>;
```

入力値の積を計算します。

範囲には少なくとも 1 個の値が含まれていなければなりません。
範囲が空の場合、この関数は `error_t::invalid_argument` を返します。

---

## 幾何平均

```cpp
template<class Range>
auto geometric_mean(Range&& range) -> xer::result<double>;
```

入力値の幾何平均を計算します。

すべての入力値は有限で非負でなければなりません。
入力値のいずれかが 0 の場合、結果は 0 になります。
入力値のいずれかが負値、NaN、無限大の場合、この関数は `error_t::invalid_argument` を返します。

範囲には少なくとも 1 個の値が含まれていなければなりません。
範囲が空の場合、この関数は `error_t::invalid_argument` を返します。

---

## 調和平均

```cpp
template<class Range>
auto harmonic_mean(Range&& range) -> xer::result<double>;
```

入力値の調和平均を計算します。

すべての入力値は有限で正でなければなりません。
入力値のいずれかが 0、負値、NaN、無限大の場合、この関数は `error_t::invalid_argument` を返します。

範囲には少なくとも 1 個の値が含まれていなければなりません。
範囲が空の場合、この関数は `error_t::invalid_argument` を返します。

---

## 中央値

```cpp
template<class Range>
auto median(Range&& range) -> xer::result<double>;
```

入力値の中央値を計算します。

入力値は内部でコピーされ、ソートされます。
値の個数が奇数の場合、中央の値が返されます。
値の個数が偶数の場合、中央の 2 値の算術平均が返されます。

範囲には少なくとも 1 個の値が含まれていなければなりません。
範囲が空の場合、この関数は `error_t::invalid_argument` を返します。

---

## 分位数

```cpp
template<class Range>
auto quantile(Range&& range, double q) -> xer::result<double>;
```

入力値の分位数を計算します。

入力値は内部でコピーされ、ソートされます。
分位数の比率 `q` は有限で、範囲 `[0.0, 1.0]` 内になければなりません。

補間規則は、ソート済み列に対する線形補間です。

```text
position = q * (n - 1)
result = values[floor(position)] * (1 - fraction)
       + values[ceil(position)]  * fraction
```

ここで `fraction` は `position` の小数部分です。

これは次を意味します。

```text
quantile(values, 0.0) == 最小値
quantile(values, 0.5) == 中央値
quantile(values, 1.0) == 最大値
```

範囲には少なくとも 1 個の値が含まれていなければなりません。
範囲が空の場合、この関数は `error_t::invalid_argument` を返します。

`q` が `[0.0, 1.0]` の外側、NaN、または無限大の場合、この関数は `error_t::invalid_argument` を返します。

---

## パーセンタイル

```cpp
template<class Range>
auto percentile(Range&& range, double p) -> xer::result<double>;
```

入力値のパーセンタイルを計算します。

パーセンタイル値 `p` は有限で、範囲 `[0.0, 100.0]` 内になければなりません。
この関数は `quantile` と同じ補間規則を使用します。

```text
percentile(values, p) == quantile(values, p / 100.0)
```

これは次を意味します。

```text
percentile(values, 0.0)   == 最小値
percentile(values, 50.0)  == 中央値
percentile(values, 100.0) == 最大値
```

範囲には少なくとも 1 個の値が含まれていなければなりません。
範囲が空の場合、この関数は `error_t::invalid_argument` を返します。

`p` が `[0.0, 100.0]` の外側、NaN、または無限大の場合、この関数は `error_t::invalid_argument` を返します。

---

## 最頻値

```cpp
template<class Range>
auto mode(Range&& range, double tolerance = 0.0) -> xer::result<std::vector<double>>;
```

入力値の最頻値を計算します。

複数の値が最高頻度を共有できるため、結果は vector です。
2 回以上現れる値がない場合、結果は空の vector です。
つまり、`mode({1, 2, 3})` はすべての値を最頻値として扱うのではなく、成功して空の vector を返します。

`tolerance` が `0.0` の場合、値はソート後に完全一致でグループ化されます。
`tolerance` が正の場合、現在のグループの先頭値からの距離が `tolerance` 以下であるソート済みの値は、同じグループとして扱われます。
許容幅付きグループで返される代表値は、そのグループ内の値の算術平均です。

例:

```cpp
auto values = std::vector<double>{1.00, 1.02, 1.04, 2.00, 2.04};
auto modes = xer::mode(values, 0.05);
```

最初の 3 値が同じグループになり、返される最頻値はおよそ `1.02` です。

`tolerance` は有限で非負でなければなりません。
`tolerance` が負値、NaN、または無限大の場合、この関数は `error_t::invalid_argument` を返します。

---

## 母分散

```cpp
template<class Range>
auto variance(Range&& range) -> xer::result<double>;
```

母分散を計算します。

母分散は、偏差平方和を `n` で割ります。

```text
sum((x - mean)^2) / n
```

この関数は少なくとも 1 個の値を必要とします。
値が 1 個の場合、母分散は `0` です。

---

## 標本分散

```cpp
template<class Range>
auto sample_variance(Range&& range) -> xer::result<double>;
```

標本分散を計算します。

標本分散は、偏差平方和を `n - 1` で割ります。

```text
sum((x - mean)^2) / (n - 1)
```

この関数は少なくとも 2 個の値を必要とします。
2 個未満の値が渡された場合、この関数は `error_t::invalid_argument` を返します。

---

## 標準偏差

```cpp
template<class Range>
auto stddev(Range&& range) -> xer::result<double>;

template<class Range>
auto sample_stddev(Range&& range) -> xer::result<double>;
```

`stddev` は母標準偏差を計算します。
これは `variance` の平方根です。

`sample_stddev` は標本標準偏差を計算します。
これは `sample_variance` の平方根です。

---

## エラー処理

統計関数は、不正な入力を `xer::result` を通じて報告します。

| 条件 | エラー |
|---|---|
| 空の範囲 | `error_t::invalid_argument` |
| 標本分散 / 標本標準偏差で値が 2 個未満 | `error_t::invalid_argument` |
| 入力内の NaN または無限大 | `error_t::invalid_argument` |
| `geometric_mean` への負の入力 | `error_t::invalid_argument` |
| `harmonic_mean` への 0 または負の入力 | `error_t::invalid_argument` |
| `quantile` の比率が `[0.0, 1.0]` の外側、NaN、または無限大 | `error_t::invalid_argument` |
| `percentile` の値が `[0.0, 100.0]` の外側、NaN、または無限大 | `error_t::invalid_argument` |
| `mode` の `tolerance` が負値、NaN、または無限大 | `error_t::invalid_argument` |
| 中間値または最終値が `double` で表現できる範囲の外側 | `error_t::range_error` |

---

## 数値的振る舞い

`sum`、`product`、`mean`、`geometric_mean`、`harmonic_mean`、`variance`、`sample_variance`、`stddev`、`sample_stddev` は、内部で 1 パスの累積を使用します。
これにより入力範囲で動作でき、入力を 2 回走査する必要を避けられます。

`geometric_mean` は、正の値に対して対数による累積を使用します。
入力値に 0 がある場合、`log(0)` を取らずに結果を 0 にします。

`median`、`quantile`、`percentile`、`mode` はソート済みデータを必要とするため、入力値をコピーします。
入力範囲は受け取れますが、入力値の個数に比例するメモリが必要です。

累積は内部で `long double` を使って行われ、公開されるスカラー結果の型は `double` です。
これにより、初期 API を単純で予測しやすいものに保ちながら、`double` に直接累積するよりも高い中間精度を得ます。

---

## 例

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

## メモ

`variance` と `stddev` は、意図的に母分散と母標準偏差を意味します。

`n - 1` を使う標本公式が必要な場合は、`sample_variance` と `sample_stddev` を使用してください。
この命名により、分母の選択が明示的になり、曖昧な既定用語に依存することを避けられます。

`<xer/statistics.h>` は、範囲の最小値または最大値を求めるヘルパーを提供しません。
その用途には、`std::ranges::min_element`、`std::ranges::max_element`、`std::min_element`、または `std::max_element` を使用してください。
