<!-- xer-reference-source-sha256: d15a677ee9c72228b13cb273e2f25842b5a1b65962c481a1b1ddf642f49942f6 -->

# `<xer/matrix.h>`

## 目的

`<xer/matrix.h>` は、xer における固定サイズ行列とアフィン変換補助機能を提供します。

このヘッダーの最初の目的は、意図的に実用的かつ限定的です。最初から完全な線形代数フレームワークにすることは意図していません。代わりに、一般的な 2D / 3D アフィン変換とその逆変換を、明確で軽量な形で表現するのに十分な機能を提供します。

---

## 主な役割

`<xer/matrix.h>` の主な役割は次の機能を提供することです。

- 固定サイズの行優先行列
- 3x3 および 4x4 行列エイリアス
- 3x1 および 4x1 列ベクトルエイリアス
- 通常の行列積
- 単位行列の生成
- 3x3 および 4x4 行列の逆行列計算
- 2D / 3D アフィン変換用の補助関数

これにより、次のような変換コードを書けます。

```cpp
const xer::vector3<double> point{2.0, 3.0, 1.0};

const auto transform =
    xer::translate2(10.0, 20.0) *
    xer::scale2(1.5, 4.0);

const auto transformed = transform * point;
```

---

## 主なエンティティ

少なくとも、`<xer/matrix.h>` は次のエンティティを提供します。

```cpp
template <std::floating_point T, std::size_t Rows, std::size_t Cols>
class matrix;

template <std::floating_point T>
using matrix3 = matrix<T, 3, 3>;

template <std::floating_point T>
using matrix4 = matrix<T, 4, 4>;

template <std::floating_point T>
using vector3 = matrix<T, 3, 1>;

template <std::floating_point T>
using vector4 = matrix<T, 4, 1>;
```

また、乗算、単位行列生成、逆行列計算、アフィン変換補助関数も提供します。

---

## `matrix<T, Rows, Cols>`

`matrix<T, Rows, Cols>` は基本となる固定サイズ行列型です。

### 基本形

```cpp
template <std::floating_point T, std::size_t Rows, std::size_t Cols>
class matrix;
```

### 要素型

要素型は浮動小数点型に制限されます。

主に想定する型は次のとおりです。

* `float`
* `double`
* `long double`

最初の実装を、幾何変換と小数値が自然に現れる数値演算へ集中させるためです。

### ストレージモデル

行列は固定サイズの行優先値として保存されます。

概念的には、行 `r`、列 `c` の要素は次のようにアクセスします。

```cpp
m(r, c)
```

行と列は 0 始まりです。`operator()` では境界チェックを行いません。

### 構築

既定構築された行列は零行列です。

また、行優先順でちょうど `Rows * Cols` 個の値から構築できます。

```cpp
xer::matrix<double, 2, 3> value{
    1.0, 2.0, 3.0,
    4.0, 5.0, 6.0
};
```

値の個数は正確でなければなりません。これにより、不完全または過剰な行列リテラルをコンパイル時に検出できます。

---

## 行列エイリアス

このヘッダーは、初期のアフィン変換機能で使うサイズの行列エイリアスを提供します。

```cpp
template <std::floating_point T>
using matrix3 = matrix<T, 3, 3>;

template <std::floating_point T>
using matrix4 = matrix<T, 4, 4>;
```

### 役割

* `matrix3<T>` は主に 2D 同次アフィン変換に使います。
* `matrix4<T>` は主に 3D 同次アフィン変換に使います。

---

## 列ベクトルエイリアス

このヘッダーは、同次列ベクトルのエイリアスも提供します。

```cpp
template <std::floating_point T>
using vector3 = matrix<T, 3, 1>;

template <std::floating_point T>
using vector4 = matrix<T, 4, 1>;
```

### 役割

* `vector3<T>` は通常、2D 同次列ベクトル `(x, y, 1)` として使います。
* `vector4<T>` は通常、3D 同次列ベクトル `(x, y, z, 1)` として使います。

これらは別個のベクトルクラスではなく、`matrix` のエイリアスです。最初の実装を単純に保ち、行列とベクトルの積を通常の行列積として扱えるようにします。

---

## 行列積

`<xer/matrix.h>` は通常の行×列の行列積を提供します。

```cpp
auto operator*(
    const matrix<T, R, C>& left,
    const matrix<T, C, K>& right) noexcept
    -> matrix<T, R, K>;
```

### 役割

この 1 つの演算で次を扱います。

* 行列 × 行列
* 行列 × 列ベクトル
* アフィン変換の合成
* 同次点へのアフィン変換の適用

### 変換合成の順序

この行列機能では列ベクトルを使います。そのため、次のような式では、

```cpp
const auto transform = translate2<double>(10.0, 20.0) * scale2<double>(2.0, 3.0);
const auto result = transform * point;
```

`point` には右端の変換が先に適用されます。この例では、先に拡大縮小が適用され、その後に平行移動が適用されます。

---

## 単位行列

このヘッダーは汎用の単位行列補助関数を提供します。

```cpp
template <std::floating_point T, std::size_t N>
auto identity_matrix() noexcept -> matrix<T, N, N>;
```

また、主要なアフィン変換サイズ向けの便利関数も提供します。

```cpp
template <std::floating_point T>
auto identity3() noexcept -> matrix3<T>;

template <std::floating_point T>
auto identity4() noexcept -> matrix4<T>;
```

---

## 逆行列

このヘッダーは 3x3 および 4x4 行列の逆行列計算を提供します。

```cpp
template <std::floating_point T>
auto inverse(const matrix<T, 3, 3>& value) noexcept
    -> xer::result<matrix<T, 3, 3>>;

template <std::floating_point T>
auto inverse(const matrix<T, 4, 4>& value) noexcept
    -> xer::result<matrix<T, 4, 4>>;
```

### エラー処理

行列が特異である、または実装された計算に対して特異に近すぎる場合、`inverse` は失敗を返します。

現在の実装では、これを `error_t::divide_by_zero` として報告します。逆行列演算には使用可能なピボット値による除算が必要だからです。

---

## 2D アフィン変換補助関数

2D 同次列ベクトル向けに、このヘッダーは 3x3 変換補助関数を提供します。

```cpp
template <std::floating_point T>
auto translate2(T tx, T ty) noexcept -> matrix3<T>;

template <std::floating_point T>
auto scale2(T sx, T sy) noexcept -> matrix3<T>;

template <std::floating_point T>
auto rotate2(cyclic<T> theta) noexcept -> matrix3<T>;
```

### 回転方向

`rotate2` は `cyclic<T>` の τrad 角を受け取り、通常の数学的慣習に従います。正の角度は反時計回りの回転です。

---

## 3D アフィン変換補助関数

3D 同次列ベクトル向けに、このヘッダーは 4x4 変換補助関数を提供します。

```cpp
template <std::floating_point T>
auto translate3(T tx, T ty, T tz) noexcept -> matrix4<T>;

template <std::floating_point T>
auto scale3(T sx, T sy, T sz) noexcept -> matrix4<T>;

template <std::floating_point T>
auto rotate_x(cyclic<T> theta) noexcept -> matrix4<T>;

template <std::floating_point T>
auto rotate_y(cyclic<T> theta) noexcept -> matrix4<T>;

template <std::floating_point T>
auto rotate_z(cyclic<T> theta) noexcept -> matrix4<T>;
```

### 回転単位

回転補助関数は τrad 単位の `cyclic<T>` 角を受け取ります。

`0.25` は 4 分の 1 回転、`0.5` は半回転、`1.0` は 1 回転を表します。

これは、xer の他の数学 API で使う角度規約と一致します。

---

## 初期行列機能の範囲

初期の行列機能は意図的に小さくしています。

重点は次の項目です。

* 3x3 行列による 2D アフィン変換
* 4x4 行列による 3D アフィン変換
* 同次列ベクトル
* 3x3 および 4x4 行列の逆変換

最初から完全な線形代数ライブラリを提供しようとはしません。

後回しまたは意図的に省略している項目には次があります。

* 動的サイズ行列
* 分解アルゴリズム
* 固有値または固有ベクトル
* 専用ベクトルクラス
* 行列式 API
* 完全な数値線形代数機能

これらは必要になった場合にだけ、後から検討します。

---

## 他のヘッダーとの関係

`<xer/matrix.h>` は次と合わせて理解してください。

* `policy_project_outline.md`
* `policy_arithmetic.md`
* `header_arithmetic.md`
* `header_cyclic.md`
* `header_quantity.md`

おおまかな境界は次のとおりです。

* `<xer/arithmetic.h>` はスカラー算術と比較補助機能を扱う
* `<xer/cyclic.h>` は正規化角度や方向のような循環値を扱う
* `<xer/quantity.h>` は物理量と単位を扱う
* `<xer/matrix.h>` は固定サイズ行列とアフィン変換を扱う

---

## ドキュメント上の注意

生成マニュアルでこのヘッダーを説明するときは、通常は次を説明すれば十分です。

* 行列型は固定サイズで行優先であること
* 列ベクトルは `matrix<T, N, 1>` のエイリアスで表されること
* 初期の重点は 2D / 3D アフィン変換であること
* 3x3 および 4x4 行列の逆行列計算を提供すること
* 回転補助関数は τrad 単位の `cyclic<T>` 角を受け取ること

詳細な数値挙動や将来の線形代数拡張は、それらの機能が追加された時点で別途文書化します。

---

## 例として示す価値が高い題材

このヘッダーでは、次のような例が特に適しています。

* 2D アフィン変換を点へ適用する
* 平行移動、拡大縮小、回転変換を合成する
* 3D アフィン変換を点へ適用する
* 逆変換を計算して元の点を復元する

これらは `examples/` の実行可能例のよい候補です。

---

## 例

```cpp
#include <xer/matrix.h>

auto main() -> int
{
    const xer::vector3<double> point{2.0, 3.0, 1.0};

    const auto transform =
        xer::translate2(10.0, 20.0) *
        xer::scale2(1.5, 4.0);

    const auto transformed = transform * point;
    const auto inverse = xer::inverse(transform);
    if (!inverse.has_value()) {
        return 1;
    }

    const auto restored = *inverse * transformed;

    static_cast<void>(restored);
    return 0;
}
```

この例は通常の xer スタイルを示しています。

* 点を同次列ベクトルとして表す
* 行列積で変換を合成する
* 行列と点を乗算して変換を適用する
* 逆行列を計算するときは `xer::result` を明示的に確認する

---

## 関連項目

* `policy_project_outline.md`
* `policy_arithmetic.md`
* `header_arithmetic.md`
* `header_cyclic.md`
* `header_quantity.md`
