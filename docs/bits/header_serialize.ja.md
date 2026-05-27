<!-- xer-reference-source-sha256: aa8a282e23405c592fe1c89b2303671e3141444596eba3f0f199adb6ee2b9a4d -->
# `<xer/serialize.h>`

## 目的

`<xer/serialize.h>` は、固定スキーマのデータを扱うための低水準バイナリ転送アーカイブを提供します。

この設計は意図的に単純です。型名、フィールド名、スキーマ情報、オブジェクト識別子、バージョンメタデータは保存しません。呼び出し側と生成コードは、正確なフィールド順序とフィールド型を知っていることを前提にします。

このヘッダーは、生成された `xfer` 関数を支えることを目的としています。スキーマジェネレータは、1つのフィールド転送関数を出力し、`binary_output_archive` または `binary_input_archive` のどちらを渡すかによって、出力と入力の両方に同じ関数を使用できます。

推奨される作業手順は次のとおりです。

1. PHP で固定構造スキーマを定義する
2. C++ 構造体とその `xfer` 関数を生成する
3. その構造体をバイナリ出力アーカイブまたはバイナリ入力アーカイブに渡す
4. 形式のバージョン管理は低水準アーカイブ層の外側で扱う

---

## 設計モデル

xer のシリアライズは、リフレクションベースのシリアライザではありません。

C++23 には、任意のユーザー定義構造体のフィールドを列挙できる標準リフレクション機能がありません。マクロ、実行時登録、侵入的な基底クラス、重いテンプレートメタプログラミングに頼る代わりに、xer は生成されたフィールド転送関数を使用します。

低水準アーカイブ層が知っているのは、スカラ値と一部の標準コンテナの転送方法だけです。ユーザー定義構造体は、固定順序で各フィールドごとにアーカイブを1回呼び出す生成コードによって扱います。

これにより、バイナリ形式をコンパクトで予測しやすいものに保てます。

---

## バイナリ形式の方針

バイナリ形式は次のように固定されています。

- スカラ値は直接格納します
- 複数バイトのスカラ値はリトルエンディアンです
- `float` は IEEE 754 binary32 のバイト列として格納します
- `double` は IEEE 754 binary64 のバイト列として格納します
- `bool` は1バイトで、`0` または `1` として格納します
- `std::u8string` は `uint64 byte_size` に続けて UTF-8 バイト列を格納します
- `std::vector<std::byte>` は `uint64 byte_size` に続けて生バイト列を格納します
- `std::array<T, N>` は要素だけを格納し、配列サイズは格納しません
- `std::vector<T>` は `uint64 element_count` に続けて要素を格納します
- `std::map<K, V>` は `uint64 element_count` に続けて map の反復順で key/value ペアを格納します

バイトオーダーマーカーは書き込みません。リトルエンディアンであることは形式の一部です。

この形式は、双方が同じスキーマを共有している場合、または同じスキーマソースから生成されている場合に適しています。意図的に自己記述型の交換形式にはしていません。

---

## 主なエンティティ

```cpp
class binary_output_archive;
class binary_input_archive;
```

アーカイブオブジェクトは、個別の `write` / `read` 関数名ではなく `operator()` を公開します。これにより、生成コードは入出力の両方向に対して1つの転送関数を使用できます。

```cpp
template<class Archive>
auto xfer(Archive& archive, sample& value) -> xer::result<void>
{
    if (auto r = archive(value.id); !r) {
        return r;
    }
    if (auto r = archive(value.name); !r) {
        return r;
    }
    return {};
}
```

`archive` が `binary_output_archive` の場合、値は書き込まれます。`binary_input_archive` の場合、同じフィールドに値が読み込まれます。

---

## 対応型

初期の低水準実装は、次の型に直接対応します。

```cpp
bool

std::uint8_t
std::uint16_t
std::uint32_t
std::uint64_t

std::int8_t
std::int16_t
std::int32_t
std::int64_t

float
double

std::u8string
std::vector<std::byte>

std::array<T, N>
std::vector<T>
std::map<K, V>
```

コンテナの要素型も、同じアーカイブで対応している型でなければなりません。

`int`、`long`、`std::size_t` のような環境依存の整数型は、直接のシリアライズ対象型として意図的に提供していません。シリアライズされる構造体では、固定幅整数型を使用してください。

`std::u8string_view` や `std::span` のような非所有型は、生成される構造体のフィールド型として使うことを想定していません。明示的に対応している箇所では出力側の便宜的な引数として現れることはありますが、シリアライズされる構造体では所有型のフィールドを使うべきです。

---

## `binary_output_archive`

```cpp
class binary_output_archive;
```

`binary_output_archive` は内部バイトバッファを所有し、シリアライズされたデータを末尾に追加します。

```cpp
auto bytes() const noexcept -> std::span<const std::byte>;
auto release() noexcept -> std::vector<std::byte>;
```

`bytes` は現在のバッファに対する非所有ビューを返します。このビューは、その後の書き込みや `release` によって無効になります。

`release` は蓄積されたバイト列をムーブして取り出し、アーカイブを空にします。

アーカイブは、対応する出力型に対して `operator()` のオーバーロードを提供します。

```cpp
xer::binary_output_archive out;
std::uint32_t id = 42;
std::u8string name = u8"xer";

out(id);
out(name);
```

出力関数は、対称性を保ち、アロケーションやサイズに関する失敗を報告するために `xer::result<void>` を返します。

---

## `binary_input_archive`

```cpp
class binary_input_archive;
```

`binary_input_archive` は、バイト span からシリアライズされたデータを読み取ります。

```cpp
explicit binary_input_archive(std::span<const std::byte> data) noexcept;

auto remaining_size() const noexcept -> std::size_t;
auto empty() const noexcept -> bool;
```

アーカイブは、対応する入力型に対して `operator()` のオーバーロードを提供します。

```cpp
xer::binary_input_archive in(bytes);
std::uint32_t id{};
std::u8string name;

in(id);
in(name);
```

入力関数は、与えられた参照に結果を格納し、`xer::result<void>` を返します。

要求された値を読むのに十分なバイトが入力に含まれていない場合、関数は次を返します。

```cpp
error_t::end_of_file
```

シリアライズ入力中の不正な bool 値や重複した map キーは、次として報告されます。

```cpp
error_t::invalid_argument
```

過度に大きな長さ値は、次として報告されます。

```cpp
error_t::length_error
```

---

## 手書きの `xfer` 関数

手書きの転送関数は、固定順序で各フィールドに対してアーカイブを呼び出すべきです。

```cpp
struct sample {
    std::uint32_t id;
    std::u8string name;
    std::vector<std::uint16_t> flags;
};

template<class Archive>
auto xfer(Archive& archive, sample& value) -> xer::result<void>
{
    if (auto r = archive(value.id); !r) {
        return r;
    }
    if (auto r = archive(value.name); !r) {
        return r;
    }
    if (auto r = archive(value.flags); !r) {
        return r;
    }
    return {};
}
```

同じ関数を出力と入力の両方に使用します。

```cpp
sample source{};
xer::binary_output_archive out;
xfer(out, source);

auto data = out.release();

sample restored{};
xer::binary_input_archive in(data);
xfer(in, restored);
```

このパターンは `examples/example_serialize_basic.cpp` で示しています。

---

## 生成された構造体と `xfer` 関数

通常の用途では、xer は構造体と `xfer` 関数を手書きするのではなく、スキーマファイルから生成することを推奨します。

スクリプトは次のとおりです。

```text
php/generate_xfer_struct.php
```

スキーマファイルは、配列を返す PHP ファイルです。短い型トークンは、スキーマファイルが読み込まれる前にジェネレータによって定義されます。

```php
<?php

declare(strict_types=1);

return [
    'namespace' => 'demo',
    'struct' => 'record',
    'fields' => [
        'id' => u32,
        'name' => s,
        'payload' => bin,
        'scores' => [[s, f64], m],
        'history' => [u16, v],
        'fixed' => [u32, [a, 4]],
    ],
];
```

ジェネレータのコマンドは次のとおりです。

```text
php php/generate_xfer_struct.php schema.php record.hpp
```

再現可能な生成例やテストのために、タイムスタンプを明示的に指定できます。

```text
php php/generate_xfer_struct.php schema.php record.hpp --generated-at=2026-05-27T00:00:00+00:00
```

`--generated-at` を省略した場合は、現在時刻が ISO 8601 形式で埋め込まれます。

生成されるヘッダーには次が含まれます。

- `struct record`
- `template<class Archive> auto xfer(Archive& ar, record& value) -> xer::result<void>`
- 生成時スキーマタイムスタンプ定数

生成時タイムスタンプは、ファイルコメントと、次のような C++ 定数の両方に埋め込まれます。

```cpp
inline constexpr char record_xfer_schema_generated_at[] = "2026-05-27T13:29:13+00:00";
```

このタイムスタンプは、シリアライズされたバイナリペイロードには保存されません。ヘッダーの生成に使われたスキーマソースを識別するためのものです。

この生成ワークフローは、次のファイルで示しています。

```text
examples/example_serialize_generated_schema.php
examples/example_serialize_generated.hpp
examples/example_serialize_generated.cpp
```

1つのスキーマから複数の構造体をまとめて生成することもできます。

```php
<?php

declare(strict_types=1);

return [
    'namespace' => 'demo',
    'structs' => [
        'packet_header' => [
            'version' => u16,
            'kind' => u16,
            'sequence' => u32,
        ],
        'sensor_sample' => [
            'id' => u32,
            'name' => s,
            'values' => [f32, v],
            'calibration' => [[s, f64], m],
            'raw' => [u8, [a, 8]],
        ],
    ],
];
```

これは両方の構造体と、それぞれの構造体に対応する `xfer` 関数を1つずつ出力します。生成されるヘッダーは、選択されたフィールド型に必要な標準ヘッダーだけをインクルードします。複数構造体のワークフローは、次のファイルで示しています。

```text
examples/example_serialize_generated_multi_schema.php
examples/example_serialize_generated_multi.hpp
examples/example_serialize_generated_multi.cpp
```

---

## PHP スキーマ型 DSL

`php/generate_xfer_struct.php` は、次のスカラトークンに対応しています。

```text
b

u8 u16 u32 u64

i8 i16 i32 i64

f32 f64

s
bin
```

これらは C++ 型に次のように対応します。

```text
b   -> bool

u8  -> std::uint8_t
u16 -> std::uint16_t
u32 -> std::uint32_t
u64 -> std::uint64_t

i8  -> std::int8_t
i16 -> std::int16_t
i32 -> std::int32_t
i64 -> std::int64_t

f32 -> float
f64 -> double

s   -> std::u8string
bin -> std::vector<std::byte>
```

コンテナ形式は、型修飾子として記述します。

```php
[T, v]          // std::vector<T>
[T, [a, N]]     // std::array<T, N>
[[K, V], m]     // std::map<K, V>
```

例:

```php
[u32, v]          // std::vector<std::uint32_t>
[u32, [a, 16]]    // std::array<std::uint32_t, 16>
[[s, f64], m]     // std::map<std::u8string, double>
```

`std::array<T, N>` は、バイナリペイロードに `N` を保存しません。`std::vector<T>`、`std::map<K, V>`、`std::u8string`、`std::vector<std::byte>` は、内容の前に `uint64` の長さを保存します。

---

## バージョニングモデル

この形式は自己記述型ではありません。構造体がバージョン間で変わる場合、呼び出し側は低水準アーカイブ層の外側で互換性を扱うべきです。

一般的な戦略には、次のようなものがあります。

- 構造体の先頭にバージョンフィールドを置く
- ペイロードを外側のバージョン付き構造体で包む
- 古いレイアウトと新しいレイアウトに対して別々の `xfer` 関数を生成する
- 適している場合は、追加を末尾フィールドに限定する
- 診断に有用な場合は、スキーマ生成時タイムスタンプを帯域外で交換する

`<xer/serialize.h>` は、この方針を意図的に低水準アーカイブ実装の外側に置いています。

---

## ZIP との関係

`<xer/serialize.h>` はバイト列を生成し、消費します。それらのバイト列は、そのまま保存したり、ストリームで送信したり、Base64 でエンコードしたり、ZIP アーカイブに入れたりできます。

シリアライズ層は、それ自体ではデータを圧縮しません。圧縮とアーカイブ処理は `<xer/zip.h>` または将来の圧縮ユーティリティの担当です。
