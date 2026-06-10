<!-- xer-reference-source-sha256: 87ff0b0a3d009579403f1f67c06c74310b385071fd2c9c7a3c80196f3ae80066 -->

# `<xer/iostream.h>`

## 目的

`<xer/iostream.h>` は、選択された xer の値型に対して、標準 iostream の書式付き挿入演算子と抽出演算子を提供します。

このヘッダーは、iostream を xer の主たる入出力モデルにするためのものではありません。xer の基本的な入出力モデルは、引き続き `binary_stream`、`text_stream`、および `<xer/stdio.h>` が提供する関数を中心とします。`<xer/iostream.h>` の役割はより限定的で、診断、テスト、例、および汎用 `%@` 書式化・読み取り支援の実装に使う橋渡しです。

このヘッダーは明示的に opt-in です。通常の C++ iostream 演算子を xer の値型に対して使いたい場合にだけインクルードします。

---

## 主なエンティティ

少なくとも、`<xer/iostream.h>` は次の型に対して `operator<<` を利用可能にします。`error_t` と `error<void>` の挿入演算子は `<xer/error.h>` で定義され、このヘッダーがそれをインクルードするため利用できます。

```cpp
xer::error_t
xer::error<void>
xer::type_info
xer::path
xer::cyclic<T>
xer::interval<T, Min, Max>
xer::quantity<T, Dim>
xer::matrix<T, Rows, Cols>
xer::vec<T, N>
xer::polar<T, 2>
xer::image::point
xer::image::pointf
xer::image::size
xer::image::sizef
xer::image::rect
xer::image::rectf
xer::basic_rgb<T>
xer::basic_gray<T>
xer::basic_cmy<T>
xer::basic_hsv<T>
xer::basic_xyz<T>
xer::basic_lab<T>
xer::basic_luv<T>
```

また、書式付き抽出が素直に定義できる次の型について `operator>>` も提供します。

```cpp
xer::error_t
xer::path
xer::cyclic<T>
xer::interval<T, Min, Max>
xer::quantity<T, Dim>
xer::matrix<T, Rows, Cols>
xer::vec<T, N>
xer::polar<T, 2>
xer::image::point
xer::image::pointf
xer::image::size
xer::image::sizef
xer::image::rect
xer::image::rectf
xer::basic_rgb<T>
xer::basic_gray<T>
xer::basic_cmy<T>
xer::basic_hsv<T>
xer::basic_xyz<T>
xer::basic_lab<T>
xer::basic_luv<T>
```

これらの抽出演算子は、診断向けの簡便な書式付き入力と、汎用 `%@` 読み取り支援のためのものです。JSON、TOML、CSV、xer 独自の直列化機能のような安定したファイル形式を置き換えるものではありません.

---

## 設計上の役割

このヘッダーの主な役割は、xer が提供する値型を、ストリームベースの汎用書式化経路で扱えるようにすることです。

特に、printf / scanf 系の拡張 `%@` の既定処理のように、内部的にストリーム挿入または抽出に依存する機能を支援します。

これらの演算子は、xer 自身のテキスト入出力 API を置き換えることを意図していません。

---

## UTF-8 の扱い

xer は UTF-8 テキストに `char8_t` と `std::u8string_view` を使います。一方、通常の iostream は `char` ストリームです。

そのため、`<xer/iostream.h>` は `path` や `type_info` のような UTF-8 指向の表示文字列を持つ型について、UTF-8 の基礎バイト列をロケール依存の変換なしで `std::ostream` に書き込みます。

`std::istream` からの書式付き抽出では、通常の narrow トークンを読み取り、必要に応じてそのバイト列を UTF-8 ストレージへコピーします。

---

## `error_t`

`error_t` の `operator<<` は、`get_error_name` が返す列挙子名を書き込みます。

```text
invalid_argument
not_found
io_error
```

`operator>>` は `error_t::` 接頭辞を付けない列挙子名を受け付けます。未知の名前を読んだ場合、ストリームの fail bit を設定します。

---

## `error<void>`

`error<void>` の `operator<<` は、`<xer/error.h>` が提供する短い診断表現を書き込みます。

```text
xer::error{code=invalid_argument}
```

エラーオブジェクトに保存されたソース位置は出力されません。これは、アサーションメッセージ、トレース出力、`%@` 書式化に適した短い既定表現にするためです。

`error<void>` の抽出演算子はありません。エラーオブジェクトは通常、書式付き入力から読むものではなく、失敗した操作によって作られるものです。

---

## `type_info`

`type_info` の `operator<<` は、`type_info::name()` が返す表示名を書き込みます。

これは診断用です。表記は基礎となる型情報機構と同様に実装依存です。

`type_info` の抽出演算子はありません。

---

## `path`

`path` の `operator<<` は、`path::str()` が返す正規化済み UTF-8 パスを書き込みます。

`operator>>` は空白で区切られた 1 個のトークンを読み取り、それから `path` を構築します。書式付き抽出の通常の性質どおり、空白を含むパスはこの演算子では扱いません。そのようなパスが必要な場合は、行単位入力を行い、明示的に `xer::path` を構築してください。

---

## `cyclic<T>`

`cyclic<T>` の `operator<<` は、`value()` が返す正規化済みスカラー値を書き込みます。

`operator>>` はスカラー値を読み取り、それから `cyclic<T>` を構築します。通常の `cyclic` 正規化規則が適用されます。

たとえば、

```text
-0.25
```

は次の正規化済み循環値として読み込まれます。

```text
0.75
```

---

## `interval<T, Min, Max>`

`interval<T, Min, Max>` の `operator<<` は、`value()` が返す保存済みスカラー値を書き込みます。

`operator>>` はスカラー値を読み取り、それから interval 値を構築します。通常の `interval` 規則が適用されます。有限の範囲外値はクランプされ、NaN や無限大のような不正な浮動小数点値が interval 構築で拒否される場合は、抽出によりストリームの fail bit が設定されます。

---

## `quantity<T, Dim>`

`quantity<T, Dim>` の `operator<<` は、基準単位系での保存値を書き込みます。

`operator>>` はスカラー値を読み取り、宛先次元の量を構築します。入力値はすでに基準単位系へ正規化済みであるものとして解釈されます。

たとえば宛先型が長さの量であれば、入力値はキロメートルやセンチメートルではなくメートルとして解釈されます。

---

## `matrix<T, Rows, Cols>`

`matrix<T, Rows, Cols>` の `operator<<` は、行優先の短い診断形式を書き込みます。

2x2 行列の出力例です。

```text
[[1, 2], [3, 4]]
```

現時点では行列の抽出演算子はありません。行列の構文解析を提供すると、診断用出力形式を安定した入力文法として固定する必要があるため、今は意図的に避けています。

---

## 数学ジオメトリ型

`vec<T, N>` と `polar<T, 2>` は、`<xer/math.h>` が提供する幾何値型です。

これらの挿入演算子は、診断しやすい短い表現を書き込みます。
抽出演算子は、対応する診断表現から値を読み取るために用意されています。

---

## 画像ジオメトリ型

`xer::image` の画像ジオメトリ補助型には、`operator<<` と `operator>>` が提供されます。

ストリーム形式は意図的に短くしています。

```text
point  -> (x, y)
size   -> {width, height}
rect   -> (x, y) {width, height}
```

浮動小数点版も同じ表記です。

```text
pointf -> (x, y)
sizef  -> {width, height}
rectf  -> (x, y) {width, height}
```

抽出は同じ形式を受け付け、句読点や値の周囲の通常の書式付き入力空白を許します。たとえば `rect` の読み取りでは次のいずれも有効です。

```text
(10,20){30,40}
(10, 20) {30, 40}
( 10, 20 ) { 30, 40 }
```

抽出文法は句読点について意図的に厳密です。点は丸括弧、サイズは波括弧、矩形は点に続くサイズです。`point(10, 20)`、`size(30, 40)`、`rect(10, 20, 30, 40)` のような形式は受け付けません。

これらの演算子により、画像ジオメトリ値はストリーム挿入・抽出に依存する汎用 `%@` 書式化・読み取り経路でも利用できます。

---

## 色型

基本色値型には `operator<<` が提供されます。

出力例です。

```text
rgb(1, 0.5, 0)
gray(0.25)
cmy(0, 0.5, 1)
hsv(0.25, 0.5, 1)
xyz(0.1, 0.2, 0.3)
lab(50, 10, -20)
luv(50, 10, -20)
```

現時点では色値の抽出演算子はありません。挿入形式は診断と `%@` 書式化のためのものであり、安定した直列化形式ではありません。

---

## 後回しにしている項目

現在の実装では、次の項目を意図的に後回しにしています。

- `error<Detail>` の `operator<<` / `operator>>`
- `matrix` の `operator>>`
- 色型の `operator>>`
- JSON、INI、TOML 値の `operator<<` / `operator>>`
- `binary_stream`、`text_stream`、`process`、`socket` などのリソースハンドルのストリーム挿入

これらの型は、追加の書式方針が必要であるか、既定の書式付き抽出に適した通常の値型ではありません。

---

## 他のヘッダーとの関係

`<xer/iostream.h>` は次のヘッダーと関係します。

- `<xer/error.h>`
- `<xer/typeinfo.h>`
- `<xer/path.h>`
- `<xer/cyclic.h>`
- `<xer/interval.h>`
- `<xer/quantity.h>`
- `<xer/matrix.h>`
- `<xer/image.h>`
- `<xer/color.h>`
- `<xer/stdio.h>`

おおまかな境界は次のとおりです。

- `<xer/stdio.h>` は通常の xer テキスト入出力ヘッダーであり続ける
- `<xer/iostream.h>` は標準 iostream 書式化との opt-in 互換性を提供する
- 個々の値型ヘッダーは iostream 支援を取り込まなくても利用できる

---

## 例

```cpp
#include <iostream>
#include <sstream>

#include <xer/color.h>
#include <xer/cyclic.h>
#include <xer/image.h>
#include <xer/interval.h>
#include <xer/iostream.h>
#include <xer/math.h>
#include <xer/matrix.h>
#include <xer/path.h>
#include <xer/quantity.h>

auto main() -> int
{
    using namespace xer::units;

    const auto path = xer::path(u8"work/file.txt");
    const auto angle = xer::cyclic<double>(1.25);
    const auto gain = xer::interval<double>(1.25);
    const auto distance = 1.5 * km;
    const auto transform = xer::matrix<double, 2, 2>(1.0, 2.0, 3.0, 4.0);
    const auto area = xer::image::rect(
        xer::image::point(10, 20),
        xer::image::size(30, 40));
    const auto color = xer::rgb(1.0f, 0.5f, 0.0f);

    std::cout << path << '\n';
    std::cout << angle << '\n';
    std::cout << gain << '\n';
    std::cout << distance << '\n';
    std::cout << transform << '\n';
    std::cout << color << '\n';

    std::istringstream input("logs/output.txt -0.25 0.5 2.5");
    xer::path read_path;
    xer::cyclic<double> read_angle;
    xer::interval<double> read_gain;
    xer::quantity<double, xer::units::length_dim> read_distance;

    input >> read_path >> read_angle >> read_gain >> read_distance;
    return input ? 0 : 1;
}
```

---

## 関連項目

- `header_error.md`
- `header_typeinfo.md`
- `header_path.md`
- `header_cyclic.md`
- `header_interval.md`
- `header_quantity.md`
- `header_matrix.md`
- `header_image.md`
- `header_color.md`
- `header_stdio.md`
