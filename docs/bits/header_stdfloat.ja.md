<!-- xer-reference-source-sha256: ad1dd9d32be7699ffc78158d6b5145ff0a9dbb0a4f3a06943cf4825306c59287 -->
# `<xer/stdfloat.h>`

## 目的

`<xer/stdfloat.h>` は、`<xer/stdint.h>` と同じ考え方で、浮動小数点型の別名と浮動小数点ユーザー定義リテラルを提供します。

このヘッダーは、C++23 の `<stdfloat>` 対応が不完全な実装でも使いやすさを保ちながら、浮動小数点の幅や最小幅に対する意図を明示できるようにすることを目的としています。

---

## 主な役割

このヘッダーは次のものを提供します。

- 実装が提供している場合の固定幅浮動小数点別名
- `float32_t` と `float64_t` の実用的なフォールバック別名
- 80 ビットおよび 128 ビット浮動小数点形式の任意の別名
- 最小幅および高速幅の浮動小数点別名
- 実装が提供している場合の任意の十進浮動小数点別名
- `xer::literals::floating_literals` 下の浮動小数点ユーザー定義リテラル

---

## 利用可能性マクロ

このヘッダーは、任意の型に対する利用可能性マクロを定義します。

例です。

```cpp
XER_HAS_FLOAT16_T
XER_HAS_FLOAT32_T
XER_HAS_FLOAT64_T
XER_HAS_FLOAT80_T
XER_HAS_FLOAT128_T
XER_HAS_BFLOAT16_T
XER_HAS_FLOAT_LEAST80_T
XER_HAS_FLOAT_FAST80_T
XER_HAS_DECIMAL32_T
XER_HAS_DECIMAL64_T
XER_HAS_DECIMAL128_T
```

これらのマクロにより、コードやテストは実装サポートに依存する機能をガードできます。

---

## 二進浮動小数点別名

少なくとも、可能な場合には次の別名が提供されます。

```cpp
float16_t
float32_t
float64_t
float80_t
float128_t
bfloat16_t
```

`float32_t` と `float64_t` は xer では常に利用できます。標準の `<stdfloat>` 別名が利用できない場合、それぞれ `float` と `double` にフォールバックします。

`float80_t`、`float128_t`、`bfloat16_t` は任意であり、実装が適切な基礎型を提供している場合にのみ利用できます。

---

## 最小幅および高速幅の浮動小数点別名

このヘッダーは、次のような最小幅および高速幅の別名を提供します。

```cpp
float_least16_t
float_least32_t
float_least64_t
float_least80_t
float_least128_t

float_fast16_t
float_fast32_t
float_fast64_t
float_fast80_t
float_fast128_t
```

`float_least80_t` は、利用可能な場合は `float80_t` を使い、そうでなければ利用可能な場合に `float128_t` を使います。

---

## 最大浮動小数点別名

```cpp
floatmax_t
```

`floatmax_t` は、xer が利用できる実用上もっとも広い二進浮動小数点型から選ばれます。

---

## 十進浮動小数点別名

実装が `<decimal/decimal>` を提供している場合、xer は次のような十進浮動小数点別名を公開します。

```cpp
decimal32_t
decimal64_t
decimal128_t

decimal_least32_t
decimal_least64_t
decimal_least128_t

decimal_fast32_t
decimal_fast64_t
decimal_fast128_t

decimalmax_t
```

これらの別名は任意であり、対応する `XER_HAS_DECIMAL...` マクロでガードするべきです。

---

## 浮動小数点リテラル

浮動小数点ユーザー定義リテラルは次の名前空間に置かれます。

```cpp
xer::literals::floating_literals
```

例です。

```cpp
_f32
_f64
_f80
_f128
_fl16
_fl32
_fl64
_fl80
_fl128
_bf16
```

変換先の型が利用できるリテラルだけが提供されます。

---

## 注記

- このヘッダーは意図的に能力ベースです。
- 任意の型は、すべてのコンパイラやターゲットで保証されるものではありません。
- 任意形式に依存するコードは、対応する利用可能性マクロを確認するべきです。
- 最小幅リテラルサフィックスは、利用可能な正確な基礎型がプラットフォームによって変わり得る場合に有用です。
