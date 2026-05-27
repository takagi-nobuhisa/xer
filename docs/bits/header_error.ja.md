<!-- xer-reference-source-sha256: 19f54010ac61f555fb9cbf0599d522b27e0be1de656bac0780ebeb5b7d391daa -->
# `<xer/error.h>`

## 目的

`<xer/error.h>` は、xer 全体で使う中核的なエラー機能と結果型の機能を提供します。

xer では、実用的な範囲で、通常の失敗を例外や特別な番兵値ではなく明示的に表します。
このヘッダーは、そのために使う共通の語彙を定義します。

---

## 主な要素

少なくとも、`<xer/error.h>` は次の要素を提供します。

```cpp
enum class xer::error_t : std::int32_t;

template <class Detail = void>
struct xer::error;

template <class T, class Detail = void>
using xer::result = std::expected<T, error<Detail>>;

constexpr auto xer::make_error(
    error_t code,
    std::source_location location = std::source_location::current()) noexcept
    -> error<void>;

template <class Detail, class T>
constexpr auto xer::make_error(
    error_t code,
    T&& value,
    std::source_location location = std::source_location::current()) noexcept
    -> error<Detail>;
```

`error_t` の列挙子の正確な集合は、ライブラリの発展に応じて増える可能性がありますが、全体の設計は安定しています。

---

## 設計上の役割

このヘッダーは、xer の基本的なエラー報告モデルを定義します。

このヘッダーの役割は、次のことを可能にすることです。

* 通常の失敗を明示的に報告する
* 通常の公開APIを理解しやすく保つ
* 必要に応じて任意の構造化された詳細情報を付加する
* 診断のためにソース位置情報を保持する

言い換えると、`<xer/error.h>` は xer 方式の失敗処理の基盤です。

---

## `xer::result`

`xer::result<T, Detail>` は、xer の標準的な結果型です。

一般的な場合、

```cpp
xer::result<T>
```

は、次の意味になります。

```cpp
std::expected<T, xer::error<void>>
```

追加の詳細情報が必要な場合は、第2テンプレート引数を指定できます。

```cpp
xer::result<T, Detail>
```

これは次の意味になります。

```cpp
std::expected<T, xer::error<Detail>>
```

### 基本方針

原則として、失敗しうる xer の公開APIは `xer::result` を返します。

これにより、通常の成功と通常の失敗が型システム上で明示されます。

### 典型的なパターン

```cpp
const auto result = some_operation();
if (!result.has_value()) {
    return result.error();
}

const auto& value = *result;
```

正確なスタイルは場合によって異なりますが、明示的に確認することが通常の前提です。

---

## `xer::error<void>`

`xer::error<void>` は、追加のペイロードが不要な場合に使う共通のエラー型です。

少なくとも次の情報を格納します。

* エラーコード
* エラーが作成されたソース位置

これにより、軽量でありながら有用な診断用コンテキストを保持できます。

### 目的

`error<void>` は、次のような場合に適しています。

* エラーカテゴリだけが重要である
* 追加の構造化データが不要である
* 呼び出し側が主に成功と失敗を区別できればよい

---

## `xer::error<Detail>`

`xer::error<Detail>` は、エラーに追加の構造化された情報が必要な場合に使います。

正確な表現は `Detail` の設計によって異なりますが、意図は次のとおりです。

* 共通のエラーコードとソース位置を保持する
* その失敗に固有の追加情報を運ぶ

これは、次のような場合に有用です。

* 位置を付加する
* 問題のある値を付加する
* 後で報告するための構造化されたコンテキストを付加する

### 設計方針

`Detail` がクラス型である場合、`error<Detail>` は継承または同等の仕組みによって、その詳細情報を自然に公開しても構いません。
`Detail` がクラス型でない場合は、メンバーとして格納しても構いません。

重要なのは内部表現そのものではなく、追加の詳細情報が明示的で型安全なままであることです。

---

## `xer::error_t`

`xer::error_t` は、xer 全体で使う共通のエラーコード列挙型です。

### 基本方針

この設計は、主に次の考え方に基づいています。

* 有用な場合は `errno` 風のカテゴリとの実用的な互換性を保つ
* 必要に応じて xer 固有のエラーカテゴリを許す
* エラー型そのものの中には成功を表す列挙子を置かない

### 一般的な解釈

* 正の値は、実用的な範囲で、対象環境の `errno` 風の意味に対応します
* 負の値は、xer 固有のカテゴリ用に予約されます

xer 固有のカテゴリの例には、次のようなものがあります。

* `logic_error`
* `invalid_argument`
* `io_error`
* `encoding_error`
* `not_found`
* `divide_by_zero`

正確な列挙子の集合は実装で定義されます。

---

## `xer::make_error`

`make_error` は、xer のエラーオブジェクトを構築する標準ヘルパーです。

### `make_error` が存在する理由

このヘルパーが存在する理由は次のとおりです。

* 呼び出し側が `std::source_location::current()` を繰り返し書かなくてよい
* ライブラリ全体でエラー構築を統一できる
* コードを簡潔で読みやすく保てる

### 基本形

追加の詳細情報がない場合:

```cpp
const auto error = xer::make_error(xer::error_t::invalid_argument);
```

追加の詳細情報がある場合:

```cpp
const auto error = xer::make_error<my_detail>(
    xer::error_t::invalid_argument,
    my_detail{/* ... */}
);
```

### ソース位置

ソース位置は、`make_error` のデフォルト引数を通じて呼び出し地点で取得されます。

これは、単に型が定義された場所ではなく、エラーが作成された場所を記録するため重要です。

---

## 他の方針との関係

`<xer/error.h>` は、次のプロジェクト全体の方針と密接に関係しています。

* 通常の失敗は `xer::result` で表す
* 通常の公開APIは、一般に `xer::result` 引数ではなく通常の値を受け取る
* 例外は、アサーション失敗や根本的に例外的な状況など、設計上適切な場合に予約する

したがって、このヘッダーは次の文書とあわせて理解する必要があります。

* `policy_project_outline.md`
* `policy_result_arguments.md`

---

## ドキュメント上の注意

`xer::result` を使う失敗しうるAPIを文書化するときは、通常、次の内容を説明すれば十分です。

* 成功値
* 一般的な失敗条件
* 重要な場合の主なエラーカテゴリ

その区別が利用者にとって重要でない限り、考えられるすべての `error_t` 値を列挙する必要は必ずしもありません。

---

## 例

```cpp
#include <xer/error.h>

auto parse_positive(int value) -> xer::result<int>
{
    if (value <= 0) {
        return std::unexpected(
            xer::make_error(xer::error_t::invalid_argument));
    }

    return value;
}
```

この例は、通常の xer パターンを示しています。

* 成功時は成功値を直接返す
* 失敗時は `std::unexpected(xer::make_error(...))` を返す

---

## 関連項目

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `header_assert.md`
