<!-- xer-reference-source-sha256: 64e335d44a61eb5ef864c8a048256600c51f3e84c6e594a2881f3fe3a5b001f3 -->

# `<xer/string.h>`

## 目的

`<xer/string.h>` は、xer の文字列関連ユーティリティを提供します。

このヘッダーは、複数の種類の機能をまとめています。

- C風の文字列操作
- UTFを意識した文字および部分文字列の検索ヘルパー
- 分割、結合、トリムなどの PHP 由来のユーティリティ関数
- 文字列指向の機能とまとめられた生メモリヘルパー
- 接頭辞・接尾辞の検査
- 大文字小文字変換および動的な文字列変換
- エラー文字列ヘルパー

目的は C 標準ライブラリを正確に再現することではありません。
その代わりに、このヘッダーは実用的な文字列関連機能を xer 全体の設計に合う形へ再構成します。

---

## 主な役割

`<xer/string.h>` の主な役割は、xer の UTF-8 指向の公開文字列モデルを中心に、実用的なテキストおよびメモリヘルパー群を提供することです。

特に、次の目的を持ちます。

- 取っつきやすさが向上する場面では、なじみのある C風の名前を提供する
- `char8_t` に基づく UTF-8 指向の公開 API を支援する
- 実用的な範囲で、割り付けを行わないヘルパーを提供する
- 通常のコードで便利な、より高水準のユーティリティ関数を提供する

そのため、このヘッダーは標準 C ライブラリよりも意図的に、低水準機能と高水準機能を混在させています。

---

## 主な関数グループ

大きく見ると、`<xer/string.h>` には次の機能グループが含まれます。

- 検索と比較
- 大文字小文字を無視した検索と比較
- コピーと連結
- 分割 / 結合 / トリム
- 文字列置換
- 生メモリヘルパー
- 接頭辞・接尾辞の検査
- 大文字小文字変換および動的な文字列変換
- エラー文字列ヘルパー

---

## 検索と比較

少なくとも、このヘッダーは次のような関数を提供します。

```cpp
strlen
strcmp
strncmp
strchr
strrchr
strstr
strrstr
strpos
strrpos
strpbrk
strspn
strcspn
```

### このグループの役割

これらの関数は、なじみのある文字列検索および比較操作を、xer の公開文字列モデルに合わせた形で提供します。

一部は C 標準ライブラリ関数に近い考え方ですが、別のものは PHP風のユーティリティ名からの影響をより強く受けています。

### 注記

* すべての関数が C や PHP の同名関数とソース互換な完全再実装であるとは限りません
* 受け付ける引数形式は、xer 独自の API 方針に従って設計されています
* 通常の公開 API は、`xer::result` 引数ではなく通常の値に基づいて文書化されます

---

## 大文字小文字を無視した検索と比較

このヘッダーは、次のような大文字小文字を無視した操作も提供します。

```cpp
strcasecmp
strncasecmp
stricmp
strnicmp
strcasechr
strcaserchr
strichr
strirchr
strcasepos
strcaserpos
strcasestr
strcaserstr
stripos
strirpos
stristr
strirstr
```

### このグループの役割

これらの関数は、ユーザーが毎回自前で処理を組み立てなくても、実用的な大文字小文字を無視したテキスト処理を行えるようにするために存在します。

### 注記

* これらの機能は意図的に単純です
* 主に ASCII と、現在ライブラリに実装されている正規化を対象としています
* 完全なロケール依存または Unicode 全域の照合動作を保証するものとして読むべきではありません

---

## コピーと連結

少なくとも、このヘッダーは次のような関数を提供します。

```cpp
strcpy
strncpy
strcat
strncat
```

### このグループの役割

これらの関数は、なじみのあるコピーおよび連結操作を提供します。ただし、その正確な振る舞いは完全な C 互換を目指すのではなく、xer の設計に従います。

### 注記

* xer の設計では、C++ コードで自然かつ実用的に使えることを優先します
* 配列、ポインター、コンテナー風の対象に対するオーバーロードが存在する場合があります
* 動的コンテナーで自動的な容量拡張が適切な場合、歴史的な C の厳密な振る舞いよりも、その振る舞いを優先することがあります

---

## 分割 / 結合 / トリム

このヘッダーは、次のような高水準ヘルパーも提供します。

```cpp
explode
implode
ltrim
rtrim
trim
ltrim_view
rtrim_view
trim_view
```

### このグループの役割

このグループは、一部 PHP に由来する便利なテキスト処理ヘルパーを提供します。

これらの関数は、通常のコードで手作業のループや範囲操作よりもコンパクトなユーティリティ API が役立つ場面を意図しています。

### トリム関数

トリム系は 2 種類の形で提供されます。

```cpp
auto ltrim(std::u8string_view value, std::u8string_view characters = {})
    -> xer::result<std::u8string>;

auto rtrim(std::u8string_view value, std::u8string_view characters = {})
    -> xer::result<std::u8string>;

auto trim(std::u8string_view value, std::u8string_view characters = {})
    -> xer::result<std::u8string>;

auto ltrim_view(std::u8string_view value, std::u8string_view characters = {})
    -> xer::result<std::u8string_view>;

auto rtrim_view(std::u8string_view value, std::u8string_view characters = {})
    -> xer::result<std::u8string_view>;

auto trim_view(std::u8string_view value, std::u8string_view characters = {})
    -> xer::result<std::u8string_view>;
```

所有する形式は新しい `std::u8string` を返します。
`*_view` 形式は元のストレージへのビューを返し、割り付けを行いません。

`characters` が空の場合、xer は PHP 互換の既定トリム集合を使用します。

```text
space, horizontal tab, line feed, carriage return, vertical tab, NUL
```

`characters` が空でない場合、それはバイト指向の文字リストとして解釈されます。
このリストは `a..z` や `0..9` のような PHP風の範囲表記に対応します。

### UTF-8 に関する重要な注記

トリム関数は、Unicode スカラー値や書記素クラスタではなく、UTF-8 コード単位に対して動作します。
そのため、ASCII 指向の境界トリムや PHP風のバイト文字リストに適しています。

これらの関数を、一般的な Unicode 空白正規化器として使わないでください。

### `*_view` バリアント

`ltrim_view`、`rtrim_view`、`trim_view` 系は、UTF-8 指向の文字列ビューに対して、割り付けを行わないトリム操作を提供するため特に重要です。

返されるビューは `value` のストレージを参照します。
呼び出し側は、元のストレージが返されたビューより長く生存することを保証する必要があります。

### 注記

* `trim_view` 形式の関数は、軽量で便利に使えることを意図しています
* トリム文字リストはバイト指向であり、`..` 範囲に対応します
* これらのヘルパーは、通常のコードと実行可能な例のどちらでも有用です
* コード例では、必要に応じて明示的な `xer::result` チェックを行いながら、これらの関数を自然に使うことが期待されます

---

## 文字列置換

`<xer/string.h>` は、PHP 由来の文字列置換ヘルパーを提供します。

```cpp
auto str_replace(
    std::u8string_view search,
    std::u8string_view replace,
    std::u8string_view subject,
    std::size_t* count = nullptr) -> xer::result<std::u8string>;
```

### この関数の役割

`str_replace` は、`subject` の中にある `search` の重なり合わないすべての出現を `replace` に置き換えます。

引数順は PHP の `str_replace` の命名伝統に従い、検索文字列、置換文字列、対象文字列の順です。

### 振る舞い

置換は UTF-8 コード単位に対して行われます。この関数は書記素クラスタ処理を試みません。

`search` が空の場合、置換は行われず、`subject` がそのまま返されます。

`count` が `nullptr` でない場合、実行された置換回数がそこに格納されます。

### 注記

この関数は単純なテキスト置換に有用です。正規表現置換やロケール依存の変換のような、より高度なテキスト処理はこのヘルパーの範囲外です。

---

## 生メモリヘルパー

`<xer/string.h>` は、関連するメモリ機能を通じて生メモリヘルパーもまとめています。

これには、次のような操作が含まれます。

```cpp
memcpy
memmove
memchr
memrchr
memcmp
memset
```

### このグループの役割

生メモリ操作は狭い意味ではテキスト操作ではありませんが、xer は実用上の利便性のため、これらを文字列指向のヘルパーとまとめています。

これは C風プログラミングにおける文字列関数とメモリ関数の歴史的な近さを反映しつつ、公開ヘッダー構成をコンパクトに保つためのものです。

### 注記

* これらの関数は低水準ヘルパーです
* `<xer/string.h>` を通じて公開される場合、これらも公開 API の一部です
* セマンティクスは標準ライブラリからだけ推測するのではなく、xer 独自の設計とテスト範囲に従って読むべきです

---

## エラー文字列ヘルパー

このヘッダーは、次のようなヘルパーも提供します。

```cpp
strerror
get_error_name
get_errno_name
```

### このグループの役割

これらのヘルパーは、エラーカテゴリを人間が読める形式またはシンボル名の形式へ変換することを意図しています。

診断、デバッグ出力、および適切な場面でのユーザー向け報告に有用です。

### 注記

* 正確な対応付け方針は xer 独自のエラーモデルに依存します
* 自由形式のテキストよりもシンボル名が望ましい場面では、`get_error_name` と `get_errno_name` が特に有用です

---

## UTF指向の公開文字列モデル

`<xer/string.h>` は、xer の一般的なテキストモデルの文脈で理解する必要があります。

### 基本的な期待

* 公開文字列 API は通常 `char8_t` を使います
* 所有する文字列は通常 `std::u8string` を使います
* 所有しないテキストビューは通常 `std::u8string_view` を使います
* 個々の Unicode スカラー値は通常 `char32_t` で表します

ヘッダー名は C の `<string.h>` に似ていますが、実際の設計方向は異なるため、この点は重要です。

### 文字検索

UTFを意識した文字検索関数は、UTF-8、UTF-16、UTF-32 テキストを検索するために `char32_t` を受け付ける場合があります。

これにより、呼び出し箇所で単一の Unicode スカラー値を明確に表現できます。

---

## 他の方針との関係

`<xer/string.h>` は、次の方針文書と密接に関係しています。

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `policy_examples.md`

概念的には、次の文書とも関係します。

* `policy_ctype.md`
* `policy_encoding.md`

正確な境界は次のとおりです。

* `<xer/string.h>` は文字列およびメモリユーティリティを扱います
* `<xer/ctype.h>` は分類および文字変換を扱います
* エンコーディング方針は、ライブラリで使われるより広いテキストモデルを定義します

---

## 文書化上の注記

このヘッダーを生成ドキュメントの一部として使う場合、通常は次の点を説明すれば十分です。

* C風の文字列ユーティリティと、xer 固有の UTF-8 指向ヘルパーを組み合わせていること
* 低水準機能と、より高水準の実用的ユーティリティの両方を含むこと
* trim / split / join ヘルパーが、ユーザー向けの重要な機能であること
* 受け付ける引数形式は、xer 独自の API 方針に従うこと

詳細な関数ごとのセマンティクスは、リファレンスマニュアルまたは生成された API 節で説明するべきです。

---

## 例として示す価値が高い題材

このヘッダーでは、次のような例が特に適しています。

* `trim_view` による UTF-8 文字列のトリム
* `explode` / `implode` によるテキストの分割と結合
* `str_replace` によるテキスト置換
* UTF-8 テキスト内での Unicode スカラー値の検索
* xer 流の、なじみのある C風の比較またはコピー

これは、実行可能な例をユーザー向けコード片の標準的な情報源にしていくというプロジェクトの方向性によく合っています。

---

## 例

```cpp id="r4yb9n"
#include <xer/stdio.h>
#include <xer/string.h>

auto main() -> int
{
    constexpr std::u8string_view input = u8"  hello  ";

    const auto trimmed = xer::trim_view(input);
    if (!trimmed.has_value()) {
        return 1;
    }

    if (!xer::puts(*trimmed).has_value()) {
        return 1;
    }

    return 0;
}
```

この例は、典型的な xer のスタイルを示しています。

* UTF-8 指向の文字列入力を使う
* 通常の公開 API を通常の値で呼び出す
* `xer::result` を明示的に確認する
* 例のテキスト出力には `<xer/stdio.h>` を使う

---

## 関連項目

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `policy_examples.md`
* `header_ctype.md`
* `header_stdlib.md`


---

## 接頭辞・接尾辞の検査

`<xer/string.h>` は、接頭辞および接尾辞のヘルパーを提供します。

```cpp
starts_with
ends_with
```

これらは文字列風の引数を受け付け、一般的な UTF-8 文字列ビューおよびリテラルの利用場面を自然に扱えることを意図しています。

---

## 大文字小文字変換および動的な文字列変換

このヘッダーは、文字列レベルの変換ヘルパーを提供します。

```cpp
strtolower
strtoupper
strtoctrans
```

`strtolower` と `strtoupper` は文字列に文字変換を適用し、変換後の文字列を返します。

`strtoctrans` は文字列に `ctrans_id` 変換を適用します。
通常の 1 コードポイント変換では、これは `toctrans` の文字列レベル版として動作します。
本質的に複数のコードポイントを必要とする変換や、隣接文字にまたがる文脈を必要とする変換では、`strtoctrans` が `toctrans` では提供できない機能を提供する場合があります。

現在対応している変換には、次のものが含まれます。

- ASCII および Latin-1 の大文字小文字変換
- 全角・半角変換
- `strtoctrans` によってすでに実装されている仮名対応の文字列変換
- 仮名からローマ字への変換

---

## 仮名からローマ字への変換

`strtoctrans` は、ひらがなと全角カタカナからのローマ字変換に対応します。

```cpp
auto standard =
    xer::strtoctrans(u8"とうきょう", xer::ctrans_id::romaji);
// tōkyō

auto alternate =
    xer::strtoctrans(u8"とうきょう", xer::ctrans_id::romaji_alt);
// toukyou
```

### 変換の種類

| `ctrans_id` | 意味 |
|---|---|
| `romaji` | 長音をマクロンで表す標準的な長音表記に従うローマ字化 |
| `romaji_alt` | 現代仮名遣いに従って母音字を並べて長音を書く代替表記 |

ローマ字化方針は、日本語のローマ字表記として採用された 2025 年内閣告示 **「ローマ字のつづり方」** に従います。
通常の `romaji` 形式は、長音にマクロンを使います。
`romaji_alt` 形式は、別途認められているマクロンなしの表記方式を提供します。

### 例

| 入力 | `romaji` | `romaji_alt` |
|---|---|---|
| `とうきょう` | `tōkyō` | `toukyou` |
| `おおさか` | `ōsaka` | `oosaka` |
| `えいが` | `ēga` | `eiga` |
| `コーヒー` | `kōhī` | `koohii` |

この変換は、次のような通常のローマ字化の詳細も扱います。

- `きゃ`、`しゃ`、`ちゃ` などの拗音結合
- `っ` などの促音
- 母音および `y` の前に区切りのアポストロフィーが必要になる場合を含む撥音 `ん`
- カタカナ文中の長音符

### 範囲

初期のローマ字変換は、次を受け付けます。

- ひらがな
- 全角カタカナ
- カタカナ長音符 `ー`
- 間隔として保持できる通常の半角および全角スペース

非対応の文字を含む入力は、次のエラーとして報告されます。

```cpp
error_t::invalid_argument
```

### `toctrans` との関係

`ctrans_id::romaji` と `ctrans_id::romaji_alt` は、文字列レベルの変換です。

これらは次で受け付けられます。

```cpp
strtoctrans
```

一方で、単一コードポイント変換である次には意味を持ちません。

```cpp
toctrans
```

そのため、`toctrans` はこれらの識別子に対して次を報告します。

```cpp
error_t::invalid_argument
```
