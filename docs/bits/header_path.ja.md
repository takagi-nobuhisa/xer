<!-- xer-reference-source-sha256: 9fa6c4075e669b32122afc7bdec3bdaa6c254fcc073ac5914e81441710d8f7e6 -->

# `<xer/path.h>`

## 目的

`<xer/path.h>` は、xer の字句的なパス型と関連するパスユーティリティを提供します。

このヘッダーの役割は `std::filesystem::path` を包むことではありません。xer は、次の考え方に基づいて設計された独自の UTF-8 ベースのパスモデルを提供します。

- 内部表現を単純かつ一貫したものに保つ
- パス処理を主に字句的な操作として扱う
- Windows 固有の重要なパス上の区別を保持する
- 字句的なパス処理と、実際のファイルシステムに依存する解決を分離する

したがって、このヘッダーは xer におけるパス表現とパス指向ユーティリティ関数の公開入口です。

---

## 主な役割

`<xer/path.h>` の主な役割は次を提供することです。

- 公開 UTF-8 パス型
- 字句的なパス結合と分解
- 絶対 / 相対判定などのパス分類補助関数
- プラットフォームネイティブのパス表現との相互変換

これにより、このヘッダーはライブラリの他の部分、とくにファイルエントリやストリーム関連 API におけるパス処理の基礎になります。

---

## 主なエンティティ

少なくとも、`<xer/path.h>` は次のエンティティを提供します。

```cpp
using native_path_char_t;
using native_path_string;
using native_path_view;

class path;

auto operator/(path lhs, const path& rhs) -> xer::result<path>;
auto basename(const path& value) noexcept -> std::u8string_view;
auto extension(const path& value) noexcept -> std::u8string_view;
auto stem(const path& value) noexcept -> std::u8string_view;
auto parent_path(const path& value) -> xer::result<path>;
auto is_absolute(const path& value) noexcept -> bool;
auto is_relative(const path& value) noexcept -> bool;

auto to_native_path(const path& value) -> std::expected<native_path_string, error<void>>;
auto from_native_path(native_path_view value) -> std::expected<path, error<void>>;
auto from_native_path(const native_path_char_t* value) -> std::expected<path, error<void>>;
```

正確なオーバーロード集合は今後拡張される可能性がありますが、これが中心となる公開形です。

---

## `path`

`path` はこのヘッダーの中心となる型です。

これは xer 独自の字句的 UTF-8 モデルでパスを表します。

### 基本形

少なくとも、このクラスは次のような形を持ちます。

```cpp
class path {
public:
    using string_type = std::u8string;
    using view_type = std::u8string_view;

    path();
    explicit path(std::u8string_view value);
    explicit path(const char8_t* value);

    auto str() const noexcept -> view_type;
    auto operator/=(const path& rhs) -> xer::result<void>;
};
```

### 役割

`path` の役割は意図的に狭くしています。

主な目的は次のとおりです。

* 内部の正規化済みパス表現を保存する
* パス不変条件を保持する
* 基本的なパス結合を支える

より複雑なパス操作は、通常はメンバー関数ではなく自由関数として提供します。

---

## 内部表現

`path` は UTF-8 の内部表現を使います。

### 基本規則

* 内部的には `std::u8string` を保存する
* 内部区切り文字は常に `/`
* 入力中の `\` は `/` に正規化する
* 先頭構成要素の字句的な意味は保持する

### なぜ重要か

この設計により、内部処理を単純に保ちながら、とくに Windows で重要な区別を保持できます。

たとえば、次の形式は区別されたままでなければなりません。

* `C:foo`
* `C:/foo`
* `/foo`
* `//server/share/foo`

これはパスモデルの中心的な設計要件の 1 つです。

---

## `str()`

```cpp
auto str() const noexcept -> std::u8string_view;
```

### 目的

`str()` はパスの内部正規化表現を返します。

### 注意

* 返される形式は字句的に正規化されたものです
* 区切り文字は `/` として返されます
* 表示向けのネイティブパス文字列ではありません

この区別は重要です。`str()` が返すのは xer のパス表現であり、プラットフォーム表示形式ではありません。

---

## パス結合

パス結合は、メンバー形式と自由関数形式の両方で支援されます。

### `operator/=`

```cpp
auto operator/=(const path& rhs) -> xer::result<void>;
```

これは基本となる変更型の結合操作です。

### `operator/`

```cpp
auto operator/(path lhs, const path& rhs) -> xer::result<path>;
```

これは非変更型の結合形式です。

### 設計方針

基本的な設計は次のとおりです。

* `operator/=` が基本の変更型操作
* `operator/` は `operator/=` に基づいて実装できる

これにより、基本的な意味を一箇所に集約できます。

---

## 字句的パス操作

xer のほとんどのパスユーティリティは自由関数です。

少なくとも次を含みます。

* `basename`
* `extension`
* `stem`
* `parent_path`
* `is_absolute`
* `is_relative`

### なぜ自由関数か

xer は、直接の変更や特権的な内部制御を必要としない操作については自由関数を好みます。

これにより、`path` 自身の責務を小さく集中したものに保てます。

---

## `basename`

```cpp
auto basename(const path& value) noexcept -> std::u8string_view;
```

### 目的

`basename` は末尾の字句的パス構成要素を返します。

### 設計方針

その挙動は PHP の `basename` に近くしつつ、xer 独自のパスモデルに従います。

### 重要な注意

* 任意の生文字列ではなく `path` を受け取る
* ロケールに依存しない
* 区切り文字として `/` だけを扱う
* `\` はすでに `path` によって正規化済みである

---

## `extension`

```cpp
auto extension(const path& value) noexcept -> std::u8string_view;
```

### 目的

`extension` は basename のうち、拡張子のような接尾部を返します。

### xer の規則

xer では、`extension` は `basename(path)` の **最初の** `.` から始まる部分を返します。

つまり、たとえば次のようになります。

* `c.txt` -> `.txt`
* `archive.tar.gz` -> `.tar.gz`
* `.foo` -> `.foo`

これは意図的な規則であり、他のライブラリと自動的に一致すると仮定してはいけません。

### 任意の開始位置

必要に応じて、basename からの相対開始位置引数を許す設計も考えられます。

---

## `stem`

```cpp
auto stem(const path& value) noexcept -> std::u8string_view;
```

### 目的

`stem` は、xer の定義する拡張子を取り除いた後の basename の先頭部分を返します。

### 例

* `c.txt` -> `c`
* `archive.tar.gz` -> `archive`
* `.foo` -> 空文字列
* `foo.` -> `foo`

したがって、正確な挙動は常に xer の `extension` 規則と合わせて解釈する必要があります。

---

## `parent_path`

```cpp
auto parent_path(const path& value) -> xer::result<path>;
```

### 目的

`parent_path` は字句的な親パスを返します。

### 重要な性質

これは **純粋に字句的な** 操作です。

次のことは行いません。

* 実際のファイルシステムを参照する
* シンボリックリンクを解決する
* 実際のファイルシステム状態に基づいて正規化する

### 挙動

先頭部分の意味を保持しつつ、末尾の構成要素を 1 つ取り除きます。

これ以上の字句的な親が存在しない場合は失敗を返します。

この挙動により、字句的な推論とファイルシステム依存の推論が明確に分離されます。

---

## 絶対 / 相対の分類

少なくとも、`<xer/path.h>` は次を提供します。

```cpp
auto is_absolute(const path& value) noexcept -> bool;
auto is_relative(const path& value) noexcept -> bool;
```

### 目的

これらの関数は、xer のパス規則に従ってパスを分類します。

### Windows 固有の重要性

Windows では、この区別は単純な先頭区切り文字の検査には還元できません。

たとえば、

* `X:foo` は絶対パスではない
* `X:/foo` は絶対パスである
* `/foo` は絶対パスである
* `//server/share/foo` は絶対パスである

これが、xer が公開意味論を単純に別ライブラリへ委譲せず、独自のパスモデルを使う理由の 1 つです。

---

## ネイティブパス変換

`<xer/path.h>` は、xer のパスとプラットフォームネイティブのパス表現との変換も提供します。

少なくとも、次を含みます。

```cpp
auto to_native_path(const path& value) -> std::expected<native_path_string, error<void>>;
auto from_native_path(native_path_view value) -> std::expected<path, error<void>>;
auto from_native_path(const native_path_char_t* value) -> std::expected<path, error<void>>;
```

### 目的

これらの関数は、xer の内部 UTF-8 字句モデルをプラットフォームネイティブのパス文字列と相互運用させるためにあります。

### ネイティブ型の役割

* `native_path_char_t` はプラットフォームネイティブのパス文字型を表す
* `native_path_string` は所有するネイティブパス文字列型を表す
* `native_path_view` はネイティブパス型のビュー形式を表す

これにより、プラットフォーム固有の詳細を変換境界に局所化できます。

### エラー処理

変換失敗は明示的に報告されます。

これはとくに次を伴う変換で重要です。

* UTF-8 検証
* UTF-16 変換
* プラットフォームネイティブ文字の制約

---

## 字句モデルと実際のファイルシステム

`<xer/path.h>` の中心的な設計原則は、パス処理を主に字句的なものとして扱うことです。

### 字句的操作

次のものは自然に字句的パス層に属します。

* パスの結合
* basename の抽出
* extension の抽出
* stem の抽出
* 字句的な親パスの計算
* 絶対 / 相対形式の分類

### ファイルシステム依存操作

次のものは概念的に別であり、字句的パス処理と混同すべきではありません。

* 相対パスを現在の作業文脈に照らして解決する
* 実際の絶対パスへ変換する
* シンボリックリンクを解決する
* 実ファイルシステム状態に基づく正規化

この分離は意図的で重要です。

---

## 他のヘッダーとの関係

`<xer/path.h>` は次と合わせて理解してください。

* `policy_project_outline.md`
* `policy_path.md`
* `header_stdio.md`

おおまかな境界は次のとおりです。

* `<xer/path.h>` はパス表現と字句的パス操作を扱う
* `<xer/stdio.h>` はパスを利用するストリームおよびファイルエントリ操作を扱う

これにより、`<xer/path.h>` はライブラリ内のパス対応 API の基礎ヘッダーになります。

---

## ドキュメント上の注意

生成マニュアルでこのヘッダーを説明するときは、通常は次を説明すれば十分です。

* xer は独自の UTF-8 字句パス型を使うこと
* 区切り文字は `/` に正規化されること
* Windows の字句的な区別を保持すること
* ほとんどのパスユーティリティは自由関数であること
* ネイティブパス変換は明示的で失敗可能であること

詳細な結合規則や端のケースは、詳細リファレンスまたは生成 API ドキュメントに属します。

---

## 例として示す価値が高い題材

このヘッダーでは、次のような例が特に適しています。

* UTF-8 テキストから `path` を構築する
* `/` で 2 つのパスを結合する
* `basename`、`extension`、`stem` を得る
* `parent_path` を得る
* ネイティブパスとの相互変換を行う

これらは `examples/` の実行可能例のよい候補です。

---

## 例

```cpp
#include <xer/path.h>

auto main() -> int
{
    const xer::path base(u8"/usr");
    const xer::path child(u8"local");

    const auto joined = base / child;
    if (!joined.has_value()) {
        return 1;
    }

    if (joined->str() != u8"/usr/local") {
        return 1;
    }

    return 0;
}
```

この例は通常の xer スタイルを示しています。

* UTF-8 テキストからパスを構築する
* 字句的パス結合を使う
* 失敗可能な操作では `xer::result` を明示的に確認する

---

## 関連項目

* `policy_project_outline.md`
* `policy_path.md`
* `header_stdio.md`
