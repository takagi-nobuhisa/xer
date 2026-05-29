<!-- xer-reference-source-sha256: 4000a013a195689aa34b2dee7dc5cbe8d169136ec46d31413269659cbcf7a1eb -->

# `<xer/cyclic.h>`

## 目的

`<xer/cyclic.h>` は、xer における循環値を扱うための `cyclic` 型と関連する補助機能を提供します。

このヘッダーは、次のような値を対象にします。

- 角度
- 位相
- 方向
- 時刻のような循環位置
- 1 周を基準として定義されるその他の量

その役割は単なる剰余算を提供することではありません。代わりに、時計回り距離や反時計回り距離のような概念を含め、循環的な意味を明示する軽量な値型を提供します。

---

## 主な役割

`<xer/cyclic.h>` の主な役割は、次の性質を持つ循環値のための、小さく明示的なモデルを提供することです。

- 1 周に正規化される
- 折り返し挙動が必要である
- 最短差分操作が有用である
- 時計回り / 反時計回りの解釈を直接表したい

このため、このヘッダーは次のようなコードで特に有用です。

- 角度と回転
- 周期的な制御値
- UI やグラフィックスにおける方向処理
- その他の 1 周基準の量

---

## 主なエンティティ

少なくとも、`<xer/cyclic.h>` は次のエンティティを提供します。

```cpp
template <std::floating_point T>
class cyclic;

template <std::floating_point T>
auto from_degree(T value) noexcept -> cyclic<T>;

template <std::floating_point T>
auto to_degree(cyclic<T> value) noexcept -> T;

template <std::floating_point T>
auto from_rad(T value) noexcept -> cyclic<T>;

template <std::floating_point T>
auto to_rad(cyclic<T> value) noexcept -> T;

template <std::floating_point T>
auto to_rad(T value) noexcept -> T;

template <std::floating_point T>
auto from_radian(T value) noexcept -> cyclic<T>;

template <std::floating_point T>
auto to_radian(cyclic<T> value) noexcept -> T;
```

正確なオーバーロード集合は今後増える可能性がありますが、これが本質的な公開形です。

---

## `cyclic<T>`

`cyclic<T>` はこのヘッダーの中心となる型です。

これは 1 周に正規化された循環値を表します。

### 基本形

少なくとも、このクラスは次のような形を持つことが想定されます。

```cpp id="0r4t03"
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
```

したがって、このヘッダーは大きなフレームワークではなく、小さな値指向のクラステンプレートを中心とします。

---

## 内部表現

`cyclic<T>` は、1 周を `1` とする正規化済み内部表現を使います。

### 基本規則

保存される値は常に半開区間に属します。

```text
[0, 1)
```

つまり、

* `0` は基準位置
* `1` は `0` と同一視される
* 構築後および算術更新後に値は正規化される

### なぜ重要か

内部的に `[0, 1)` を使うことで、値の循環的性質を次のような外部単位からきれいに分離できます。

* 度
* ラジアン
* その他の 1 周基準の外部尺度

これにより、この型は小さく単位非依存になります。

---

## 対応する値型

`cyclic<T>` は浮動小数点型をテンプレート引数に取ります。

想定されるテンプレート引数は次です。

* `float`
* `double`
* `long double`

整数型は受け付けません。

### なぜ浮動小数点型か

主な用途では、循環値は離散的な剰余整数ではなく、連続値としてモデル化するのが自然です。

これは特に次に適しています。

* 方向制御
* 角度補間
* 位相関連処理
* リアルタイムグラフィックスや UI 処理

---

## 正規化

`cyclic<T>` オブジェクトは常に正規化済みの値を保存します。

### 意味

概念的には、正規化とは任意の値を `[0, 1)` に写すことです。

例です。

* `0.3` は `0.3` のまま
* `1.3` は `0.3` になる
* `-0.2` は `0.8` になる

### 設計方針

具体的な実装は公開上の関心事ではありません。重要なのは次の不変条件です。

```text
0 <= value < 1
```

この不変条件は、この型が提供するすべての操作の基礎です。

---

## `value()`

```cpp id="ke9w5i"
auto value() const noexcept -> T;
```

### 目的

`value()` は内部の正規化済み表現を返します。

### 意味

返される値は常に `[0, 1)` にあります。

これは型自身が使う生の循環表現です。

### 注意

これは度やラジアン値とは別物です。それらの変換は別の補助関数が扱います。

---

## 時計回り距離と反時計回り距離

`cyclic<T>` の特徴の 1 つは、円周上の方向を明示することです。

少なくとも、これは次の関数で表されます。

```cpp id="ca9elv"
auto cw(cyclic to) const noexcept -> T;
auto ccw(cyclic to) const noexcept -> T;
```

### `cw`

`cw(to)` は `this` から `to` までの時計回り距離を返します。

### `ccw`

`ccw(to)` は `this` から `to` までの反時計回り距離を返します。

### 範囲

これらの距離は次の範囲で返されます。

```text
[0, 1)
```

### なぜ重要か

この明示的な方向モデルは、`cyclic<T>` が単なる浮動小数点値と手作業の剰余算ではなく、独自の型として存在する主な理由の 1 つです。

---

## `diff`

```cpp id="j4di47"
auto diff(cyclic to) const noexcept -> T;
```

### 目的

`diff(to)` は `this` から `to` までの最短の符号付き差分を返します。

### 符号の意味

* 正は反時計回り
* 負は時計回り

### 範囲

返される値は次の範囲にあります。

```text
[-0.5, 0.5)
```

差分がちょうど半周の場合は、`-0.5` 側へ正規化されます。

### なぜ重要か

この操作は、実用的なコードで次のことを行いたい場合に特に有用です。

* 最短角度移動
* 簡潔な方向差分ロジック
* 折り返しを手作業で扱わない円周上の比較

---

## 等価判定

`cyclic<T>` は、厳密なビット単位等価を主な等価モデルとして使いません。

代わりに、明示的な近似等価補助関数を提供します。

```cpp id="v86flg"
auto eq(cyclic to) const noexcept -> bool;
auto ne(cyclic to) const noexcept -> bool;

auto eq(cyclic to, T epsilon) const noexcept -> bool;
auto ne(cyclic to, T epsilon) const noexcept -> bool;
```

### なぜ `eq` / `ne` があるか

この設計により、等価が厳密ではなく許容誤差に基づくことを明確にします。

### なぜ `==` と `!=` が主 API ではないか

通常の比較演算子を近似等価に使うと、厳密等価であるかのように誤読しやすくなります。

そのため、xer では明示的な名前付き関数を優先します。

### 既定の許容誤差

既定の許容誤差は次に保存されます。

```cpp id="tvnkmz"
static constexpr T default_epsilon;
```

これにより、浮動小数点型に応じた実用的な既定幅を提供します。

---

## 算術演算子

少なくとも、`cyclic<T>` は次の演算子を提供する可能性があります。

* 単項 `+`
* 単項 `-`
* 二項 `+`
* 二項 `-`
* `+=`
* `-=`

### 意味

これらの演算子は円周上の算術として解釈されます。

つまり、

* 結果は常に `[0, 1)` に正規化される
* 加算は円周上を前へ進むことを意味する
* 減算は円周上を後ろへ戻ることを意味する

### 重要な注意

これらは抽象的な数学上の普通の実数演算子ではありません。型の正規化規則によって定義される循環演算です。

---

## 比較演算子を提供しない理由

次のような順序比較演算子は、意図したモデルに含まれません。

* `<`
* `<=`
* `>`
* `>=`
* `<=>`

### 理由

順序比較は、通常の実数と同じ意味では循環値に本質的なものではありません。

同様に、`==` と `!=` も推奨される公開等価モデルではありません。意図した設計は近似比較だからです。

---

## 単位変換補助関数

`<xer/cyclic.h>` は、通常の角度単位との相互変換のための自由関数を提供します。

少なくとも次があります。

```cpp id="rwmkpt"
template <std::floating_point T>
auto from_degree(T value) noexcept -> cyclic<T>;

template <std::floating_point T>
auto to_degree(cyclic<T> value) noexcept -> T;

template <std::floating_point T>
auto from_rad(T value) noexcept -> cyclic<T>;

template <std::floating_point T>
auto to_rad(cyclic<T> value) noexcept -> T;

template <std::floating_point T>
auto to_rad(T value) noexcept -> T;

template <std::floating_point T>
auto from_radian(T value) noexcept -> cyclic<T>;

template <std::floating_point T>
auto to_radian(cyclic<T> value) noexcept -> T;
```

### なぜ自由関数か

単位変換は `cyclic` オブジェクト自身の責務とは扱いません。

これにより、型の内部を単位非依存に保ちつつ、API 境界で変換できます。

### 意味

これらの関数は次の間で変換します。

* 外部の度 / ラジアン値
* 1 周を `1` とする τrad スカラー値
* 内部の 1 周基準表現

`from_rad` と `to_rad` は、ラジアン変換の推奨される短い名前です。`from_radian` と `to_radian` は互換性のための別名として残ります。

`to_degree(T)` と `to_rad(T)` は、`cw`、`ccw`、`diff`、`angle` の戻り値のような τrad スカラー値も受け取ります。これらのスカラーオーバーロードは入力を正規化しません。

---

## 数学定数との関係

ラジアン変換は自然に π に依存します。

xer の設計では、π のような数学定数を `cyclic<T>` のメンバーとして直接埋め込みません。代わりに、それらは専用の内部定数支援と概念的に関係する別の支援機能として扱います。

これにより、`cyclic<T>` 自体は一般的な定数提供ではなく、循環値処理へ集中できます。

---

## 他のヘッダーとの関係

`<xer/cyclic.h>` は次と合わせて理解してください。

* `policy_project_outline.md`
* `policy_cyclic.md`
* `header_quantity.md`

おおまかな境界は次のとおりです。

* `<xer/cyclic.h>` は循環値と循環操作を扱う
* `<xer/quantity.h>` は物理量と単位を扱う
* 角度量は通常の量として表せる一方、`cyclic` は循環的意味が明示的に必要なときに使う

この区別は xer の設計で重要です。

---

## 角度量との関係

xer の設計で重要なのは、`cyclic<T>` がすべての角度量の普遍的な保存モデルでは **ない** という点です。

### 意味

* 回転数を含む通常の角度量は、単位付きの量としてモデル化する方がよい
* `cyclic<T>` は循環的解釈のためのもの
* 最短差分、時計回り距離、反時計回り距離が本当の関心事であるときに `cyclic<T>` へ変換する

これにより、`cyclic<T>` はあらゆる角度風の値を置き換えるものではなく、焦点を絞った実用的な道具になります。

---

## ドキュメント上の注意

生成マニュアルでこのヘッダーを説明するときは、通常は次を説明すれば十分です。

* `cyclic<T>` は値を 1 周に正規化して保存すること
* 時計回り距離と反時計回り距離が明示的な操作であること
* 最短の符号付き差分は `diff` で提供されること
* 等価は近似的で、`eq` / `ne` で表すこと
* 度 / ラジアン変換は自由関数で扱うこと

詳細な数値上の端のケースは、詳細リファレンスまたは生成 API 節に属します。

---

## 例として示す価値が高い題材

このヘッダーでは、次のような例が特に適しています。

* 生の 1 周基準値から `cyclic<float>` を構築する
* `from_degree` で度から変換する
* `to_degree` で度へ変換する
* 時計回り距離と反時計回り距離を測る
* `diff` で最短差分を計算する
* `eq` で値を比較する

これらは `examples/` の実行可能例のよい候補です。

---

## 例

```cpp
#include <xer/cyclic.h>

auto main() -> int
{
    const auto a = xer::from_degree(36.0);
    const auto b = xer::from_degree(108.0);

    const auto d = a.diff(b);
    if (d <= 0.0) {
        return 1;
    }

    const auto deg = xer::to_degree(a);
    if (deg != 36.0) {
        return 1;
    }

    return 0;
}
```

この例は通常の xer スタイルを示しています。

* 自由変換補助関数で循環値を作る
* 循環操作を明示的に使う
* 円周上の方向を API 自身の一部として扱う

---

## 関連項目

* `policy_project_outline.md`
* `policy_cyclic.md`
* `header_quantity.md`

---

## 比率変換

`cyclic<T>` は `interval` との対称性のために、比率指向のメンバー関数を提供します。

```cpp
constexpr auto ratio() const noexcept -> T;
static constexpr auto from_ratio(T ratio) noexcept -> cyclic;
```

`ratio()` は `[0, 1)` の正規化済み内部位置を返します。これは `value()` の別名です。

`from_ratio()` は 1 周基準の比率から循環値を構築し、通常の循環正規化を適用します。

```cpp
auto a = xer::cyclic<float>::from_ratio(1.25f);
// a.value() == 0.25f
```

---

## `interval` との明示的変換

`cyclic` と `interval` の暗黙変換は提供しません。端点の意味が異なるため、変換はソースコード上で見えるべきです。

interval ヘッダーは明示的な補助関数を提供します。

```cpp
auto to_cyclic(interval<T, Min, Max> value) noexcept -> cyclic<T>;
auto to_interval(cyclic<T> value) -> interval<T>;
```

`to_cyclic` は interval をその比率を通じて写します。`to_interval` は cyclic 値を既定 interval `[0, 1]` へ写します。

カスタム境界の interval には、`interval<T, Min, Max>::from_ratio(value.ratio())` を使ってください。
