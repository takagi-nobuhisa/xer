<!-- xer-reference-source-sha256: 6f608f9b28a6910acc13a5d6b9700e846074b8ec78984352f197116fbc4affd3 -->

# `<xer/interval.h>`

## 目的

`<xer/interval.h>` は、境界付き浮動小数点値型を提供します。

主なエンティティは `xer::interval<T, Min, Max>` です。これは、固定された閉区間内に制約された有限スカラー値を格納する軽量な値型です。

既定の区間は `[0, 1]` です。この区間は、色成分、アルファ値、正規化比率、不透明度、明るさ、ゲインなど、境界を持つ制御値に有用です。

---


次の図は、`interval<T>` と `cyclic<T>` の違いを比較したものです。

![xer cyclic and interval concepts](images/cyclic_interval_concepts.png)

## 主なエンティティ

`<xer/interval.h>` は、少なくとも次のエンティティを提供します。

```cpp
template <
    std::floating_point T,
    T Min = static_cast<T>(0),
    T Max = static_cast<T>(1)>
class interval;
```

実装は対応する内部ヘッダーで提供されます。

```cpp
#include <xer/bits/interval.h>
```

ユーザーコードでは、通常、公開ヘッダーをインクルードします。

```cpp
#include <xer/interval.h>
```

---

## 設計上の役割

`interval` は、不変条件を維持することを主な目的とする小さな数値値型です。

`interval<T, Min, Max>` では、格納値は常に次を満たします。

```text
Min <= value() <= Max
```

格納値は常に有限値です。

有限の範囲外値は、もっとも近い境界値へクランプされます。`NaN` や無限大のような無効な浮動小数点値は、例外を投げて拒否されます。

これにより、通常の使用中に既知の範囲から逸脱してはいけない値を扱いやすくなります。

---

## `cyclic` との関係

`interval` は `cyclic` と近い関係にありますが、表す概念は異なります。

`cyclic<T>` は `[0, 1)` に正規化される循環値を表します。

典型例は次のとおりです。

- 色相
- 角度
- 位相
- 方向

`interval<T, Min, Max>` は `[Min, Max]` 内の線形な境界付き値を表します。

典型例は次のとおりです。

- 赤、緑、青の成分
- アルファ値
- グレースケール値
- 明るさ
- ゲイン
- 不透明度
- 正規化比率

この区別は、色処理では特に重要です。色相は自然に循環しますが、色成分は循環しません。

---

## テンプレートパラメータ

```cpp
template <
    std::floating_point T,
    T Min = static_cast<T>(0),
    T Max = static_cast<T>(1)>
class interval;
```

### `T`

`T` は格納される浮動小数点型です。

主に想定される型は次のとおりです。

- `float`
- `double`
- `long double`

整数型は受け付けません。

### `Min`

`Min` は下限値です。下限は区間に含まれます。

### `Max`

`Max` は上限値です。上限は区間に含まれます。

この型は次を要求します。

```cpp
Min < Max
```

空の区間や逆向きの区間は受け付けません。

---

## 既定の区間

既定の形式は次のとおりです。

```cpp
xer::interval<float>
```

これは次を意味します。

```cpp
xer::interval<float, 0.0f, 1.0f>
```

これは正規化値にもっともよく使われる形式です。

例:

```cpp
using component = xer::interval<float>;

auto r = component(1.25f);  // 1.0f として格納
auto g = component(0.5f);   // 0.5f として格納
auto b = component(-0.25f); // 0.0f として格納
```

---

## カスタム区間

浮動小数点の非型テンプレートパラメータとして、独自の境界を指定できます。

例:

```cpp
using gain = xer::interval<float, -1.0f, 1.0f>;

auto center = gain(0.0f);
auto upper = gain(2.0f);   // 1.0f にクランプ
auto lower = gain(-2.0f);  // -1.0f にクランプ
```

これは、値が `[0, 1]` 以外の自然な範囲を持つ場合に有用です。

---

## 構築

### 既定構築

既定構築では、格納値は `Min` に初期化されます。

```cpp
xer::interval<float> x;
// x.value() == 0.0f

xer::interval<float, -1.0f, 1.0f> y;
// y.value() == -1.0f
```

### 生の値からの構築

生のスカラー値からの構築は明示的です。

```cpp
explicit constexpr interval(T value);
```

有限値は受け付けられ、区間内へクランプされます。

`xer::interval<float>` の場合:

```cpp
auto a = xer::interval<float>(0.5f);   // 0.5f
auto b = xer::interval<float>(-0.5f);  // 0.0f
auto c = xer::interval<float>(1.5f);   // 1.0f
```

`NaN` と無限大は例外で拒否されます。

---

## 例外ポリシー

`interval` は、有効な有限区間値として表現できない値に対して `std::domain_error` を投げます。

少なくとも次の場合は例外になります。

- `NaN` からの構築
- 正の無限大からの構築
- 負の無限大からの構築
- `NaN` の代入
- 無限大の代入

有限の範囲外値は例外ではなく、境界値へクランプされます。

---

## メンバー型と定数

`interval` は、格納型や境界値を参照するためのメンバー型および定数を提供します。

概念的には次のような情報を利用できます。

```cpp
using value_type = T;
static constexpr T min_value = Min;
static constexpr T max_value = Max;
```

これにより、ジェネリックコードで区間の値型や境界を扱いやすくなります。

---

## 値へのアクセス

### `value`

```cpp
auto value() const noexcept -> T;
```

`value()` は、格納されている生の浮動小数点値を返します。

戻り値は常に有限で、`[Min, Max]` の範囲内です。

---

## 代入

### `assign`

`assign` は、新しい値を割り当てます。

有限値は区間内へクランプされます。`NaN` や無限大は例外で拒否されます。

### `T` からの代入

`interval` は、格納型 `T` からの代入をサポートします。

```cpp
xer::interval<float> x;
x = 1.5f; // 1.0f にクランプ
```

代入後も、不変条件は維持されます。

---

## 比率変換

### `ratio`

`ratio()` は、現在の値を既定の `[0, 1]` 比率に変換します。

```cpp
auto r = value.ratio();
```

`Min` は `0` に、`Max` は `1` に対応します。

### `from_ratio`

`from_ratio` は、`[0, 1]` 比率から区間値を作ります。

```cpp
auto x = my_interval::from_ratio(0.5f);
```

比率が範囲外の場合は、通常の構築と同じく境界へクランプされます。`NaN` や無限大は拒否されます。

---

## 比較

`interval` は、同じ区間型同士の比較をサポートします。

比較は格納値に基づいて行われます。

値型であるため、等価比較や順序比較は自然に扱えます。ただし、異なる区間型同士の比較については、意味を慎重に扱う必要があります。

---

## interval 値同士の算術演算

`interval` 値同士の算術演算では、演算結果も同じ区間型として格納されます。

結果が区間外に出る場合は、通常の代入・構築と同じく境界値へクランプされます。無効な浮動小数点結果は例外で拒否されます。

---

## 右辺スカラー値との算術演算

`interval` 値と右辺スカラー値の演算もサポートされます。

```cpp
auto x = xer::interval<float>(0.5f);
auto y = x * 2.0f;
```

演算結果は区間内へ収められます。

---

## 左辺スカラー値との乗算

スカラー値を左辺に置いた乗算も、意味が明確なものについてはサポートされます。

```cpp
auto y = 2.0f * x;
```

これは、係数を掛ける操作として自然に読めるためです。

---

## サポートしない左辺スカラー形式

一方、左辺スカラーの加算や減算など、意味が曖昧になりやすい形は意図的に提供しない場合があります。

`interval` は任意の数値の代替ではなく、境界付き値を表す型です。演算子の範囲は、その意味が自然であるものに限定されます。

---

## 複合代入

`interval` は、対応する算術演算に合わせて複合代入を提供します。

```cpp
x += y;
x -= y;
x *= scalar;
x /= scalar;
```

各操作後も、不変条件は維持されます。

---

## 単項演算子

`interval` は、値型として自然な単項演算子を提供する場合があります。

ただし、単項マイナスのように結果が区間外へ出やすい操作では、通常のクランプ規則や例外規則に従います。

---

## エラー処理モデル

`interval` は値型であり、通常の範囲外入力はエラーではなくクランプで処理します。

一方、`NaN` や無限大は、有効な区間値として意味を持たないため例外で拒否します。

この設計により、通常の UI 値や色成分などでは扱いやすく、異常な浮動小数点値は早期に検出できます。

---

## 典型的な用途

### 色成分

```cpp
using component = xer::interval<float>;
```

RGB や CMY のような正規化色成分に適しています。

### ゲイン

```cpp
using gain = xer::interval<float, -1.0f, 1.0f>;
```

中心値を持つ制御量に適しています。

### 明るさ調整

明るさ、不透明度、正規化比率など、有限範囲内で扱いたい値にも適しています。

---

## 他のヘッダーとの関係

`<xer/interval.h>` は次のヘッダーやポリシーと関係します。

- `<xer/cyclic.h>`
- `<xer/color.h>`
- `<xer/arithmetic.h>`
- `policy_interval.md`
- `policy_color.md`

---

## ドキュメント上の注意

このヘッダーを説明するときは、次の点を明確にします。

- `interval` は境界付き浮動小数点値型であること
- 既定の区間は `[0, 1]` であること
- 有限の範囲外値はクランプされること
- `NaN` と無限大は例外で拒否されること
- `cyclic` とは異なり、循環値ではなく線形境界値であること

---

## 例

```cpp
#include <xer/interval.h>

#include <xer/stdio.h>

auto main() -> int
{
    using component = xer::interval<float>;

    const auto a = component(0.5f);
    const auto b = component(1.5f);
    const auto c = component(-0.5f);

    if (a.value() != 0.5f) {
        return 1;
    }

    if (b.value() != 1.0f) {
        return 1;
    }

    if (c.value() != 0.0f) {
        return 1;
    }

    return 0;
}
```

---

## 関連項目

- `policy_interval.md`
- `policy_color.md`
- `header_cyclic.md`
- `header_color.md`
- `header_arithmetic.md`

---

## `cyclic` との明示的な変換

`interval` と `cyclic` は、どちらも `[0, 1]` 付近の値を扱うため、表面上は似ています。

しかし、意味は異なります。

- `interval` は線形な境界付き値です。
- `cyclic` は循環する正規化値です。

そのため、両者を暗黙に混ぜるのではなく、必要に応じて明示的に変換するのが安全です。
