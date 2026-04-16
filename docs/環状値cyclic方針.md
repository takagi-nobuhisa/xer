# 環状値 `cyclic` に関する方針

## 概要

XER では、角度、位相、方位、時刻のような **環状の値** を扱うために、`cyclic` 型および関連機能を提供する。

この機能は、単なるモジュラー算術による循環だけではなく、**時計回り** (`cw`) および **反時計回り** (`ccw`) の概念を明示的に扱えることを目的とする。

内部表現は **1 周を 1 とする正規化表現** とし、外部表現である度数法やラジアンへの変換は別途行う。

---

## 基本方針

### 対象

`cyclic` は、少なくとも次のような値を表現するために用いる。

- 角度
- 位相
- 方位
- 周期的な位置
- 1 周を基準とする任意の環状量

### 内部表現

`cyclic` の内部表現は、浮動小数点値 `T` を用いた **`[0, 1)`** の区間とする。

- `0` は基準位置を表す
- `1` は `0` と同一視し、内部表現としては保持しない
- 常に `0 <= value < 1` を満たすように正規化する

ここでの `1` は **1 周** を意味する無次元値である。

### 型

`cyclic` は浮動小数点型をテンプレート引数に取るクラステンプレートとする。

```cpp
template <std::floating_point T>
class cyclic;
```

想定する主な実引数は次のとおりである。

- `float`
- `double`
- `long double`

整数型は受け付けない。

### 想定する主な利用

`cyclic` の実用上の主な利用対象としては、少なくとも次のようなものを想定する。

- Pan / Tilt
- Hue
- 照明・映像・UI 制御における向きや色相
- 一般的なリアルタイム制御用途

これらの用途では `float` で十分なことが多いため、`cyclic` は `float` による利用を強く想定してよい。  
一方で、必要に応じて `double` や `long double` も利用できるようにする。

---

## 内部表現を `[0, 1)` とする理由

内部表現を `[0, 1)` に統一することで、外部単位に依存せずに環状性そのものを扱える。

たとえば、角度であれば次のように解釈できる。

- 度数法: `value * 360`
- ラジアン: `value * 2π`

この方針により、内部では常に同一の演算規則を用い、単位変換は入出力境界へ分離できる。

---

## 正規化方針

### 基本方針

任意の浮動小数点値から `cyclic` を構築または更新する際は、値を **`[0, 1)`** に正規化する。

### 正規化の意味

正規化は、概念的には **`x mod 1` を `[0, 1)` に写す処理**とする。

### 実装方針

実装上は、`std::floor` を用いた `x - floor(x)` 相当の処理でよい。

たとえば概念的には次のような処理である。

- `0.3` → `0.3`
- `1.3` → `0.3`
- `-0.2` → `0.8`
- `-1.2` → `0.8`

### `std::modf` を固定しない理由

正規化は小数部の取り出しとして理解できるが、実装を `std::modf` に固定しない。  
これは、**`constexpr` で実装しやすい形を優先するため**である。

### 不変条件

`cyclic` の内部値は常に次を満たす。

```text
0 <= value < 1
```

この不変条件は、コンストラクタ、代入、加減算後など、内部値を変更しうるすべての操作で維持する。

---

## 回転方向の方針

### 正方向

XER における `cyclic` の正方向は、数学的流儀に従い **反時計回り**とする。

- 正方向: `ccw`
- 負方向: `cw`

### 命名

関数名は冗長化を避け、次の短い名称を用いる。

- `cw`
- `ccw`

`clockwise` や `counterclockwise` は採用しない。

---

## `cyclic` 型の性格

`cyclic` は、浮動小数点値 1 個を保持する軽量な値型として設計する。

- 値セマンティクスを持つ
- コピー可能
- ムーブ可能
- 小さな値型として扱う

このため、メンバー関数や演算子の引数は、原則として **参照ではなく値渡し**とする。

特に `float` を用いる場合、値渡しの方が軽量かつ自然であることを重視する。

---

## 既定の許容誤差

`cyclic` における等値判定の既定許容誤差は、`cyclic` 型自身の静的メンバーとして保持する。

```cpp
static constexpr T default_epsilon =
    std::numeric_limits<T>::epsilon() * static_cast<T>(16);
```

これは、

- 型スイッチを避けること
- `float` / `double` / `long double` に自然に追従させること
- `std::numeric_limits<T>::epsilon()` そのものよりは実用的な幅を持たせること

を目的とする。

---

## メンバー関数

`cyclic` のメンバー関数は、`this` を **from**、仮引数を **to** とする向きで統一する。

### 値取得

```cpp
auto value() const noexcept -> T;
```

- 内部表現そのものを返す
- 返される値は常に `[0, 1)` に入る

### 時計回り距離

```cpp
auto cw(cyclic to) const noexcept -> T;
```

- `this` から `to` へ **時計回り**に進む距離を返す
- 返り値は `0` 以上 `1` 未満とする

### 反時計回り距離

```cpp
auto ccw(cyclic to) const noexcept -> T;
```

- `this` から `to` へ **反時計回り**に進む距離を返す
- 返り値は `0` 以上 `1` 未満とする

### 最短符号付き差

```cpp
auto diff(cyclic to) const noexcept -> T;
```

- `this` から `to` への **最短の符号付き差**を返す
- 正なら `ccw`
- 負なら `cw`

返り値の範囲は次の半開区間とする。

```text
[-0.5, 0.5)
```

したがって、ちょうど半周差の場合は `-0.5` 側へ正規化する。

### 等値判定

`cyclic` に対する等値判定は、厳密比較ではなく **誤差を考慮した実質的等値判定**とする。

このため、`==` および `!=` は採用せず、メンバー関数として `eq` および `ne` を提供する。

```cpp
auto eq(cyclic to) const noexcept -> bool;
auto ne(cyclic to) const noexcept -> bool;

auto eq(cyclic to, T epsilon) const noexcept -> bool;
auto ne(cyclic to, T epsilon) const noexcept -> bool;
```

#### 判定方法

判定は、代表値の単純差ではなく、**環上の最短差**に基づいて行う。

概念的には次の規則とする。

```text
abs(diff(to)) <= epsilon
```

#### `==` / `!=` を採用しない理由

`==` および `!=` を近似比較として定義すると、厳密比較と誤解されやすい。  
また、許容誤差付き比較であることがコード上から読み取りにくい。

そのため、`cyclic` では明示的なメンバー関数として `eq` / `ne` を採用する。

---

## 演算子

### 提供する演算子

`cyclic` には、少なくとも次の演算子を提供してよい。

- 単項 `+`
- 単項 `-`
- 二項 `+`
- 二項 `-`
- `+=`
- `-=`

### 意味

これらの演算子は、通常の実数としての加減算ではなく、**環上での加減算**として扱う。

- 加減算後の値は常に `[0, 1)` へ正規化する
- `a + b` は環上で `b` だけ進めた結果を表す
- `a - b` は環上で `b` だけ戻した結果を表す

### 比較演算子

`==`, `!=`, `<`, `<=`, `>`, `>=`, `<=>` は提供しない。

理由は次のとおりである。

- `==`, `!=` は厳密比較と誤解されやすい
- `cyclic` における等値判定は許容誤差付きである
- 順序比較は環状値として本質的な順序ではない

---

## 数学定数との関係

### 基本方針

`cyclic` におけるラジアン変換では、πを表す数学定数を利用する。

ただし、π定数は `cyclic` のメンバーとしては持たせず、`xer/bits` 下の専用ヘッダに分離する。

### 専用ヘッダ

少なくとも、次のような専用ヘッダを設けてよい。

```text
xer/bits/math_constants.h
```

このヘッダには、当面は π 定数を定義し、必要に応じて将来ほかの数学定数を追加してよい。

### 定数名

少なくとも、次の定数を定義してよい。

```cpp
template <std::floating_point T>
inline constexpr T pi_v = ...;

template <std::floating_point T>
inline constexpr T 𝜋 = pi_v<T>;
```

### `𝜋` を別名として定義する理由

1 文字のグローバル識別子は一般には敬遠されやすいが、`𝜋` は数学で広く用いられる既知の記号であり、意味も明確である。  
また、`pi_v` の正式名称を別に持たせるため、可読性や保守性の逃げ道も確保できる。

そのため、`𝜋` は正式に使用してよい **数学記号としての別名** として扱う。

---

## フリー関数による単位変換

### 基本方針

度数法およびラジアンとの変換は、`cyclic` 本体の責務ではなく、**フリー関数**として提供する。

### 理由

- `cyclic` 自体は単位を持たない内部表現である
- 単位変換は入出力境界で行う方が責務分離として自然である
- 度やラジアン以外の 1 周基準量も将来的に扱いやすい

### 例

少なくとも次のような関数を想定してよい。

```cpp
template <std::floating_point T>
auto from_degree(T value) noexcept -> cyclic<T>;

template <std::floating_point T>
auto to_degree(cyclic<T> value) noexcept -> T;

template <std::floating_point T>
auto from_radian(T value) noexcept -> cyclic<T>;

template <std::floating_point T>
auto to_radian(cyclic<T> value) noexcept -> T;
```

関数名の詳細は実装時に再検討してよいが、メンバー関数にはしない。

---

## 代表的な意味づけ

### 例: `cw` / `ccw`

内部値が

- `a = 0.1`
- `b = 0.3`

であるとき、次のように解釈する。

- `a.ccw(b)` は `0.2`
- `a.cw(b)` は `0.8`

ここで `a` は from、`b` は to である。

### 例: `diff`

- `a.diff(b)` が正なら `ccw`
- `a.diff(b)` が負なら `cw`

たとえば、`0.9` から `0.1` への最短差は `+0.2` とする。

---

## 将来的な拡張候補

将来的には、少なくとも次のような拡張を検討できる。

- 方位角専用の薄いラッパ型
- 時刻専用の薄いラッパ型
- 1 周の分割数を前提とした離散的な環状値型
- 区間や扇形のような環状区間型
- `sin`, `cos`, `atan2` 等との連携補助関数
- degree / radian 以外の単位変換関数

ただし、初期段階ではそこまで拡げず、まずは `cyclic<T>` 自体の基本設計を確立することを優先する。

---

## 暫定 API 一覧

現時点で想定する暫定 API は次のとおりである。

```cpp
template <std::floating_point T>
class cyclic {
public:
    using value_type = T;

    static constexpr T default_epsilon =
        std::numeric_limits<T>::epsilon() * static_cast<T>(16);

    constexpr cyclic() noexcept;
    constexpr explicit cyclic(T value) noexcept;

    constexpr auto value() const noexcept -> T;

    constexpr auto cw(cyclic to) const noexcept -> T;
    constexpr auto ccw(cyclic to) const noexcept -> T;
    constexpr auto diff(cyclic to) const noexcept -> T;

    constexpr auto eq(cyclic to) const noexcept -> bool;
    constexpr auto ne(cyclic to) const noexcept -> bool;

    constexpr auto eq(cyclic to, T epsilon) const noexcept -> bool;
    constexpr auto ne(cyclic to, T epsilon) const noexcept -> bool;

    constexpr auto operator+() const noexcept -> cyclic;
    constexpr auto operator-() const noexcept -> cyclic;

    constexpr auto operator+=(cyclic value) noexcept -> cyclic&;
    constexpr auto operator-=(cyclic value) noexcept -> cyclic&;
};

template <std::floating_point T>
constexpr auto operator+(cyclic<T> left, cyclic<T> right) noexcept -> cyclic<T>;

template <std::floating_point T>
constexpr auto operator-(cyclic<T> left, cyclic<T> right) noexcept -> cyclic<T>;

template <std::floating_point T>
constexpr auto from_degree(T value) noexcept -> cyclic<T>;

template <std::floating_point T>
constexpr auto to_degree(cyclic<T> value) noexcept -> T;

template <std::floating_point T>
constexpr auto from_radian(T value) noexcept -> cyclic<T>;

template <std::floating_point T>
constexpr auto to_radian(cyclic<T> value) noexcept -> T;
```

---

## 補足

`cyclic` は、単なる剰余演算のラッパではなく、**環状値としての向き付き距離と実用的比較を扱うための型**として位置付ける。

そのため、次の点を重視する。

- 内部表現は単純であること
- 外部単位と分離されていること
- `cw`, `ccw`, `diff` により方向付きの意味が明確であること
- 等値判定は厳密比較ではなく、実用的な近似比較であること
- API 名は過度に冗長でないこと
