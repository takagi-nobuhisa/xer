<!-- xer-reference-source-sha256: 61f5a6f03fe89e16ad8c38557b8cff62cce853ab61dcdfcebb4fc0f421ac0ec5 -->

# `<xer/math.h>`

## 目的

`<xer/math.h>` は、xer の軽量な実数数学ヘルパーを提供します。

初期の対象範囲は、意図的に実用的な範囲に限定されています。これは数値解析ライブラリの代替ではありません。振る舞いとエラー処理が明示的な、小さくてよく使うヘルパーを提供します。

---

## 型

### `vec`

```cpp
template<class T, std::size_t N = 2>
struct vec;
```

小さな位置ベクトルまたは数学的なベクトルを表します。

提供されるのは `N == 2`、`N == 3`、`N == 4` のみです。`<xer/math.h>` は任意次元の汎用ベクトル型を提供することを目的としていないため、主テンプレートは意図的に定義されていません。

各特殊化は、名前付きの座標メンバーを提供します。

| 型 | メンバー |
|---|---|
| `vec<T, 2>` | `x`, `y` |
| `vec<T, 3>` | `x`, `y`, `z` |
| `vec<T, 4>` | `x`, `y`, `z`, `w` |

`vec<T>` は `vec<T, 2>` と同じです。

各特殊化は、範囲チェックを行わない添字アクセスも提供します。

```cpp
auto operator[](std::size_t index) noexcept -> T&;
auto operator[](std::size_t index) const noexcept -> const T&;
```

`operator[]` は範囲チェックを行いません。範囲外の添字を渡した場合の動作は未定義です。

チェック付きアクセスには `at` を使用します。

```cpp
auto at(std::size_t index) noexcept
    -> xer::result<std::reference_wrapper<T>>;

auto at(std::size_t index) const noexcept
    -> xer::result<std::reference_wrapper<const T>>;
```

`index` が範囲外の場合、`at` は `error_t::out_of_range` を返します。

### `polar`

```cpp
template<class T, std::size_t N = 2>
struct polar;
```

極座標を表します。

現在提供されるのは `polar<T, 2>` のみです。

```cpp
template<class T>
struct polar<T, 2> {
    T r;
    cyclic<T> theta;
};
```

`theta` は τrad で表された循環角です。`1` は 1 回転を意味します。

---

## 提供される関数

### 三角関数

```cpp
template<std::floating_point T>
auto sin(T theta) noexcept -> T;

template<std::floating_point T>
auto sin(cyclic<T> theta) noexcept -> T;

template<std::floating_point T>
auto cos(T theta) noexcept -> T;

template<std::floating_point T>
auto cos(cyclic<T> theta) noexcept -> T;

template<std::floating_point T>
auto tan(T theta) noexcept -> T;

template<std::floating_point T>
auto tan(cyclic<T> theta) noexcept -> T;
```

xer の角度単位を使って通常の三角関数を計算します。

スカラー版は `theta` を τrad 値として解釈します。`1` は 1 回転を意味します。`cyclic<T>` 版は、循環角の正規化済みの値を使用します。

例:

```text
sin(0.25) == 1
cos(0.5) == -1
tan(0.125) == 1
```

双曲線関数は、意図的に `<xer/math.h>` では提供しません。それらの関数が必要な場合は、C++ 標準ライブラリを直接使用してください。

### 逆三角関数

```cpp
template<std::floating_point T>
auto asin(T value) noexcept -> T;

template<std::floating_point T>
auto acos(T value) noexcept -> T;

template<std::floating_point T>
auto atan(T value) noexcept -> T;

template<std::floating_point T>
auto atan2(T y, T x) noexcept -> T;
```

逆三角関数を計算し、τrad のスカラー値を返します。

典型的な戻り値の範囲は、対応する標準ライブラリ関数の戻り値をラジアンから τrad に変換したものに従います。

| 関数 | 典型的な戻り値の範囲 |
|---|---|
| `asin` | `[-0.25, 0.25]` |
| `acos` | `[0, 0.5]` |
| `atan` | `(-0.25, 0.25)` |
| `atan2` | `[-0.5, 0.5]` |

定義域エラーについては、これらの関数は基になる標準ライブラリ関数の振る舞いに従います。たとえば不正な浮動小数点入力に対して NaN を返します。これらの関数は `xer::result` を返しません。

### `to_polar`

```cpp
template<std::floating_point T>
auto to_polar(vec<T, 2> v) noexcept -> polar<T, 2>;
```

2 次元直交座標ベクトルを極座標に変換します。

返される半径は `std::hypot(v.x, v.y)` で計算されます。返される角度は `std::atan2(v.y, v.x)` で計算され、τrad の循環値に変換されます。

### `to_cartesian`

```cpp
template<std::floating_point T>
auto to_cartesian(polar<T, 2> p) noexcept -> vec<T, 2>;
```

2 次元極座標を直交座標ベクトルに変換します。

座標成分は次のように計算されます。

```text
x = r * cos(theta * τ)
y = r * sin(theta * τ)
```

### `dot`

```cpp
template<class T, std::size_t N>
auto dot(vec<T, N> a, vec<T, N> b) noexcept -> T;
```

2 つのベクトルの内積を計算します。

`T` は算術型でなければなりません。`N` は対応している `vec` の次元、つまり `2`、`3`、`4` のいずれかでなければなりません。

この関数は、対応する成分どうしの積の総和を返します。整数ベクトルの場合、戻り値も整数です。

### `length`

```cpp
template<class T, std::size_t N>
auto length(vec<T, N> v) noexcept -> std::common_type_t<T, double>;
```

ベクトルのユークリッド長を計算します。

`T` は算術型でなければなりません。`N` は対応している `vec` の次元、つまり `2`、`3`、`4` のいずれかでなければなりません。

戻り値の型は `std::common_type_t<T, double>` です。そのため、整数ベクトルでも浮動小数点の長さが得られます。

### `distance`

```cpp
template<class T, std::size_t N>
auto distance(vec<T, N> a, vec<T, N> b) noexcept -> std::common_type_t<T, double>;
```

2 つのベクトル間のユークリッド距離を計算します。位置は位置ベクトルとして表されるため、この関数は 2 点間の距離の計算にも使用できます。

`T` は算術型でなければなりません。`N` は対応している `vec` の次元、つまり `2`、`3`、`4` のいずれかでなければなりません。

戻り値の型は `std::common_type_t<T, double>` です。そのため、整数ベクトルでも浮動小数点の距離が得られます。

### `normalize`

```cpp
template<class T, std::size_t N>
auto normalize(vec<T, N> v) noexcept
    -> xer::result<vec<std::common_type_t<T, double>, N>>;
```

`v` と同じ向きで長さが `1` のベクトルを返します。

`T` は算術型でなければなりません。`N` は対応している `vec` の次元、つまり `2`、`3`、`4` のいずれかでなければなりません。

返されるベクトルは、成分型として `std::common_type_t<T, double>` を使用します。そのため、整数ベクトルでも小数成分を失わずに正規化できます。

`v` がゼロベクトルの場合、この関数は `error_t::invalid_argument` を返します。

### `angle`

```cpp
template<class T, std::size_t N>
auto angle(vec<T, N> a, vec<T, N> b) noexcept
    -> xer::result<std::common_type_t<T, double>>;
```

2 つのベクトルのなす角の大きさを τrad で計算します。

`T` は算術型でなければなりません。`N` は対応している `vec` の次元、つまり `2`、`3`、`4` のいずれかでなければなりません。

戻り値の型は `std::common_type_t<T, double>` です。そのため、整数ベクトルでも浮動小数点の角度が得られます。返される値は角度の大きさであるため、`cyclic<T>` ではありません。範囲は `0` から `0.5` で、`0.25` は直角、`0.5` は平角です。

どちらかのベクトルがゼロベクトルの場合、この関数は `error_t::invalid_argument` を返します。

### `rotate`

```cpp
template<class T, std::floating_point Angle>
auto rotate(vec<T, 2> v, cyclic<Angle> theta) noexcept
    -> vec<std::common_type_t<T, Angle, double>, 2>;
```

2 次元ベクトルを原点の周りで回転します。

`T` は算術型でなければなりません。`theta` は τrad で表された循環角です。`1` は 1 回転を意味します。

戻り値の型は、成分型として `std::common_type_t<T, Angle, double>` を使用します。そのため、整数ベクトルでも小数成分を失わずに回転できます。

正の角度は、通常の数学的座標系で反時計回りに回転します。

### `cross`

```cpp
template<class T>
auto cross(vec<T, 3> a, vec<T, 3> b) noexcept -> vec<T, 3>;
```

2 つの 3 次元ベクトルの外積を計算します。

`T` は算術型でなければなりません。この関数は現在、`vec<T, 3>` のみに対応しています。

### `heron`

```cpp
template<std::floating_point T>
auto heron(T a, T b, T c) -> xer::result<T>;
```

ヘロンの公式を使って、3 辺の長さから三角形の面積を計算します。

辺の長さは非負であり、三角形を作れるものでなければなりません。辺の長さが負の場合、または辺の長さが三角不等式に反する場合、この関数は `error_t::invalid_argument` を返します。退化した三角形は受け入れられ、0 を返します。

実装では、辺の長さをソートしたあと、並べ替えた形のヘロンの公式を使用します。これにより、最も直接的な `s * (s - a) * (s - b) * (s - c)` の形と比べて、避けられる桁落ちをいくらか回避できます。

### `quadratic`

```cpp
template<std::floating_point T>
auto quadratic(T a, T b, T c)
    -> xer::result<std::array<std::optional<T>, 2>>;
```

次の二次方程式を解きます。

```text
a * x * x + b * x + c == 0
```

係数 `a` は 0 であってはなりません。`a == 0` の場合、この関数は `error_t::invalid_argument` を返します。

返される配列には、先頭要素から順に相異なる実根が格納されます。

| 結果 | 意味 |
|---|---|
| `{ nullopt, nullopt }` | 実根なし |
| `{ x, nullopt }` | 重解を含む 1 個の実根 |
| `{ x1, x2 }` | 2 個の相異なる実根 |

2 個の実根が返される場合、それらは昇順に並べられます。

### `cubic`

```cpp
template<std::floating_point T>
auto cubic(T a, T b, T c, T d)
    -> xer::result<std::array<std::optional<T>, 3>>;
```

次の三次方程式を解きます。

```text
a * x * x * x + b * x * x + c * x + d == 0
```

係数 `a` は 0 であってはなりません。`a == 0` の場合、この関数は `error_t::invalid_argument` を返します。

返される配列には、先頭要素から順に相異なる実根が格納されます。空の要素は `std::nullopt` で表されます。返される根は昇順に並べられます。

---

## 設計メモ

`quadratic` と `cubic` は実根のみを返します。非実根はエラーではありません。単に返される optional 配列には格納されません。

複素根が必要な場合は、`<xer/complex.h>` を使用してください。

重根は実数関数では 1 回だけ表されます。これにより、交点、衝突時刻、実数領域での制約のような一般的な実用チェックに使いやすい結果になります。

## 座標とベクトルの方針

位置は位置ベクトルとして表します。`<xer/math.h>` は、数学的な座標用の独立した点型を提供しません。

`vec` 型は 2 次元、3 次元、4 次元に限定されています。これにより、この機能が汎用線形代数パッケージになるのではなく、実用的な幾何、グラフィックス、座標変換、単純な物理計算に焦点を合わせたものになります。
