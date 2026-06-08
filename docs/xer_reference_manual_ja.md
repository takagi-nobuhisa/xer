# xer C++ Utility Library リファレンスマニュアル

対象バージョン: **v0.8.0a2**

---

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
* `end_of_file`
* `divide_by_zero`

正確な列挙子の集合は実装で定義されます。

逐次入力操作では、入力を使い切って次の項目を読めない場合に `end_of_file` を使用します。名前やキーで指定した対象が存在しない場合、検索操作では `not_found` を使用します。

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

---

# `<xer/assert.h>`

## 目的

`<xer/assert.h>` は、xer のアサーション機能を提供します。

これらの機能は主に xer 自身のテストプログラムで使いますが、軽量な確認用途としてライブラリ利用者に公開される場合もあります。

標準Cの `assert` とは異なり、xer のアサーションはプロセスを即座に終了しません。
かわりに、例外を送出することで失敗を報告します。

---

## 主な要素

少なくとも、`<xer/assert.h>` は次の要素を提供します。

```cpp
xer_assert(expr)
xer_assert_not(expr)
xer_assert_eq(lhs, rhs)
xer_assert_ne(lhs, rhs)
xer_assert_lt(lhs, rhs)
xer_assert_throw(expr, exception_type)
xer_assert_nothrow(expr)

class xer::assertion_error;
```

正確な実装詳細は変わる可能性がありますが、これらの名前が中核的なアサーションインターフェースを構成します。

---

## 設計上の役割

このヘッダーは、実行テスト内で明示的で読みやすい確認を支えるために存在します。

このヘッダーの役割は、次のことを可能にすることです。

* テストコードで期待条件を確認する
* 失敗時に現在のテストの流れを即座に停止する
* ソース位置などの診断情報を保持する
* 既定のアサーション動作としてプロセス終了を避ける

これにより、このアサーション機能は自動テストの実行やライブラリ動作のデバッグに適したものになります。

---

## 標準 `assert` との違い

標準CおよびC++の `assert` 機能は、通常、条件が失敗するとプロセスを中断します。

xer は意図的に別の設計を採用しています。

### 標準 `assert`

* プロセスを終了することで失敗を報告する
* 主に内部仮定をデバッグするためのもの
* 構造化された失敗報告を求めるテストフレームワークにはあまり適していない

### xer のアサーション

* `xer::assertion_error` を送出することで失敗を報告する
* 実行テストに適している
* ソースレベルの診断用コンテキストを保持する
* 適切な場合には、軽量な利用者側チェックにも使える

この違いは意図的なものです。

---

## `xer::assertion_error`

`xer::assertion_error` は、xer のアサーションが失敗したときに送出される例外型です。

### 目的

この型の目的は、テストランナーや周辺コードが捕捉して報告できる形で、アサーション失敗情報を運ぶことです。

### 期待される内容

少なくとも、アサーション失敗の診断情報から次のことを特定できる必要があります。

* どのアサーションが失敗したか
* どこで失敗したか
* どの式または比較が関係していたか

アサーションマクロによっては、値に関する追加情報が含まれる場合もあります。

---

## アサーションマクロ

## `xer_assert`

```cpp
xer_assert(expr)
```

このマクロは、`expr` が真であることを確認します。

`expr` が偽の場合、`xer::assertion_error` を送出します。

### 典型的な使用例

```cpp
xer_assert(result.has_value());
```

---

## `xer_assert_not`

```cpp
xer_assert_not(expr)
```

このマクロは、`expr` が偽であることを確認します。

`expr` が真の場合、`xer::assertion_error` を送出します。

### 典型的な使用例

```cpp
xer_assert_not(buffer.empty());
```

---

## `xer_assert_eq`

```cpp
xer_assert_eq(lhs, rhs)
```

このマクロは、`lhs == rhs` であることを確認します。

比較が偽の場合、`xer::assertion_error` を送出します。

### 目的

このマクロは、2つの値が等しいことがテスト対象の重要な条件である場合に使います。

### 典型的な使用例

```cpp
xer_assert_eq(value, 42);
```

---

## `xer_assert_ne`

```cpp
xer_assert_ne(lhs, rhs)
```

このマクロは、`lhs != rhs` であることを確認します。

比較が偽の場合、`xer::assertion_error` を送出します。

### 典型的な使用例

```cpp
xer_assert_ne(ptr, nullptr);
```

---

## `xer_assert_lt`

```cpp
xer_assert_lt(lhs, rhs)
```

このマクロは、`lhs < rhs` であることを確認します。

比較が偽の場合、`xer::assertion_error` を送出します。

### 目的

これは、基本方針で現在定義されている順序関係向けのアサーションです。

### 典型的な使用例

```cpp
xer_assert_lt(index, size);
```

---

## `xer_assert_throw`

```cpp
xer_assert_throw(expr, exception_type)
```

このマクロは、`expr` を評価したときに指定された例外型が送出されることを確認します。

`expr` がその例外型を送出しない場合、`xer::assertion_error` を送出します。

### 引数の順序

引数の順序は意図的なものです。

* 第1引数: 評価する式
* 第2引数: 期待する例外型

この順序はプロジェクトのテスト方針に従っています。

### 典型的な使用例

```cpp
xer_assert_throw(f(), std::runtime_error);
```

---

## `xer_assert_nothrow`

```cpp
xer_assert_nothrow(expr)
```

このマクロは、`expr` を評価しても例外が送出されないことを確認します。

`expr` が例外を送出した場合、`xer::assertion_error` を送出します。

### 典型的な使用例

```cpp
xer_assert_nothrow(run_test_case());
```

---

## 診断方針

アサーション失敗は、開発中に役立つ診断情報を提供するべきです。

少なくとも、次のことを特定できる必要があります。

* ソースファイル
* 行
* アサーションの形式
* 比較または確認された式のテキスト

`xer_assert_eq` のような値比較アサーションでは、実用的な場合、観測された左辺値と右辺値を含めることも望ましいです。

ただし、アサーション機能は、あらゆる型に対して完全な整形を保証することを目的としていません。

この点は重要です。

* 主に xer の開発と軽量なテストのためのものです
* 実用的で読みやすいままであるべきです
* 考えられるすべての出力ケースに対する過剰な特別扱いを積み重ねるべきではありません

---

## 想定範囲

これらのアサーションマクロは、主に次の用途を想定しています。

* xer の実行テスト
* 開発中の小さなユーティリティ確認
* 便利な場合の軽量な利用者側検証

あらゆる場面で、高機能な外部テストフレームワークを置き換えることを意図したものではありません。

---

## 他の方針との関係

`<xer/assert.h>` は、次の文書とあわせて理解する必要があります。

* `policy_project_outline.md`
* `policy_testing_and_php.md`

プロジェクト概要では、アサーション失敗を通常の実行時失敗とは別扱いにする理由を説明しています。
テスト方針では、実行テストにおける xer アサーションの役割を説明しています。

---

## ドキュメント上の注意

このヘッダーを生成マニュアルで参照するときは、通常、次の内容を説明すれば十分です。

* xer のアサーションは中断ではなく例外送出を行うこと
* 利用可能なマクロ名
* テストと軽量な検証におけるこれらのマクロの想定役割

完全な運用哲学は、ヘッダーごとのAPI要約ではなく方針文書に属します。

---

## 例

```cpp
#include <xer/assert.h>

auto main() -> int
{
    xer_assert_eq(1 + 1, 2);
    xer_assert_not(false);
    xer_assert_nothrow(static_cast<void>(0));
    return 0;
}
```

この例は一般的なスタイルを示しています。

* 明示的なアサーションマクロを使う
* 失敗時は `xer::assertion_error` を送出させる
* 確認を読みやすく局所的に保つ

---

## 関連項目

* `policy_project_outline.md`
* `policy_testing_and_php.md`
* `header_error.md`

---

# `<xer/typeinfo.h>`

## 目的

`<xer/typeinfo.h>` は、xer の軽量な型情報ヘルパーを提供します。

主な目的は、診断、トレース、その他の開発向け出力で型名を使いやすくすることです。
このヘッダーは、標準C++の型情報を、xer のUTF-8指向のテキストモデルに合う形でラップします。

---

## 主な要素

少なくとも、`<xer/typeinfo.h>` は次の要素を提供します。

```cpp
class xer::type_info;

#define xer_typeid(...)
```

正確な実装では、`xer::type_info` を比較したり順序付きコンテナで使ったりできるように、内部で `std::type_index` を使う場合があります。

---

## `xer::type_info`

`xer::type_info` は、C++の実行時型情報に対する軽量なラッパーです。

少なくとも次の操作を提供します。

```cpp
auto raw_name() const noexcept -> const char*;
auto name() const -> std::u8string;
auto index() const noexcept -> std::type_index;
auto operator==(const type_info& rhs) const noexcept -> bool;
auto operator!=(const type_info& rhs) const noexcept -> bool;
auto operator<(const type_info& rhs) const noexcept -> bool;
```

### `name()`

`name()` は、人間が読む診断用のUTF-8型名を返します。

GCCでは、実装が提供する型名をデマングルする場合があります。
デマングルに失敗した場合は、実装が提供する生の名前をそのまま返す場合があります。

返される名前は表示と診断を目的としたものであり、安定したシリアライズやABI非依存の比較に使うためのものではありません。

---

## `xer_typeid`

`xer_typeid(...)` は、`typeid` が受け付けるものと同種のオペランドから `xer::type_info` オブジェクトを作成します。

これは可変長マクロなので、カンマを含むテンプレート型オペランドも自然に渡せます。

例:

```cpp
const auto a = xer_typeid(int);
const auto b = xer_typeid(std::pair<int, long>);
const auto c = xer_typeid(value);
```

---

## 設計上の役割

このヘッダーは、主に診断とトレースの基盤です。

特に、コードで次のものを表示できるようにします。

- トレース対象オブジェクトの型
- 実装が提供する型名またはデマングル済み型名
- ルックアップテーブル用の順序付き型キー

---

## 他のヘッダーとの関係

`<xer/typeinfo.h>` は、トレースなどの将来的な診断機能と関係します。

また、型名をUTF-8テキストとして出力する場合には、`<xer/stdio.h>` の書式付き出力機能と組み合わせても有用です。

---

## ドキュメント上の注意

型名は本質的に実装依存です。
したがって、ドキュメントでは `name()` を、安定したプログラム上の識別子ではなく、診断表示用の機能として説明するべきです。

---

## 例

```cpp
#include <xer/stdio.h>
#include <xer/typeinfo.h>

#include <utility>

auto main() -> int
{
    const auto info = xer_typeid(std::pair<int, long>);

    if (!xer::puts(info.name()).has_value()) {
        return 1;
    }

    return 0;
}
```

---

## 関連項目

* `header_stdio.md`

---

> **未訳:** この節の日本語版はまだ最新ではありません。
> そのため、暫定的に英語版の内容を掲載しています。
> 
> Header: `xer/diag.h`
> Reason: Japanese fragment was translated from a different English source hash.

# `<xer/diag.h>`

## Purpose

`<xer/diag.h>` provides lightweight diagnostic facilities for xer.

It groups tracing and logging support under one public diagnostic header while keeping the shared category and level vocabulary common to both facilities.

---

## Main Entities

At minimum, `<xer/diag.h>` provides the following entities:

```cpp
using xer::diag_level_t = int;

inline constexpr xer::diag_level_t xer::diag_error;
inline constexpr xer::diag_level_t xer::diag_warning;
inline constexpr xer::diag_level_t xer::diag_info;
inline constexpr xer::diag_level_t xer::diag_debug;
inline constexpr xer::diag_level_t xer::diag_verbose;

enum class xer::diag_category : std::uint32_t;

xer_print(expression)
xer_print(label, expression)
xer_trace(category, level, object)
xer_log(category, level, message)
xer_log(category, level, format, ...)
```

It also provides functions for setting trace and log output streams and levels.

---

## Design Role

This header is intended for diagnostics, development-time tracing, and simple runtime logging.

Simple printing, tracing, and logging share the same public diagnostic header. Tracing and logging share:

* `diag_category`
* `diag_level_t`
* the named level constants

Their output destinations and current levels are configured independently.

---


## Simple Print

`xer_print(expression)` writes one line to the simple diagnostic print stream.
The expression is evaluated once, and the source expression text is used as the label:

```cpp
int value = 42;
xer_print(value);
```

Conceptual output:

```text
value = 42
```

`xer_print(label, expression)` writes one line with an explicit UTF-8 label:

```cpp
xer_print(u8"answer", value);
```

Conceptual output:

```text
answer: 42
```

The value is formatted through xer's `%@` printf conversion. For `result<T>`, a successful result prints the contained value, and a failed result prints `error(name)`. For `result<void>`, a successful result prints `ok`.

`xer_print` is intended for examples, quick checks, and lightweight diagnostics. It is not disabled by `NDEBUG`.

---

## Trace

`xer_trace(category, level, object)` prints one diagnostic line containing:

* the diagnostic category
* the diagnostic level
* the source expression text
* the statically derived type name
* the value formatted through xer's `%@` printf conversion

The output form is conceptually:

```text
[category][level] expression (type) = value
```

When `NDEBUG` is defined, `xer_trace` expands to a no-op expression and does not evaluate its arguments.

---

## Log

`xer_log(category, level, message)` writes one simple log record.

`xer_log(category, level, format, ...)` writes one formatted log record. The message body uses xer printf formatting rules, including `%@`.

Unlike `xer_trace`, logging is not disabled merely because `NDEBUG` is defined. It can be disabled at compile time by defining `XER_ENABLE_LOG` to `0` before including `<xer/diag.h>`.

## Log Record Format

Each log call writes one CSV data record.

The columns are fixed as follows:

```text
timestamp,category,level,message
```

No header row is written automatically.

The timestamp uses local time and has millisecond precision:

```text
YYYY-MM-DD HH:MM:SS.mmm
```

A typical log record is:

```csv
2026-04-26 18:42:15.123,io,30,"opened sample.txt"
```

The first three fields are generated by xer and are written without CSV quoting.
The message field is always written as a quoted CSV field. Double quote characters in the message are escaped by doubling them.

## Log Cost Policy

Logging is designed to remain reasonably lightweight.

* disabled log messages are filtered before formatting arguments are evaluated
* simple message logging does not use `sprintf`
* formatted message logging uses `sprintf` only after the level check succeeds
* the timestamp prefix up to whole seconds is cached per thread
* the level field is formatted without `printf`
* the message field is quoted directly to the stream without constructing a separate quoted string

---

## Output Streams

Print output defaults to the standard output text stream. Trace and log output default to the standard error text stream.
They can be changed independently through the corresponding stream-setting functions.

---

## Notes

These facilities are intentionally small.
They are not a full logging framework, but they provide a useful diagnostic foundation for xer itself and for small programs using xer.

---

# `<xer/scope.h>`

## 目的

`<xer/scope.h>` は、xer の小さなスコープベースのユーティリティ機能を提供します。

現時点では、このヘッダーは `xer::scope_exit` を提供します。`xer::scope_exit` は、ガードオブジェクトの破棄時に登録された呼び出し可能オブジェクトを呼び出す軽量なスコープガードです。

これは、ブロックを抜けるときに自動的に実行すべきクリーンアップ処理に役立ちます。

---

## 主なエンティティ

少なくとも、`<xer/scope.h>` は次のエンティティを提供します。

```cpp
template <class F>
class scope_exit;
```

また、通常のラムダ式や関数オブジェクトを自然に渡せるように、推論ガイドも提供します。

```cpp
template <class F>
scope_exit(F&&) -> scope_exit<std::decay_t<F>>;
```

---

## `scope_exit`

`scope_exit` はムーブ専用の RAII ヘルパーです。

呼び出し可能オブジェクトを保持し、ガードが有効な間はデストラクタからそのオブジェクトを呼び出します。

```cpp
auto guard = xer::scope_exit([&] noexcept {
    cleanup();
});
```

### 基本動作

`scope_exit` オブジェクトは、構築直後から有効です。

オブジェクトが破棄されるときの動作は次のとおりです。

* 有効な場合、登録された呼び出し可能オブジェクトを呼び出す
* 解放済みの場合、何もしない

このため、クリーンアップが必要なリソースや状態変更の近くでクリーンアップ処理を登録する用途に適しています。

---

## 典型的な用途

`scope_exit` は、次のような処理に役立ちます。

* `chdir` 後にカレントディレクトリを元に戻す
* 一時ファイルや一時ディレクトリを削除する
* リソースのロックを解除する
* グローバル設定またはプロセス全体の設定を元に戻す
* ローカルな状態変更をロールバックする
* よりよいラッパーがない非 RAII リソースを閉じる、または解放する

複数の return 経路に沿ってクリーンアップを行う必要がある場合に、とくに有用です。

---

## ムーブ専用セマンティクス

`scope_exit` はコピー不可であり、ムーブ構築可能です。

コピーを許可すると、どちらのオブジェクトがクリーンアップ処理を所有するのか不明確になるため、コピーは許可されません。

ムーブ構築は、クリーンアップ責任を新しいオブジェクトへ移します。
ムーブ元のガードは解放されるため、登録された呼び出し可能オブジェクトが二重に呼び出されることはありません。

ムーブ代入は提供されません。

---

## ガードの解放

ガードは明示的に解放できます。

```cpp
guard.release();
```

`release()` を呼び出した後は、ガードの破棄時に登録された呼び出し可能オブジェクトは呼び出されません。

これは、後続の処理が正常に完了しなかった場合だけクリーンアップが必要になるような場面で役立ちます。

---

## ガードが有効かどうかの確認

現在の有効状態は次の関数で確認できます。

```cpp
auto active() const noexcept -> bool;
```

破棄時に呼び出し可能オブジェクトが呼び出される状態であれば `true` を返します。

---

## 例外方針

`scope_exit` のデストラクタは `noexcept` です。

登録される呼び出し可能オブジェクトは、例外を投げないことが期待されます。

`scope_exit` のデストラクタ実行中に呼び出し可能オブジェクトが例外を投げた場合、`std::terminate` が呼び出されます。

この方針は、デストラクタから実行されるクリーンアップコードに対する通常の期待に従っています。
スコープガードは、通常の回復可能な失敗を報告する場所としては適していません。デストラクタは `xer::result` を返せないためです。

クリーンアップ処理の失敗に意味がある場合、呼び出し側は通常、スコープを抜ける前にその失敗を明示的に処理するか、失敗に対処できない場合は意図して無視するべきです。

---

## 標準および実験的機能との関係

xer は独自の `scope_exit` を提供します。

これは、標準 C++ の `<scope>` ヘッダーのラッパーではありません。
標準 C++ は現在、通常の標準ライブラリの一部としてこのようなヘッダーを提供していません。

類似機能は実験的な機能やライブラリ拡張として存在する場合がありますが、xer ではヘッダーオンリーかつ GCC を中心とする移植性方針に合うように、このユーティリティを小さく自己完結したものにしています。

---

## 設計上の役割

`scope_exit` は意図的に小さく保たれています。

完全なスコープガードフレームワークを提供しようとはしていません。

とくに、このヘッダーは現時点で次のものを提供しません。

* `scope_success`
* `scope_fail`
* `unique_resource`

初期の目的は、「現在のスコープを抜けるときにこのクリーンアップ処理を実行する」という一般的なパターンを支援することだけです。

---

## xer のエラーモデルとの関係

xer では通常、失敗する可能性がある通常の処理を `xer::result` で表します。

ただし、`scope_exit` 自体はデストラクタから結果を返しません。

そのため、`scope_exit` に登録するクリーンアップ処理は、通常、失敗を安全に無視できる処理か、想定される文脈では失敗しないことが分かっている処理にするべきです。

よくあるパターンは、クリーンアップ処理の結果を明示的に破棄することです。

```cpp
auto guard = xer::scope_exit([&] noexcept {
    static_cast<void>(xer::chdir(original));
});
```

これにより、クリーンアップ失敗を無視する判断が見える形になります。

---

## 例

```cpp
#include <xer/scope.h>
#include <xer/stdio.h>

auto main() -> int
{
    const auto original = xer::getcwd();
    if (!original.has_value()) {
        return 1;
    }

    {
        auto restore = xer::scope_exit([&] noexcept {
            static_cast<void>(xer::chdir(*original));
        });

        if (!xer::chdir(xer::path(u8"work")).has_value()) {
            return 1;
        }

        // 別のカレントディレクトリ内で作業する。
    }

    return 0;
}
```

この例は、典型的な xer のスタイルを示しています。

* 状態を明示的に取得または記録する
* `scope_exit` でクリーンアップを登録する
* 実用的な場合はクリーンアップラムダに `noexcept` を付ける
* 失敗に対処できない場合は、クリーンアップ結果を明示的に破棄する

---

## ドキュメント上の注意

このヘッダーを生成ドキュメントで扱う場合、通常は次の点を説明すれば十分です。

* `scope_exit` はムーブ専用のスコープガードである
* デストラクタから登録済みの呼び出し可能オブジェクトを呼び出す
* `release()` はクリーンアップ処理を無効化する
* 登録される呼び出し可能オブジェクトは例外を投げるべきではない
* `scope_success` と `scope_fail` はこの段階では意図的に提供していない

詳細な例は、抽象的な RAII 理論よりも実用的なクリーンアップパターンに焦点を当てるべきです。

---

## 例として示す価値が高い題材

このヘッダーには、次のような例がとくに適しています。

* `chdir` 後にカレントディレクトリを元に戻す
* スコープ終了時に一時ファイルを削除する
* 正常完了後にガードを解放する
* ガードをムーブしてクリーンアップ責任を移す

これらは `examples/` 以下の実行可能な例として適した候補です。

---

## 関連項目

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `header_stdio.md`

---

> **未訳:** この節の日本語版はまだ最新ではありません。
> そのため、暫定的に英語版の内容を掲載しています。
> 
> Header: `xer/convert.h`
> Reason: Japanese fragment is missing.

# `<xer/convert.h>`

## Purpose

`<xer/convert.h>` provides the generic `xer::to<T>` conversion function.

Unlike a C++ cast expression, `xer::to<T>` may parse text, format values, validate numeric ranges, normalize selected xer value types, and fail. Therefore it returns `xer::result<T>`.

---

## Main Role

This header provides a single practical conversion entry point for cases where the requested conversion is more than a simple C++ cast:

```cpp
template <class To, class From>
auto to(From&& from) -> xer::result<To>;
```

Typical uses include:

```cpp
auto n = xer::to<int>(u8"123");
auto s = xer::to<std::u8string>(123);
auto p = xer::to<xer::path>(u8"dir\\file.txt");
```

---

## Basic Rules

`xer::to<T>` applies the following rules in order:

1. If the source and destination types are the same value type, the value is returned directly.
2. Numeric-to-numeric conversion is range-checked with `xer::in_range` before conversion.
3. `signed char` and `unsigned char` are treated as integer types.
4. `char` is treated as a character type, not as a numeric type.
5. Explicitly encoded text strings may be parsed, formatted, or transcoded.
6. Ambiguous narrow `char` strings are not interpreted as text.
7. Unsupported or unsafe conversions return `error_t::invalid_argument`.

---

## Numeric Conversion

```cpp
auto a = xer::to<int>(u8"123");
auto b = xer::to<unsigned char>(u8"255");
auto c = xer::to<double>(u8"3.5");
```

String-to-number conversion accepts explicitly encoded character strings based on these character types:

```cpp
char8_t
char16_t
char32_t
wchar_t
```

Examples:

```cpp
xer::to<int>(u8"123");
xer::to<int>(u"123");
xer::to<int>(U"123");
xer::to<int>(L"123");
```

The entire input must be consumed by the numeric conversion. For example, `u8"123x"` is rejected.

Numeric-to-numeric conversion checks whether the source value is within the destination type's representable range:

```cpp
xer::to<unsigned char>(255); // success
xer::to<unsigned char>(256); // error_t::range
xer::to<unsigned>(-1);       // error_t::range
```

---

## Character Conversion

`char` is treated as a character type.

```cpp
auto s = xer::to<std::u8string>('A'); // u8"A"
auto c = xer::to<char>(u8"A");       // 'A'
```

`char` is not treated as a numeric value:

```cpp
xer::to<int>('A'); // error_t::invalid_argument
```

`signed char` and `unsigned char` are numeric:

```cpp
xer::to<int>(static_cast<signed char>(-5));
xer::to<int>(static_cast<unsigned char>(250));
```

---

## Text Conversion

The following owning destination string types are supported:

```cpp
std::u8string
std::u16string
std::u32string
std::wstring
```

Examples:

```cpp
auto u8  = xer::to<std::u8string>(123);
auto u16 = xer::to<std::u16string>(u8"あ");
auto u32 = xer::to<std::u32string>(u8"あ");
auto w   = xer::to<std::wstring>(u8"あ");
```

`wchar_t` strings are handled according to the target platforms supported by xer:

- 16-bit `wchar_t` is treated as UTF-16.
- 32-bit `wchar_t` is treated as UTF-32.

---

## Narrow `char` Strings

`char` strings are intentionally not interpreted as text by `xer::to<T>`.

```cpp
xer::to<int>("123");                 // error_t::invalid_argument
xer::to<std::u8string>("abc");       // error_t::invalid_argument
xer::to<xer::path>("dir/file.txt");  // error_t::invalid_argument
```

This avoids silently treating platform-dependent narrow strings as UTF-8, locale text, or native path text.

Use an explicitly encoded string instead:

```cpp
xer::to<int>(u8"123");
xer::to<std::u8string>(u8"abc");
xer::to<xer::path>(u8"dir/file.txt");
```

---

## Path Conversion

`xer::path` can be created from explicitly encoded text:

```cpp
auto p1 = xer::to<xer::path>(u8"dir\\file.txt");
auto p2 = xer::to<xer::path>(u"dir\\file.txt");
auto p3 = xer::to<xer::path>(U"dir\\file.txt");
auto p4 = xer::to<xer::path>(L"dir\\file.txt");
```

The resulting `xer::path` stores normalized UTF-8 text and normalizes backslashes to slashes through the ordinary `xer::path` constructor.

---

## Example

```cpp
#include <xer/convert.h>
#include <xer/path.h>
#include <xer/stdio.h>

int main()
{
    auto count = xer::to<int>(u8"42");
    auto path = xer::to<xer::path>(u8"data\\out.txt");
    auto text = xer::to<std::u8string>(3.5);

    if (!count || !path || !text) {
        return 1;
    }

    xer::printf(u8"count = %@\n", *count);
    xer::printf(u8"path = %s\n", path->str().data());
    xer::printf(u8"text = %s\n", text->c_str());
    return 0;
}
```

---

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

---

# `<xer/ctype.h>`

## 目的

`<xer/ctype.h>` は、xer における文字分類機能と文字変換機能を提供します。

このヘッダーは、密接に関連する次の2つの領域を扱います。

- 英字、数字、空白、表示可能文字などのカテゴリへの文字分類
- 大文字・小文字変換や関連する変換処理

役割としては C の `<ctype.h>` や `<wctype.h>` に近いものですが、設計は標準ライブラリ構造をそのまま再現するのではなく、xer 独自のテキストモデルと API 方針に従います。

---

## 主な役割

`<xer/ctype.h>` の主な役割は、xer の他の部分と整合する、単純で明示的な文字処理モデルを提供することです。

特に、次のものを提供することを目指しています。

- ロケールに依存しない基本動作
- 通常の ASCII 向け操作と、より拡張された操作の明確な区別
- 統一された文字引数型
- 固定の関数名だけでは足りない場合に使える、動的な分類・変換機構

これにより、このヘッダーは単純な判定にも、呼び出し側が分類種別や変換種別を実行時に選びたい場合にも適したものになります。

---

## 基本設計方針

### ロケール非依存

基本的な `is` 関数と `to` 関数はロケールに依存しません。

その動作は、環境依存のロケール規則ではなく、`"C"` ロケールに対応するものです。

これは、xer 全体の設計がロケールへの依存をできるだけ小さくしようとしているために重要です。

### 文字型

個別の文字分類関数および文字変換関数の引数型は、`char32_t` に統一されています。

これは、個々の Unicode スカラー値を `char32_t` として扱うという xer の一般方針と一致します。

### ASCII を基本範囲とする

基本的な個別関数は、ASCII 範囲だけを対象にします。

つまり、次のように動作します。

- 分類関数は、非 ASCII 入力に対して `false` を返す
- 変換関数は、非 ASCII 入力に対して失敗を返す

これは一時的な制限ではなく、意図的な設計選択です。

---

## 個別の文字分類関数

少なくとも、`<xer/ctype.h>` は次のような個別の文字分類関数を提供します。

```cpp
isalpha
isdigit
isalnum
islower
isupper
isspace
isblank
iscntrl
isprint
isgraph
ispunct
isxdigit
isascii
isoctal
isbinary
```

### これらの関数の役割

これらの関数は、一般的なカテゴリを直接かつ読みやすく判定するためのものです。

呼び出し側がどのカテゴリを判定したいかをすでに知っている通常の場面を想定しています。

### 戻り値の型

個別の文字分類関数は次を返します。

```cpp
bool
```

### 非 ASCII 入力に対する動作

これらの関数は ASCII 文字だけを分類します。

非 ASCII 文字が渡された場合は、`false` を返します。

これにより、各関数の意味を単純で予測しやすいものに保っています。

---

## 個別の文字変換関数

少なくとも、`<xer/ctype.h>` は次のような個別の文字変換関数を提供します。

```cpp
tolower
toupper
```

### これらの関数の役割

これらの関数は、一般的な文字変換を直接行うためのものです。

呼び出し側が固定された既知の変換を行いたい通常の場面を想定しています。

### 戻り値の型

個別の文字変換関数は次を返します。

```cpp
xer::result<char32_t>
```

### 動作

これらの関数は ASCII 範囲だけを対象にします。

その動作は次のとおりです。

* 入力が変換対象であれば、変換後の文字を返す
* 入力が変換対象でなければ、元の文字を成功結果として返す
* 入力が非 ASCII であれば、失敗を返す

### `toascii` について

伝統的な C 系の `toascii` は、現時点では採用していません。

これは、`toascii` が下位7ビットを取り出す低レベルのビットマスク操作として理解されることが多く、それが xer の文字処理方針に自然には合わないためです。

---

## 動的な文字分類

固定の `is` 関数に加えて、xer は動的な分類関数を提供します。

```cpp
enum class ctype_id;
auto isctype(char32_t c, ctype_id id) noexcept -> bool;
```

### `isctype` の役割

`isctype` は、`ctype_id` の値を通じて分類種別を動的に選べるようにします。

これは次のような場合に便利です。

* 分類種別が実行時に決まる
* 1つの関数で複数のカテゴリを扱いたい
* 多数の固定 `is...` 関数に対して手作業で分岐したくない

### 設計方針

`ctype_id` には、次の両方を含めることができます。

* ASCII 限定カテゴリ
* 非 ASCII 文字も扱う拡張カテゴリ

これにより、基本的なケースと拡張されたケースの両方を、1つの統一 API で扱えます。

### 動作

ASCII 限定カテゴリが指定された場合、`isctype` は対応する個別の `is` 関数と同じように動作します。

拡張カテゴリが指定された場合、非 ASCII 文字も分類できることがあります。

これにより、たとえば `isdigit` は単純に保ちながら、明示的に必要な場合には `isctype` によってより豊かな分類を扱えるようになります。

---

## 動的な文字変換

xer は動的な変換関数も提供します。

```cpp
enum class ctrans_id;
auto toctrans(char32_t c, ctrans_id id) -> xer::result<char32_t>;
```

### `toctrans` の役割

`toctrans` は、`ctrans_id` を通じて変換種別を動的に選べるようにします。

これは次のような場合に便利です。

* 目的の変換が実行時に選ばれる
* 1つの API で複数の変換種別を扱いたい
* 多数の固定関数名を増やさずに拡張変換カテゴリを提供したい

### 動作

ASCII 限定カテゴリが指定された場合、`toctrans` は対応する個別の変換関数と同じように動作します。

拡張カテゴリが指定された場合、非 ASCII 文字の変換を行えることがあります。

---

## 拡張変換領域

拡張変換カテゴリは、少なくとも次の領域を扱うことがあります。

* 日本語用途の全角・半角変換
* ひらがなとカタカナの相互変換
* ギリシア文字およびキリル文字の大文字・小文字変換
* 文字列レベルでの仮名からローマ字への変換

これらは、基本的な ASCII 関数から自動的に含意される動作ではなく、明示的な拡張機能として扱われます。

### 文字列専用の変換カテゴリ

一部の `ctrans_id` 値は、複数の入力コードポイントや出力コードポイントを必要としたり、隣接する文字に依存したりします。
そのような変換は、単一コードポイントの `toctrans` ではなく、`strtoctrans` を通じて公開されます。

ローマ字カテゴリは、その最初の例です。

```cpp
ctrans_id::romaji
ctrans_id::romaji_alt
```

これらは `strtoctrans` では有効ですが、`toctrans` に渡した場合は `error_t::invalid_argument` が報告されます。

---

## 全角・半角変換

`toctrans` における全角・半角変換は、主に日本語用途を想定しています。

### 対象文字

対象集合には、少なくとも次のものを含めることができます。

* ASCII 互換の英字、数字、記号、空白
* 半角カタカナ
* 全角カタカナ
* 仮名、濁点、半濁点に関連する句読点や記号

### 除外対象

ひらがなは、全角・半角変換の対象外です。

### 単一文字としての制限

`toctrans` は単一の `char32_t` を返すため、本来なら複数の出力文字が必要になる変換では、一部の情報が落ちることがあります。

たとえば、濁点または半濁点を持つ全角カタカナを半角形式に変換する場合、1文字の結果が要求されるため、現在のモデルではその記号が落ちることがあります。

---

## 仮名変換

`toctrans` は、全角ひらがなと全角カタカナの相互変換もサポートします。

代表的なカテゴリには次のものがあります。

* `katakana`
* `hiragana`

### 意味

* `katakana`: 全角ひらがなを対応する全角カタカナへ変換する
* `hiragana`: 全角カタカナを対応する全角ひらがなへ変換する

### 除外対象

半角カタカナは仮名変換の対象外です。

これにより、変換モデルを単純に保ち、文字種変換と幅変換が混ざることを避けています。

---

## Latin-1 と拡張分類

`<xer/ctype.h>` は、Latin-1 関連のケース向けに拡張分類ヘルパーを提供することもあります。

これには、次のような関数を含めることができます。

```cpp
islatin1_upper
islatin1_lower
islatin1_alpha
islatin1_alnum
islatin1_print
islatin1_graph
```

### これらの関数の役割

これらは、次の中間段階を提供します。

* 厳密に ASCII のみを扱う個別関数
* 完全に動的な分類、またはより野心的な Unicode 全体の分類

基本 API を完全なロケール駆動または完全な Unicode プロパティ駆動の仕組みに変えることなく、実用的な文字処理を小さく明示的に拡張したい場合に有用です。

---

## 他の方針との関係

`<xer/ctype.h>` は、次の文書とあわせて理解する必要があります。

* `policy_project_outline.md`
* `policy_encoding.md`

また、次の文書とも関連します。

* `header_string.md`
* `header_stdlib.md`

大まかな境界は次のとおりです。

* `<xer/ctype.h>` は分類と文字変換を扱う
* `<xer/string.h>` は文字列およびメモリユーティリティを扱う
* `<xer/stdlib.h>` はマルチバイト変換と関連機能を扱う
* エンコーディング方針は、これらの文字操作が意味を持つより広いモデルを定義する

---

## ドキュメント上の注意

このヘッダーを生成ドキュメントで扱う場合、通常は次の点を説明すれば十分です。

* 基本的な個別関数は ASCII 向けであり、ロケールに依存しないこと
* `char32_t` が標準の引数型であること
* `isctype` と `toctrans` が動的な分類と変換を提供すること
* 拡張カテゴリは、基本 API によって自動的に含意されるのではなく、明示的に存在すること

カテゴリごとの詳細な意味は、リファレンスマニュアルまたは生成された API セクションで説明するべきです。

---

## 例として示す価値がある題材

このヘッダーには、次のような例が特に適しています。

* `isalpha` や `isdigit` による文字判定
* `tolower` や `toupper` による文字変換
* `isctype` による動的分類
* `toctrans` による仮名変換または幅変換
* ローマ字識別子は `<xer/string.h>` で説明される文字列レベルの `strtoctrans` に属する、という点の理解

これらは、`examples/` 以下の実行可能な例の候補として適しています。

---

## 例

```cpp
#include <xer/ctype.h>

auto main() -> int
{
    const auto lower = xer::tolower(U'A');
    if (!lower.has_value()) {
        return 1;
    }

    if (*lower != U'a') {
        return 1;
    }

    if (!xer::isdigit(U'7')) {
        return 1;
    }

    return 0;
}
```

この例は、通常のスタイルを示しています。

* 個別の文字値には `char32_t` を使う
* 基本的な分類を直接呼び出す
* 変換では `xer::result` を明示的に確認する

---

## 関連項目

* `policy_project_outline.md`
* `policy_encoding.md`
* `header_string.md`
* `header_stdlib.md`

---

## Unicode スカラー分類

動的分類器には、Unicode の妥当性チェックも含まれています。

```cpp
is_unicode_scalar_value
is_unicode_bmp_scalar_value

ctype_id::unicode
ctype_id::unicode_bmp
```

`is_unicode_scalar_value` は、有効な Unicode スカラー値に対して `true` を返します。
サロゲートコードポイントは拒否されます。

`is_unicode_bmp_scalar_value` は、基本多言語面内の有効な Unicode スカラー値に対してのみ `true` を返します。

これらの関数はコードポイントの妥当性チェックです。Unicode 識別子のチェックではありません。


---

## 点字パターン分類

動的分類器には、Unicode 点字パターンのカテゴリが含まれています。

```cpp
ctype_id::braille
```

このカテゴリは、Unicode 点字パターンブロック内のコードポイントに対して `true` を返します。

```text
U+2800..U+28FF
```

`U+2800 BRAILLE PATTERN BLANK` も含まれます。

これはブロックレベルの分類です。
コードポイントが Unicode 点字パターン文字であるかだけを確認します。日本語点字表記、英語点字表記、縮約、分かち書き規則、その他の高水準の点字テキスト規則は検証しません。

現時点では個別の `isbraille` ヘルパーは提供していません。
点字パターン分類が必要な場合は、`isctype(c, ctype_id::braille)` を呼び出してください。

---

## 全角・半角分類

`ctype_id` には、明示的な全角・半角カテゴリが含まれています。

```cpp
fullwidth_kana
halfwidth_kana
fullwidth_digit
halfwidth_digit
fullwidth_alpha
halfwidth_alpha
fullwidth_punct
halfwidth_punct
fullwidth_space
halfwidth_space
fullwidth_graph
halfwidth_graph
fullwidth_print
halfwidth_print
fullwidth
halfwidth
```

これらのカテゴリは日本語テキスト処理を想定したもので、`isctype` を通じて利用できます。

---

## 全角・半角変換

`ctrans_id` には、分類カテゴリに対応する全角・半角変換カテゴリが含まれています。

これらは次を通じて利用できます。

```cpp
auto toctrans(char32_t c, ctrans_id id) -> xer::result<char32_t>;
```

これらの変換は単一コードポイント変換です。
濁点または半濁点を持つ全角仮名文字が複数の半角コードポイントを必要とする場合、現在の単一文字戻り値モデルでは、その記号が失われることがあります。

---

## ローマ字変換識別子

`ctrans_id` には、次のものも含まれています。

```cpp
romaji
romaji_alt
```

これらの識別子は、変換クラスを表すため、他の変換種別と同じ場所で定義されています。
しかし、ローマ字変換は本質的に文字列指向です。

- 拗音は2つの仮名文字を組み合わせることがある
- 促音は後続の音節に依存する
- 長音は、すでに出力された母音を長音符付き母音に置き換えることがある
- 撥音 `ん` は、次の音節によってアポストロフィが必要になることがある

そのため、ローマ字識別子は次を通じて使います。

```cpp
strtoctrans
```

`<xer/string.h>` の機能であり、文字レベルの `toctrans` ではありません。

詳細なローマ字化の動作は `header_string.md` に記載されています。

---

# `<xer/braille.h>`

## 目的

`<xer/braille.h>` は、xer における点字関連の低水準構成要素を提供します。

現段階では、このヘッダーは次のものを提供します。

- UTF-8文字列ビューとして表される共通の点字符号定数
- 英字、数字、英語点字の句読点、および情報処理用点字の句読点に対する1文字変換ヘルパー
- 自動的なモード指示符付きのASCII英数字・句読点テキスト変換
- このヘッダーで宣言される、`xer::ja` 配下の日本語固有の仮名点字ヘルパー

このヘッダーは、完全な日本語点訳は行いません。特に、漢字の読みを決定することも、完全な点字分かち書きを自力で行うこともありません。

これにより、最初の点字レイヤーを小さく再利用しやすいものにしています。呼び出し側は、提供される定数と変換ヘルパーを組み合わせられ、より高水準の変換APIが解析とモード制御を担当できます。

---

## 主な役割

`<xer/braille.h>` の主な役割は、点字文字列を構築するための再利用可能な低水準部品を公開することです。

現在の実装は次のものを提供します。

- 日本語点字の数字指示符と外字符
- 日本語点字の大文字符
- `ip_` 名を持つ情報処理用点字の指示符
- 1文字の英字変換
- すでに数字モードが有効である前提での1文字の数字変換
- 1文字の英数字ディスパッチ
- 1文字の英語点字句読点変換
- 1文字の情報処理用点字変換
- 通常の点字指示符を自動的に付与するASCII英数字・句読点テキスト変換
- 情報処理用点字指示符を自動的に付与するASCII英数字・句読点テキスト変換
- `xer::ja` 配下の日本語固有の仮名点字ヘルパー

定数は次の型で表されます。

```cpp
std::u8string_view
```

1文字変換関数は次の型を返します。

```cpp
xer::result<std::u8string_view>
```

これにより、返される点字断片に対するメモリ割り当てを避けつつ、未対応の入力文字を明示的に報告できます。

日本語仮名テキスト変換関数は `xer::ja` 配下に置かれ、次の型を返します。

```cpp
xer::result<std::u8string>
```

この関数はUTF-8入力をデコードし、対応する複数仮名列を結合し、所有する点字文字列を返します。

---

## 主なエンティティ

`<xer/braille.h>` は現在、次の定数と関数を提供します。

```cpp
namespace xer::braille {

inline constexpr std::u8string_view numeric_indicator;
inline constexpr std::u8string_view alphabetic_indicator;
inline constexpr std::u8string_view capital_indicator;
inline constexpr std::u8string_view double_capital_indicator;

inline constexpr std::u8string_view ip_lowercase_indicator;
inline constexpr std::u8string_view ip_uppercase_indicator;
inline constexpr std::u8string_view ip_single_uppercase_indicator;
inline constexpr std::u8string_view ip_double_uppercase_indicator;
inline constexpr std::u8string_view ip_numeric_indicator;

[[nodiscard]] constexpr auto alpha_to_braille(char32_t c)
    -> result<std::u8string_view>;

[[nodiscard]] constexpr auto digit_to_braille(char32_t c)
    -> result<std::u8string_view>;

[[nodiscard]] constexpr auto alnum_to_braille(char32_t c)
    -> result<std::u8string_view>;

[[nodiscard]] constexpr auto punct_to_braille(char32_t c)
    -> result<std::u8string_view>;

[[nodiscard]] constexpr auto ip_alpha_to_braille(char32_t c)
    -> result<std::u8string_view>;

[[nodiscard]] constexpr auto ip_digit_to_braille(char32_t c)
    -> result<std::u8string_view>;

[[nodiscard]] constexpr auto ip_alnum_to_braille(char32_t c)
    -> result<std::u8string_view>;

[[nodiscard]] constexpr auto ip_punct_to_braille(char32_t c)
    -> result<std::u8string_view>;

[[nodiscard]] auto alnum_punct_text_to_braille(std::u8string_view text)
    -> result<std::u8string>;

[[nodiscard]] auto ip_alnum_punct_text_to_braille(std::u8string_view text)
    -> result<std::u8string>;

} // namespace xer::braille

namespace xer::ja {

[[nodiscard]] constexpr auto japanese_punct_to_braille(char32_t c)
    -> result<std::u8string_view>;

[[nodiscard]] constexpr auto kana_to_braille(char32_t c)
    -> result<std::u8string_view>;

[[nodiscard]] auto kana_text_to_braille(std::u8string_view text)
    -> result<std::u8string>;

} // namespace xer::ja
```

---

日本語固有のヘルパーは `<xer/braille.h>` で宣言されますが、`xer::ja` 配下に置かれます。言語中立のヘルパー、および英語点字・情報処理用点字のヘルパーは `xer::braille` 配下に残ります。

## 日本語点字の指示符

### `numeric_indicator`

```cpp
inline constexpr std::u8string_view numeric_indicator = u8"⠼";
```

`numeric_indicator` は日本語点字の数字指示符です。

Unicode点字パターンの3-4-5-6の点です。

例:

```cpp
std::u8string text;
text += xer::braille::numeric_indicator;
text += u8"⠁⠃⠉";
```

---

### `alphabetic_indicator`

```cpp
inline constexpr std::u8string_view alphabetic_indicator = u8"⠰";
```

`alphabetic_indicator` は日本語点字の外字符です。

Unicode点字パターンの5-6の点です。

例:

```cpp
std::u8string text;
text += xer::braille::alphabetic_indicator;
text += u8"⠁⠃⠉";
```

---

### `capital_indicator`

```cpp
inline constexpr std::u8string_view capital_indicator = u8"⠠";
```

`capital_indicator` は日本語点字の大文字符です。

Unicode点字パターンの6の点です。

例:

```cpp
std::u8string text;
text += xer::braille::capital_indicator;
text += u8"⠁";
```

---

### `double_capital_indicator`

```cpp
inline constexpr std::u8string_view double_capital_indicator = u8"⠠⠠";
```

`double_capital_indicator` は連続する2つの大文字符を表します。

二重大文字接頭辞が必要な点字文字列を構築するときに有用です。

例:

```cpp
std::u8string text;
text += xer::braille::double_capital_indicator;
text += u8"⠁⠃⠉";
```

---

## 情報処理用点字の指示符

情報処理用点字の指示符は、`ip_` 接頭辞付きで `xer::braille` に直接公開されます。

`ip_` 接頭辞により、情報処理用点字の名前を短く保ちながら、通常の日本語点字の指示符と区別しています。

### `ip_lowercase_indicator`

```cpp
inline constexpr std::u8string_view ip_lowercase_indicator = u8"⠰";
```

`ip_lowercase_indicator` は情報処理用点字の小文字符です。

Unicode点字パターンの5-6の点です。

---

### `ip_uppercase_indicator`

```cpp
inline constexpr std::u8string_view ip_uppercase_indicator = u8"⠠";
```

`ip_uppercase_indicator` は情報処理用点字の大文字符です。

Unicode点字パターンの6の点です。

---

### `ip_single_uppercase_indicator`

```cpp
inline constexpr std::u8string_view ip_single_uppercase_indicator = u8"⠠";
```

`ip_single_uppercase_indicator` は1文字だけの大文字指示符を表します。

現段階では、`ip_uppercase_indicator` と同じ点字セルです。

---

### `ip_double_uppercase_indicator`

```cpp
inline constexpr std::u8string_view ip_double_uppercase_indicator = u8"⠠⠠";
```

`ip_double_uppercase_indicator` は連続する2つの大文字指示符を表します。

---

### `ip_numeric_indicator`

```cpp
inline constexpr std::u8string_view ip_numeric_indicator = u8"⠼";
```

`ip_numeric_indicator` は情報処理用点字の数字指示符です。

Unicode点字パターンの3-4-5-6の点です。

---

## 1文字変換ヘルパー

1文字変換ヘルパーは、1つの入力文字を対応する点字セルまたは短い点字断片へ変換します。

これらの関数はモード指示符を出力しません。必要な場合、呼び出し側が `numeric_indicator`、`alphabetic_indicator`、`capital_indicator`、その他の指示符を追加する責任を持ちます。

すべての変換ヘルパーは `xer::result<std::u8string_view>` を返します。

入力文字が選択したヘルパーで対応されていない場合、関数は `error_t::invalid_argument` を返します。

---

## `alpha_to_braille`

```cpp
[[nodiscard]] constexpr auto alpha_to_braille(char32_t c)
    -> result<std::u8string_view>;
```

`alpha_to_braille` は、1つのASCII英字を英語点字のアルファベットセルへ変換します。

受け付ける入力範囲は次のとおりです。

- `A` から `Z`
- `a` から `z`

大文字と小文字は同じ点字セルに対応します。この関数は大文字符を出力しません。

| 入力 | 出力 |
|---|---|
| `a` / `A` | `⠁` |
| `b` / `B` | `⠃` |
| `c` / `C` | `⠉` |
| `d` / `D` | `⠙` |
| `e` / `E` | `⠑` |
| `f` / `F` | `⠋` |
| `g` / `G` | `⠛` |
| `h` / `H` | `⠓` |
| `i` / `I` | `⠊` |
| `j` / `J` | `⠚` |
| `k` / `K` | `⠅` |
| `l` / `L` | `⠇` |
| `m` / `M` | `⠍` |
| `n` / `N` | `⠝` |
| `o` / `O` | `⠕` |
| `p` / `P` | `⠏` |
| `q` / `Q` | `⠟` |
| `r` / `R` | `⠗` |
| `s` / `S` | `⠎` |
| `t` / `T` | `⠞` |
| `u` / `U` | `⠥` |
| `v` / `V` | `⠧` |
| `w` / `W` | `⠺` |
| `x` / `X` | `⠭` |
| `y` / `Y` | `⠽` |
| `z` / `Z` | `⠵` |

例:

```cpp
std::u8string text{xer::braille::alphabetic_indicator};
text += *xer::braille::alpha_to_braille(U'x');
text += *xer::braille::alpha_to_braille(U'e');
text += *xer::braille::alpha_to_braille(U'r');
```

---

## `digit_to_braille`

```cpp
[[nodiscard]] constexpr auto digit_to_braille(char32_t c)
    -> result<std::u8string_view>;
```

`digit_to_braille` は、1つのASCII数字を対応する数字点字セルへ変換します。

受け付ける入力範囲は次のとおりです。

- `0` から `9`

この関数は、必要な場合には呼び出し側がすでに数字指示符を出力していることを前提とします。

数字 `1` から `9` は `a` から `i` と同じセルに対応し、数字 `0` は `j` と同じセルに対応します。

| 入力 | 出力 |
|---|---|
| `1` | `⠁` |
| `2` | `⠃` |
| `3` | `⠉` |
| `4` | `⠙` |
| `5` | `⠑` |
| `6` | `⠋` |
| `7` | `⠛` |
| `8` | `⠓` |
| `9` | `⠊` |
| `0` | `⠚` |

例:

```cpp
std::u8string text{xer::braille::numeric_indicator};
text += *xer::braille::digit_to_braille(U'1');
text += *xer::braille::digit_to_braille(U'2');
text += *xer::braille::digit_to_braille(U'3');
```

---

## `alnum_to_braille`

```cpp
[[nodiscard]] constexpr auto alnum_to_braille(char32_t c)
    -> result<std::u8string_view>;
```

`alnum_to_braille` は、1つのASCII英数字を点字セルへ変換します。

これは `alpha_to_braille` と `digit_to_braille` の小さなディスパッチャーです。

受け付ける入力範囲は次のとおりです。

- `A` から `Z`
- `a` から `z`
- `0` から `9`

この関数は、外字符、大文字符、数字指示符、その他のモード指示符を出力しません。

例:

```cpp
std::u8string text;
text += *xer::braille::alnum_to_braille(U'x');
text += *xer::braille::alnum_to_braille(U'e');
text += *xer::braille::alnum_to_braille(U'r');
text += *xer::braille::alnum_to_braille(U'1');
text += *xer::braille::alnum_to_braille(U'2');
text += *xer::braille::alnum_to_braille(U'3');
```

---

## `punct_to_braille`

```cpp
[[nodiscard]] constexpr auto punct_to_braille(char32_t c)
    -> result<std::u8string_view>;
```

`punct_to_braille` は、1つの英語点字句読点文字を点字セルへ変換します。

現在の実装は、基本的なGrade 1英語点字の句読点を対象とします。
情報処理用点字の句読点、日本語の句読点、文脈依存の句読点規則は実装しません。

対応する文字は次のとおりです。

| 入力 | 出力 | 備考 |
|---|---|---|
| `,` | `⠂` | コンマ |
| `;` | `⠆` | セミコロン |
| `:` | `⠒` | コロン |
| `.` | `⠲` | ピリオド |
| `!` | `⠖` | 感嘆符 |
| `(` | `⠶` | 丸括弧 |
| `)` | `⠶` | 丸括弧 |
| `?` | `⠦` | 疑問符 |
| `“` | `⠦` | 開き引用符 |
| `*` | `⠔` | アスタリスク |
| `”` | `⠴` | 閉じ引用符 |
| `'` | `⠄` | アポストロフィ |
| `-` | `⠤` | ハイフン |
| `‐` | `⠤` | ハイフン |

ASCIIの二重引用符 `"` は対応していません。1文字変換関数では、それが開き引用符か閉じ引用符かを判定できないためです。

---
## 情報処理用の1文字変換ヘルパー

情報処理用ヘルパーは、すでに情報処理用点字モードが選択されている前提で、1つのASCII文字を変換します。

これらの関数は、`ip_lowercase_indicator`、`ip_single_uppercase_indicator`、`ip_double_uppercase_indicator`、`ip_numeric_indicator`、その他のモード指示符を出力しません。

---

## `ip_alpha_to_braille`

```cpp
[[nodiscard]] constexpr auto ip_alpha_to_braille(char32_t c)
    -> result<std::u8string_view>;
```

`ip_alpha_to_braille` は、1つのASCII英字を対応する情報処理用点字のアルファベットセルへ変換します。

現段階では、アルファベットセルは `alpha_to_braille` と同じです。
大文字と小文字は同じセルに対応し、この関数は大文字または小文字の指示符を出力しません。

---

## `ip_digit_to_braille`

```cpp
[[nodiscard]] constexpr auto ip_digit_to_braille(char32_t c)
    -> result<std::u8string_view>;
```

`ip_digit_to_braille` は、1つのASCII数字を対応する情報処理用点字の数字セルへ変換します。

現段階では、数字セルは `digit_to_braille` と同じです。
この関数は `ip_numeric_indicator` を出力しません。

---

## `ip_alnum_to_braille`

```cpp
[[nodiscard]] constexpr auto ip_alnum_to_braille(char32_t c)
    -> result<std::u8string_view>;
```

`ip_alnum_to_braille` は、1つのASCII英数字を `ip_alpha_to_braille` または `ip_digit_to_braille` へ振り分けます。

この関数は情報処理用点字のモード指示符を出力しません。

---

## `ip_punct_to_braille`

```cpp
[[nodiscard]] constexpr auto ip_punct_to_braille(char32_t c)
    -> result<std::u8string_view>;
```

`ip_punct_to_braille` は、1つの表示可能なASCII句読点文字を情報処理用点字セルへ変換します。

`punct_to_braille` と異なり、この関数は情報処理用点字の句読点を対象とします。一部の句読点は複数の点字セルで表されます。

対応する文字は次のとおりです。

| 入力 | 出力 |
|---|---|
| `!` | `⠖` |
| `"` | `⠶` |
| `#` | `⠩` |
| `$` | `⠹` |
| `%` | `⠻` |
| `&` | `⠯` |
| `'` | `⠄` |
| `(` | `⠦` |
| `)` | `⠴` |
| `*` | `⠡` |
| `+` | `⠬` |
| `,` | `⠂` |
| `-` | `⠤` |
| `.` | `⠲` |
| `/` | `⠌` |
| `:` | `⠐⠂` |
| `;` | `⠆` |
| `<` | `⠔⠔` |
| `=` | `⠒⠒` |
| `>` | `⠢⠢` |
| `?` | `⠐⠦` |
| `@` | `⠪` |
| `[` | `⠷` |
| `\\` | `⠫` |
| `]` | `⠾` |
| `^` | `⠘` |
| `_` | `⠐⠤` |
| `` ` `` | `⠐⠑` |
| `{` | `⠣` |
| `|` | `⠳` |
| `}` | `⠜` |
| `~` | `⠐⠉` |

---

## 自動モード指示符付きのASCIIテキスト変換

ASCIIテキスト変換ヘルパーは、短いASCII断片を変換しながら、モード指示符を自動的に出力します。

これらは、入力断片がASCII英数字・句読点断片であることを呼び出し側がすでに分かっている場合に有用です。
MeCabによる読みと文節間隔が必要な日本語テキストには、`mecab_braille_translate` や `mecab_ip_braille_translate` などのMeCabレベルのヘルパーを使用してください。

---

## `alnum_punct_text_to_braille`

```cpp
[[nodiscard]] auto alnum_punct_text_to_braille(std::u8string_view text)
    -> result<std::u8string>;
```

`alnum_punct_text_to_braille` は、ASCIIの英字、数字、空白、および対応する英語点字句読点を通常の点字テキストへ変換します。

この関数は次のものを自動的に出力します。

- 小文字英字列の前の `alphabetic_indicator`
- 1文字の大文字の前の `capital_indicator`
- 2文字以上の大文字列の前の `double_capital_indicator`
- 数字列の前の `numeric_indicator`

ASCII空白は保持され、現在のモードをリセットします。

句読点は `punct_to_braille` を通じて変換され、同様に現在のモードをリセットします。
`+` などの未対応の句読点は `error_t::invalid_argument` を返します。情報処理用点字の句読点には `ip_alnum_punct_text_to_braille` を使用してください。

---

## `ip_alnum_punct_text_to_braille`

```cpp
[[nodiscard]] auto ip_alnum_punct_text_to_braille(std::u8string_view text)
    -> result<std::u8string>;
```

`ip_alnum_punct_text_to_braille` は、ASCIIの英字、数字、空白、および情報処理用句読点を情報処理用点字テキストへ変換します。

この関数は次のものを自動的に出力します。

- 小文字英字列の前の `ip_lowercase_indicator`
- 1文字の大文字の前の `ip_single_uppercase_indicator`
- 2文字以上の大文字列の前の `ip_double_uppercase_indicator`
- 数字列の前の `ip_numeric_indicator`

ASCII空白は保持され、現在のモードをリセットします。

句読点は `ip_punct_to_braille` を通じて変換され、同様に現在のモードをリセットします。
このヘルパーは、`+`、`=`、`<`、`>`、`&`、`|`、`_`、`~` など、プログラミング言語風の記号を含むASCII断片に対する推奨の低水準変換関数です。

---


## `xer::ja::japanese_punct_to_braille`

```cpp
[[nodiscard]] constexpr auto japanese_punct_to_braille(char32_t c)
    -> result<std::u8string_view>;
```

`xer::ja::japanese_punct_to_braille` は、1つの日本語句読点を日本語点字セルへ変換します。

この関数は `punct_to_braille` とは別です。`punct_to_braille` は基本的な英語点字句読点を対象とし、`xer::ja::japanese_punct_to_braille` は日本語テキストで使われる日本語仮名点字の句読点を対象とします。

対応する文字は次のとおりです。

| 入力 | 出力 | 備考 |
|---|---|---|
| `。` | `⠲` | 句点 |
| `、` | `⠰` | 読点 |
| `？` | `⠢` | 日本語疑問符 |
| `?` | `⠢` | 日本語句読点として扱われるASCII疑問符 |
| `！` | `⠖` | 日本語感嘆符 |
| `!` | `⠖` | 日本語句読点として扱われるASCII感嘆符 |
| `・` | `⠂` | 中点 |
| `「` | `⠤` | かぎ括弧 |
| `」` | `⠤` | かぎ括弧 |
| `『` | `⠰⠤` | 二重かぎ括弧 |
| `』` | `⠰⠤` | 二重かぎ括弧 |
| `（` | `⠶` | 丸括弧 |
| `）` | `⠶` | 丸括弧 |
| `(` | `⠶` | 日本語句読点として扱われるASCII丸括弧 |
| `)` | `⠶` | 日本語句読点として扱われるASCII丸括弧 |
| `…` | `⠄⠄⠄` | 三点リーダー |
| `‥` | `⠄⠄` | 二点リーダー |

`xer::ja::japanese_punct_to_braille` は句読点の周囲に空白を挿入しません。空白の扱いは `mecab_braille_wakati` などのより高水準のテキスト変換関数が担当します。

---

## `xer::ja::kana_to_braille`

```cpp
[[nodiscard]] constexpr auto kana_to_braille(char32_t c)
    -> result<std::u8string_view>;
```

`xer::ja::kana_to_braille` は、1つの日本語仮名文字を日本語点字セルへ変換します。

同じ音節に対して、ひらがなとカタカナの両方を受け付けます。

この関数は次のものを扱います。

- 基本的な仮名
- `ゐ` / `ヰ`
- `ゑ` / `ヱ`
- `ん` / `ン`
- 長音符 `ー`
- 促音 `っ` / `ッ`
- 濁音の仮名
- 半濁音の仮名
- `ゔ` / `ヴ`

一部の仮名文字は複数の点字セルに対応します。たとえば、濁音と半濁音の仮名は、符号に続いて基底仮名セルで表されます。

この関数は1つの入力文字だけを変換します。複数の入力文字を結合しないため、`きゃ`、`シェ`、`ティ` などの列は `xer::ja::kana_to_braille` ではなく `xer::ja::kana_text_to_braille` によって処理されます。

---

## `xer::ja::kana_text_to_braille`

```cpp
[[nodiscard]] auto kana_text_to_braille(std::u8string_view text)
    -> result<std::u8string>;
```

`xer::ja::kana_text_to_braille` は、UTF-8仮名テキストを日本語点字テキストへ変換します。

この関数は次の低水準テキスト変換を行います。

- UTF-8入力をデコードする
- ASCII空白を分かち書き区切りとして保持する
- `xer::ja::japanese_punct_to_braille` を通じて日本語句読点を変換する
- `xer::ja::kana_to_braille` を通じて通常の仮名を変換する
- 変換前に対応する小書き仮名列を結合する

この関数は、不正なUTF-8入力に対して `error_t::encoding_error` を返し、未対応の文字または未対応の小書き仮名の組み合わせに対して `error_t::invalid_argument` を返します。

### 対応する複数仮名列

`xer::ja::kana_text_to_braille` は、通常の拗音列と、いくつかの拡張外来音仮名列に対応します。

対応する基底仮名＋小書き仮名のグループは次のとおりです。

- `きゃ` / `きゅ` / `きょ` / `きぇ`
- `しゃ` / `しゅ` / `しょ` / `しぇ`
- `ちゃ` / `ちゅ` / `ちょ` / `ちぇ`
- `にゃ` / `にゅ` / `にょ` / `にぇ`
- `ひゃ` / `ひゅ` / `ひょ` / `ひぇ`
- `みゃ` / `みゅ` / `みょ`
- `りゃ` / `りゅ` / `りょ`
- `ぎゃ` / `ぎゅ` / `ぎょ`
- `じゃ` / `じゅ` / `じょ` / `じぇ`
- `すぃ`
- `ずぃ`
- `ぢゃ` / `ぢゅ` / `ぢょ`
- `びゃ` / `びゅ` / `びょ`
- `ぴゃ` / `ぴゅ` / `ぴょ`
- `いぇ`
- `うぃ` / `うぇ` / `うぉ`
- `くぁ` / `くぃ` / `くぇ` / `くぉ`
- `ぐぁ` / `ぐぃ` / `ぐぇ` / `ぐぉ`
- `つぁ` / `つぃ` / `つぇ` / `つぉ`
- `てぃ` / `てゅ`
- `でぃ` / `でゅ`
- `とぅ`
- `どぅ`
- `ふぁ` / `ふぃ` / `ふぇ` / `ふぉ` / `ふゅ` / `ふょ`
- `ゔぁ` / `ゔぃ` / `ゔぇ` / `ゔぉ` / `ゔゅ` / `ゔょ`

同じ組み合わせは、`キャ`、`キェ`、`シェ`、`スィ`、`ズィ`、`ティ`、`ファ`、`ヴォ` などのカタカナ形でも受け付けられます。

未対応の小書き仮名の組み合わせは、推測されずに拒否されます。

### 範囲

`xer::ja::kana_text_to_braille` は、まだ低水準の仮名変換関数です。

この関数は次のことを行いません。

- 漢字から読みを決定する
- `は`、`へ`、`を` などの助詞を補正する
- 完全な日本語点字分かち書きを行う
- 日本語テキスト中のASCII断片に対して、数字指示符または外字符を自動的に出力する
- 混在日本語テキストに対して、通常の点字と情報処理用点字を自動的に選択する

既知のASCII断片を直接変換する場合は、`alnum_punct_text_to_braille` または `ip_alnum_punct_text_to_braille` を使用してください。
MeCab由来の読みと、おおよその点字向け分かち書きが必要な場合は、`mecab_braille_wakati`、`mecab_ip_braille_wakati`、`mecab_braille_translate`、または `mecab_ip_braille_translate` を使用してください。

---

## エラー処理

点字変換ヘルパーは、未対応入力を明示的に報告するために `xer::result` を使用します。

一般的なエラーは次のとおりです。

| エラー | 意味 |
|---|---|
| `error_t::invalid_argument` | 入力文字または文字列が、選択した変換ヘルパーで対応されていない。 |
| `error_t::encoding_error` | UTF-8テキスト変換関数が不正なUTF-8入力を受け取った。 |

1文字ヘルパーはメモリ割り当てを行いません。`xer::ja::kana_text_to_braille` は入力文字を結合し、複数の出力断片を追加する場合があるため、所有する `std::u8string` を返します。

---

## ヘッダー依存関係

`<xer/braille.h>` は点字関連APIの公開ヘッダーです。

実装詳細は `xer/bits/` 配下に分割されており、次のものを含みます。

```cpp
#include <xer/bits/braille_symbols.h>
#include <xer/bits/braille_chars.h>
```

ユーザーコードでは、`xer/bits/` ヘッダーを直接インクルードするのではなく、次のようにしてください。

```cpp
#include <xer/braille.h>
```

---

# `<xer/stdlib.h>`

## 目的

`<xer/stdlib.h>` は、xer における汎用ユーティリティ群を提供します。

このヘッダーの役割は、C 標準ライブラリの `<stdlib.h>` と精神的には似ていますが、字義どおりの再現を意図したものではありません。
かわりに、xer の設計に合う実用的な機能、とくに次の領域の機能を集めています。

- 数値変換
- マルチバイト文字変換
- ソートと検索
- 環境へのアクセス
- 擬似乱数生成
- 算術系 API に関連する小さなユーティリティ構造体

そのため、このヘッダーは、狭い単一の抽象化でまとめられたものというより、実用上の関連によって内容が結び付けられた広範なユーティリティヘッダーとして機能します。

---

## 主な役割

`<xer/stdlib.h>` の主な役割は、次のようなヘッダーにはより自然には属さないユーティリティ機能を提供することです。

- `<xer/string.h>`
- `<xer/ctype.h>`
- `<xer/stdio.h>`
- `<xer/path.h>`
- `<xer/time.h>`

特に、次の機能の置き場所になります。

- 文字列から数値への変換
- 整数から文字列への変換
- マルチバイト変換 API
- 検索およびソートのヘルパー
- 擬似乱数ユーティリティ
- 環境変数へのアクセス

これは、伝統的に `<stdlib.h>` に結び付けられてきた広くやや雑多な役割に対応しつつ、細部を xer 独自の方針に合わせるものです。

---

## 主な関数グループ

大まかに見ると、`<xer/stdlib.h>` には次の機能グループが含まれます。

- 整数除算結果構造体
- 検索とソート
- 数値変換
- マルチバイト変換
- 環境へのアクセス
- 擬似乱数生成

---

## 整数除算結果構造体

このヘッダーは、商と余りを扱う操作に関連する小さな構造体を提供する場合があります。たとえば次のようなものです。

```cpp
template <class T>
struct rem_quot;

using div_t;
using ldiv_t;
using lldiv_t;

using i8div_t;
using i16div_t;
using i32div_t;
using i64div_t;

using u8div_t;
using u16div_t;
using u32div_t;
using u64div_t;
```

### このグループの役割

これらの型は、商と余りをまとめて扱う場合のために、名前付きの構造体を提供します。

除算系のヘルパーが複数の出力先に値を書き込むのではなく、両方の値を構造化された形で返したい場合に便利です。

### 注意

* 正確な名前付けと型の範囲は、xer 独自の設計に従います
* 概念的には算術ヘルパーに関連しますが、ユーティリティ構造体としての側面は `<xer/stdlib.h>` に自然に属します

---

## 検索とソート

少なくとも、このヘッダーは次のような関数を提供します。

```cpp
bsearch
qsort
```

### このグループの役割

これらの関数は、C 標準ライブラリ風の古典的な汎用検索およびソート操作を提供します。

### 設計方針

名前は馴染みのあるものですが、xer のより広いスタイルに従って理解する必要があります。

* 厳密な歴史的忠実性よりも、実用上の使いやすさを優先します
* 周辺の型やエラーモデルは、伝統的な C の期待とは異なる場合があります
* ドキュメントでは、C 由来の思い込みに頼るのではなく、実際の xer における振る舞いを記述するべきです

---

## 数値変換

`<xer/stdlib.h>` の主要な部分のひとつが数値変換です。

少なくとも、このヘッダーは次のような関数を提供する場合があります。

```cpp
ato
atoi
atol
atoll

itostr
itoa

strto
strtol
strtoll
strtoul
strtoull

strtof
strtod
strtold
strtof32
strtof64
```

### このグループの役割

これらの関数は、テキストを数値に変換します。

xer はロケール依存の解釈に頼るのではなく、明示的なテキスト方針とエンコーディング方針を用いるため、これらの関数は特に重要です。

### 設計方針

これらの機能は、標準ライブラリを厳密に再現することを意図していません。
かわりに、xer のモデルに合うように再構成されています。

重要な性質には、次のようなものが含まれます。

* `char`、`char8_t`、`wchar_t`、`char16_t`、`char32_t` の入力列に対して、ASCII 互換の解析をサポートすること
* 失敗を明示的に報告すること
* xer で採用された場合には、2 進数プレフィックスなどの実用的な構文解析機能を扱うこと
* ライブラリがサポートする環境間で一貫した振る舞いを提供すること

### 注意

浮動小数点変換では、次のようなテキスト形式を認識する場合があります。

* `inf`
* `infinity`
* `nan`

整数変換でも、次のような実用的な形式をサポートする場合があります。

* 10 進数
* 8 進数
* 16 進数
* xer で採用された場合の `0b...` による 2 進数

正確に受け入れられる文法は、詳細な API ドキュメントに属します。数値解析の多重定義は、対応する C++ 文字型について、`std::basic_string_view`、`std::basic_string`、NUL 終端ポインタ入力を受け取り、ASCII の数字と符号を解析します。

### 整数から文字列への変換

`itostr` は整数値を ASCII 互換の数字列に変換し、呼び出し側が与えた出力先に格納します。
`itoa` は `itostr` の別名です。

```cpp
template <class Integer, class CharT>
auto itostr(Integer value, std::basic_string<CharT>& str, int radix = 10)
    -> result<std::basic_string<CharT>*>;

template <class Integer, class CharT, std::size_t N>
auto itostr(Integer value, CharT (&s)[N], int radix = 10)
    -> result<CharT*>;
```

同じオーバーロードは `itoa` という名前でも利用できます。

出力文字型には、`char`、`char8_t`、`wchar_t`、`char16_t`、`char32_t` を使用できます。
生成される文字は ASCII 数字に限られます。
基数が 10 を超える場合は、小文字の `a` から `z` を使用します。

対応する基数は 2 以上 36 以下です。
範囲外の基数を指定した場合は `error_t::invalid_argument` を返します。
符号付き整数値が負の場合は、先頭に `-` を出力します。

`std::basic_string` 版は、変換が成功した後で出力先文字列を置き換えます。
成功時には出力先文字列へのポインターを返します。
文字配列版は、与えられたバッファーに NUL 終端文字列を書き込み、バッファーへのポインターを返します。
バッファーが不足している場合は `error_t::length_error` を返し、バッファーを変更しません。

例:

```cpp
std::u8string text;
const auto result = xer::itostr(255, text, 16);
// text == u8"ff"
```

---

## マルチバイト変換

xer における `<xer/stdlib.h>` の最も重要な役割のひとつが、マルチバイト文字変換です。

少なくとも、このヘッダーは次の状態型と関連機能を提供します。

```cpp
enum class multibyte_encoding;
struct mbstate_t;
```

また、次のような関数を提供します。

```cpp
mblen
mbtotc
tctomb
mbstotcs
tcstombs
```

### このグループの役割

これらの関数は、マルチバイトテキストと文字指向の表現との間の変換を提供します。

これは、xer が標準ライブラリの領域を独自のエンコーディング方針に基づいて意図的に再設計していることが最も明確に表れる場所のひとつです。

### 基本的な設計方針

xer のマルチバイト変換モデルは、次の考え方に基づいています。

* サポートするエンコーディングは限定的かつ明示的である
* ロケールを設計の中心に置かない
* `char`、`unsigned char`、`char8_t` を明示的に区別する
* 変換先または変換元の文字型として、`wchar_t`、`char16_t`、`char32_t` をサポートする
* 状態付き変換は `xer::mbstate_t*` によって明示的に表現する

### オーバーロードに基づく設計

これらの関数は、テンプレートではなくオーバーロード集合として提供されます。

これにより、次のような組み合わせを扱えます。

* `char`、`unsigned char`、`char8_t` によるバイト指向の入出力
* `wchar_t`、`char16_t`、`char32_t` による文字指向の入出力

### エンコーディング方針との関係

`<xer/stdlib.h>` のこの部分は、常により広いエンコーディング方針と合わせて読むべきです。
この部分は、このヘッダー全体の中でも特に方針に強く依存している領域のひとつです。

---

## `xer::mbstate_t`

`xer::mbstate_t` は、状態付きマルチバイト変換で使われる中間状態を保持します。

### 目的

この型が存在する目的は、変換状態を次のように扱えるようにすることです。

* 明示的である
* 呼び出しをまたいで持ち運べる
* 内部の静的ストレージに隠さない

### 設計方針

`xer::mbstate_t` は、少なくとも次のような情報を保持します。

* 有効なエンコーディング
* 未完成のバイト列
* 保持しているバイト数

これは、未完成のマルチバイト入力は、エンコーディング文脈を記憶していなければ正しく解釈できないために必要です。

### 注意

* xer は隠れた内部静的変換状態を使いません
* 呼び出し側が独立した変換呼び出しを望む場合は、状態引数を省略します
* 呼び出し側が継続的な状態付き変換を望む場合は、`xer::mbstate_t*` を渡します

---

## 環境へのアクセス

このヘッダーは、次のような環境変数アクセス機能を提供します。

```cpp
struct environ_entry;
using environ_arg = std::pair<std::u8string_view, std::u8string_view>;

class environs;

auto getenv(std::u8string_view name) -> xer::result<std::u8string>;
auto get_environs() -> xer::result<environs>;
```

### このグループの役割

この関数グループは、プロセスの環境情報へのアクセスを提供します。

`getenv` は、名前を指定して 1 つの環境変数を取得します。
`get_environs` は、すべての環境変数の UTF-8 スナップショットを取得し、それを `environs` オブジェクトとして返します。

### `environs`

`environs` は、環境変数エントリを UTF-8 文字列として所有します。
これは `get_environs` が呼び出された時点のプロセス環境のスナップショットです。
その後にプロセス環境が変更されても、すでに作成済みの `environs` オブジェクトは更新されません。

少なくとも、`environs` は次を提供します。

```cpp
auto size() const noexcept -> std::size_t;
auto empty() const noexcept -> bool;
auto entries() const noexcept -> std::span<const environ_entry>;
auto at(std::size_t index) const -> xer::result<environ_arg>;
auto find(std::u8string_view name) const -> xer::result<std::u8string_view>;
```

`at` は、1 つのエントリについて名前と値のビューを返します。
`find` は、要求された名前に一致する最初の値を返します。
要求された名前が空の場合、`find` は `error_t::invalid_argument` で失敗します。
その名前が存在しない場合は、`error_t::not_found` で失敗します。

### プラットフォーム上のエンコーディング方針

Windows では、`get_environs` は `GetEnvironmentStringsW` を使用します。そのため、環境文字列は UTF-16 として取得され、UTF-8 に変換されます。

Linux では、`get_environs` はプロセスの環境配列を読み取り、それぞれの名前と値が妥当な UTF-8 であることを要求します。

`=` を含まないエントリ、および名前部分が空のエントリは無視されます。

### 注意

xer の他の機能と同様に、正確な引数規約と戻り値規約は、C 標準ライブラリからの推測ではなく xer のドキュメントに従って読むべきです。

---

## 擬似乱数生成

このヘッダーは、次のような機能を提供する場合があります。

```cpp
rand
srand
class rand_context
```

### このグループの役割

これらの関数と型は、単純な擬似乱数生成機能を提供します。

### 設計方針

ここでの目的は、`<random>` の完全な汎用性と競合することではありません。
かわりに、伝統的な C の機能に近い、より軽く近づきやすいユーティリティ層を提供しつつ、xer の設計にも合うようにすることが目的です。

### `rand_context`

`rand_context` のような文脈型は、次のような場合に便利です。

* 乱数状態を明示したい場合
* グローバル状態への依存を減らしたい場合
* 複数の独立した生成器が必要な場合

正確な API は詳細なリファレンスドキュメントに属します。

---

## xer のテキストモデルとの関係

`<xer/stdlib.h>` は、xer のテキストモデルおよびエンコーディングモデルと密接に結び付いています。

これは特に次の機能について当てはまります。

* 数値変換
* マルチバイト変換

### 重要な前提

* UTF-8 は主要な公開文字列表現です
* `char8_t`、`std::u8string`、`std::u8string_view` は公開テキスト API の中心です
* 個々の Unicode スカラー値の通常の型は `char32_t` です
* マルチバイト変換では、`char`、`unsigned char`、`char8_t` を明示的に区別します

つまり、`<xer/stdlib.h>` は単なる雑多なユーティリティヘッダーではありません。
xer では、明示的なエンコーディング認識テキスト処理のための重要なヘッダーのひとつでもあります。

---

## 他のヘッダーとの関係

`<xer/stdlib.h>` は、次のヘッダーおよび方針文書と合わせて理解するべきです。

* `header_string.md`
* `header_ctype.md`
* `policy_multibyte.md`
* `policy_encoding.md`
* `policy_project_outline.md`

大まかな境界は次のとおりです。

* `<xer/string.h>` は文字列およびメモリユーティリティを扱います
* `<xer/ctype.h>` は文字分類と文字変換を扱います
* `<xer/stdlib.h>` は数値変換、マルチバイト変換、その他の雑多なユーティリティ機能を扱います
* エンコーディング方針とマルチバイト方針文書は、API の背後にあるより深いモデルを定義します

---

## ドキュメント上の注意

このヘッダーを生成ドキュメントの一部として使用する場合、通常は次を説明すれば十分です。

* 狭く範囲を絞った抽象化ではなく、ユーティリティヘッダーとして機能すること
* 数値変換とマルチバイト変換が、このヘッダーの最も重要な部分であること
* マルチバイト変換が、ロケール中心の C の振る舞いではなく xer 独自のエンコーディング方針に従うこと
* オーバーロードが、バイト指向の型と文字指向の型を明示的に区別すること

詳細な関数ごとの意味論は、リファレンスマニュアルまたは生成された API 節で記述するべきです。

---

## 例として示す価値が高いトピック

このヘッダーでは、次のような例が特に適しています。

* UTF-8 テキストを整数値または浮動小数点値に変換する
* `mbtotc` で 1 文字のマルチバイト文字を変換する
* `tctomb` で 1 文字を戻す
* `mbstotcs` または `tcstombs` でマルチバイト文字列全体を変換する
* 最小限の例で `rand` / `srand` または `rand_context` を使う

これらは `examples/` 以下の実行可能な例の候補として適しています。

---

## 例

```cpp
#include <xer/stdlib.h>

auto main() -> int
{
    const auto result = xer::strtol(u8"123");
    if (!result.has_value()) {
        return 1;
    }

    if (*result != 123) {
        return 1;
    }

    return 0;
}
```

この例は、xer の一般的なスタイルを示しています。

* 通常の値を渡して変換 API を呼び出す
* `xer::result` を明示的に確認する
* 通常の失敗を通常の制御フローの一部として扱う

---

## 関連項目

* `policy_project_outline.md`
* `policy_encoding.md`
* `policy_multibyte.md`
* `header_string.md`
* `header_ctype.md`

---

# `<xer/kansuji.h>`

## 目的

`<xer/kansuji.h>` は、実用的な日本語整数表記の変換機能を提供します。

対応する内容は次のとおりです。

- `std::uint64_t` から漢数字テキストを生成する
- 実用的な漢数字テキストを `std::uint64_t` へ解析する
- 複数の明示的な出力スタイル
- 解析時の限定的で実用的な大字バリアント

初期 API は意図的に非負整数だけを対象にします。負数と小数は現在の範囲外です。

---

## 主な役割

`<xer/kansuji.h>` の主な役割は、次を可能にすることです。

- 大きな整数を日本語の大単位表記で書く
- 読みやすい出力スタイルを明示的に選ぶ
- 呼び出し側が手作業で正規化しなくても一般的な日本語数値テキストを解析する
- 不正なテキストとオーバーフローを xer の通常の `xer::result` エラーモデルで報告する

この機能は、日本語テキスト生成、ユーザー向けテキストの解析、縦書き向け表記、実用的な大字形式の出力で特に有用です。

---

## 主なエンティティ

少なくとも `<xer/kansuji.h>` は次を提供します。

```cpp
enum class kansuji_style : std::uint8_t;

inline constexpr kansuji_style k10;
inline constexpr kansuji_style k十;
inline constexpr kansuji_style k一〇;
inline constexpr kansuji_style k拾;

auto to_kansuji(std::uint64_t value, kansuji_style style)
    -> std::u8string;

auto from_kansuji(std::u8string_view text)
    -> xer::result<std::uint64_t>;
```

---

## `kansuji_style` とスタイルセレクタ

出力形式は `kansuji_style` で選択します。通常、呼び出し側は公開セレクタ定数を直接使います。

| セレクタ | 基本方針 | `123456789012` の例 |
|---|---|---|
| `xer::ja::k10` | アラビア数字と日本語大単位 | `1234億5678万9012` |
| `xer::ja::k十` | 通常の位取り漢数字 | `千二百三十四億五千六百七十八万九千十二` |
| `xer::ja::k一〇` | 桁ごとの漢数字 | `一二三四億五六七八万九〇一二` |
| `xer::ja::k拾` | 実用的な大字位取り漢数字 | `壱千弐百参拾四億五千六百七拾八万九千壱拾弐` |

セレクタ名は、各スタイルで数値 `10` をどのように書くかに基づいています。

```cpp
xer::ja::k10
xer::ja::k十
xer::ja::k一〇
xer::ja::k拾
```

---

## `to_kansuji`

```cpp
auto to_kansuji(std::uint64_t value, kansuji_style style)
    -> std::u8string;
```

### 目的

`to_kansuji` は、符号なし 64 ビット整数を日本語数値テキストへ変換します。

### 出力スタイル

#### `xer::ja::k10`

`xer::ja::k10` は、日本語の 4 桁単位グループ内でアラビア数字を使います。

```cpp
xer::ja::to_kansuji(UINT64_C(123456789012), xer::ja::k10);
// 1234億5678万9012
```

#### `xer::ja::k十`

`xer::ja::k十` は通常の位取り漢数字を使います。

```cpp
xer::ja::to_kansuji(UINT64_C(123456789012), xer::ja::k十);
// 千二百三十四億五千六百七十八万九千十二
```

`十`、`百`、`千` では、生成される形式は先頭の `一` を省略します。

| 値 | 出力 |
|---:|---|
| `10` | `十` |
| `100` | `百` |
| `1000` | `千` |
| `110` | `百十` |

#### `xer::ja::k一〇`

`xer::ja::k一〇` は各 10 進数字を独立して書きます。ゼロは `〇` として生成されます。

```cpp
xer::ja::to_kansuji(UINT64_C(123456789012), xer::ja::k一〇);
// 一二三四億五六七八万九〇一二
```

このスタイルは、年号や一部の縦書き文脈など、数字を一桁ずつ読む慣習がある場合に適しています。

| 値 | 出力 |
|---:|---|
| `10` | `一〇` |
| `2026` | `二〇二六` |
| `9012` | `九〇一二` |

#### `xer::ja::k拾`

`xer::ja::k拾` は実用的な大字位取りスタイルを生成します。

```cpp
xer::ja::to_kansuji(UINT64_C(110), xer::ja::k拾);
// 壱百壱拾
```

現在の生成方針では次を使います。

- `壱`
- `弐`
- `参`
- `拾`

その他の数字は通常の漢数字のままです。

`xer::ja::k十` と異なり、生成される大字出力では小単位の前の `壱` を**省略しません**。

| 値 | 出力 |
|---:|---|
| `10` | `壱拾` |
| `100` | `壱百` |
| `110` | `壱百壱拾` |
| `1000` | `壱千` |
| `10000` | `壱万` |

---

## ゼロの出力

`to_kansuji(0, style)` は、スタイルごとに次の出力を使います。

| スタイル | 出力 |
|---|---|
| `xer::ja::k10` | `0` |
| `xer::ja::k十` | `零` |
| `xer::ja::k一〇` | `〇` |
| `xer::ja::k拾` | `零` |

---

## 大単位構造

現在の実装は次の日本語大単位をサポートします。

```text
万
億
兆
京
```

数値は 4 桁グループに分割されます。たとえば次の数値は、

```text
123456789012
```

次のようにグループ化されます。

```text
1234億5678万9012
```

値がゼロのグループは生成出力から省略されます。

---

## `from_kansuji`

```cpp
auto from_kansuji(std::u8string_view text)
    -> xer::result<std::uint64_t>;
```

### 目的

`from_kansuji` は、実用的な日本語整数表記を `std::uint64_t` へ解析します。

### 受け付ける表記系

パーサーは、`to_kansuji` が生成する主な表記系を受け付けます。

例:

```cpp
xer::ja::from_kansuji(u8"12億34万5");
xer::ja::from_kansuji(u8"一二億三四万五");
xer::ja::from_kansuji(u8"十二億三十四万五");
```

これら 3 つの例はいずれも次を表します。

```text
12億0034万0005
```

解析結果は次です。

```text
1200340005
```

パーサーは全体がゼロである次の形式も受け付けます。

```text
0
零
〇
```

---

## 受け付ける小単位バリアント

通常の位取り漢数字では、小単位の前の `一` について、省略形と明示形の両方を受け付けます。

| テキスト | 値 |
|---|---:|
| `十` | `10` |
| `一十` | `10` |
| `百` | `100` |
| `一百` | `100` |
| `千` | `1000` |
| `一千` | `1000` |

`xer::ja::k十` の生成では不自然な `一十`、`一百`、`一千` は使いませんが、解析では受け付けます。

---

## 大字の解析

`from_kansuji` は実用的な大字の一部を受け付け、内部で正規化します。

| 入力 | 通常形 |
|---|---|
| `壱` | `一` |
| `弐` | `二` |
| `参` | `三` |
| `拾` | `十` |
| `佰` | `百` |
| `阡` | `千` |
| `萬` | `万` |

例:

```text
拾
壱拾
百拾
壱百壱拾
阡佰拾
壱阡壱佰壱拾
```

これらは実用的な大字バリアントとして受け付けられます。

現在の実装では、次のようなまれな大字数字は受け付けません。

```text
肆
伍
陸
漆
捌
玖
```

---

## 不正な入力

構文的に不正なテキストは次として報告されます。

```cpp
error_t::invalid_argument
```

これには少なくとも次が含まれます。

- 空テキスト
- 未対応文字
- 未対応のまれな大字数字
- 前に数値グループを持たない大単位
- 不正な順序の大単位
- 壊れた位取り漢数字
- 明示的にゼロ埋めされた大単位グループ

不正な入力例:

```text
万
億
一億万
一万億
一億〇
十百
十二三
1億0001万1
一億〇〇〇一万一
```

---

## オーバーフロー

解析された値が `std::uint64_t` を超える場合、パーサーは次を報告します。

```cpp
error_t::overflow_error
```

例:

```cpp
xer::ja::from_kansuji(u8"1844京6744兆737億955万1616");
// overflow_error
```

---

## エラーモデル

`from_kansuji` は xer の通常の失敗モデルに従います。

```cpp
const auto parsed = xer::ja::from_kansuji(u8"十二億三十四万五");
if (!parsed) {
    // ここで parsed.error().code を参照できます。
}
```

通常の解析失敗に例外は使いません。

---

## 後回しにしている項目と制限

次の項目は初期実装では意図的に範囲外です。

- 負数
- 小数
- 網羅的な大字対応
- `肆`、`伍`、`陸`、`漆`、`捌`、`玖` などのまれな大字数字
- より広い歴史的表記
- 日本語の数値表現に対する自由形式の自然言語解釈

---

## 他ヘッダーとの関係

`<xer/kansuji.h>` は次と関連します。

- `<xer/error.h>`
- 日本語テキスト処理という広い意味での `<xer/string.h>`
- `policy_kansuji.md`
- `policy_encoding.md`

---

# `<xer/mecab.h>`

## 目的

`<xer/mecab.h>` は、xer における初期の MeCab ベース日本語テキスト解析 API を提供します。

現在の実装は、もっとも低レベルな公開基盤に重点を置いています。

- MeCab を子プロセスとして起動する
- UTF-8 の入力テキストを MeCab に渡す
- UTF-8 の解析出力を受け取る
- 形態素トークンの結果を返す
- 生の feature テキストを保持する
- 一般的な MeCab/IPADIC 形式の列を分割済み feature フィールドとして提供する
- 実用的な文節風の句範囲と記号範囲を導出する
- MeCab 由来の読みを仮名テキストへ変換する
- 句範囲を使って仮名分かち書きテキストを生成する
- 仮名変換と `strtoctrans` を組み合わせてローマ字分かち書きテキストを生成する
- 句範囲、MeCab 由来の仮名読み、日本語句読点処理、ASCII 断片変換、`<xer/braille.h>` を組み合わせて日本語点字分かち書きテキストを生成する
- ASCII 断片向けに情報処理点字の変種を生成する
- 便宜ラッパーにより、入力テキストを解析して直接点字へ変換する

ルビ生成などのより高レベルな日本語テキスト処理は、この解析層の上に構築する予定です。点字向け分かち書き変換については、仮名層の上に初期ヘルパーが用意されています。

---

## 主な役割

現段階の `<xer/mecab.h>` の主な役割は、MeCab の形態素解析結果を xer ユーザーが直接確認し再利用できる形で公開することです。

生の feature 文字列は保持されます。また、xer はそれを `mecab_features` に分割するため、品詞や読みなどの一般的な項目を、ユーザーコードでカンマ区切り feature 文字列を再解析せずに参照できます。

トークン層の上では、xer は `mecab_split_phrases` により、実用的な文節風の範囲と独立した記号範囲を導出します。MeCab 自体は文節境界を返さないため、この層は xer の規則ベース近似です。

仮名層は、利用可能な場合には `mecab_features::読み` を使用し、読みベースの実用的な変換ヘルパーとして `mecab_to_kana` と `mecab_kana_wakati` を提供します。

点字層は、トークン、句、仮名、句読点、ASCII 断片変換の各層の上に構築されます。`mecab_braille_wakati` は、MeCab トークン列向けの実用的な日本語点字分かち書きヘルパーです。`mecab_kana_wakati` と異なり、記号範囲を直接扱うため、日本語句読点を点字出力により自然に付けられます。また、ASCII 英数字・句読点断片は MeCab の読みではなく、元の表層形から変換します。

情報処理点字版は `mecab_ip_braille_wakati` です。同じ日本語読み規則と空白規則を使いますが、ASCII 断片を情報処理点字として変換します。

入力テキストを直接渡したい呼び出し側のために、`mecab_braille_translate` と `mecab_ip_braille_translate` は、`mecab_parse` と対応する点字分かち書きヘルパーを組み合わせます。

ローマ字層は、仮名層と `strtoctrans` の上に構築されます。`mecab_romaji_wakati` は、実用的なローマ字分かち書きヘルパーです。ローマ字化の前に助詞読み補正を行うため、`は`、`へ`、`を` などの助詞は最終出力で `wa`、`e`、`o` になります。

xer は MeCab ライブラリへリンクしません。代わりに、xer のプロセス機能を使って `mecab` コマンドを子プロセスとして実行します。

これにより、MeCab 由来の解析データを通常の `xer::result` API で利用可能にしつつ、xer のヘッダーオンリーモデルと互換性のある統合にしています。

---

## 環境上の前提

現在の実装は、MeCab の入出力が UTF-8 であることを前提にしています。

この機能で使う通常の対象環境として、プロジェクトでは次を確認しています。

- 通常のパッケージ導入手順で MeCab をインストールした Ubuntu
- 通常の MeCab パッケージを使用する MSYS2 UCRT64

どちらの場合も、設計確認時には `mecab -D` が辞書エンコーディングとして UTF-8 を報告しました。

そのため、xer は次のように動作します。

- UTF-8 入力テキストを MeCab に送る
- MeCab 出力を UTF-8 として検証する
- MeCab プロセスの前後で文字セット変換を行わない

---

## 主なエンティティ

`<xer/mecab.h>` は現在、次を提供します。

```cpp
struct mecab_options {
    xer::path program;
};

struct mecab_features {
    std::u8string 品詞;
    std::u8string 品詞細分類1;
    std::u8string 品詞細分類2;
    std::u8string 品詞細分類3;
    std::u8string 活用型;
    std::u8string 活用形;
    std::u8string 原形;
    std::u8string 読み;
    std::u8string 発音;
    std::vector<std::u8string> 項目;
};

struct mecab_token {
    std::u8string surface;
    std::u8string feature;
    mecab_features features;
};

enum class mecab_phrase_kind {
    bunsetsu,
    symbol,
};

struct mecab_phrase {
    mecab_phrase_kind kind = mecab_phrase_kind::bunsetsu;
    std::size_t index = 0;
    std::size_t count = 0;
};

enum class mecab_kana_kind {
    mixed,
    hiragana,
    katakana,
};

struct mecab_kana_options {
    mecab_kana_kind kind = mecab_kana_kind::mixed;
    bool particle_reading = true;
};

struct mecab_romaji_options {
    mecab_kana_options kana;
    ctrans_id romaji = ctrans_id::romaji;
};

[[nodiscard]]
auto mecab_split_phrases(
    std::span<const mecab_token> tokens)
    -> std::vector<mecab_phrase>;

[[nodiscard]]
auto mecab_to_kana(
    std::span<const mecab_token> tokens,
    const mecab_kana_options& options = {})
    -> std::u8string;

[[nodiscard]]
auto mecab_kana_wakati(
    std::span<const mecab_token> tokens,
    const mecab_kana_options& options = {})
    -> std::u8string;

[[nodiscard]]
auto mecab_braille_wakati(
    std::span<const mecab_token> tokens,
    const mecab_kana_options& options = {})
    -> xer::result<std::u8string>;

[[nodiscard]]
auto mecab_ip_braille_wakati(
    std::span<const mecab_token> tokens,
    const mecab_kana_options& options = {})
    -> xer::result<std::u8string>;

[[nodiscard]]
auto mecab_braille_translate(
    std::u8string_view text,
    const mecab_options& parse_options = {},
    const mecab_kana_options& kana_options = {})
    -> xer::result<std::u8string>;

[[nodiscard]]
auto mecab_ip_braille_translate(
    std::u8string_view text,
    const mecab_options& parse_options = {},
    const mecab_kana_options& kana_options = {})
    -> xer::result<std::u8string>;

[[nodiscard]]
auto mecab_romaji_wakati(
    std::span<const mecab_token> tokens,
    const mecab_romaji_options& options = {})
    -> xer::result<std::u8string>;

[[nodiscard]]
auto mecab_parse(
    std::u8string_view text,
    const mecab_options& options = {})
    -> xer::result<std::vector<mecab_token>>;
```

---

## `mecab_options`

```cpp
struct mecab_options {
    xer::path program;
};
```

`mecab_options` は、xer が MeCab 実行ファイルをどのように見つけるかを制御します。

### `program`

`program` は MeCab 実行ファイルのパスを明示的に指定します。

`program` が空の場合、xer は `PATH` 環境変数から、プラットフォームで通常使われる実行ファイル名を検索します。

- Windows: `mecab.exe`
- POSIX 風環境: `mecab`

例:

```cpp
xer::ja::mecab_options options {
    .program = xer::path(u8"/usr/bin/mecab"),
};
```

通常の Ubuntu や MSYS2 UCRT64 のインストールでは、呼び出し側は普通これを空のままにします。

---

## `mecab_features`

```cpp
struct mecab_features {
    std::u8string 品詞;
    std::u8string 品詞細分類1;
    std::u8string 品詞細分類2;
    std::u8string 品詞細分類3;
    std::u8string 活用型;
    std::u8string 活用形;
    std::u8string 原形;
    std::u8string 読み;
    std::u8string 発音;
    std::vector<std::u8string> 項目;
};
```

`mecab_features` は、MeCab の生 feature 文字列を分割した形で保持します。

名前付きメンバーは、通常の MeCab/IPADIC 形式の feature 順序に従います。

| Member | Source field | Meaning |
|---|---:|---|
| `品詞` | 0 | 品詞 |
| `品詞細分類1` | 1 | 品詞細分類1 |
| `品詞細分類2` | 2 | 品詞細分類2 |
| `品詞細分類3` | 3 | 品詞細分類3 |
| `活用型` | 4 | 活用型 |
| `活用形` | 5 | 活用形 |
| `原形` | 6 | 原形 |
| `読み` | 7 | 読み |
| `発音` | 8 | 発音 |

`項目` は、名前付きメンバーを持たない辞書固有フィールドを含め、すべてのカンマ区切りフィールドを順序どおりに保持します。フィールドが存在しない場合、対応する名前付きメンバーは空文字列になります。

IPADIC 形式の順序と異なる feature レイアウトを持つ辞書との実用的な互換性のため、直接の IPADIC 形式フィールドが存在しない場合、`*` である場合、または仮名らしくない場合、xer は分割済みの `項目` フィールドを走査して仮名のみの値を探し、`読み` と `発音` を補うことがあります。これは仮名分かち書きやローマ字分かち書きなどの高レベルヘルパー向けの便宜機能であり、完全な辞書正規化層ではありません。

メンバー名は MeCab feature 用語に直接対応するため、意図的に日本語識別子を使っています。xer は識別子を ASCII に制限しません。これらのメンバーを直接参照する場合、利用者はその識別子を扱えるソースコード環境を使う必要があります。

`mecab_features` は文字列を所有します。`mecab_token::feature` への `std::u8string_view` は保存しないため、`mecab_token` は `std::vector` 要素として安全にコピー可能です。

---

## `mecab_token`

```cpp
struct mecab_token {
    std::u8string surface;
    std::u8string feature;
    mecab_features features;
};
```

`mecab_token` は、MeCab が返す 1 個のトークンを表します。

### `surface`

`surface` はトークンの表層テキストです。

### `feature`

`feature` は、MeCab の `%H` フォーマッタが出力する生の MeCab feature 文字列です。

正確な内容は、インストールされている MeCab 辞書に依存します。xer は、デバッグ用途や辞書固有データを必要とする利用者のために、生テキストとしてこれを保持します。

### `features`

`features` は、`feature` から導出された解析済み feature データです。

生 feature 文字列を再解析せずに品詞を調べる、読みを取得する、活用形を確認する、といった一般的な日本語処理向けに用意されています。

---

## `mecab_phrase_kind`

```cpp
enum class mecab_phrase_kind {
    bunsetsu,
    symbol,
};
```

`mecab_phrase_kind` は、`mecab_split_phrases` が返す範囲の種類を識別します。

| Value | Meaning |
|---|---|
| `bunsetsu` | MeCab トークンから導出された文節風の句 |
| `symbol` | 記号トークン、または連続する記号トークンの範囲 |

`symbol` は意図的に `bunsetsu` から分離されています。これにより、仮名の空白付け、ローマ字化、点字向け変換など、句読点やその他の記号を個別に扱う必要がある後続処理が容易になります。

---

## `mecab_phrase`

```cpp
struct mecab_phrase {
    mecab_phrase_kind kind = mecab_phrase_kind::bunsetsu;
    std::size_t index = 0;
    std::size_t count = 0;
};
```

`mecab_phrase` は、元の `mecab_token` 列の部分範囲を表します。

この範囲はトークン文字列を所有せず、コピーもしません。`index` は元のトークン列における最初のトークンのインデックスであり、`count` は範囲内のトークン数です。

この表現は意図的に単純です。呼び出し側は元のトークンオブジェクトを再利用し、feature を調べ、次の処理段階で必要な形で表層形や読みを結合できます。

---

## `mecab_split_phrases`

```cpp
[[nodiscard]]
auto mecab_split_phrases(
    std::span<const mecab_token> tokens)
    -> std::vector<mecab_phrase>;
```

### 目的

`mecab_split_phrases` は、MeCab トークン列を実用的な文節風の句範囲と記号範囲に分割します。

MeCab 自体は文節分割を提供しません。そのため、xer は `mecab_token::features` の分割済み feature フィールドから近似的な句境界を導出します。

### 基本規則

現在の規則セットは、言語学的に完全なものではなく実用重視です。

- `features.品詞` が `記号` であるトークンは `mecab_phrase_kind::symbol` として出力される
- 連続する記号は 1 つの `symbol` 範囲にまとめられる
- `助詞`、`助動詞`、接尾辞風トークン、非自立トークンは直前の `bunsetsu` に付く
- `接頭詞` は、次のトークンを同じ `bunsetsu` に保つことで後続トークンに付く
- 連続する `名詞` トークンは、実用的な複合語規則として同じ `bunsetsu` に残る
- `活用形` が `連用` で始まる `動詞` または `形容詞` は、後続の自立語と同じ範囲に残る
- その他の自立語は通常、新しい `bunsetsu` を開始する

### 例

次のテキストに対応するトークン列がある場合:

```text
私は明日、学校へ行きます。
```

`mecab_split_phrases` は、次と同等の範囲を生成することを意図しています。

```text
bunsetsu: 私 は
bunsetsu: 明日
symbol: 、
bunsetsu: 学校 へ
bunsetsu: 行き ます
symbol: 。
```

実際のトークン表層形と feature 値は、インストールされている MeCab 辞書に依存します。

### 空入力

空のトークンスパンは、空の句ベクターを返します。

### エラーモデル

`mecab_split_phrases` は MeCab を起動せず、外部資源も確保しません。`xer::result` ではなく通常の `std::vector<mecab_phrase>` を返します。

この関数は、`tokens` が `mecab_parse` によって生成されたか、互換性のある feature データを含むことを前提とします。feature データが欠けていたり辞書固有であったりする場合、結果は自然さに欠ける可能性がありますが、それでも文書化された規則に従います。

---

## `mecab_kana_kind`

```cpp
enum class mecab_kana_kind {
    mixed,
    hiragana,
    katakana,
};
```

`mecab_kana_kind` は、MeCab 由来の読みをどの仮名で書くかを制御します。

| Value | Meaning |
|---|---|
| `mixed` | 既定ではひらがなを使い、カタカナ風の入力トークンはカタカナとして保持する |
| `hiragana` | 読みをひらがなへ変換する |
| `katakana` | 読みをカタカナへ変換する |

`mixed` が既定値です。通常の日本語読みを読みやすく保ちながら、`コンピューター` のような一般的なカタカナ語を保持できるためです。

---

## `mecab_kana_options`

```cpp
struct mecab_kana_options {
    mecab_kana_kind kind = mecab_kana_kind::mixed;
    bool particle_reading = true;
};
```

`mecab_kana_options` は、読みベースの仮名変換を制御します。

### `kind`

`kind` は仮名出力スタイルを選択します。

既定値は `mecab_kana_kind::mixed` です。

### `particle_reading`

`particle_reading` が `true` の場合、xer は一般的な助詞について発音寄りの読みを使います。

| Surface | Condition | Hiragana output | Katakana output |
|---|---|---|---|
| `は` | `features.品詞 == u8"助詞"` | `わ` | `ワ` |
| `へ` | `features.品詞 == u8"助詞"` | `え` | `エ` |
| `を` | always | `お` | `オ` |

仮名分かち書き、ローマ字化、点字向け処理では通常、発音寄りの助詞読みが必要になるため、既定値は `true` です。

`particle_reading` が `false` の場合、この関数は MeCab の読みフィールドを使い、読みが利用できない場合はトークンの表層形を使います。

---

## `mecab_to_kana`

```cpp
[[nodiscard]]
auto mecab_to_kana(
    std::span<const mecab_token> tokens,
    const mecab_kana_options& options = {})
    -> std::u8string;
```

### 目的

`mecab_to_kana` は、MeCab トークン列を仮名テキストへ変換します。

各トークンは独立して変換されます。`mecab_features::読み` が利用可能で、かつ `*` でない場合はそれを使います。それ以外の場合は `mecab_token::surface` にフォールバックします。

この関数はトークン間に空白を挿入しません。呼び出し側が目的のトークン範囲や句範囲をすでに持っている場合に便利です。

### 例

次のテキストに対応するトークンの場合:

```text
私はコンピューターを使います。
```

既定の `mixed` モードでは、次に近い仮名テキストを生成することを意図しています。

```text
わたしわコンピューターおつかいます。
```

実際の読みは、インストールされている MeCab 辞書に依存します。

---

## `mecab_kana_wakati`

```cpp
[[nodiscard]]
auto mecab_kana_wakati(
    std::span<const mecab_token> tokens,
    const mecab_kana_options& options = {})
    -> std::u8string;
```

### 目的

`mecab_kana_wakati` は、MeCab トークン列を仮名分かち書きテキストへ変換します。

この関数はまず `mecab_split_phrases` を呼び出し、返された句範囲の間に ASCII 空白を 1 つ挿入します。記号は独立した範囲として保持されるため、この低レベルヘルパーでは句読点やその他の記号も空白で分離されます。

たとえば、次のテキストに対応するトークン列がある場合:

```text
私はコンピューターを使います。
```

次に近い出力を生成することを意図しています。

```text
わたしわ コンピューターお つかいます 。
```

`mecab_kana_kind::hiragana` では、カタカナ風の入力語もひらがなへ変換されます。

```text
わたしわ こんぴゅーたーお つかいます 。
```

`mecab_kana_kind::katakana` では、出力はカタカナ寄りになります。

```text
ワタシワ コンピューターオ ツカイマス 。
```

### エラーモデル

`mecab_kana_wakati` は MeCab を起動せず、`xer::result` も返しません。入力トークン列がすでに `mecab_parse` または互換性のある同等の情報源によって生成されていることを前提とします。

句読点を前の句へ付ける表示向けの空白処理は、後でこの上に重ねられます。現在のヘルパーは、後続のローマ字化や点字向け処理でその分離が必要になることが多いため、意図的に記号を独立させています。

---

## `mecab_braille_wakati`

```cpp
[[nodiscard]]
auto mecab_braille_wakati(
    std::span<const mecab_token> tokens,
    const mecab_kana_options& options = {})
    -> xer::result<std::u8string>;
```

### 目的

`mecab_braille_wakati` は、MeCab トークン列を日本語点字分かち書きテキストへ変換します。

この関数は `mecab_split_phrases` を使い、文節風の範囲と記号範囲を分けて処理します。

通常の文節風の範囲では、普通は同じ仮名オプションで `mecab_to_kana` を呼び出し、その結果の仮名テキストを `xer::ja::kana_text_to_braille` で変換します。

トークン表層形が ASCII 英数字・句読点断片である場合、この関数は MeCab の読みを使わず、元の表層テキストを `xer::braille::alnum_punct_text_to_braille` で変換します。これにより、`ABC123` や `UTF-8` のような断片が、点字出力でも可視の ASCII 形を保てます。

記号範囲では、各記号を `<xer/braille.h>` が使う日本語句読点変換層で直接変換します。これにより、`。`、`、`、`」`、`）` などの句読点の前に不要な空白を挿入することを避けます。

空白は次のように制御されます。

- 通常の文節風範囲は ASCII 空白で区切られる
- `「`、`『`、`（`、`(` などの開き記号は、必要な前置空白のあと、後続の句に付く
- 閉じ記号、文末記号、読点、リーダー類は、後続空白を要求する
- 助詞または助動詞風トークンが閉じ引用符や閉じ括弧の後に続く場合、余分な空白を強制せず同じ点字句に付く
- ASCII 記号のみのトークン範囲は、`+`、`=`、`&&` などのトークン化された断片を ASCII 点字変換経路で扱えるように、点字記号範囲として扱われる
- 記号自体は表層テキストではなく点字句読点として出力される

たとえば、次のテキストに対応するトークン列がある場合:

```text
私は猫です。
```

次の仮名表現に相当する点字分かち書きに近い出力を生成することを意図しています。

```text
わたしわ ねこです。
```

このとき、日本語句点の前に余分な空白は入りません。

引用発話のあとに助詞が続くようなテキストでは、点字向け空白層は閉じ引用符の直後に不要な切れ目を作らないことを意図しています。たとえば、表層パターン `」と` の周辺が該当します。

正確な読みと句境界は、インストールされている MeCab 辞書に依存します。

### エラーモデル

`mecab_braille_wakati` は、点字変換層が失敗する可能性があるため `xer::result<std::u8string>` を返します。

`xer::ja::kana_text_to_braille`、`xer::braille::alnum_punct_text_to_braille`、日本語句読点変換層からのエラーは伝播されます。たとえば、トークン列が日本語点字句読点として対応していない記号を含む場合、または ASCII 断片が通常の英語点字句読点変換で対応していない句読点を含む場合、この関数は `error_t::invalid_argument` を返します。

`mecab_braille_wakati` は MeCab を起動しません。入力トークン列がすでに `mecab_parse` または互換性のある同等の情報源によって生成されていることを前提とします。

---

## `mecab_ip_braille_wakati`

```cpp
[[nodiscard]]
auto mecab_ip_braille_wakati(
    std::span<const mecab_token> tokens,
    const mecab_kana_options& options = {})
    -> xer::result<std::u8string>;
```

### 目的

`mecab_ip_braille_wakati` は、`mecab_braille_wakati` の情報処理点字版です。

日本語トークンは、`mecab_braille_wakati` と同じ MeCab 読み、仮名変換、句読点、空白規則で変換されます。ASCII 英数字・句読点断片は、元の表層テキストから `xer::braille::ip_alnum_punct_text_to_braille` で変換されます。

この変種は、`C++23`、`UTF-8`、`x>=10` など、通常の英語点字句読点より情報処理点字句読点の方が適切な、プログラミング言語風 ASCII 断片を含む混在日本語テキストを意図しています。

### エラーモデル

`mecab_ip_braille_wakati` は `xer::result<std::u8string>` を返します。

仮名変換、日本語句読点変換、情報処理 ASCII 変換からのエラーは伝播されます。この関数は MeCab を起動せず、入力トークン列がすでに `mecab_parse` または互換性のある同等の情報源によって生成されていることを前提とします。

---

## `mecab_romaji_options`

```cpp
struct mecab_romaji_options {
    mecab_kana_options kana;
    ctrans_id romaji = ctrans_id::romaji;
};
```

`mecab_romaji_options` は、ローマ字分かち書き変換を制御します。

### `kana`

`kana` は、ローマ字化の前に行う仮名変換を制御します。

既定では `mecab_kana_options::particle_reading` が有効なままなので、一般的な助詞はローマ字化される前に発音で変換されます。

たとえば、助詞は次のようにローマ字化されることを意図しています。

| Surface | Kana after particle correction | Romaji |
|---|---|---|
| `は` | `わ` | `wa` |
| `へ` | `え` | `e` |
| `を` | `お` | `o` |

### `romaji`

`romaji` は `strtoctrans` のローマ字化モードを選択します。

対応する値は次のとおりです。

| Value | Meaning |
|---|---|
| `ctrans_id::romaji` | マクロンを使う長音表記 |
| `ctrans_id::romaji_alt` | 仮名つづりベースの代替表記 |

その他の `ctrans_id` 値は `error_t::invalid_argument` で拒否されます。

---

## `mecab_romaji_wakati`

```cpp
[[nodiscard]]
auto mecab_romaji_wakati(
    std::span<const mecab_token> tokens,
    const mecab_romaji_options& options = {})
    -> xer::result<std::u8string>;
```

### 目的

`mecab_romaji_wakati` は、MeCab トークン列をローマ字分かち書きテキストへ変換します。

この関数はまず `mecab_split_phrases` を呼び出します。各 `bunsetsu` 範囲は仮名へ変換され、その後 `strtoctrans` でローマ字化されます。各 `symbol` 範囲は表層テキストとして保持され、`strtoctrans` には渡されません。句範囲の間には ASCII 空白が 1 つ挿入されます。

たとえば、次のテキストに対応するトークン列がある場合:

```text
私は猫です。
```

次に近い出力を生成することを意図しています。

```text
watashiwa nekodesu 。
```

`ctrans_id::romaji_alt` では、長音に仮名つづりベースの代替形式を使います。

正確な結果は、読みとトークン境界が辞書に依存するため、インストールされている MeCab 辞書に依存します。

### エラーモデル

`mecab_to_kana` や `mecab_kana_wakati` と異なり、`mecab_romaji_wakati` は `xer::result<std::u8string>` を返します。

`options.romaji` が `ctrans_id::romaji` または `ctrans_id::romaji_alt` でない場合、`error_t::invalid_argument` を報告します。

`strtoctrans` が仮名列をローマ字化できない場合、`strtoctrans` からのエラーが伝播されます。

`mecab_romaji_wakati` は MeCab を起動しません。入力トークン列がすでに `mecab_parse` または互換性のある同等の情報源によって生成されていることを前提とします。

---

## `mecab_parse`

```cpp
[[nodiscard]]
auto mecab_parse(
    std::u8string_view text,
    const mecab_options& options = {})
    -> xer::result<std::vector<mecab_token>>;
```

### 目的

`mecab_parse` は MeCab を起動し、生の形態素解析結果を返します。

### 内部で使用する出力形式

xer は MeCab に対して、1 トークン 1 行で次の形式を出力するよう明示的に要求します。

```text
surface<TAB>feature
```

`EOS` マーカーは内部で消費され、トークンとしては返されません。

概念的には、xer は MeCab に対して、通常トークンと未知語トークンが同等の生出力構造になるよう設定します。

```text
%m<TAB>%H
```

これにより、パーサーは人間向けの MeCab 既定出力形式に依存しません。

### 空入力

空の入力文字列は受け付けられます。

```cpp
const auto tokens = xer::ja::mecab_parse(u8"");
```

成功時、結果は空のトークンベクターです。

### 基本例

```cpp
const auto tokens = xer::ja::mecab_parse(u8"私は猫です。");
if (!tokens) {
    return;
}

for (const auto& token : *tokens) {
    // token.surface
    // token.feature
    // token.features.品詞
    // token.features.読み
}
```

正確なトークン化、feature 文字列、分割 feature フィールドは、インストールされている辞書に依存します。

---

## `mecab_braille_translate`

```cpp
[[nodiscard]]
auto mecab_braille_translate(
    std::u8string_view text,
    const mecab_options& parse_options = {},
    const mecab_kana_options& kana_options = {})
    -> xer::result<std::u8string>;
```

### 目的

`mecab_braille_translate` は、UTF-8 入力テキストを MeCab で解析し、その結果のトークン列を `mecab_braille_wakati` で変換します。

これは、中間のトークン列を調べる必要がない呼び出し側向けの便宜ラッパーです。MeCab は日本語テキストの読みを決定します。ASCII 英数字・句読点断片は、通常点字の ASCII 断片変換層により元の表層テキストから変換されます。

### エラーモデル

`mecab_braille_translate` は両方の段階からのエラーを伝播します。

- `mecab_parse`
- `mecab_braille_wakati`

つまり、この関数は MeCab 実行エラー、UTF-8 エラー、点字変換エラーを報告できます。

---

## `mecab_ip_braille_translate`

```cpp
[[nodiscard]]
auto mecab_ip_braille_translate(
    std::u8string_view text,
    const mecab_options& parse_options = {},
    const mecab_kana_options& kana_options = {})
    -> xer::result<std::u8string>;
```

### 目的

`mecab_ip_braille_translate` は、UTF-8 入力テキストを MeCab で解析し、その結果のトークン列を `mecab_ip_braille_wakati` で変換します。

日本語テキストは `mecab_braille_translate` と同じ方法で処理されます。ASCII 英数字・句読点断片は情報処理点字の ASCII 断片変換層で変換されます。

この関数は、コード風または技術的な ASCII 断片を含む可能性がある日本語テキスト向けの便利な入口です。

### エラーモデル

`mecab_ip_braille_translate` は両方の段階からのエラーを伝播します。

- `mecab_parse`
- `mecab_ip_braille_wakati`

---

## 実行ファイルの解決

`mecab_options::program` が空の場合、xer は次の手順を行います。

1. `PATH` を読む
2. 各パス要素を検索する
3. プラットフォームで通常使われる MeCab 実行ファイル名を確認する
4. 最初に一致したファイルを実行する

実行ファイルが見つからない場合、`mecab_parse` は次を返します。

```cpp
error_t::not_found
```

---

## エラーモデル

`mecab_parse` は `xer::result<std::vector<mecab_token>>` を返します。

現在の実装では次のエラーを使います。

| Condition | Error |
|---|---|
| 入力テキストが妥当な UTF-8 ではない | `error_t::encoding_error` |
| MeCab 出力が妥当な UTF-8 ではない | `error_t::encoding_error` |
| 実行ファイルの自動検索で MeCab が見つからない | `error_t::not_found` |
| MeCab を実行できない、MeCab が失敗終了する、または想定外の出力を出す | `error_t::process_error` |

一部の低レベルなプロセスまたはストリームの失敗は、最終的な MeCab レベルの検証段階より前に発生した場合、それぞれの xer エラーコードを保持することがあります。

---

## 辞書依存性

`mecab_token::feature` と `mecab_token::features` は辞書に依存します。

異なる MeCab 辞書は、次の点で異なる可能性があります。

- テキストを異なる単位に分割する
- 異なる feature 列レイアウトを報告する
- 異なる読みや原形フィールドを生成する

xer は `%H` が出力するカンマ区切り構造に従って feature 文字列を分割し、通常の MeCab/IPADIC 形式のフィールド位置を使って名前付きメンバーを埋めます。これは実用的な便宜機能であり、あらゆる辞書に対する完全な正規化層ではありません。

高レベルな xer 日本語テキスト処理機能は、必要に応じて独自の対応解釈方針を後で定義する可能性があります。

---

## 現在の範囲

現段階の `<xer/mecab.h>` は、低レベルな MeCab 形態素解析基盤を提供します。

実装済み:

- UTF-8 MeCab 子プロセス起動
- 実行ファイルパスの解決
- トークン収集
- 表層テキストの保持
- 生 feature テキストの保持
- 分割済み feature フィールドの保持
- 一般的な MeCab/IPADIC 形式の名前付き feature メンバー
- `mecab_split_phrases` による実用的な文節風の句分割と記号分割
- `mecab_to_kana` による MeCab 由来の読みベースの仮名変換
- `mecab_kana_wakati` による仮名分かち書き
- `mecab_braille_wakati` と `mecab_ip_braille_wakati` による点字分かち書き
- `mecab_braille_translate` と `mecab_ip_braille_translate` による直接点字変換
- `mecab_romaji_wakati` によるローマ字分かち書き

このヘッダーで未実装:

- ルビ向け構造
- 単語数または文節数カウントヘルパー

これらは生の層の上に構築する予定であり、`policy_mecab.md` のポリシーレベルで説明されています。

---

## 他ヘッダーとの関係

`<xer/mecab.h>` は次と関係します。

- `<xer/process.h>`
- `<xer/path.h>`
- `<xer/error.h>`
- `policy_mecab.md`
- `policy_project_outline.md`
- `<xer/toctrans.h>`
- `<xer/braille.h>`

---

# `<xer/furigana.h>`

## 目的

`<xer/furigana.h>` は、軽量なふりがな整形ヘルパーを提供します。

現在の機能は形態素解析から意図的に独立しています。次の値を受け取り、整形済みの UTF-8 文字列を生成します。

- 親文字
- 読み
- 出力スタイル

そのため単独でも有用であり、将来の MeCab ベースの自動ふりがな生成における再利用可能な部品にもなります。

---

## 主な役割

`<xer/furigana.h>` の主な役割は、すでに分かっている読みを表示向けのテキスト表現へ変換することです。

現在の実装は次の形式をサポートします。

- HTML ruby マークアップ
- 丸括弧付きふりがなテキスト

このヘッダーは、読みを自力で決定しようとは**しません**。読みの抽出は、MeCab または呼び出し側のロジックに基づく上位の日本語処理の役割です。

---

## 主なエンティティ

`<xer/furigana.h>` は次を提供します。

```cpp
enum class furigana_style : std::uint8_t {
    html,
    paren,
};

inline constexpr furigana_style ruby_html;
inline constexpr furigana_style ruby_paren;

[[nodiscard]]
auto to_furigana(
    std::u8string_view text,
    std::u8string_view reading,
    furigana_style style)
    -> std::u8string;
```

---

## `furigana_style`

```cpp
enum class furigana_style : std::uint8_t {
    html,
    paren,
};
```

`furigana_style` は、`to_furigana` が使用する出力表現を選択します。

通常、呼び出し側は公開セレクタ定数を直接使います。

```cpp
xer::ja::ruby_html
xer::ja::ruby_paren
```

---

## `ruby_html`

`ruby_html` は HTML ruby マークアップを生成します。

```cpp
const auto result =
    xer::ja::to_furigana(u8"学校", u8"がっこう", xer::ja::ruby_html);
```

結果:

```html
<ruby>学校<rt>がっこう</rt></ruby>
```

生成される形は次のとおりです。

```html
<ruby>BASE_TEXT<rt>READING</rt></ruby>
```

### HTML エスケープ

`ruby_html` では、親文字と読みの両方が HTML エスケープされます。

現在の段階では、内部エスケープヘルパーは次の文字を扱います。

| 文字 | 出力 |
|---|---|
| `&` | `&amp;` |
| `<` | `&lt;` |
| `>` | `&gt;` |
| `"` | `&quot;` |
| `'` | `&#39;` |

例:

```cpp
const auto result =
    xer::ja::to_furigana(u8"A&B", u8"えー&びー", xer::ja::ruby_html);
```

結果:

```html
<ruby>A&amp;B<rt>えー&amp;びー</rt></ruby>
```

---

## `ruby_paren`

`ruby_paren` は単純な丸括弧付きふりがな表現を生成します。

```cpp
const auto result =
    xer::ja::to_furigana(u8"学校", u8"がっこう", xer::ja::ruby_paren);
```

結果:

```text
学校(がっこう)
```

生成される形は次のとおりです。

```text
BASE_TEXT(READING)
```

このスタイルでは、エスケープや特別な変換は行われません。

---

## `to_furigana`

```cpp
[[nodiscard]]
auto to_furigana(
    std::u8string_view text,
    std::u8string_view reading,
    furigana_style style)
    -> std::u8string;
```

### 目的

`to_furigana` は、親文字と読みを、選択されたふりがなスタイルに従って整形します。

### 例

```cpp
xer::ja::to_furigana(u8"漢字", u8"かんじ", xer::ja::ruby_html);
// <ruby>漢字<rt>かんじ</rt></ruby>
```

```cpp
xer::ja::to_furigana(u8"漢字", u8"かんじ", xer::ja::ruby_paren);
// 漢字(かんじ)
```

### 戻り値の型

`to_furigana` は `std::u8string` を返します。

これは整形ヘルパーであり、読み解析や外部プロセス実行を行いません。そのため `xer::result` は返しません。

---

## 空入力

空の親文字と空の読みは受け付けられます。

```cpp
xer::ja::to_furigana(u8"", u8"", xer::ja::ruby_html);
// <ruby><rt></rt></ruby>
```

```cpp
xer::ja::to_furigana(u8"", u8"", xer::ja::ruby_paren);
// ()
```

---

## 内部 HTML エスケープヘルパー

`ruby_html` で使われる HTML エスケープは、内部ヘルパーヘッダーで実装されています。

```text
xer/bits/escape_html.h
```

このヘルパーは意図的に小さく、内部用に保たれています。将来、`htmlspecialchars` 風 API などの上位 HTML 関連機能から再利用することを想定していますが、`furigana.h` が重い公開 HTML ヘッダーへ依存することは避けています。

---

## 将来の MeCab ベースふりがなとの関係

`to_furigana` は MeCab から意図的に独立しています。

将来の MeCab ベースヘルパーは、次のように構成できます。

1. テキストを解析する
2. 読み候補を決定する
3. `to_furigana` を呼び出して HTML ruby または丸括弧表現を生成する

この分離により、整形ロジックを再利用しやすくし、自動読み判定ロジックを低レベルの整形器から切り離しています。

---

## 他ヘッダーとの関係

`<xer/furigana.h>` は次と関連します。

- `<xer/mecab.h>`: 将来の上位読み取得元
- `<xer/string.h>`: 広い意味での日本語テキスト処理領域
- `policy_mecab.md`

---

# `<xer/ja.h>`

## 目的

`<xer/ja.h>` は、日本語固有の xer 機能をまとめる便利なアンブレラヘッダーです。

次をインクルードします。

```cpp
#include <xer/furigana.h>
#include <xer/kansuji.h>
#include <xer/mecab.h>
#include <xer/bits/ja_is.h>
#include <xer/bits/ja_kanji.h>
#include <xer/bits/ja_to.h>
```

これらのヘッダーが提供する API は `xer::ja` 名前空間に置かれます。

---

## 名前空間ポリシー

日本語固有 API は `xer::ja` に集約します。

これにより、メインの `xer` 名前空間は言語非依存のユーティリティに集中させつつ、v1.0.0 より前から深い日本語対応を提供できます。

例:

```cpp
xer::ja::to_kansuji(2026, xer::ja::k十);
xer::ja::from_kansuji(u8"二千二十六");
xer::ja::to_furigana(u8"学校", u8"がっこう", xer::ja::ruby_html);
xer::ja::is_hiragana(U'あ');
xer::ja::is_katakana(U'ア');
xer::ja::is_kanji(U'漢');
xer::ja::is_name_kanji(U'凜');
xer::ja::jis_kanji_level_of(U'亜');
xer::ja::to_hiragana(u8"カタカナ");
xer::ja::to_katakana(u8"ひらがな");
xer::ja::normalize_kana(u8"ｶﾞｷﾞｸﾞｹﾞｺﾞ");
xer::ja::mecab_parse(u8"私は猫です。");
```

---

## 文字分類

`xer::ja::is_hiragana` は、コードポイントが実用的なひらがなかどうかを調べます。Unicode の Hiragana ブロックと長音記号 `U+30FC` を受け付けます。

`xer::ja::is_katakana` は、コードポイントが実用的なカタカナかどうかを調べます。全角カタカナ、半角カタカナ、Katakana Phonetic Extensions、Kana Supplement/Extended ブロック、長音記号を含みます。

`xer::ja::is_kana` は、コードポイントが実用的なひらがなまたは実用的なカタカナかどうかを調べます。

`xer::ja::is_kanji` は、コードポイントが一般的な CJK 統合漢字または CJK 互換漢字の範囲に属するかどうかを調べます。Unicode では多くの漢字/hanzi/hanja コードポイントが共有されるため、この関数は言語固有の用法を識別しようとはしません。

`xer::ja::is_japanese_punctuation` は、日本語テキストで一般的に使われる句読点かどうかを調べます。

`xer::ja::is_japanese` は、コードポイントが仮名、漢字、または日本語句読点かどうかを調べます。

`xer::ja::contains_hiragana`、`xer::ja::contains_katakana`、`xer::ja::contains_kana`、`xer::ja::contains_kanji`、`xer::ja::contains_japanese` は、UTF-8 文字列が少なくとも 1 つの該当コードポイントを含むかどうかを調べます。空入力は `false` を返します。不正な UTF-8 入力は `encoding_error` を返します。

`xer::ja::is_all_hiragana`、`xer::ja::is_all_katakana`、`xer::ja::is_all_kana` は、UTF-8 文字列内のすべてのコードポイントが実用的なひらがな、カタカナ、または仮名テキストに属するかどうかを調べます。空入力は `false` を返します。不正な UTF-8 入力は `encoding_error` を返します。

`contains_*` と `is_all_*` 述語は、対応するコードポイント述語と同じ実用的な文字集合を使います。`is_all_*` の仮名述語は、空白、句読点、漢字、ラテン文字を受け付けません。

```cpp
xer::ja::is_hiragana(U'あ'); // true
xer::ja::is_katakana(U'ア'); // true
xer::ja::is_kana(U'ｱ'); // true
xer::ja::is_kanji(U'漢'); // true
xer::ja::is_japanese_punctuation(U'。'); // true
xer::ja::is_japanese(U'日'); // true

xer::ja::contains_hiragana(u8"abcあ"); // true
xer::ja::contains_katakana(u8"abcア"); // true
xer::ja::contains_kana(u8"abcｱ"); // true
xer::ja::contains_kanji(u8"abc漢"); // true
xer::ja::contains_japanese(u8"hello日本語"); // true
xer::ja::contains_japanese(u8""); // false

xer::ja::is_all_hiragana(u8"こんにちはー"); // true
xer::ja::is_all_katakana(u8"コンニチハー"); // true
xer::ja::is_all_kana(u8"こんにちはコンニチハー"); // true
xer::ja::is_all_kana(u8""); // false
xer::ja::is_all_kana(u8"こんにちは。"); // false
```

これらの関数は内部実装ヘッダー `<xer/bits/ja_is.h>` で定義され、`<xer/ja.h>` から利用できます。

---

## 漢字分類テーブル

`xer::ja::name_kanji_class_of` は、コードポイントの実用的な日本語人名用漢字クラスを返します。

```cpp
namespace xer::ja {

enum class name_kanji_class {
    none = 0,
    name = 1,
    jouyou = 2,
    kyouiku = 3,
};

}
```

このクラスは包含関係に従って順序付けられています。教育漢字は常用漢字の部分集合として扱われ、常用漢字は日本語の名に使える漢字の部分集合として扱われます。

```cpp
xer::ja::is_name_kanji(U'凜'); // true
xer::ja::is_jouyou_kanji(U'鬱'); // true
xer::ja::is_kyouiku_kanji(U'日'); // true
```

`xer::ja::jis_kanji_level_of` は JIS 漢字水準を返します。

```cpp
namespace xer::ja {

enum class jis_kanji_level {
    none = 0,
    level_1 = 1,
    level_2 = 2,
    level_3 = 3,
    level_4 = 4,
};

}
```

便利な述語も用意されています。

```cpp
xer::ja::is_jis_level_1_kanji(U'亜'); // true
xer::ja::is_jis_level_2_kanji(U'弌'); // true
xer::ja::is_jis_level_3_kanji(U'俱'); // true
xer::ja::is_jis_level_4_kanji(U'㐆'); // true
```

実装は基本 CJK 統合漢字範囲についてコンパクトな 1 バイトフラグを保持し、その範囲外の対応コードポイントには疎なフォールバックテーブルを使います。下位 2 ビットは `name_kanji_class` を保持し、ビット 2..4 は `jis_kanji_level` を保持します。

これらの関数は内部実装ヘッダー `<xer/bits/ja_kanji.h>` で定義され、`<xer/ja.h>` から利用できます。

---

## 仮名変換

`xer::ja::to_hiragana` は、UTF-8 テキスト内の全角カタカナをひらがなへ変換します。

```cpp
const auto text = xer::ja::to_hiragana(u8"カタカナとヴ");
// text == u8"かたかなとゔ"
```

`xer::ja::to_katakana` は、UTF-8 テキスト内のひらがなを全角カタカナへ変換します。

```cpp
const auto text = xer::ja::to_katakana(u8"ひらがなとゔ");
// text == u8"ヒラガナトヴ"
```

`xer::ja::normalize_kana` は、ひらがな/カタカナの文字種を保ちながら、実用的な仮名表記を正規化します。半角カタカナを全角カタカナへ変換し、合成済み仮名が存在する場合は分離された濁点または半濁点を合成します。

```cpp
const auto text = xer::ja::normalize_kana(u8"ｶﾞｷﾞｸﾞｹﾞｺﾞ ﾊﾟﾋﾟﾌﾟﾍﾟﾎﾟ");
// text == u8"ガギグゲゴ パピプペポ"

const auto text2 = xer::ja::normalize_kana(u8"がハ゜");
// text2 == u8"がパ"
```

これらの関数は無関係なコードポイントを変更せず、不正な UTF-8 入力を `encoding_error` として報告できるように `xer::result<std::u8string>` を返します。

これらの関数は内部実装ヘッダー `<xer/bits/ja_to.h>` で定義され、`<xer/ja.h>` から利用できます。

---

## 注意

`<xer/ja.h>` は独自の実装層を定義しません。これはインクルード専用の便利ヘッダーです。

`<xer/kansuji.h>`、`<xer/furigana.h>`、`<xer/mecab.h>` のような個別ヘッダーは、呼び出し側がより小さな構成要素だけをインクルードしたい場合にも利用できます。仮名変換は現在 `<xer/ja.h>` からのみ利用できます。

---

# `<xer/unicode.h>`

## 目的

`<xer/unicode.h>` は、実用的な Unicode ユーティリティを提供します。

現在の対象範囲は次のとおりです。

- `std::u8string_view` のコードポイント走査
- `std::u16string_view` のコードポイント走査
- `std::wstring_view` のコードポイント走査
- `std::u8string_view` の拡張書記素クラスタ走査
- `std::u16string_view` の拡張書記素クラスタ走査
- `std::wstring_view` の拡張書記素クラスタ走査
- `std::u8string_view` の書記素クラスタ単位の文字列操作
- `std::u16string_view` の書記素クラスタ単位の文字列操作
- `std::wstring_view` の書記素クラスタ単位の文字列操作
- コードポイントおよび単一書記素クラスタに対する実用的な絵文字判定
- UTF-8 テキストの NFC 正規化
- UTF-8 テキストの NFC 状態確認

コードポイント走査、書記素クラスタ走査、書記素クラスタ単位の文字列操作、および絵文字判定そのものは ICU を使用しません。NFC 正規化は ICU C API を使用します。

現時点では、`<xer/unicode.h>` は ICU ベースの正規化実装を含むため、この公開ヘッダーをインクルードするには ICU 開発用ヘッダーが利用可能である必要があります。

---

## 外部依存

`<xer/unicode.h>` は NFC 正規化ユーティリティを公開するため、ICU C API ヘッダーを必要とします。

```cpp
#include <unicode/utypes.h>
#include <unicode/ustring.h>
#include <unicode/unorm2.h>
```

正規化実装では、`u_strFromUTF8`、`u_strToUTF8`、`unorm2_getNFCInstance`、`unorm2_normalize`、`unorm2_isNormalized` などの ICU C API 関数を使用します。

xer はアプリケーションのビルドシステム設定を管理しません。ユーザーは、自分の環境に適したインクルードパスとリンクオプションを指定する必要があります。

たとえば、多くの Unix 系環境では、必要なリンクオプションを `pkg-config` で取得できます。

```bash
pkg-config --cflags --libs icu-uc
```

直接コンパイルするコマンドは、たとえば次のようになります。

```bash
g++ -std=c++23 -I. example.cpp -licuuc -licudata
```

MSYS2 環境では、ICU データライブラリ名が `icudata` ではなく `icudt` の場合があるため、典型的なコマンドは次のようになります。

```bash
g++ -std=c++23 -I. example.cpp -licuuc -licudt
```

xer のテストランナーは、既知の環境を個別に処理します。

---

## 主なエンティティ

`<xer/unicode.h>` は、次のコードポイント走査型を提供します。

```cpp
struct code_point {
    std::size_t offset;
    std::size_t size;
    char32_t value;
};
```

`offset` および `size` フィールドは、元のコード単位で表されます。

- `std::u8string_view` では UTF-8 コード単位、実質的にはバイト
- `std::u16string_view` では UTF-16 コード単位
- `std::wstring_view` ではワイドコード単位

デコードされた `value` は、`char32_t` で表される Unicode スカラー値です。

このヘッダーは、次のデコード関数を提供します。

```cpp
auto next_code_point(std::u8string_view text, std::size_t offset = 0)
    -> xer::result<xer::code_point>;

auto prev_code_point(std::u8string_view text, std::size_t offset)
    -> xer::result<xer::code_point>;

auto next_code_point(std::u16string_view text, std::size_t offset = 0)
    -> xer::result<xer::code_point>;

auto prev_code_point(std::u16string_view text, std::size_t offset)
    -> xer::result<xer::code_point>;

auto next_code_point(std::wstring_view text, std::size_t offset = 0)
    -> xer::result<xer::code_point>;

auto prev_code_point(std::wstring_view text, std::size_t offset)
    -> xer::result<xer::code_point>;
```

また、範囲ヘルパーも提供します。

```cpp
auto code_points(std::u8string_view text)
    -> xer::code_point_range<char8_t>;

auto code_points(std::u16string_view text)
    -> xer::code_point_range<char16_t>;

auto code_points(std::wstring_view text)
    -> xer::code_point_range<wchar_t>;
```

範囲要素を参照解除した値は次の型です。

```cpp
xer::result<xer::code_point>
```

これにより、走査中の不正な入力が明示的になります。

`<xer/unicode.h>` は、次の書記素クラスタ走査型を提供します。

```cpp
struct grapheme_cluster {
    std::size_t offset;
    std::size_t size;
};
```

`offset` および `size` フィールドは、元のコード単位で表されます。書記素クラスタは複数のコードポイントを含むことがあるため、`grapheme_cluster` は意図的に `char32_t` 値を持ちません。

このヘッダーは、次の書記素クラスタ関数を提供します。

```cpp
auto next_grapheme_cluster(std::u8string_view text, std::size_t offset = 0)
    -> xer::result<xer::grapheme_cluster>;

auto prev_grapheme_cluster(std::u8string_view text, std::size_t offset)
    -> xer::result<xer::grapheme_cluster>;

auto next_grapheme_cluster(std::u16string_view text, std::size_t offset = 0)
    -> xer::result<xer::grapheme_cluster>;

auto prev_grapheme_cluster(std::u16string_view text, std::size_t offset)
    -> xer::result<xer::grapheme_cluster>;

auto next_grapheme_cluster(std::wstring_view text, std::size_t offset = 0)
    -> xer::result<xer::grapheme_cluster>;

auto prev_grapheme_cluster(std::wstring_view text, std::size_t offset)
    -> xer::result<xer::grapheme_cluster>;
```

また、範囲ヘルパーも提供します。

```cpp
auto grapheme_clusters(std::u8string_view text)
    -> xer::grapheme_cluster_range<char8_t>;

auto grapheme_clusters(std::u16string_view text)
    -> xer::grapheme_cluster_range<char16_t>;

auto grapheme_clusters(std::wstring_view text)
    -> xer::grapheme_cluster_range<wchar_t>;
```

範囲要素を参照解除した値は次の型です。

```cpp
xer::result<xer::grapheme_cluster>
```

これにより、走査中の不正な入力が明示的になります。

このヘッダーは、書記素クラスタ単位の文字列操作も提供します。

```cpp
auto grapheme_length(std::u8string_view text)
    -> xer::result<std::size_t>;

auto grapheme_substr(
    std::u8string_view text,
    std::size_t offset,
    std::size_t count = std::u8string_view::npos)
    -> xer::result<std::u8string_view>;

auto grapheme_left(std::u8string_view text, std::size_t count)
    -> xer::result<std::u8string_view>;

auto grapheme_right(std::u8string_view text, std::size_t count)
    -> xer::result<std::u8string_view>;
```

`std::u16string_view` および `std::wstring_view` のオーバーロードも提供されます。返される部分文字列値は元のテキストへのビューです。

このヘッダーは、実用的な絵文字判定も提供します。

```cpp
auto is_emoji(char32_t value) noexcept -> bool;

auto is_emoji(std::u8string_view text)
    -> xer::result<bool>;

auto is_emoji(std::u16string_view text)
    -> xer::result<bool>;

auto is_emoji(std::wstring_view text)
    -> xer::result<bool>;
```

文字列ビュー版は、入力全体が 1 個の実用的な絵文字書記素クラスタである場合にだけ `true` を返します。空入力は `false` を返します。不正な UTF-8、UTF-16、またはワイドテキストはエラーとして報告されます。

このヘッダーは NFC ユーティリティも提供します。

```cpp
auto normalize_nfc(std::u8string_view text)
    -> xer::result<std::u8string>;

auto is_normalized_nfc(std::u8string_view text)
    -> xer::result<bool>;
```

---

## `code_point`

```cpp
struct code_point {
    std::size_t offset;
    std::size_t size;
    char32_t value;
};
```

### 目的

`code_point` は、1 個のデコード済み Unicode スカラー値と、それに対応する元の範囲を表します。

元の文字列は所有しません。デコードされたコードポイントがどこで見つかり、元のコード単位をいくつ占めていたかだけを記録します。

### オフセットとサイズ

`offset` はコードポイントが始まるインデックスです。

`size` は、そのコードポイントが占める元のコード単位数です。

たとえば、UTF-8 テキストでは次のようになります。

```cpp
std::u8string_view text = u8"Aあ😀";
```

コードポイントは次のとおりです。

```text
offset=0 size=1 value=U+0041
offset=1 size=3 value=U+3042
offset=4 size=4 value=U+1F600
```

UTF-16 テキストでは、U+1F600 のような補助平面文字はサロゲートペアで表されるため、2 個のコード単位を占めます。

`std::wstring_view` では、単位はプラットフォームの `wchar_t` 表現に依存します。`wchar_t` が 16 ビットのプラットフォームでは、補助平面文字は 2 個のワイドコード単位を占めます。`wchar_t` が 32 ビットのプラットフォームでは、1 個のワイドコード単位を占めます。

---

## `next_code_point`

```cpp
auto next_code_point(std::u8string_view text, std::size_t offset = 0)
    -> xer::result<xer::code_point>;

auto next_code_point(std::u16string_view text, std::size_t offset = 0)
    -> xer::result<xer::code_point>;

auto next_code_point(std::wstring_view text, std::size_t offset = 0)
    -> xer::result<xer::code_point>;
```

### 目的

`next_code_point` は、`offset` から始まるコードポイントをデコードします。

### 入力モデル

`offset` 引数は、元の文字列ビューのコード単位で表されます。

入力は、対応するエンコーディングとして well-formed である必要があります。

- `std::u8string_view` は well-formed な UTF-8 を含んでいる必要があります
- `std::u16string_view` は well-formed な UTF-16 を含んでいる必要があります
- `std::wstring_view` はプラットフォームの `wchar_t` 幅に応じた well-formed なワイドテキストを含んでいる必要があります

### 戻り値モデル

戻り値の型は次のとおりです。

```cpp
xer::result<xer::code_point>
```

この関数は、次の場合に失敗する可能性があります。

- `offset` が文字列ビューの範囲外である
- `offset` が文字列ビューの末尾と等しい
- 入力に不正な UTF-8 が含まれている
- 入力に不正な UTF-16 サロゲートペアが含まれている
- デコードされた値が Unicode スカラー値ではない

範囲外のオフセットは `xer::error_t::out_of_range` として報告されます。
不正な符号化テキストは `xer::error_t::encoding_error` として報告されます。

### 例

```cpp
constexpr std::u8string_view text = u8"Aあ";

const auto first = xer::next_code_point(text, 0);
const auto second = xer::next_code_point(text, first->offset + first->size);
```

---

## `prev_code_point`

```cpp
auto prev_code_point(std::u8string_view text, std::size_t offset)
    -> xer::result<xer::code_point>;

auto prev_code_point(std::u16string_view text, std::size_t offset)
    -> xer::result<xer::code_point>;

auto prev_code_point(std::wstring_view text, std::size_t offset)
    -> xer::result<xer::code_point>;
```

### 目的

`prev_code_point` は、`offset` の直前にあるコードポイントをデコードします。

`offset` 引数は one-past 位置です。`text.size()` を渡すと、最後のコードポイントをデコードします。

### 戻り値モデル

戻り値の型は次のとおりです。

```cpp
xer::result<xer::code_point>
```

この関数は、次の場合に失敗する可能性があります。

- `offset` が文字列ビューの範囲外である
- `offset` が `0` である
- `offset` の直前のバイト列またはコード単位列が、ちょうど 1 個の well-formed なコードポイントを形成していない

### 例

```cpp
constexpr std::u8string_view text = u8"Aあ";

const auto last = xer::prev_code_point(text, text.size());
```

---

## `code_points`

```cpp
auto code_points(std::u8string_view text)
    -> xer::code_point_range<char8_t>;

auto code_points(std::u16string_view text)
    -> xer::code_point_range<char16_t>;

auto code_points(std::wstring_view text)
    -> xer::code_point_range<wchar_t>;
```

### 目的

`code_points` は、元のテキストを Unicode コードポイント単位で走査する軽量入力範囲を作成します。

### 要素型

イテレーターを参照解除した値は次の型です。

```cpp
const xer::result<xer::code_point>&
```

つまり、各要素は使用前に確認する必要があります。

不正なシーケンスはエラー要素として現れます。エラー後にイテレーターを進めると終端へ移動します。

### 例

```cpp
constexpr std::u8string_view text = u8"Aあ😀";

for (const auto& item : xer::code_points(text)) {
    if (!item.has_value()) {
        // malformed input
        break;
    }

    const xer::code_point cp = *item;
    // cp.offset, cp.size, cp.value
}
```

---

## 書記素クラスタとの関係

コードポイント走査は、書記素クラスタ走査とは同じではありません。

たとえば、次の組み合わせは 2 個のコードポイントですが、ユーザーから見た 1 文字になることがあります。

```text
U+304B HIRAGANA LETTER KA
U+3099 COMBINING KATAKANA-HIRAGANA VOICED SOUND MARK
```

コードポイント API は、これを意図的に 2 個のコードポイントとして公開します。

書記素クラスタ API はこの層の上に構築され、単一の `char32_t` ではなく元の文字列範囲を返します。これは、1 個の書記素クラスタが複数のコードポイントを含むことがあるためです。

---


## `grapheme_cluster`

```cpp
struct grapheme_cluster {
    std::size_t offset;
    std::size_t size;
};
```

### 目的

`grapheme_cluster` は、1 個の拡張書記素クラスタと、それに対応する元の範囲を表します。

元の文字列は所有しません。クラスタがどこで始まり、元のコード単位をいくつ占めているかだけを記録します。

`code_point` と異なり、`char32_t` 値は持ちません。ユーザーから見える 1 文字は、基底文字と結合記号、異体字セレクター付きの絵文字、または絵文字 ZWJ シーケンスのように、複数の Unicode コードポイントから構成されることがあります。

---

## `next_grapheme_cluster`

```cpp
auto next_grapheme_cluster(std::u8string_view text, std::size_t offset = 0)
    -> xer::result<xer::grapheme_cluster>;

auto next_grapheme_cluster(std::u16string_view text, std::size_t offset = 0)
    -> xer::result<xer::grapheme_cluster>;

auto next_grapheme_cluster(std::wstring_view text, std::size_t offset = 0)
    -> xer::result<xer::grapheme_cluster>;
```

### 目的

`next_grapheme_cluster` は、`offset` から始まる拡張書記素クラスタをデコードします。

実装はコードポイント走査層を基盤とし、結合記号、異体字セレクター、絵文字修飾子、絵文字 ZWJ シーケンス、地域指示記号ペア、CRLF、およびハングル音節シーケンスなど、実用的な拡張書記素クラスタ列をグループ化します。

### 戻り値モデル

戻り値の型は次のとおりです。

```cpp
xer::result<xer::grapheme_cluster>
```

この関数は、次の場合に失敗する可能性があります。

- `offset` が文字列ビューの範囲外である
- `offset` が文字列ビューの末尾と等しい
- 入力に不正な UTF-8 が含まれている
- 入力に不正な UTF-16 サロゲートペアが含まれている
- デコードされた値が Unicode スカラー値ではない

範囲外のオフセットは `xer::error_t::out_of_range` として報告されます。不正な符号化テキストは `xer::error_t::encoding_error` として報告されます。

### 例

```cpp
constexpr std::u8string_view text = u8"A\u0301B";

const auto first = xer::next_grapheme_cluster(text, 0);
// first->offset == 0
// first->size == 3
```

---

## `prev_grapheme_cluster`

```cpp
auto prev_grapheme_cluster(std::u8string_view text, std::size_t offset)
    -> xer::result<xer::grapheme_cluster>;

auto prev_grapheme_cluster(std::u16string_view text, std::size_t offset)
    -> xer::result<xer::grapheme_cluster>;

auto prev_grapheme_cluster(std::wstring_view text, std::size_t offset)
    -> xer::result<xer::grapheme_cluster>;
```

### 目的

`prev_grapheme_cluster` は、`offset` の直前にある拡張書記素クラスタをデコードします。

`offset` 引数は、クラスタ境界の one-past 位置です。`text.size()` を渡すと最後の書記素クラスタをデコードします。クラスタ内部のオフセットを渡した場合は `xer::error_t::encoding_error` として報告されます。

---

## `grapheme_clusters`

```cpp
auto grapheme_clusters(std::u8string_view text)
    -> xer::grapheme_cluster_range<char8_t>;

auto grapheme_clusters(std::u16string_view text)
    -> xer::grapheme_cluster_range<char16_t>;

auto grapheme_clusters(std::wstring_view text)
    -> xer::grapheme_cluster_range<wchar_t>;
```

### 目的

`grapheme_clusters` は、元のテキストを拡張書記素クラスタ単位で走査する軽量入力範囲を作成します。

参照解除された要素型は次のとおりです。

```cpp
xer::result<xer::grapheme_cluster>
```

### 例

```cpp
constexpr std::u8string_view text = u8"A\u0301B👩‍💻";

for (const auto& item : xer::grapheme_clusters(text)) {
    if (!item.has_value()) {
        // malformed input
        break;
    }

    // item->offset and item->size describe the cluster span.
}
```


---

## `grapheme_length`

```cpp
auto grapheme_length(std::u8string_view text)
    -> xer::result<std::size_t>;

auto grapheme_length(std::u16string_view text)
    -> xer::result<std::size_t>;

auto grapheme_length(std::wstring_view text)
    -> xer::result<std::size_t>;
```

### 目的

`grapheme_length` は、元の文字列ビューに含まれる拡張書記素クラスタ数を数えます。

これは、元のコード単位数を数える `text.size()` とは異なります。UTF-8 の日本語テキストでは、`text.size()` はバイト数です。`grapheme_length` は、xer の実用的な書記素クラスタ規則にもとづく、ユーザーから見える文字数を得るためのものです。

### 戻り値モデル

戻り値の型は次のとおりです。

```cpp
xer::result<std::size_t>
```

不正な UTF-8 または UTF-16 入力は `xer::error_t::encoding_error` として報告されます。

### 例

```cpp
constexpr std::u8string_view text = u8"A\u0301B👩‍💻";
const auto length = xer::grapheme_length(text);
// *length == 3
```

---

## `grapheme_substr`

```cpp
auto grapheme_substr(
    std::u8string_view text,
    std::size_t offset,
    std::size_t count = std::u8string_view::npos)
    -> xer::result<std::u8string_view>;

auto grapheme_substr(
    std::u16string_view text,
    std::size_t offset,
    std::size_t count = std::u16string_view::npos)
    -> xer::result<std::u16string_view>;

auto grapheme_substr(
    std::wstring_view text,
    std::size_t offset,
    std::size_t count = std::wstring_view::npos)
    -> xer::result<std::wstring_view>;
```

### 目的

`grapheme_substr` は、書記素クラスタのインデックスで選択した部分文字列ビューを返します。

`offset` および `count` パラメーターは、書記素クラスタ数です。バイト数でも UTF-16 コード単位数でもありません。返される値は元の文字列へのビューです。

`offset` が書記素クラスタ長と等しい場合、結果は入力末尾の空ビューになります。`offset` が書記素クラスタ長より大きい場合、この関数は `xer::error_t::out_of_range` を返します。

### 例

```cpp
constexpr std::u8string_view text = u8"A\u0301B👩‍💻C";
const auto part = xer::grapheme_substr(text, 1, 2);
// *part == u8"B👩‍💻"
```

---

## `grapheme_left`

```cpp
auto grapheme_left(std::u8string_view text, std::size_t count)
    -> xer::result<std::u8string_view>;

auto grapheme_left(std::u16string_view text, std::size_t count)
    -> xer::result<std::u16string_view>;

auto grapheme_left(std::wstring_view text, std::size_t count)
    -> xer::result<std::wstring_view>;
```

### 目的

`grapheme_left` は、先頭から `count` 個の書記素クラスタを、元の文字列へのビューとして返します。`count` が書記素クラスタ長より大きい場合は、入力全体のビューを返します。

---

## `grapheme_right`

```cpp
auto grapheme_right(std::u8string_view text, std::size_t count)
    -> xer::result<std::u8string_view>;

auto grapheme_right(std::u16string_view text, std::size_t count)
    -> xer::result<std::u16string_view>;

auto grapheme_right(std::wstring_view text, std::size_t count)
    -> xer::result<std::wstring_view>;
```

### 目的

`grapheme_right` は、末尾から `count` 個の書記素クラスタを、元の文字列へのビューとして返します。`count` が書記素クラスタ長より大きい場合は、入力全体のビューを返します。

`grapheme_right` は最初に書記素クラスタ長を求めるため、元のビュー内のどこかに不正な入力があれば報告されます。

### 例

```cpp
constexpr std::u8string_view text = u8"A\u0301B👩‍💻C";
const auto right = xer::grapheme_right(text, 2);
// *right == u8"👩‍💻C"
```

---

## `is_emoji`

```cpp
auto is_emoji(char32_t value) noexcept -> bool;

auto is_emoji(std::u8string_view text)
    -> xer::result<bool>;

auto is_emoji(std::u16string_view text)
    -> xer::result<bool>;

auto is_emoji(std::wstring_view text)
    -> xer::result<bool>;
```

### 目的

`is_emoji` は、英語・日本語のユーザー向けテキストで実用的に使える絵文字判定を提供します。

`char32_t` オーバーロードは、Unicode スカラー値が xer で絵文字基底として扱われるかどうかを調べます。これはコードポイント分類をすばやく行うためのものです。

文字列ビュー版は、入力全体が 1 個の絵文字書記素クラスタであるかどうかを調べます。旗、キーキャップ絵文字、異体字セレクター付き絵文字、肌色修飾子、ZWJ 絵文字シーケンスなどには、このオーバーロードを使用します。

### 入力モデル

文字列ビュー版は UTF-8、UTF-16、またはワイドテキストを受け取ります。`true` を返すには、入力がちょうど 1 個の書記素クラスタを含んでいる必要があります。空入力は `false` を返します。

### 戻り値モデル

`char32_t` オーバーロードは `bool` を返し、エラーを報告しません。

文字列ビュー版は次を返します。

```cpp
xer::result<bool>
```

入力に不正な UTF-8、UTF-16、またはワイドテキストが含まれている場合、失敗する可能性があります。

### 対象範囲

これは小さく実用的な判定器であり、すべての Unicode 絵文字プロパティを生成テーブルで完全実装したものではありません。xer の既存の書記素クラスタ処理を再利用し、英語・日本語テキストでよく使われる絵文字を対象にします。対象には、絵文字ピクトグラフ、旗、キーキャップ絵文字、異体字セレクター形式、絵文字修飾子、ZWJ シーケンスが含まれます。

### 例

```cpp
const auto face = xer::is_emoji(std::u8string_view{u8"😀"});
const auto worker = xer::is_emoji(std::u8string_view{u8"👩‍💻"});
const auto letter = xer::is_emoji(std::u8string_view{u8"A"});
```

この例では、`face` と `worker` は `true` を含み、`letter` は `false` を含むことが期待されます。

---

## `normalize_nfc`

```cpp
auto normalize_nfc(std::u8string_view text)
    -> xer::result<std::u8string>;
```

### 目的

`normalize_nfc` は、有効な UTF-8 テキストを Unicode 正規化形式 C に変換します。

NFC は、まず正準分解を適用し、その後に正準合成を行う正規化形式です。正準等価なテキストについて、合成済み文字が利用可能な場合に一貫したバイト表現へそろえるために有用です。

たとえば、次のシーケンスは、

```text
U+304B HIRAGANA LETTER KA
U+3099 COMBINING KATAKANA-HIRAGANA VOICED SOUND MARK
```

次のように正規化できます。

```text
U+304C HIRAGANA LETTER GA
```

同様に、次のシーケンスは、

```text
U+0041 LATIN CAPITAL LETTER A
U+030A COMBINING RING ABOVE
```

次のように正規化できます。

```text
U+00C5 LATIN CAPITAL LETTER A WITH RING ABOVE
```

### 入力モデル

入力は次の型で与えます。

```cpp
std::u8string_view
```

入力は有効な UTF-8 である必要があります。無効な UTF-8 はエラーとして報告されます。

### 出力モデル

成功時、この関数は次を返します。

```cpp
std::u8string
```

返される文字列は、NFC に正規化された有効な UTF-8 です。

### 戻り値モデル

戻り値の型は次のとおりです。

```cpp
xer::result<std::u8string>
```

この関数は、次の場合に失敗する可能性があります。

- 入力が有効な UTF-8 ではない
- 入力または出力サイズが ICU C API の長さパラメーターで受け付けられる範囲を超えている
- ICU が内部エラーを報告した

典型的な無効 UTF-8 入力は `xer::error_t::encoding_error` として報告されます。

### 例

```cpp
const auto result = xer::normalize_nfc(u8"か\u3099");
if (result.has_value()) {
    // *result is u8"が"
}
```

---

## `is_normalized_nfc`

```cpp
auto is_normalized_nfc(std::u8string_view text)
    -> xer::result<bool>;
```

### 目的

`is_normalized_nfc` は、有効な UTF-8 テキストがすでに Unicode NFC に正規化されているかどうかを確認します。

これは、すでに目的の正規化形式になっているテキストを書き換えずに済ませたい場合に有用です。

### 入力モデル

入力は次の型で与えます。

```cpp
std::u8string_view
```

入力は有効な UTF-8 である必要があります。無効な UTF-8 はエラーとして報告されます。

### 出力モデル

成功時、この関数は次を返します。

```cpp
bool
```

値は、入力がすでに NFC であれば `true`、そうでなければ `false` です。

### 戻り値モデル

戻り値の型は次のとおりです。

```cpp
xer::result<bool>
```

この関数は、次の場合に失敗する可能性があります。

- 入力が有効な UTF-8 ではない
- 入力サイズが ICU C API の長さパラメーターで受け付けられる範囲を超えている
- ICU が失敗を報告した

典型的な無効 UTF-8 入力は `xer::error_t::encoding_error` として報告されます。

### 例

```cpp
const auto before = xer::is_normalized_nfc(u8"か\u3099");
const auto after = xer::is_normalized_nfc(u8"が");
```

この例では、`before` は `false` を含み、`after` は `true` を含むことが期待されます。

---

## 設計メモ

`<xer/unicode.h>` は、`<xer/string.h>` のような通常の文字列処理ヘッダーから意図的に独立しています。

コードポイント走査層は小さく、テーブルを使いません。UTF-8 および UTF-16 構造を検証し、不正な入力を `xer::result` で報告し、元の範囲をコード単位で記録します。

書記素クラスタ走査層は、コードポイント層の上に構築されています。これは、実用的なユーザー可視文字の走査を目的としつつ、テキストをコピーせずに元の範囲を返します。書記素クラスタ単位の文字列操作は、長さおよび部分文字列の便利なヘルパーを提供しつつ、元のテキストへのビューを返します。絵文字判定も同じコードポイント層と書記素クラスタ層の上に構築されるため、複数コードポイントからなる絵文字シーケンスを 1 個のユーザー可視単位として確認できます。

この層の既定の言語対象は英語および日本語テキストです。結合記号、異体字セレクター、絵文字修飾子、絵文字 ZWJ シーケンス、地域指示記号ペア、CRLF、ハングル音節シーケンスなど、実用的な英語・日本語のユーザー向けテキストに必要な一般的シーケンスを扱います。あらゆる文字体系に対応する完全な Unicode テキスト境界エンジンではなく、言語固有の tailoring も提供しません。より広い文字体系への対応が必要なユーザーは、xer の公開ソースコードを拡張するか、専用の Unicode 境界サービスを使用できます。

Unicode 正規化は Unicode 正規化データを必要とするため、単純な UTF-8 文字列ユーティリティより重い処理です。xer はヘッダーオンリーライブラリ内に大きな生成済み Unicode テーブルを埋め込むのではなく、この責務を ICU に委譲します。

初期の正規化 API は NFC のみを公開します。NFC は、ファイル名、検索キー、辞書、テキスト整形など、多くの実用的な用途で最も使いやすい正規化形式だからです。その他の正規化形式は、基本的な API 形状を変更せずに後から追加できます。

---

---

# `<xer/bytes.h>`

## 目的

`<xer/bytes.h>` は、バイト列変換ヘルパーを提供します。

このヘッダーの目的は、通常のバイト風ストレージやテキストストレージを、Base64 エンコード、バイナリストリーム、ソケット、プロセスパイプなどのバイト指向 API に渡しやすくすることです。

---

## 主なエンティティ

少なくとも、`<xer/bytes.h>` は次の関数を提供します。

```cpp
auto to_bytes_view(std::string_view value) noexcept
    -> std::span<const std::byte>;

auto to_bytes_view(std::u8string_view value) noexcept
    -> std::span<const std::byte>;

auto to_bytes_view(std::span<const char> value) noexcept
    -> std::span<const std::byte>;

auto to_bytes_view(std::span<const char8_t> value) noexcept
    -> std::span<const std::byte>;

auto to_bytes_view(std::span<const unsigned char> value) noexcept
    -> std::span<const std::byte>;

auto to_bytes_view(std::span<const std::byte> value) noexcept
    -> std::span<const std::byte>;

auto to_bytes(std::string_view value) -> std::vector<std::byte>;
auto to_bytes(std::u8string_view value) -> std::vector<std::byte>;
auto to_bytes(std::span<const char> value) -> std::vector<std::byte>;
auto to_bytes(std::span<const char8_t> value) -> std::vector<std::byte>;
auto to_bytes(std::span<const unsigned char> value) -> std::vector<std::byte>;
auto to_bytes(std::span<const std::byte> value) -> std::vector<std::byte>;
```

---

## `to_bytes_view`

`to_bytes_view` は、渡されたストレージを指す非所有の `std::span<const std::byte>` ビューを作成します。

割り当てやコピーは行いません。返される span は、入力と同じメモリを参照します。

返される値はビューなので、呼び出し側は入力ストレージが返された span より長く生存することを保証する必要があります。

---

## `to_bytes`

`to_bytes` は、渡されたストレージの所有する `std::vector<std::byte>` コピーを作成します。

これは、元の文字列や span の寿命に依存しない独立したバイト列が必要な場合に有用です。

---

## 設計上の注意

これらのヘルパーは文字エンコーディング変換を行いません。

たとえば、`std::u8string_view` を `to_bytes_view` に渡すと、UTF-8 コード単位をバイトとして公開します。テキストを検証、正規化、または別のエンコーディングとして再解釈することはありません。

区別は単純です。

- `to_bytes_view` は非所有であり、コピーしない
- `to_bytes` は所有し、コピーする

---

## 他のヘッダーとの関係

`<xer/bytes.h>` は、特に次のヘッダーと一緒に使うと有用です。

- `<xer/base64.h>`
- `<xer/stdio.h>`
- `<xer/socket.h>`
- `<xer/process.h>`

おおまかな境界は次のとおりです。

- `<xer/bytes.h>` は、バイト風ストレージやテキストストレージを明示的なバイト列に変換する
- `<xer/base64.h>` は、バイト列と Base64 テキストを相互変換する
- `<xer/stdio.h>` は、バイナリストリームとテキストストリームを扱う

---

## 例

```cpp
#include <string_view>

#include <xer/base64.h>
#include <xer/bytes.h>

auto main() -> int
{
    constexpr std::u8string_view text = u8"hello";

    const auto bytes = xer::to_bytes_view(text);
    const auto encoded = xer::base64_encode(bytes);
    if (!encoded.has_value()) {
        return 1;
    }

    return 0;
}
```

---

# `<xer/binary.h>`

## 目的

`<xer/binary.h>` は、小規模なバイナリデータ用ユーティリティ関数を提供します。

現在の対象範囲は意図的に狭くしています。このヘッダーは、固定幅符号なし整数の分割と合成、ビット順反転、xer の128ビット符号なし整数型に対するバイト順反転、単純なチェックサム計算、CRC計算、バイナリから16進文字列への変換、16進文字列からバイナリへの変換、バイト列とファイルに対する実用的なハッシュ計算を扱います。

これらの関数は、入力値を固定幅のバイナリ値として扱います。CPUのネイティブエンディアン設定には依存しません。

---

## 主なエンティティ

最低限、`<xer/binary.h>` は次の機能を提供します。

```cpp
enum class byte_order {
    little_endian,
    big_endian,
};

using std::byteswap;

auto high_u8(std::uint16_t value) noexcept -> std::uint8_t;
auto low_u8(std::uint16_t value) noexcept -> std::uint8_t;
auto make_u16(std::uint8_t high, std::uint8_t low) noexcept -> std::uint16_t;

auto high_u16(std::uint32_t value) noexcept -> std::uint16_t;
auto low_u16(std::uint32_t value) noexcept -> std::uint16_t;
auto make_u32(std::uint16_t high, std::uint16_t low) noexcept -> std::uint32_t;

auto high_u32(std::uint64_t value) noexcept -> std::uint32_t;
auto low_u32(std::uint64_t value) noexcept -> std::uint32_t;
auto make_u64(std::uint32_t high, std::uint32_t low) noexcept -> std::uint64_t;
```

`xer::uint128_t` が利用できる場合、このヘッダーはさらに次を提供します。

```cpp
auto high_u64(xer::uint128_t value) noexcept -> std::uint64_t;
auto low_u64(xer::uint128_t value) noexcept -> std::uint64_t;
auto make_u128(std::uint64_t high, std::uint64_t low) noexcept -> xer::uint128_t;

auto byteswap(xer::uint128_t value) noexcept -> xer::uint128_t;
```

このヘッダーはビット順反転も提供します。

```cpp
auto reverse_bits(std::uint8_t value) noexcept -> std::uint8_t;
auto reverse_bits(std::uint16_t value) noexcept -> std::uint16_t;
auto reverse_bits(std::uint32_t value) noexcept -> std::uint32_t;
auto reverse_bits(std::uint64_t value) noexcept -> std::uint64_t;
```

`xer::uint128_t` が利用できる場合は次も提供されます。

```cpp
auto reverse_bits(xer::uint128_t value) noexcept -> xer::uint128_t;
```

単純なチェックサムについて、このヘッダーは8ビット、16ビット、32ビット形式の加算チェックサム、XORチェックサム、および便利な別名を提供します。また、CRC16とCRC32の計算ヘルパーも提供します。

バイナリ/テキスト変換について、このヘッダーは `bin2hex` と `hex2bin` を提供します。実用的なハッシュ計算について、このヘッダーは `md5`、`sha1`、`sha256` を提供します。

---

## 整数の分割と合成

`high_uN` 関数と `low_uN` 関数は、符号なし整数値の上位半分または下位半分を取り出します。

```cpp
auto high = xer::high_u8(std::uint16_t{0x1234}); // 0x12
auto low = xer::low_u8(std::uint16_t{0x1234});  // 0x34
```

`make_uN` 関数は、上位部分と下位部分から、より大きな符号なし整数値を合成します。

```cpp
auto value = xer::make_u16(0x12, 0x34); // 0x1234
```

命名規則は、取り出される部分のサイズを示します。

- `high_u8` と `low_u8` は16ビット値から8ビット部分を取り出します
- `high_u16` と `low_u16` は32ビット値から16ビット部分を取り出します
- `high_u32` と `low_u32` は64ビット値から32ビット部分を取り出します
- `high_u64` と `low_u64` は128ビット値から64ビット部分を取り出します

合成関数は、合成される結果のサイズを示します。

- `make_u16` は2つの8ビット値から16ビット値を合成します
- `make_u32` は2つの16ビット値から32ビット値を合成します
- `make_u64` は2つの32ビット値から64ビット値を合成します
- `make_u128` は2つの64ビット値から128ビット値を合成します

これらの関数は、メモリ上のオブジェクト表現ではなく、整数値に対して動作します。その振る舞いはネイティブエンディアンに依存しません。

---

## `byteswap`

`<xer/binary.h>` は `std::byteswap` を `xer` 名前空間に取り込みます。

```cpp
using std::byteswap;
```

これにより、C++23標準のバイトスワップを、`std::byteswap` が対応している標準の符号なし整数型に対して `xer::byteswap` として利用できます。

`xer::uint128_t` が利用できる場合、xer は128ビットの多重定義も提供します。

```cpp
auto byteswap(xer::uint128_t value) noexcept -> xer::uint128_t;
```

たとえば、上位半分と下位半分が次の128ビット値があるとします。

```text
0x0011223344556677_8899aabbccddeeff
```

これはバイトスワップによって次になります。

```text
0xffeeddccbbaa9988_7766554433221100
```

128ビット多重定義は、C++23の `std::byteswap` が xer の拡張 `uint128_t` 型を対象にしていないため提供されています。

---

## `reverse_bits`

`reverse_bits` は、固定幅符号なし整数値の全ビットの順序を反転します。

```cpp
auto value = xer::reverse_bits(std::uint8_t{0b0001'0010}); // 0b0100'1000
```

これはバイト順反転とは異なります。

```cpp
std::uint16_t value = 0x1234;

xer::byteswap(value);     // 0x3412
xer::reverse_bits(value); // 0x2c48
```

`reverse_bits` は入力を固定幅の値として扱います。たとえば、8ビット多重定義は正確に8ビットを反転し、32ビット多重定義は正確に32ビットを反転します。

---

## `byte_order`

`byte_order` は、チェックサム計算で連続するバイトを16ビットワードまたは32ビットワードへまとめる方法を指定します。

```cpp
enum class byte_order {
    little_endian,
    big_endian,
};
```

この列挙型は、バイト列を16ビットまたは32ビットワードとして解釈する場合にだけ使われます。

8ビットチェックサムでは、各入力バイトがすでに1つのチェックサム単位なので、バイト順の設定は不要です。

---

## 8ビットチェックサム

8ビットチェックサム関数は、個々のバイトに対して単純なチェックサムを計算します。

```cpp
auto checksum8(std::span<const std::byte> bytes) noexcept -> std::uint8_t;
auto checksum_add8(std::span<const std::byte> bytes) noexcept -> std::uint8_t;
auto checksum_xor8(std::span<const std::byte> bytes) noexcept -> std::uint8_t;
```

`checksum8` は通常の加算式8ビットチェックサムです。`checksum_add8` と等価です。

`checksum_add8` はすべてのバイトを256を法として加算します。

`checksum_xor8` はすべてのバイトをXORします。

例:

```cpp
const std::array<std::byte, 4> bytes {
    std::byte{0x01},
    std::byte{0x02},
    std::byte{0x03},
    std::byte{0x04},
};

const auto sum = xer::checksum8(std::span<const std::byte>(bytes));      // 0x0a
const auto x = xer::checksum_xor8(std::span<const std::byte>(bytes));   // 0x04
```

---

## 16ビットチェックサム

16ビットチェックサム関数は、入力バイトを16ビットワードへまとめてから、加算チェックサムまたはXORチェックサムを計算します。

```cpp
auto checksum16(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint16_t;

auto checksum_add16(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint16_t;

auto checksum_xor16(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint16_t;
```

`checksum16` は通常の加算式16ビットチェックサムです。`checksum_add16` と等価です。

バイト順は、各2バイトをどのようにワードへ変換するかを制御します。

バイト `{0x01, 0x02}` の場合:

- `byte_order::big_endian` はワードを `0x0102` として読み取ります
- `byte_order::little_endian` はワードを `0x0201` として読み取ります

入力のバイト数が奇数の場合、最後に不足しているバイトは `0x00` として扱われます。

たとえば、ビッグエンディアンのグループ化では、`{0x01, 0x02, 0x03}` は次のように扱われます。

```text
0x0102, 0x0300
```

リトルエンディアンのグループ化では、同じ入力は次のように扱われます。

```text
0x0201, 0x0003
```

---

## 32ビットチェックサム

32ビットチェックサム関数は、入力バイトを32ビットワードへまとめてから、加算チェックサムまたはXORチェックサムを計算します。

```cpp
auto checksum32(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint32_t;

auto checksum_add32(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint32_t;

auto checksum_xor32(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint32_t;
```

`checksum32` は通常の加算式32ビットチェックサムです。`checksum_add32` と等価です。

バイト `{0x01, 0x02, 0x03, 0x04}` の場合:

- `byte_order::big_endian` はワードを `0x01020304` として読み取ります
- `byte_order::little_endian` はワードを `0x04030201` として読み取ります

入力バイト数が4の倍数ではない場合、最後の不完全なワードはゼロバイトで埋められます。

たとえば、ビッグエンディアンのグループ化では、`{0x01, 0x02, 0x03}` は次のように扱われます。

```text
0x01020300
```

リトルエンディアンのグループ化では、同じ入力は次のように扱われます。

```text
0x00030201
```

---

## チェックサム入力形式

チェックサム関数は4種類の入力形式に対して提供されます。

### `std::span<const std::byte>`

span多重定義は、メモリ上のバイト列に対する主要な多重定義です。

```cpp
auto checksum8(std::span<const std::byte> bytes) noexcept -> std::uint8_t;
auto checksum_add8(std::span<const std::byte> bytes) noexcept -> std::uint8_t;
auto checksum_xor8(std::span<const std::byte> bytes) noexcept -> std::uint8_t;

auto checksum16(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint16_t;

auto checksum_add16(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint16_t;

auto checksum_xor16(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint16_t;

auto checksum32(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint32_t;

auto checksum_add32(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint32_t;

auto checksum_xor32(std::span<const std::byte> bytes, byte_order order) noexcept
    -> std::uint32_t;
```

これらの多重定義はメモリ割り当てを行わず、失敗しません。

### ポインタとサイズ

C風のバイトバッファ向けに、ポインタとサイズを受け取る多重定義が提供されます。

```cpp
auto checksum8(const void* data, std::size_t size) noexcept
    -> xer::result<std::uint8_t>;

auto checksum_add8(const void* data, std::size_t size) noexcept
    -> xer::result<std::uint8_t>;

auto checksum_xor8(const void* data, std::size_t size) noexcept
    -> xer::result<std::uint8_t>;

auto checksum16(const void* data, std::size_t size, byte_order order) noexcept
    -> xer::result<std::uint16_t>;

auto checksum_add16(const void* data, std::size_t size, byte_order order) noexcept
    -> xer::result<std::uint16_t>;

auto checksum_xor16(const void* data, std::size_t size, byte_order order) noexcept
    -> xer::result<std::uint16_t>;

auto checksum32(const void* data, std::size_t size, byte_order order) noexcept
    -> xer::result<std::uint32_t>;

auto checksum_add32(const void* data, std::size_t size, byte_order order) noexcept
    -> xer::result<std::uint32_t>;

auto checksum_xor32(const void* data, std::size_t size, byte_order order) noexcept
    -> xer::result<std::uint32_t>;
```

`data` が `nullptr` で `size` がゼロではない場合、これらの多重定義は `error_t::invalid_argument` で失敗します。

`data == nullptr` かつ `size == 0` は受け入れられ、空のバイト列を表します。

### イテレータ範囲

バイト風の範囲向けに、イテレータ範囲多重定義が提供されます。

```cpp
template<std::input_iterator InputIt>
auto checksum8(InputIt first, InputIt last) -> std::uint8_t;

template<std::input_iterator InputIt>
auto checksum_add8(InputIt first, InputIt last) -> std::uint8_t;

template<std::input_iterator InputIt>
auto checksum_xor8(InputIt first, InputIt last) -> std::uint8_t;

template<std::input_iterator InputIt>
auto checksum16(InputIt first, InputIt last, byte_order order) -> std::uint16_t;

template<std::input_iterator InputIt>
auto checksum_add16(InputIt first, InputIt last, byte_order order) -> std::uint16_t;

template<std::input_iterator InputIt>
auto checksum_xor16(InputIt first, InputIt last, byte_order order) -> std::uint16_t;

template<std::input_iterator InputIt>
auto checksum32(InputIt first, InputIt last, byte_order order) -> std::uint32_t;

template<std::input_iterator InputIt>
auto checksum_add32(InputIt first, InputIt last, byte_order order) -> std::uint32_t;

template<std::input_iterator InputIt>
auto checksum_xor32(InputIt first, InputIt last, byte_order order) -> std::uint32_t;
```

イテレータの値型は `std::byte`、または `std::uint8_t` に変換可能なバイト風の整数値でなければなりません。

これらの多重定義は、`std::vector<std::byte>`、`std::array<std::uint8_t, N>`、および同様のバイト指向ストレージで役立ちます。

### ファイルパス

ファイル多重定義は、ファイル全体のチェックサムを計算します。

```cpp
auto checksum8(const path& filename) -> xer::result<std::uint8_t>;
auto checksum_add8(const path& filename) -> xer::result<std::uint8_t>;
auto checksum_xor8(const path& filename) -> xer::result<std::uint8_t>;

auto checksum16(const path& filename, byte_order order) -> xer::result<std::uint16_t>;
auto checksum_add16(const path& filename, byte_order order) -> xer::result<std::uint16_t>;
auto checksum_xor16(const path& filename, byte_order order) -> xer::result<std::uint16_t>;

auto checksum32(const path& filename, byte_order order) -> xer::result<std::uint32_t>;
auto checksum_add32(const path& filename, byte_order order) -> xer::result<std::uint32_t>;
auto checksum_xor32(const path& filename, byte_order order) -> xer::result<std::uint32_t>;
```

これらの多重定義はファイル全体の内容を読み取ってからチェックサムを計算します。ファイルI/Oの失敗は `xer::result` を通じて報告されます。

---

## 加算チェックサムとXORチェックサムの意味

加算チェックサム関数は、各チェックサム単位を加算し、結果の下位ビットだけを保持します。

- `checksum8` と `checksum_add8` は下位8ビットを保持します
- `checksum16` と `checksum_add16` は下位16ビットを保持します
- `checksum32` と `checksum_add32` は下位32ビットを保持します

`checksum8`、`checksum16`、`checksum32` は、加算チェックサム関数の便利な別名です。加算方式であることを呼び出し箇所で明示したい場合は、`checksum_add8`、`checksum_add16`、`checksum_add32` を使ってください。

XORチェックサム関数は、各チェックサム単位をXORし、対応する幅の結果を返します。

これらは意図的に単純なチェックサムです。小規模なバイナリ形式、単純な診断、単純なプロトコルとの互換性に役立ちます。

これらは暗号学的ハッシュではありません。一般的なCRCアルゴリズムとの互換性が必要な場合は、`crc16` または `crc32` を使ってください。


---

## CRC16とCRC32

`crc16` と `crc32` は、バイト列に対して標準的なCRC値を計算します。

```cpp
auto crc16(std::span<const std::byte> bytes) noexcept -> std::uint16_t;
auto crc32(std::span<const std::byte> bytes) noexcept -> std::uint32_t;
```

`crc16` はCRC-16/ARCのパラメータを使います。

- 多項式: `0xa001`
- 初期値: `0x0000`
- 最終XOR: なし
- `"123456789"` に対するチェック値: `0xbb3d`

`crc32` はCRC-32/ISO-HDLCのパラメータを使います。

- 多項式: `0xedb88320`
- 初期値: `0xffffffff`
- 最終XOR: `0xffffffff`
- `"123456789"` に対するチェック値: `0xcbf43926`

これらの関数はバイトに対して動作します。16ビットおよび32ビットの単純チェックサムとは異なり、CRC計算では `byte_order` を使いません。

---

## CRC入力形式

CRC関数は、チェックサム関数と同じ入力形式ポリシーに従います。

### `std::span<const std::byte>`

span多重定義は、メモリ上のバイト列に対する主要な多重定義です。

```cpp
auto crc16(std::span<const std::byte> bytes) noexcept -> std::uint16_t;
auto crc32(std::span<const std::byte> bytes) noexcept -> std::uint32_t;
```

これらの多重定義はメモリ割り当てを行わず、失敗しません。

### ポインタとサイズ

C風のバイトバッファ向けに、ポインタとサイズを受け取る多重定義が提供されます。

```cpp
auto crc16(const void* data, std::size_t size) noexcept -> xer::result<std::uint16_t>;
auto crc32(const void* data, std::size_t size) noexcept -> xer::result<std::uint32_t>;
```

`data` が `nullptr` で `size` がゼロではない場合、これらの多重定義は `error_t::invalid_argument` で失敗します。

`data == nullptr` かつ `size == 0` は受け入れられ、空のバイト列を表します。

### イテレータ範囲

バイト風の範囲向けに、イテレータ範囲多重定義が提供されます。

```cpp
template<std::input_iterator InputIt>
auto crc16(InputIt first, InputIt last) -> std::uint16_t;

template<std::input_iterator InputIt>
auto crc32(InputIt first, InputIt last) -> std::uint32_t;
```

イテレータの値型は `std::byte`、または `std::uint8_t` に変換可能なバイト風の整数値でなければなりません。

### ファイルパス

ファイル多重定義は、ファイル全体のCRCを計算します。

```cpp
auto crc16(const path& filename) -> xer::result<std::uint16_t>;
auto crc32(const path& filename) -> xer::result<std::uint32_t>;
```

これらの多重定義はファイル全体の内容を読み取ってからCRCを計算します。ファイルI/Oの失敗は `xer::result` を通じて報告されます。

---

## CRCの空入力

空入力は有効です。

空入力に対して:

- `crc16` は `0x0000` を返します
- `crc32` は `0x00000000` を返します

---

## 空入力

空入力は有効です。

空入力に対して、すべてのチェックサム関数はゼロを返します。

---

## 他のヘッダーとの関係

`<xer/binary.h>` は次のヘッダーと併用すると有用です。

- `<xer/bytes.h>`
- `<xer/stdio.h>`
- `<xer/stdint.h>`

おおまかな境界は次のとおりです。

- `<xer/bytes.h>` はテキストまたはバイト風ストレージを明示的なバイトビューまたはバイトベクターへ変換します
- `<xer/binary.h>` は小規模なバイナリ値操作、単純なチェックサム計算、CRC計算、16進変換、ハッシュ計算を行います
- `<xer/stdio.h>` はストリームベースのバイナリI/Oとファイル全体の操作を扱います

---

## 例

```cpp
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include <xer/binary.h>

auto main() -> int
{
    const auto value = xer::make_u16(0x12, 0x34);
    const auto high = xer::high_u8(value);
    const auto low = xer::low_u8(value);

    if (value != 0x1234 || high != 0x12 || low != 0x34) {
        return 1;
    }

    const std::array<std::byte, 4> bytes {
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
    };

    const auto checksum = xer::checksum16(
        std::span<const std::byte>(bytes),
        xer::byte_order::big_endian);

    if (checksum != 0x0406) {
        return 1;
    }

    const auto crc = xer::crc32(std::span<const std::byte>(bytes));
    if (crc != 0xb63cfbcd) {
        return 1;
    }

    return 0;
}
```

---

## バイナリから16進文字列への変換

`bin2hex` はバイナリデータを小文字の16進文字列に変換します。

```cpp
auto bin2hex(std::span<const std::byte> bytes) -> std::u8string;

auto bin2hex(const void* data, std::size_t size) noexcept
    -> xer::result<std::u8string>;

template<std::input_iterator InputIt>
auto bin2hex(InputIt first, InputIt last) -> std::u8string;
```

各入力バイトは2つの小文字16進文字で表されます。空入力は有効で、空文字列を返します。

```cpp
const std::array<std::byte, 3> bytes {
    std::byte{0x12},
    std::byte{0xab},
    std::byte{0x00},
};

auto hex = xer::bin2hex(std::span<const std::byte>(bytes)); // u8"12ab00"
```

ポインタとサイズを受け取る多重定義は、`data == nullptr` かつ `size != 0` のとき `error_t::invalid_argument` で失敗します。`data == nullptr` かつ `size == 0` は空のバイト列として受け入れられます。

イテレータ範囲多重定義は、`std::byte` と、`std::uint8_t` に変換可能なバイト風の整数値を受け入れます。`std::array<std::byte, N>`、`std::vector<std::byte>`、`std::array<std::uint8_t, N>` などのコンテナで役立ちます。

---

## 16進文字列からバイナリへの変換

`hex2bin` は16進文字列をバイナリデータへ変換します。これはPHPの `hex2bin` 関数をモデルにしています。

```cpp
auto hex2bin(std::u8string_view hex) -> xer::result<std::vector<std::byte>>;
```

`hex2bin` は `0` から `9`、`a` から `f`、`A` から `F` の文字を受け入れます。2つの16進文字が1バイトを形成するため、入力長は偶数でなければなりません。

```cpp
auto bytes = xer::hex2bin(u8"12ab00");
```

入力長が奇数の場合、または入力に16進文字以外が含まれる場合、関数は `error_t::invalid_argument` で失敗します。空入力は有効で、空のベクターを返します。

---


## ハッシュ関数

`md5`、`sha1`、`sha256` は、バイト列とファイルのメッセージダイジェストを計算します。

```cpp
auto md5(std::span<const std::byte> bytes) noexcept
    -> std::array<std::byte, 16>;

auto sha1(std::span<const std::byte> bytes) noexcept
    -> std::array<std::byte, 20>;

auto sha256(std::span<const std::byte> bytes) noexcept
    -> std::array<std::byte, 32>;
```

返される配列には、生のダイジェストバイトが含まれます。16進文字列ではありません。一般的な16進表現が必要な場合は `bin2hex` を使ってください。

例:

```cpp
const auto text = std::string_view("abc");
const auto bytes = std::as_bytes(std::span(text));

const auto digest = xer::sha256(bytes);
const auto hex = xer::bin2hex(digest.begin(), digest.end());
// hex == u8"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"
```

ダイジェストサイズは次のとおりです。

- `md5`: 16バイト、128ビット
- `sha1`: 20バイト、160ビット
- `sha256`: 32バイト、256ビット

MD5とSHA-1は、主に既存のファイル形式、ツール、テストデータとの互換性のために提供されています。暗号学的なセキュリティ機構として使ってはいけません。これら3つの関数の中では、より強い現代的なハッシュが必要な場合はSHA-256が推奨されます。

---

## ハッシュ入力形式

ハッシュ関数は4種類の入力形式に対して提供されます。

### `std::span<const std::byte>`

span多重定義は、メモリ上のバイト列に対する主要な多重定義です。

```cpp
auto md5(std::span<const std::byte> bytes) noexcept
    -> std::array<std::byte, 16>;

auto sha1(std::span<const std::byte> bytes) noexcept
    -> std::array<std::byte, 20>;

auto sha256(std::span<const std::byte> bytes) noexcept
    -> std::array<std::byte, 32>;
```

これらの多重定義はメモリ割り当てを行わず、失敗しません。

### ポインタとサイズ

C風のバイトバッファ向けに、ポインタとサイズを受け取る多重定義が提供されます。

```cpp
auto md5(const void* data, std::size_t size) noexcept
    -> xer::result<std::array<std::byte, 16>>;

auto sha1(const void* data, std::size_t size) noexcept
    -> xer::result<std::array<std::byte, 20>>;

auto sha256(const void* data, std::size_t size) noexcept
    -> xer::result<std::array<std::byte, 32>>;
```

`data` が `nullptr` で `size` がゼロではない場合、これらの多重定義は `error_t::invalid_argument` で失敗します。

`data == nullptr` かつ `size == 0` は受け入れられ、空のバイト列を表します。

### イテレータ範囲

バイト風の範囲向けに、イテレータ範囲多重定義が提供されます。

```cpp
template<std::input_iterator InputIt>
auto md5(InputIt first, InputIt last) -> std::array<std::byte, 16>;

template<std::input_iterator InputIt>
auto sha1(InputIt first, InputIt last) -> std::array<std::byte, 20>;

template<std::input_iterator InputIt>
auto sha256(InputIt first, InputIt last) -> std::array<std::byte, 32>;
```

イテレータの値型は `std::byte`、または `std::uint8_t` に変換可能なバイト風の整数値でなければなりません。

これらの多重定義は、`std::array<std::byte, N>`、`std::vector<std::byte>`、`std::array<std::uint8_t, N>` などのコンテナで役立ちます。

### ファイルパス

ファイル多重定義は、ファイル全体のハッシュを計算します。

```cpp
auto md5(const xer::path& filename) -> xer::result<std::array<std::byte, 16>>;
auto sha1(const xer::path& filename) -> xer::result<std::array<std::byte, 20>>;
auto sha256(const xer::path& filename) -> xer::result<std::array<std::byte, 32>>;
```

これらの多重定義はファイル全体の内容を読み取ってからハッシュを計算します。ファイルI/Oの失敗は `xer::result` を通じて報告されます。

---

## 既知のハッシュテスト値

既知のダイジェスト値には次のものがあります。

| Input | MD5 | SHA-1 | SHA-256 |
| --- | --- | --- | --- |
| empty input | `d41d8cd98f00b204e9800998ecf8427e` | `da39a3ee5e6b4b0d3255bfef95601890afd80709` | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
| `a` | `0cc175b9c0f1b6a831c399e269772661` | `86f7e437faa5a7fce15d1ddcb9eaeaea377667b8` | `ca978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb` |
| `abc` | `900150983cd24fb0d6963f7d28e17f72` | `a9993e364706816aba3e25717850c26c9cd0d89d` | `ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad` |
| `message digest` | `f96b697d7cb7938d525a2f31aaf161d0` | `c12252ceda8be8994d5fa0290a47231c1d16aae3` | `f7846f55cf23e14eebeab5b4e1550cad5b509e3348fbc4efa3a1413d393cb650` |
| `abcdefghijklmnopqrstuvwxyz` | `c3fcd3d76192e4007dfb496cca67e13b` | `32d10c7b8cf96570ca04ce37f2a19d84240d3a89` | `71c480df93d6ae2f1efad1447c66c9525e316218cf51fc8d9ed832f2daf18b73` |

---

## ハッシュ関数の空入力

空入力は有効です。

空入力に対して:

- `md5` は `d41d8cd98f00b204e9800998ecf8427e` を返します
- `sha1` は `da39a3ee5e6b4b0d3255bfef95601890afd80709` を返します
- `sha256` は `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` を返します

---

# `<xer/base64.h>`

## 目的

`<xer/base64.h>` は、xer の Base64 エンコードおよびデコード機能を提供します。

Base64 は、小さなバイナリからテキストへの変換機能として扱われます。これは JSON、INI、TOML のような構造化データ形式ではなく、通常の文字列処理でもありません。その役割は、バイナリのバイト列をテキストベースのデータに埋め込める UTF-8 テキストへ変換し、そのテキスト表現をバイト列へ戻すことです。

初期実装では、意図的に標準 Base64 形式のみをサポートしています。URL セーフ Base64 やパディングなし Base64 などのバリアントは後回しです。

---

## 主な役割

`<xer/base64.h>` の主な役割は、次のことを可能にすることです。

- バイナリデータを標準 Base64 テキストへエンコードする
- 標準 Base64 テキストをバイナリデータへデコードする
- 不正なエンコード済みテキストを xer の通常の `xer::result` エラーモデルで扱う
- 基本形を変えずに後から拡張できる、コンパクトな公開 API を提供する

このため、このヘッダーは単純なバイナリペイロード処理、テキストベースの交換、設定データ、診断、小さなユーティリティプログラムに有用です。

---

## 主なエンティティ

少なくとも、`<xer/base64.h>` は次の関数を提供します。

```cpp
auto base64_encode(std::span<const std::byte> data)
    -> xer::result<std::u8string>;

auto base64_decode(std::u8string_view text)
    -> xer::result<std::vector<std::byte>>;
```

現在の API は意図的に小さく保たれています。追加のオーバーロードやオプションは後から追加される可能性があります。

---

## `base64_encode`

```cpp
auto base64_encode(std::span<const std::byte> data)
    -> xer::result<std::u8string>;
```

### 目的

`base64_encode` は、バイナリのバイト列を標準 Base64 テキストへ変換します。

### 入力モデル

入力は次の形で与えます。

```cpp
std::span<const std::byte>
```

これにより、この関数が明示的にバイト指向であることが分かります。入力はテキストとして解釈されず、UTF-8 として検証されることもありません。

### 出力モデル

成功時、関数は次の値を返します。

```cpp
std::u8string
```

出力は ASCII Base64 文字だけを含むため、有効な UTF-8 です。

### 返却モデル

返却型は `xer::result<std::u8string>` です。

現在の最小エンコーダには、通常、内容に依存する失敗条件はありません。ただし、将来のバリアントが公開 API の形を変えずに通常の失敗を報告できるように、この関数は引き続き `xer::result` を返します。

将来の失敗条件としては、出力サイズ制限、整形オプション、ストリーム指向出力の失敗、無効なオプションの組み合わせなどが考えられます。

---

## `base64_decode`

```cpp
auto base64_decode(std::u8string_view text)
    -> xer::result<std::vector<std::byte>>;
```

### 目的

`base64_decode` は、標準 Base64 テキストをバイナリデータへ戻します。

### 入力モデル

入力は次の形で与えます。

```cpp
std::u8string_view
```

意味を持つのは、ASCII Base64 文字、`=`、および無視される ASCII 空白だけです。非 ASCII 入力は、サポート対象の Base64 アルファベットに含まれないため拒否されます。

### 出力モデル

成功時、関数は次の値を返します。

```cpp
std::vector<std::byte>
```

出力はバイナリデータです。テキストとしては解釈されません。

---

## サポートされる Base64 形式

初期実装は、次のアルファベットを使う標準 Base64 をサポートします。

```text
ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/
```

パディングには次を使用します。

```text
=
```

### エンコード時の動作

`base64_encode` の現在の動作は次のとおりです。

- 標準 Base64 テキストを出力する
- パディングが必要な場合は常にパディングを出力する
- 改行を挿入しない
- 空白を挿入しない
- URL セーフ出力をサポートしない
- パディングなし出力をサポートしない

例:

```text
f      -> Zg==
fo     -> Zm8=
foo    -> Zm9v
foobar -> Zm9vYmFy
```

### デコード時の動作

`base64_decode` の現在の動作は次のとおりです。

- 標準 Base64 アルファベットを受け付ける
- `=` パディングは有効な末尾位置でのみ受け付ける
- ASCII 空白を無視する
- Base64 文字でない文字を拒否する
- 不正なパディングを拒否する
- 有効入力長が 4 の倍数でない入力を拒否する
- 非正準なパディングビットを拒否する

ASCII 空白とは、次の文字を意味します。

```text
space, tab, LF, CR, FF, VT
```

空白はデコード時にのみ無視されます。エンコーダは空白を生成しません。

---

## エラー処理

`<xer/base64.h>` は、xer の通常の失敗モデルに従います。

つまり、次のようになります。

- 通常の失敗は `xer::result` で報告される
- 不正なエンコード済みテキストは例外で処理されない
- 呼び出し側は、返された `xer::result` が値を持つかどうかを明示的に確認する

初期デコーダは、不正な Base64 入力を次のエラーとして報告します。

```cpp
error_t::invalid_argument
```

これには、少なくとも次の場合が含まれます。

- 不正な Base64 文字
- 不正な有効入力長
- 最終 quartet より前のパディング
- 不正なパディングパターン
- 非正準な未使用パディングビット

この段階では、詳細なエラー位置は報告されません。位置情報付き診断が後で有用になった場合は、エラー詳細型を別途拡張できます。

---

## 後回しにしている項目と制限

次の項目は、初期実装では意図的に後回しにしています。

### URL セーフ Base64

`+` と `/` の代わりに `-` と `_` を使う URL セーフ Base64 は、まだサポートしていません。

将来の API では、URL セーフ Base64 用のオプションまたは別バリアントを追加する可能性があります。

### パディングなし Base64

パディングなし Base64 は、まだサポートしていません。

現在のデコーダは、空白を除去した後の有効入力長が 4 の倍数であることを要求します。そのため、`=` パディングを省略することに依存した入力は拒否されます。

将来の API では、パディングなし Base64 を受け付ける、または出力するオプションを追加する可能性があります。

### エンコード時の MIME 風行折り返し

エンコーダは改行を挿入しません。

デコーダは ASCII 空白を無視するため、多くの行折り返し済み Base64 文字列を読み取れます。ただし、行折り返し付き出力の生成はまだ提供していません。

将来の API では、行幅オプションを追加する可能性があります。

### ストリーミングエンコードおよびデコード

初期 API は、完全な入力データを受け取り、完全な出力データを返します。

ストリーミング Base64 処理は後回しです。これには、`binary_stream` や `text_stream` との直接統合も含まれます。

### カスタムアルファベット

カスタム Base64 アルファベットはサポートしていません。

初期実装では、標準アルファベットだけを受け付けます。

### 詳細なエラー位置

初期デコーダは、不正な入力を `error_t::invalid_argument` として報告しますが、不正な文字や不正なパディングの正確な位置は報告しません。

詳細な位置情報は、後で `xer::error<Detail>` によって追加される可能性があります。

### テキストエンコーディング変換

Base64 テキスト自体は ASCII であり、したがって有効な UTF-8 ですが、`<xer/base64.h>` は文字エンコーディング変換を行いません。

`base64_decode` への入力は UTF-8 指向のテキストストレージとして扱われますが、意味を持つのは ASCII Base64 の部分集合だけです。バイナリ出力はバイト列として返され、UTF-8 としては解釈されません。

---

## 他のヘッダーとの関係

`<xer/base64.h>` は、次のヘッダーおよび方針に関連します。

- `<xer/error.h>`
- `<xer/stdio.h>`
- `policy_project_outline.md`
- `policy_result_arguments.md`
- `policy_encoding.md`

おおまかな境界は次のとおりです。

- `<xer/string.h>` は、一般的な文字列およびメモリユーティリティを扱う
- `<xer/stdio.h>` は、ストリーム I/O を扱う
- `<xer/json.h>`、`<xer/ini.h>`、`<xer/toml.h>` は、構造化または半構造化データ形式を扱う
- `<xer/base64.h>` は、バイト指向のバイナリからテキストへの変換を扱う

この分離は意図的なものです。Base64 はテキスト形式と組み合わせると便利ですが、それ自体は構造化データ形式ではありません。

---

## ドキュメント上の注意

このヘッダーを生成ドキュメントで扱う場合、通常は次の点を説明すれば十分です。

- Base64 エンコードはバイト指向である
- エンコード出力は ASCII 文字だけを含む UTF-8 テキストである
- デコードはバイナリバイト列を返す
- エンコードとデコードの両方が `xer::result` を返す
- 初期実装は標準のパディング付き Base64 のみをサポートする
- URL セーフ、パディングなし、折り返し付き出力、ストリーミングの各バリアントは後回しである

詳細なオプション動作は、そのようなオプションが実際に導入されたときに追加するべきです。

---

## 例として示す価値が高い題材

このヘッダーには、次のような例がとくに適しています。

- 小さなバイト列を Base64 へエンコードする
- Base64 テキストをバイト列へデコードする
- `xer::to_bytes_view` と組み合わせて文字列リテラルをエンコードする
- 不正な Base64 入力を明示的に処理する

これらは `examples/` 以下の実行可能な例として適した候補です。

---

## 例

```cpp
#include <span>
#include <string_view>

#include <xer/base64.h>
#include <xer/bytes.h>

int main()
{
    constexpr std::u8string_view text = u8"hello";

    const auto encoded = xer::base64_encode(xer::to_bytes_view(text));
    if (!encoded.has_value()) {
        return 1;
    }

    const auto decoded = xer::base64_decode(*encoded);
    if (!decoded.has_value()) {
        return 1;
    }

    return 0;
}
```

この例は、基本的な流れを示しています。

- `to_bytes_view` で UTF-8 テキストのバイト列ビューを得る
- `base64_encode` で Base64 テキストへ変換する
- `base64_decode` でバイト列へ戻す
- 各段階で `xer::result` を明示的に確認する

---

# `<xer/zip.h>`

## 目的

`<xer/zip.h>` は xer の ZIP アーカイブ読み書き機能を提供します。

ZIP は技術的にはアーカイブ形式ですが、実用上は馴染みのある圧縮・展開形式でもあります。xer では、初期 ZIP API を、将来シリアライズ済みデータパッケージ、同梱リソース、通常のファイル交換を支えられる小さな圧縮・アーカイブユーティリティとして扱います。

初期 API は意図的に小さくしています。逐次読み取り、名前検索、単純なアーカイブ作成、エントリ全体の読み取り、単純な展開ヘルパーをサポートします。コメントと ZIP64 対応は後回しです。

---

## 外部依存

`<xer/zip.h>` は zlib の開発用ヘッダーと zlib ライブラリを必要とします。

公開ヘッダーは、利用可能な場合に `__has_include` で `<zlib.h>` を確認し、ヘッダーが見つからなければコンパイル時診断を出します。このヘッダーを使うプログラムは、典型的な Unix 系環境での `-lz` のように、zlib とリンクする必要もあります。

プロジェクトのテストランナーは `xer/zip.h` を `zip` feature として検出し、zlib が利用可能な場合には対応するテストとコード例を zlib とリンクします。

---

## 主な役割

`<xer/zip.h>` の主な役割は、次のことを可能にすることです。

- ZIP アーカイブを開く
- エントリのメタデータを逐次読む
- 正確な名前でエントリを検索する
- エントリ名、サイズ、圧縮方式名を取得する
- エントリデータを読んで展開する
- 1つのエントリまたはすべてのエントリをファイルシステムへ展開する
- ZIP アーカイブを作成する
- メモリ上のバイト列または元ファイルを deflate エントリとして追加する
- 書き込み側を明示的に確定し、最終化エラーを報告できるようにする
- アーカイブ終端を xer の通常のエラーモデルで報告する

この設計では、アーカイブ終端に `std::optional` を返すことを避けています。エントリ列の末尾に到達したことは、次のように報告されます。

```cpp
error_t::end_of_file
```

これにより、`zip_read` は xer のほかの逐次入力操作と整合します。

---

## 主なエンティティ

少なくとも、`<xer/zip.h>` は次の型と関数を提供します。

```cpp
class zip_archive;
class zip_entry;

auto zip_open(std::u8string_view filename) -> xer::result<zip_archive>;

auto zip_create(std::u8string_view filename) -> xer::result<zip_archive>;

auto zip_read(zip_archive& archive) -> xer::result<zip_entry>;

auto zip_entry_name(const zip_entry& entry) -> xer::result<std::u8string>;

auto zip_entry_filesize(const zip_entry& entry) -> xer::result<std::uint64_t>;

auto zip_entry_compressed_size(const zip_entry& entry)
    -> xer::result<std::uint64_t>;

auto zip_entry_compression_method(const zip_entry& entry)
    -> xer::result<std::u8string>;

auto zip_entry_read(zip_archive& archive, const zip_entry& entry)
    -> xer::result<std::vector<std::byte>>;

auto zip_locate_name(zip_archive& archive, std::u8string_view entry_name)
    -> xer::result<zip_entry>;

auto zip_entry_read_by_name(
    zip_archive& archive,
    std::u8string_view entry_name) -> xer::result<std::vector<std::byte>>;

auto zip_entry_extract(
    zip_archive& archive,
    const zip_entry& entry,
    std::u8string_view target_filename) -> xer::result<void>;

auto zip_entry_extract_to(
    zip_archive& archive,
    const zip_entry& entry,
    std::u8string_view destination_dir) -> xer::result<void>;

auto zip_extract_to(
    zip_archive& archive,
    std::u8string_view destination_dir) -> xer::result<void>;

auto zip_add_from_bytes(
    zip_archive& archive,
    std::u8string_view entry_name,
    std::span<const std::byte> data) -> xer::result<void>;

auto zip_add_file(
    zip_archive& archive,
    std::u8string_view source_path,
    std::u8string_view entry_name) -> xer::result<void>;

auto zip_commit(zip_archive& archive) -> xer::result<void>;
```

通常のケースでは失敗が想定されないメタデータアクセサーを含め、すべての公開操作は `xer::result` を返します。これにより API の対称性を保ち、将来の検証、変換、バックエンド変更の余地を残します。

---

## `zip_archive`

```cpp
class zip_archive;
```

`zip_archive` はムーブ専用のアーカイブハンドルです。

読み取り時には、基盤となるバイナリストリームと中央ディレクトリの読み取り位置を所有します。書き込み時には、出力ストリームと保留中の中央ディレクトリレコードを所有します。破棄時には基盤ストリームを自動的に閉じます。コピーは無効です。

呼び出し側は通常、読み取りには `zip_open`、書き込みには `zip_create` を使ってこのオブジェクトを取得します。

書き込み側を使う場合、呼び出し側は明示的に `zip_commit` を呼び出すべきです。破棄時にストリームを閉じることはできますが、最終化エラーを `xer::result` として報告することはできません。

---

## `zip_entry`

```cpp
class zip_entry;
```

`zip_entry` は、中央ディレクトリから読まれた1つのアーカイブエントリのメタデータを格納します。

これは少なくとも次の情報を含む軽量な値オブジェクトです。

- エントリ名
- 非圧縮サイズ
- 圧縮サイズ
- 圧縮方式識別子
- フラグ
- ローカルヘッダーオフセット

呼び出し側は内部表現の詳細に依存せず、公開されている `zip_entry_*` 関数を使うべきです。

---

## `zip_open`

```cpp
auto zip_open(std::u8string_view filename) -> xer::result<zip_archive>;
```

### 目的

`zip_open` は ZIP アーカイブを読み取り用に開きます。

### 入力モデル

ファイル名は UTF-8 パス文字列です。内部では、ファイルを開く前に xer のパス処理を通して変換されます。

### 対応するアーカイブ

初期実装は、単一ディスク中央ディレクトリを持つ通常の非 ZIP64 アーカイブに対応します。

次のものは `error_t::invalid_argument` として拒否されます。

- 不正な ZIP ファイル
- end-of-central-directory レコードの欠落
- マルチディスクアーカイブ
- ZIP64 アーカイブ
- 整合しない中央ディレクトリ範囲

### 戻り値モデル

成功すると、開かれた `zip_archive` を返します。

失敗すると、`xer::result` のエラー情報を返します。ファイルを開く際の失敗は通常のファイルエラーモデルで報告されます。形式レベルの失敗は、一般に `error_t::invalid_argument` として報告されます。

---

## `zip_create`

```cpp
auto zip_create(std::u8string_view filename) -> xer::result<zip_archive>;
```

### 目的

`zip_create` は ZIP アーカイブを書き込み用に開きます。

返されたアーカイブは書き込み側です。`zip_commit` が中央ディレクトリを書き込んでストリームを閉じるまで、完全な ZIP ファイルではありません。

### 出力モデル

初期の書き込み側は、通常の非 ZIP64 単一ディスクアーカイブを作成します。エントリ名は ZIP の UTF-8 フラグを設定した UTF-8 名として格納されます。

---

## `zip_read`

```cpp
auto zip_read(zip_archive& archive) -> xer::result<zip_entry>;
```

### 目的

`zip_read` は、アーカイブの中央ディレクトリから次のエントリメタデータを読みます。

### 逐次モデル

アーカイブは現在の中央ディレクトリ位置を保持します。各成功呼び出しは、その位置を次のエントリへ進めます。

これにより、エントリ一覧全体をメモリ上に構築せずに済みます。これは多数のエントリを持つ大きなアーカイブで重要です。

### アーカイブ終端

それ以上エントリがない場合、この関数は次を返します。

```cpp
error_t::end_of_file
```

これは空の optional 値ではなく、エラー結果です。

### 未対応のエントリメタデータ

初期実装は、暗号化エントリ、マルチディスクエントリ参照、ZIP64 サイズのエントリを `error_t::invalid_argument` として拒否します。

---

## `zip_entry_name`

```cpp
auto zip_entry_name(const zip_entry& entry) -> xer::result<std::u8string>;
```

`zip_entry_name` はエントリ名を UTF-8 文字列として返します。

初期実装は、有効な UTF-8 であるエントリ名を受け入れます。ZIP の UTF-8 名フラグが設定されていて、格納された名前が有効な UTF-8 でない場合、その操作は `zip_read` の実行中に `error_t::encoding_error` で失敗します。

CP437 名変換はまだ実装されていません。そのため、非 UTF-8 名は推測されずに拒否されます。

---

## `zip_entry_filesize`

```cpp
auto zip_entry_filesize(const zip_entry& entry) -> xer::result<std::uint64_t>;
```

`zip_entry_filesize` は、エントリの非圧縮サイズをバイト単位で返します。

この名前は PHP の `zip_entry_filesize` の語彙に従いつつ、C++ 整数型を `xer::result` 経由で返します。

---

## `zip_entry_compressed_size`

```cpp
auto zip_entry_compressed_size(const zip_entry& entry)
    -> xer::result<std::uint64_t>;
```

`zip_entry_compressed_size` は、エントリの圧縮サイズをバイト単位で返します。

この関数名は PHP の `zip_entry_compressedsize` という綴りではなく snake_case を使います。これは C++ API であり、正確な互換性が必要ない箇所では読みやすさを優先するためです。

---

## `zip_entry_compression_method`

```cpp
auto zip_entry_compression_method(const zip_entry& entry)
    -> xer::result<std::u8string>;
```

`zip_entry_compression_method` は、圧縮方式名の文字列を返します。

初期実装は、stored エントリに対して次を返します。

```text
store
```

また、deflated エントリに対して次を返します。

```text
deflate
```

その他の方式識別子に対しては次を返します。

```text
unknown
```

未対応方式のデータ読み取りは `error_t::invalid_argument` で失敗します。

---

## `zip_entry_read`

```cpp
auto zip_entry_read(zip_archive& archive, const zip_entry& entry)
    -> xer::result<std::vector<std::byte>>;
```

### 目的

`zip_entry_read` は1つのエントリ本体を読んで展開します。

そのエントリは、同じアーカイブから取得されたものでなければなりません。初期 API は、別アーカイブ由来の使用を検証しようとはしません。

### 対応する圧縮方式

初期実装は次に対応します。

- stored エントリ
- deflated エントリ

stored エントリはそのまま返されます。deflated エントリは zlib による raw deflate で展開されます。

### 出力モデル

成功すると、この関数は展開済みのエントリバイト列を含む `std::vector<std::byte>` を返します。

これは意図的に所有権を持つバイトベクターです。大きなエントリを扱う用途が必要になれば、ストリーミング形式のエントリ読み取りを後で追加できます。

---

## `zip_locate_name`

```cpp
auto zip_locate_name(zip_archive& archive, std::u8string_view entry_name)
    -> xer::result<zip_entry>;
```

### 目的

`zip_locate_name` は、名前が `entry_name` と正確に一致するエントリをアーカイブ中央ディレクトリから検索します。

この関数は中央ディレクトリを走査し、最初に一致したエントリを返します。`zip_read` が使う逐次位置は変更しないため、呼び出し側は直接検索と後続の逐次読み取りを混在できます。

### 失敗モデル

要求された名前を持つエントリが存在しない場合、この関数は次を返します。

```cpp
error_t::not_found
```

不正な中央ディレクトリデータは、`zip_read` と同様に `error_t::invalid_argument` として報告されます。

---

## `zip_entry_read_by_name`

```cpp
auto zip_entry_read_by_name(
    zip_archive& archive,
    std::u8string_view entry_name) -> xer::result<std::vector<std::byte>>;
```

### 目的

`zip_entry_read_by_name` は、名前でエントリを見つけ、展開済みの本体を読みます。

これは次の処理を行う便利関数です。

```cpp
auto entry = zip_locate_name(archive, entry_name);
auto body = zip_entry_read(archive, *entry);
```

見つからないエントリは `error_t::not_found` として報告されます。未対応の圧縮方式や不正なエントリデータは、`zip_entry_read` と同じ方法で報告されます。

---

## `zip_entry_extract`

```cpp
auto zip_entry_extract(
    zip_archive& archive,
    const zip_entry& entry,
    std::u8string_view target_filename) -> xer::result<void>;
```

### 目的

`zip_entry_extract` は1つのエントリ本体を読み、指定されたファイルシステムパスへ書き込みます。

`target_filename` の親ディレクトリは必要に応じて作成されます。エントリ名が `/` で終わる場合、対象パスはファイルではなくディレクトリとして作成されます。

この関数は、エントリ名を展開先パスとして解釈しません。呼び出し側が正確な出力ファイル名をすでに選んでいる場合に適しています。

---

## `zip_entry_extract_to`

```cpp
auto zip_entry_extract_to(
    zip_archive& archive,
    const zip_entry& entry,
    std::u8string_view destination_dir) -> xer::result<void>;
```

### 目的

`zip_entry_extract_to` は、エントリ名を相対パスとして使い、1つのエントリを展開先ディレクトリ配下へ展開します。

### パス安全性

エントリ名が空、絶対パス、ドライブ相対、NUL コード単位を含む、中間に空の要素を含む、または `.` / `..` 要素を含む場合、そのエントリ名は `error_t::invalid_argument` として拒否されます。これにより、`../outside.txt` や `/tmp/outside.txt` のような通常のパストラバーサルを防ぎます。

名前が `/` で終わるディレクトリエントリはディレクトリを作成します。ファイルエントリは不足している親ディレクトリを作成し、その後に展開済みバイト列を書き込みます。

---

## `zip_extract_to`

```cpp
auto zip_extract_to(
    zip_archive& archive,
    std::u8string_view destination_dir) -> xer::result<void>;
```

### 目的

`zip_extract_to` は、すべてのエントリを展開先ディレクトリ配下へ展開します。

この関数は中央ディレクトリを直接走査し、`zip_read` が使う逐次位置を変更しません。これにより、呼び出し側はアーカイブを展開した後でも、同じ位置から後続の逐次走査を行えます。

すべてのエントリ名には、`zip_entry_extract_to` と同じパス安全性規則が適用されます。いずれかのエントリが安全でない、または展開できない場合、この関数はエラーを返します。そのエラーより前に書き込まれたエントリはロールバックされません。

---

## `zip_add_from_bytes`

```cpp
auto zip_add_from_bytes(
    zip_archive& archive,
    std::u8string_view entry_name,
    std::span<const std::byte> data) -> xer::result<void>;
```

### 目的

`zip_add_from_bytes` は、メモリ上の1つのバイト列を ZIP アーカイブ書き込み側へ追加します。

エントリは raw deflate で圧縮され、ローカルファイルヘッダーとともに書き込まれます。中央ディレクトリレコードは `zip_commit` が呼ばれるまでメモリ上に保持されます。

### 制限

初期の書き込み側は ZIP64 に対応しません。そのため、エントリ名、圧縮サイズ、非圧縮サイズ、ローカルヘッダーオフセット、エントリ数は、通常の ZIP フィールドに収まらなければなりません。

エントリ名は空でない有効な UTF-8 文字列でなければなりません。非 UTF-8 名は `error_t::encoding_error` で失敗します。

---

## `zip_add_file`

```cpp
auto zip_add_file(
    zip_archive& archive,
    std::u8string_view source_path,
    std::u8string_view entry_name) -> xer::result<void>;
```

### 目的

`zip_add_file` は元ファイルを読み、その内容を1つの ZIP エントリとして追加します。

これは `file_get_contents` と `zip_add_from_bytes` の便利ラッパーです。初期実装は、圧縮前に元ファイル全体をメモリへ読み込みます。大きなファイルを扱う用途が必要になれば、ストリーミング形式の file-to-ZIP 出力を後で追加できます。

---

## `zip_commit`

```cpp
auto zip_commit(zip_archive& archive) -> xer::result<void>;
```

### 目的

`zip_commit` は ZIP アーカイブ書き込み側を最終化します。

中央ディレクトリと end-of-central-directory レコードを書き込み、ストリームを flush して閉じます。最終化時にエラーが発生する可能性があるため、呼び出し側は書き込み側アーカイブではこれを必須手順として扱うべきです。

`zip_commit` 後に書き込み操作を呼び出すと、`error_t::invalid_argument` で失敗します。

---

## エラー処理

`<xer/zip.h>` は xer の通常の失敗モデルに従います。

つまり、次のようになります。

- 通常の失敗は `xer::result` で報告される
- アーカイブ終端は `error_t::end_of_file` として報告される
- 不正なアーカイブと不正な操作順序は `error_t::invalid_argument` として報告される
- 不正なエントリ名エンコーディングは `error_t::encoding_error` として報告される
- ストリーム失敗は通常のファイルエラーまたは I/O エラーを通じて報告される

初期実装は、詳細な ZIP 解析位置を提供しません。位置付き診断が後で有用になれば、エラー詳細型を別途追加できます。

---

## 後回しにしている項目と制限事項

次の項目は意図的に後回しにしています。

- エントリコメントとアーカイブコメント
- ZIP64
- マルチディスクアーカイブ
- 暗号化エントリ
- CP437 ファイル名変換
- ストリーミング形式のエントリ本体読み取り
- stored エントリ書き込みオプション
- データ記述子指向の書き込み対応
- 公開オプションとしての CRC 検証
- ストリーミング形式の file-to-ZIP 書き込み

最初の目標は、xer の `xer::result` と逐次 EOF モデルに合う、小さく予測しやすい PHP 風の ZIP リーダー・ライターです。

---

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

---

# `<xer/parse.h>`

## 目的

`<xer/parse.h>` は、解析済み入力のエラーに使う共通の構造化詳細型を提供します。

このヘッダーは、JSON、INI、TOML、XBF ビットマップフォントデータ、その他の個別のパーサーやローダーから意図的に独立しています。解析失敗位置と解析失敗理由は複数の構造化入力 API で有用なので、xer は形式ごとの詳細型ではなく共通語彙を提供します。

---

## 主なエンティティ

少なくとも、`<xer/parse.h>` は次のエンティティを提供します。

```cpp
enum class xer::parse_error_reason;

struct xer::parse_error_detail;
```

`parse_error_detail` は `xer::result<T, Detail>` の `Detail` 引数として使うことを想定しています。

---

## `parse_error_reason`

`parse_error_reason` は、構造化入力の失敗に対する形式非依存の理由コードです。

現在の理由集合には次のものがあります。

```cpp
enum class parse_error_reason {
    none,
    invalid_syntax,
    invalid_encoding,
    invalid_token,
    invalid_key,
    duplicate_key,
    duplicate_table,
    invalid_string,
    invalid_escape,
    invalid_unicode_escape,
    invalid_number,
    integer_out_of_range,
    invalid_date_time,
    invalid_array,
    invalid_table,
    invalid_magic,
    unsupported_version,
    invalid_header,
    invalid_range,
    invalid_offset,
    truncated_input,
};
```

理由は `error_t` より詳しい分類です。たとえば次のように使います。

- テキストパーサーは `error_t::invalid_argument` と `parse_error_reason::duplicate_key` を組み合わせて使う場合があります。
- XBF ビットマップフォントローダーは `error_t::invalid_argument` と `parse_error_reason::invalid_magic` を組み合わせて使う場合があります。

### バイナリ構造向けの理由

次の理由は、固定ヘッダー、オフセット、範囲、有限のバイト範囲を持つバイナリ形式またはその他の構造化入力を対象としています。

- `invalid_magic`
- `unsupported_version`
- `invalid_header`
- `invalid_range`
- `invalid_offset`
- `truncated_input`

これらは現在、XBF ビットマップフォントデータの検証時に `xer::image::bitmap_font_load` で使われています。

---

## `parse_error_detail`

```cpp
struct parse_error_detail {
    std::size_t offset = 0;
    std::size_t line = 0;
    std::size_t column = 0;
    parse_error_reason reason = parse_error_reason::none;
};
```

### 位置の意味

`offset` は入力先頭からの 0 始まりの位置です。

- UTF-8 テキスト形式では、UTF-8 コード単位で数えます。
- バイナリ形式では、バイト単位で数えます。

`line` は行情報が利用できる場合の 1 始まりの行番号です。行情報が利用できない場合は `0` です。

`column` は列情報が利用できる場合の 1 始まりの列番号です。UTF-8 テキスト形式では、表示セル、Unicode スカラー値、書記素クラスタではなく、UTF-8 コード単位で数えます。列情報が利用できない場合は `0` です。

この規則により、パーサー診断を xer の `char8_t` ベースのテキストモデルに合わせつつ、バイナリローダーは擬似的な行番号や列番号を作らずに正確なバイト位置を報告できます。

---

## 想定される使い方

構造化詳細を報告できるテキストパーサーは、次のような結果を返せます。

```cpp
auto toml_decode(std::u8string_view text)
    -> xer::result<toml_value, parse_error_detail>;
```

バイナリローダーも同じ詳細型を使えます。

```cpp
auto bitmap_font_load(const xer::path& filename)
    -> xer::result<xer::image::bitmap_font, parse_error_detail>;
```

解析または読み込みが失敗した場合、呼び出し側は共通のエラーコードと解析固有の詳細の両方を調べられます。

```cpp
const auto result = xer::toml_decode(text);
if (!result.has_value()) {
    const auto& error = result.error();
    // error.code
    // error.offset
    // error.line
    // error.column
    // error.reason
}
```

`error<Detail>` はクラス型の詳細を自然に公開するため、`offset`、`line`、`column`、`reason` は返されたエラーオブジェクトから直接利用できます。

---

## 他のヘッダーとの関係

`<xer/parse.h>` は次のヘッダーと関係します。

- `<xer/error.h>`
- `<xer/json.h>`
- `<xer/ini.h>`
- `<xer/toml.h>`
- `<xer/image.h>`

大まかな境界は次のとおりです。

- `<xer/error.h>` は共通のエラーコードと結果モデルを提供します。
- `<xer/parse.h>` は構造化入力の失敗に対する共通の解析詳細を提供します。
- 個別の形式ヘッダーは、その形式固有の値型とパーサーを提供します。

---

## ドキュメント上の注意

このヘッダーを生成マニュアルで参照する場合、通常は次の点を説明すれば十分です。

- `parse_error_detail` は解析失敗位置と理由を保持すること
- `line` と `column` は利用できない場合 `0` であること
- UTF-8 テキストの `offset` と `column` は UTF-8 コード単位で数えること
- 同じ詳細型をテキストパーサーとバイナリローダーの両方で使えること

詳細な形式固有の構文エラー規則は、それぞれの形式ヘッダーの説明に置くべきです。

---

# `<xer/json.h>`

## 目的

`<xer/json.h>` は、xer の JSON 読み書き機能を提供します。

現在の実装は、実用的な JSON 値の表現、解析、保存、読み込みを扱います。完全なアプリケーションフレームワークではなく、xer のエラーモデルと UTF-8 中心のテキストモデルに合う小さな JSON 層です。

---

## 主な役割

`<xer/json.h>` の主な役割は次を可能にすることです。

- JSON テキストを xer の値型へ解析する
- xer の JSON 値を JSON テキストへエンコードする
- JSON ファイルを読み込む、または保存する
- 通常の失敗を `xer::result` で明示的に報告する
- UTF-8 入力を前提にした予測可能な JSON 処理を提供する

このヘッダーは、設定ファイル、小さなデータ交換、テストデータ、診断用の構造化出力に適しています。

---

## 主なエンティティ

代表的には、`<xer/json.h>` は次のような JSON 値型と入出力関数を提供します。

```cpp
class json;

auto parse_json(std::u8string_view text)
    -> xer::result<json>;

auto load_json(const xer::path& filename)
    -> xer::result<json>;

auto save_json(const xer::path& filename, const json& value)
    -> xer::result<void>;
```

実際の公開 API の詳細は実装に従いますが、中心となる考え方は、JSON 値を所有する型と `xer::result` ベースの入出力関数を提供することです。

---

## JSON 値モデル

JSON 値は、JSON の基本的な値種別を表します。

- null
- boolean
- number
- string
- array
- object

文字列は UTF-8 テキストとして扱われます。JSON テキストの解析では、不正な UTF-8 や不正な JSON 構文は通常の失敗として報告されます。

---

## 解析

JSON 解析関数は UTF-8 JSON テキストを受け取り、JSON 値を返します。

```cpp
const auto value = xer::parse_json(u8"{\"name\":\"xer\"}");
if (!value.has_value()) {
    return 1;
}
```

解析に失敗した場合、関数は `xer::result` のエラーを返します。通常の解析失敗に例外は使いません。

---

## 読み込みと保存

ファイルベースのヘルパーは、JSON テキストをファイルから読み込む、またはファイルへ保存するために使います。

```cpp
const auto value = xer::load_json(xer::path(u8"config.json"));
if (!value.has_value()) {
    return 1;
}
```

保存側も `xer::result<void>` を返すことで、ファイル I/O エラーを明示的に扱えるようにします。

---

## エラーモデル

`<xer/json.h>` は xer の通常の失敗モデルに従います。

- 構文エラー
- 不正な UTF-8
- ファイル I/O エラー
- サポート範囲外の値

これらは、設計上必要な場合を除いて例外ではなく `xer::result` で報告されます。

---

## 設計上の注意

この JSON 層は、標準ライブラリや外部ライブラリの巨大な JSON フレームワークを置き換えることを目的としていません。

目的は、xer の他の機能と同じスタイルで、実用的で小さく、明示的な JSON 処理を提供することです。

特に重要な点は次のとおりです。

- 入出力テキストは UTF-8 を中心に扱う
- 失敗は `xer::result` で明示する
- 値の所有権と寿命を単純に保つ
- ファイル処理は `<xer/path.h>` や `<xer/stdio.h>` の方針と整合させる

---

## 他ヘッダーとの関係

`<xer/json.h>` は次と関連します。

- `<xer/error.h>`
- `<xer/path.h>`
- `<xer/stdio.h>`
- `<xer/string.h>`
- `policy_project_outline.md`
- `policy_encoding.md`

---

## ドキュメント上の注意

このヘッダーを生成リファレンスで扱う場合、少なくとも次を説明すれば十分です。

- JSON 値を扱うヘッダーであること
- JSON 解析、読み込み、保存を提供すること
- 通常の失敗は `xer::result` で報告すること
- UTF-8 テキストモデルに従うこと

詳細な JSON 値 API は、実装の安定化に合わせて個別に記述します。

---

# `<xer/ini.h>`

## 目的

`<xer/ini.h>` は、xer の INI 形式読み書き機能を提供します。

INI は厳密な単一標準を持つ形式ではありません。そのため、このヘッダーは「すべての INI 方言」を扱うのではなく、設定ファイル用途に向いた実用的で予測可能なサブセットを提供します。

---

## 主な役割

`<xer/ini.h>` の主な役割は次を可能にすることです。

- UTF-8 の INI テキストを解析する
- セクションとキー/値の設定データを保持する
- INI データを UTF-8 テキストとして保存する
- ファイルから読み込む、またはファイルへ保存する
- 構文エラーや I/O エラーを `xer::result` で明示的に扱う

この機能は、小さな設定ファイル、テスト用データ、手で編集しやすい設定表現に適しています。

---

## 主なエンティティ

代表的には、`<xer/ini.h>` は INI データ型と入出力関数を提供します。

```cpp
class ini;

auto parse_ini(std::u8string_view text)
    -> xer::result<ini>;

auto load_ini(const xer::path& filename)
    -> xer::result<ini>;

auto save_ini(const xer::path& filename, const ini& value)
    -> xer::result<void>;
```

実際の公開 API の詳細は実装に従いますが、中心となる考え方は、設定データを所有する型と `xer::result` ベースの読み書き関数を提供することです。

---

## INI データモデル

INI データは、セクションとキー/値の組を中心に扱います。

典型的な入力例です。

```ini
[general]
name = xer
enabled = true

[path]
root = ./data
```

セクション名、キー、値は UTF-8 テキストとして扱われます。

---

## 解析

INI 解析関数は UTF-8 テキストを受け取り、INI データを返します。

```cpp
const auto config = xer::parse_ini(u8"[app]\nname=xer\n");
if (!config.has_value()) {
    return 1;
}
```

構文エラーや不正な UTF-8 は通常の失敗として `xer::result` で報告されます。

---

## 読み込みと保存

ファイルベースのヘルパーは、INI ファイルを読み込む、または保存するために使います。

```cpp
const auto config = xer::load_ini(xer::path(u8"config.ini"));
if (!config.has_value()) {
    return 1;
}
```

保存時のファイル I/O エラーも `xer::result<void>` によって明示的に報告されます。

---

## 方言と制限

INI 形式には広く受け入れられた単一仕様がありません。

そのため、`<xer/ini.h>` は実装が定める実用的な方言を対象とします。呼び出し側は、Windows INI、PHP INI、各種アプリケーション固有 INI のすべてと完全互換であると仮定すべきではありません。

この方針により、xer は次を優先します。

- UTF-8 入力の明確な扱い
- 予測可能な構文
- 明示的な失敗報告
- 小さな設定ファイルでの実用性

---

## エラーモデル

`<xer/ini.h>` は xer の通常の失敗モデルに従います。

少なくとも次のような失敗が考えられます。

- 不正な UTF-8
- 構文エラー
- 不正なセクションまたはキーの形
- ファイル読み込みエラー
- ファイル書き込みエラー

通常の失敗に例外は使わず、`xer::result` で返します。

---

## 他ヘッダーとの関係

`<xer/ini.h>` は次と関連します。

- `<xer/error.h>`
- `<xer/path.h>`
- `<xer/stdio.h>`
- `<xer/string.h>`
- `policy_project_outline.md`
- `policy_encoding.md`

---

## ドキュメント上の注意

このヘッダーを生成リファレンスで扱う場合、少なくとも次を説明すれば十分です。

- INI 形式の設定データを扱うヘッダーであること
- 解析、読み込み、保存を提供すること
- INI は方言が多いため、xer の実用的なサブセットとして扱うこと
- 通常の失敗は `xer::result` で報告すること

---

# `<xer/toml.h>`

## 目的

`<xer/toml.h>` は、xer における TOML のデコードおよびエンコード機能を提供します。

TOML は、型付きの設定データ形式として扱われます。
このヘッダーの目的は、xer の段階的な開発方針に収まる程度に実装を小さく保ちながら、単純な UTF-8 TOML テキストの実用的な読み書きを支援することです。

現在の実装は、TOML の実用的なサブセットをサポートします。
TOML v1.0.0 への完全な互換性は主張しません。

---

## 主な役割

`<xer/toml.h>` の主な役割は、次のことを可能にすることです。

- UTF-8 TOML テキストを構造化された xer の値モデルへ解析する
- 真偽値、整数、浮動小数点数、文字列、配列、テーブルを調べる
- サポートされる値モデルを UTF-8 TOML テキストへシリアライズする
- INI とは異なる型付き設定形式として TOML を使用する

これにより、このヘッダーは、INI より多くの構造を必要としつつ、初期段階では TOML の全機能までは必要としない設定データに有用です。

---

## 主なエンティティ

少なくとも、`<xer/toml.h>` は次のエンティティを提供します。

```cpp
struct toml_value;

using toml_array = std::vector<toml_value>;
using toml_table = std::vector<std::pair<std::u8string, toml_value>>;

auto toml_decode(std::u8string_view text)
    -> xer::result<toml_value, parse_error_detail>;
auto toml_encode(const toml_value& value) -> xer::result<std::u8string>;
```

正確なヘルパー関数とサポートされる TOML 構文は今後拡張される可能性がありますが、これらは初期 TOML 機能の中核となる公開エンティティです。

---

## `toml_value`

`toml_value` は、xer における TOML の中心的な値型です。

1つの TOML 値を構造化された形で格納します。

### サポートされる値の種類

現在の実装は、次の値の種類をサポートします。

* 真偽値
* 整数
* 浮動小数点数
* 文字列
* ローカル日付
* ローカル時刻
* ローカル日時
* オフセット付き日時
* 配列
* テーブル

内部表現は次に対応します。

```cpp
std::variant<
    bool,
    std::int64_t,
    double,
    std::u8string,
    toml_local_date,
    toml_local_time,
    toml_local_datetime,
    toml_offset_datetime,
    toml_array,
    toml_table
>
```

### 注意

* 整数値は `std::int64_t` として格納されます
* 浮動小数点値は `double` として格納されます
* 文字列は UTF-8 の `std::u8string` として格納されます
* 日付/時刻値は TOML 専用の小さな値構造体に格納されます
* 配列は `std::vector<toml_value>` として格納されます
* テーブル配列は、要素がテーブルである配列として表現されます
* テーブルは順序付きのキー値ペアとして格納されます

---

## `toml_array`

```cpp
using toml_array = std::vector<toml_value>;
```

`toml_array` は TOML 値の配列を表します。

現在の実装では、配列は通常の順序付きベクターとして格納されます。

### 注意

* 配列要素の順序は保持されます
* 配列は、サポートされる種類が異なる値を含められます
* 配列はテーブル値を含められます
* テーブル配列構文は、要素がテーブル値である配列として表現されます

---

## `toml_table`

```cpp
using toml_table = std::vector<std::pair<std::u8string, toml_value>>;
```

`toml_table` は TOML テーブルを表します。

テーブルは、map 風のコンテナーではなく、順序付きのキー値ペアとして表現されます。

### 順序付きペアを使う理由

この表現は値モデルを単純に保ち、ソース上の順序を保持します。

また、公開データ形式モデルをハッシュマップやツリーマップの意味論へ早すぎる段階で固定しないという、xer の一般的な傾向にも合っています。

### 重複キー

TOML では、同じテーブル内の重複キーは許可されません。

デコーダーは重複キーを不正な入力として拒否します。

---

## 検査とアクセサー

`toml_value` は検査関数とアクセサー関数を提供します。

少なくとも、次の検査関数が提供されます。

```cpp
is_bool
is_integer
is_float
is_string
is_array
is_table
```

少なくとも、次のアクセサー関数が提供されます。

```cpp
as_bool
as_integer
as_float
as_string
as_array
as_table
```

アクセサー関数は、値が要求された種類を持つ場合は格納値へのポインターを返し、そうでない場合は `nullptr` を返します。

### 例

```cpp
const auto* table = value.as_table();
if (table == nullptr) {
    return 1;
}
```

このスタイルは、型検査を明示的にし、通常の種類判定のために例外を投げることを避けます。

---

## サポートされる TOML サブセット

現在の実装は、次のような TOML 風入力をサポートします。

```toml
title = "xer"
enabled = true
count = 123
hex = 0xFF
mask = 0b1010_0101
ratio = 1.5
large = 1_000_000
ports = [8000, 8001, 8002]
point = { x = 1, y = 2 }
items = [{ name = "one" }, { name = "two" }]
released = 2026-04-30
created = 2026-04-30T23:59:58+09:00

[project]
name = "xer"
version = "example"
```

### サポートされる形式

初期デコーダーは次をサポートします。

* 空行
* `#` で始まるコメント
* 文字列外の行末コメント
* 裸キー
* 引用符付きキー
* ドット付きキー
* キー値ペア
* 通常のテーブルと入れ子テーブル
* 基本二重引用符付き文字列
* リテラル文字列
* 複数行の基本文字列とリテラル文字列
* 真偽値
* 符号付き10進整数
* 16進、8進、2進整数
* 数字間の数値区切り文字
* 有限および特殊な浮動小数点数
* 配列
* インラインテーブル
* ローカル日付、ローカル時刻、ローカル日時、オフセット付き日時の値
* テーブル配列

### 改行

デコーダーは次の改行を受け付けます。

* LF
* CRLF
* CR

---

## 後回しの TOML 機能と制限事項

現在の実装は、日付/時刻値とテーブル配列を含む、TOML の実用的なサブセットをサポートします。

次の領域は意図的に制限されているか、後回しにされています。

* TOML v1.0.0 への完全準拠
* エンコード時に元の整形やコメントを保持すること
* TOML のオフセット付き日時構文を超えるタイムゾーン名
* TOML v1.0.0 が要求するすべての意味論的規則の完全な検証

したがって、この実装は完全な TOML 実装ではなく、実用的な TOML サブセットとして説明する必要があります。

---

## キー

実装は裸キー、引用符付きキー、ドット付きキーをサポートします。

裸キーには次を含められます。

* ASCII 英字
* ASCII 数字
* `_`
* `-`

例:

```toml
name = "xer"
build-target = "ucrt64"
version_1 = "example"
```

引用符付きキーは、値と同じ単一行の基本文字列またはリテラル文字列構文を使用します。

```toml
"site.name" = "xer"
'literal.key' = 10
```

ドット付きキーは、必要に応じて入れ子テーブルを暗黙に作成します。

```toml
server.port = 8080
database."connection.pool".size = 4
```

この表現では、各ドット付きキーセグメントは通常のテーブルキーとして格納されます。引用符付きセグメントには、分割されないドットを含められます。

---

## テーブル

テーブル行は次の形式です。

```toml
[project]
```

入れ子テーブル名も、ドット付きキー構文を通じてサポートされます。

```toml
[project.package]
name = "xer"

[project."build.target"]
name = "ucrt64"
```

テーブル名はキーパスとして解析されます。そのテーブルは、別のテーブルが宣言されるまで、後続のキー値エントリーの出力先になります。

### 注意

* 明示的なテーブル宣言の重複は拒否されます
* ドット付きキーによって暗黙に作成されたテーブルは、後で明示的に宣言できます
* `[[project]]` のようなテーブル配列構文はサポートされ、要素がテーブルである配列として表現されます

---

## 文字列

実装は、基本文字列、リテラル文字列、複数行基本文字列、複数行リテラル文字列をサポートします。

```toml
name = "xer"
path = 'C:\\Users\\xer'
description = """
first line
second line"""
literal_block = '''
C:\\Users\\xer
'''
```

基本文字列は次のエスケープをサポートします。

```text
\"
\\
\b
\t
\n
\f
\r
\uXXXX
\UXXXXXXXX
```

リテラル文字列はエスケープシーケンスを処理しません。複数行文字列は、TOML 風の挙動に従い、開始デリミターの直後の最初の改行を取り除きます。

---

## 真偽値

次の真偽値がサポートされます。

```toml
enabled = true
disabled = false
```

これらは `bool` として格納されます。

---

## 整数

現在の実装は、符号付き10進整数と、接頭辞付きの非10進整数をサポートします。数値区切り文字は数字間で使用できます。

```toml
count = 123
offset = -5
positive = +123
large = 1_000_000
hex = 0xFF
octal = 0o755
binary = 0b1010_0101
```

これらは `std::int64_t` として格納されます。

サポートされる整数接頭辞は、16進の `0x`、8進の `0o`、2進の `0b` です。接頭辞の前に先頭の `+` または `-` を使用できます。区切り文字は、対応する基数の数字間でのみ有効です。

---

## 浮動小数点数

実装は、有限の10進浮動小数点数と TOML の特殊な浮動小数点値をサポートします。数値区切り文字は、有限10進形式の10進数字間で使用できます。

```toml
ratio = 1.5
scale = 1e-3
large = 1_000.25_5
positive = +1.5
negative = -1.5
positive_inf = inf
negative_inf = -inf
not_a_number = nan
```

これらは `double` として格納されます。

特殊値 `inf`、`+inf`、`-inf`、`nan`、`+nan`、`-nan` が受け付けられます。区切り文字は有限10進形式の数字間でのみ有効です。`1_.0`、`1._0`、`1.0e_3` のような形式は拒否されます。

---

## 日付と時刻の値

TOML デコーダーは、ローカル日付、ローカル時刻、ローカル日時、オフセット付き日時をサポートします。

例:

```toml
date = 2026-04-30
time = 23:59:58.123456
local = 2026-04-30T23:59:58
offset = 2026-04-30T23:59:58+09:00
utc = 2026-04-30T14:59:58Z
```

値モデルはこれらを `toml_local_date`、`toml_local_time`、`toml_local_datetime`、`toml_offset_datetime` として格納します。小数秒はマイクロ秒精度で格納されます。

---

## 配列

実装は配列をサポートします。

```toml
ports = [8000, 8001, 8002]
mixed = ["xer", true, 3]
```

配列は要素の順序を保持します。

実装は配列を `toml_array` として格納します。

### 注意

* 配列は、サポートされるスカラー値、配列、インラインテーブルを含められます
* テーブル配列構文は、要素がテーブルである配列として表現されます

---

## インラインテーブル

実装はインラインテーブルをサポートします。

```toml
point = { x = 1, y = 2 }
package = { name = "xer", metadata.version = "example" }
items = [{ name = "one" }, { name = "two" }]
```

インラインテーブルは通常の `toml_table` 値として格納されます。インラインテーブル内のドット付きキーは、通常のドット付きキーで使われる表現と同じ表現で入れ子テーブル値を作成します。

インラインテーブル内の末尾カンマは拒否されます。

---

## テーブル配列

デコーダーは TOML のテーブル配列構文をサポートします。

```toml
[[products]]
name = "Hammer"

[[products]]
name = "Nail"
```

値モデルでは、これは要素がテーブル値である `toml_array` として表現されます。入れ子のテーブル配列は、親配列内の最新のテーブル要素に追加されます。

エンコーダーは、テーブル文脈に現れる、要素がテーブルである配列を `[[...]]` として出力します。

---

## コメント

`#` は、文字列外に現れるとコメントを開始します。

```toml
name = "xer" # comment
```

文字列内の `#` は、文字列の一部として扱われます。

```toml
name = "x#r"
```

---

## `toml_decode`

```cpp
auto toml_decode(std::u8string_view text)
    -> xer::result<toml_value, parse_error_detail>;
```

### 目的

`toml_decode` は UTF-8 TOML テキストを解析し、`toml_value` を返します。

### 入力モデル

入力テキストは次の形で渡されます。

```cpp
std::u8string_view
```

これは、xer の UTF-8 指向の公開テキストモデルに従います。

### 戻り値モデル

成功時、`toml_decode` は `toml_value` を返します。

返される値はテーブル値です。

### 失敗条件

少なくとも、次の場合にデコードは失敗します。

* 入力に不正な UTF-8 が含まれる
* キー値行に `=` が含まれない
* キーが不正な形式である
* テーブル宣言が不正な形式である
* キーが重複している
* テーブルが重複している
* 値がサポートされない構文を使用している
* 値が不正な形式である

不正な UTF-8 はエンコーディングエラーとして扱われます。
不正な TOML 構造は不正な引数として扱われます。

---

## 解析エラー詳細

`toml_decode` は `xer::result<toml_value, parse_error_detail>` を返します。

解析失敗時、エラーオブジェクトは通常の xer エラーコードに加え、`<xer/parse.h>` の `offset`、`line`、`column`、`reason` フィールドを含みます。

位置フィールドは UTF-8 コード単位で数えられます。`line` と `column` は 1 始まり、`offset` は 0 始まりです。

---

## `toml_encode`

```cpp
auto toml_encode(const toml_value& value) -> xer::result<std::u8string>;
```

### 目的

`toml_encode` は、サポートされる TOML 値モデルを UTF-8 TOML テキストへシリアライズします。

### 入力モデル

エンコードする値はテーブル値でなければなりません。

### 出力モデル

成功時は次を返します。

```cpp
std::u8string
```

生成されるテキストは `\n` 改行を使用します。

### シリアライズ形式

エンコーダーは、通常のキー値エントリーを先に出力します。

```toml
title = "sample"
enabled = true
count = 123
```

その後、テーブルセクションを出力します。

```toml
[project]
name = "xer"
version = "example"
```

先行する出力がある場合、テーブルの前に空行が挿入されます。

### 表現可能性

現在の TOML 実装はサブセットのみをサポートするため、`toml_encode` はそのサブセットで表現できない値を拒否します。

たとえば、次を拒否します。

* トップレベル値がテーブルではない
* サポートされないキー形式
* 不正な UTF-8 文字列

---

## エラー処理

`<xer/toml.h>` は xer の通常の失敗モデルに従います。

つまり、次のようになります。

* 解析失敗は `xer::result` を通じて報告されます
* シリアライズ失敗は `xer::result` を通じて報告されます
* 通常の失敗は、既定では例外として表現されません

通常のパターンは次のとおりです。

```cpp
const auto decoded = xer::toml_decode(text);
if (!decoded.has_value()) {
    return 1;
}
```

これは、失敗可能な公開 API は `xer::result` を返し、通常の公開 API は `xer::result` 引数ではなく通常の値を受け取るべきである、という xer の一般方針に従っています。

---

## 他のヘッダーとの関係

`<xer/toml.h>` は、次の文書およびヘッダーと合わせて理解する必要があります。

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `policy_encoding.md`
* `header_string.md`
* `header_json.md`
* `header_ini.md`

おおまかな境界は次のとおりです。

* `<xer/string.h>` は一般的な文字列およびテキストユーティリティを扱います
* `<xer/json.h>` は構造化データ形式としての JSON を扱います
* `<xer/ini.h>` は単純な文字列値設定データ形式としての INI を扱います
* `<xer/toml.h>` は型付き設定データ形式としての TOML を扱います

この分離は意図的なものです。
TOML は、文字列ヘルパーやストリームヘルパーではなく、独立したデータ形式機能として扱われます。

---

## ドキュメント上の注意

このヘッダーを生成ドキュメントで扱う場合、通常は次を説明すれば十分です。

* TOML 処理は UTF-8 テキストを使用すること
* 現在の実装は実用的な TOML サブセットであること
* トップレベルのデコード結果はテーブル値であること
* TOML 値は型付きであること
* 重複キーと重複テーブルは拒否されること
* 16進、8進、2進整数がサポートされること
* 数値区切り文字は数字間でのみサポートされること
* 残る制限事項は実用的なサブセットの制限として説明すべきであること

サポート機能の詳細は、サポートが拡張されるたびに実装と同期しておく必要があります。

---

## 例として示す価値がある話題

このヘッダーには、次のような例が特に適しています。

* 単純な TOML テキストのデコード
* トップレベルテーブルから値を読む
* セクションテーブルから値を読む
* `toml_value` テーブルを TOML テキストへエンコードする

これらは `examples/` の実行可能な例に適した候補です。

---

## 例

```cpp
#include <xer/toml.h>

auto main() -> int
{
    const auto decoded = xer::toml_decode(
        u8"title = \"sample\"\n"
        u8"enabled = true\n"
        u8"\n"
        u8"[project]\n"
        u8"name = \"xer\"\n"
        u8"version = \"example\"\n");

    if (!decoded.has_value()) {
        return 1;
    }

    const auto* root = decoded->as_table();
    if (root == nullptr) {
        return 1;
    }

    const auto encoded = xer::toml_encode(*decoded);
    if (!encoded.has_value()) {
        return 1;
    }

    return 0;
}
```

この例は一般的なスタイルを示しています。

* `toml_decode` で UTF-8 TOML テキストを解析する
* 結果のトップレベルテーブルを調べる
* `toml_encode` で再びシリアライズする
* 失敗可能な各手順で `xer::result` を明示的に確認する

---

## 関連項目

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `policy_encoding.md`
* `header_json.md`
* `header_ini.md`

---

## TOML の検索および読み込み/保存ヘルパー

このヘッダーは、デコード済み TOML 値を調べるためのヘルパー関数と、TOML ファイルを読み込みまたは保存するためのヘルパー関数も提供します。

```cpp
auto toml_find(toml_value& value, std::u8string_view path) noexcept
    -> toml_value*;

auto toml_find(const toml_value& value, std::u8string_view path) noexcept
    -> const toml_value*;

auto toml_load(const path& filename)
    -> xer::result<toml_value, parse_error_detail>;

auto toml_save(const path& filename, const toml_value& value)
    -> xer::result<void>;
```

`toml_find` ヘルパーは、すでにデコードされたメモリー上の値を調べ、既存の値へのポインターを返します。要求された項目が存在しない場合、または検索対象の値の形が合わない場合は `nullptr` を返します。

`toml_find` の `path` 引数は、`project.name` や `database.port` のような単純なドット区切りの検索パスです。これは完全な TOML キーパーサーではありません。特に、TOML の引用符付きキー構文はこのヘルパーでは解釈されず、引用符付きのドットは検索パス内で特別扱いされません。

読み込みヘルパーは UTF-8 ファイル読み込みとデコードを組み合わせ、`xer::result<toml_value, parse_error_detail>` を返します。解析が始まる前にファイル I/O が失敗した場合、返されるエラーは `parse_error_reason::none` を使用し、`offset`、`line`、`column` は 0 のままにします。

保存ヘルパーはエンコードと UTF-8 ファイル書き込みを組み合わせ、`xer::result<void>` を返します。

---

# `<xer/stdio.h>`

## 目的

`<xer/stdio.h>` は、xer のストリームベースの入出力機能を提供します。

その役割は C 標準ライブラリの `<stdio.h>` と精神的には似ていますが、文字どおりの再実装を目的とするものではありません。
代わりに、明示的なストリーム型、明示的なエンコーディング、xer の通常の失敗モデルを中心に、実用的な I/O を再構成します。

このヘッダーは、次のような主要なユーザー向け経路を提供するため、xer の中でも特に重要な公開ヘッダーの一つです。

- バイナリストリーム I/O
- テキストストリーム I/O
- 書式付き入出力
- ファイルエントリ操作
- CSV 入出力
- ストリーム状態と位置指定
- ストリームの巻き戻し
- ストリーム内容の便利操作
- ファイル全体の内容を扱う便利操作

---

## 主な役割

`<xer/stdio.h>` の主な役割は、次の原則に基づく一貫した I/O モデルを提供することです。

- `FILE*` を公開抽象として直接公開しない
- バイナリ I/O とテキスト I/O を明示的に区別する
- ストリームの生存期間管理に RAII を使用する
- テキストエンコーディングをロケールではなく明示的に扱う
- 通常の失敗を `xer::result` で報告する

これにより、古典的な C スタイル関数名の親しみやすさを保ちながら、現代的な C++ コードからより安全に使いやすいヘッダーになります。

---

## 中核となるストリーム型

少なくとも、`<xer/stdio.h>` は次の公開ストリーム型を提供します。

```cpp
class binary_stream;
class text_stream;
```

これらはこのヘッダーの中心的な抽象です。

### `binary_stream`

`binary_stream` はバイナリ入出力を表します。

これは次の用途に使われます。

* バイナリモードで開いたファイル
* メモリを背後に持つバイナリストリーム
* バイナリ一時ストリーム
* その他のバイト指向ストリーム対象

### `text_stream`

`text_stream` はテキスト入出力を表します。

これは次の用途に使われます。

* 明示的なテキストエンコーディングで開いたファイル
* UTF-8 または CP932 のテキスト入力元
* 文字列を背後に持つテキストストリーム
* テキスト一時ストリーム
* 標準のテキスト指向入出力対象

### 設計方針

これら 2 つのストリーム型は意図的に分離されています。

xer は、モード切り替えを持つ 1 つのストリームクラスとしてこれらをモデル化しません。
代わりに、バイナリ I/O とテキスト I/O の区別を型レベルで明示します。

---

## ムーブ専用 RAII オブジェクト

`binary_stream` と `text_stream` はムーブ専用の RAII オブジェクトです。

### 意味

これは少なくとも次のことを意味します。

* コピーできない
* ムーブできる
* オブジェクトの生存期間を通じてストリームリソースを獲得・解放する
* デストラクタが自動クリーンアップを行う

### なぜ重要か

これにより、ストリーム所有権が明示され、生ハンドル共有に伴う多くの曖昧さを避けられます。

また、明示的な所有権と明示的な失敗処理を好む xer の全体的な設計にも合っています。

---

## ストリームを開く

`<xer/stdio.h>` は、ファイル、メモリ、文字列からストリームを開く関数を提供します。

### ファイルを開く

少なくとも、公開されるファイルオープン形式は次のとおりです。

```cpp
auto fopen(const path& filename, const char* mode) noexcept -> xer::result<binary_stream>;
auto fopen(const path& filename, const char* mode, encoding_t encoding) noexcept -> xer::result<text_stream>;
```

これら 2 つのオーバーロードは、バイナリオープンとテキストオープンを分離します。

### メモリを開く

メモリを背後に持つストリームについては、次のような形式を提供する場合があります。

```cpp
auto memopen(std::span<std::byte> memory, const char* mode) noexcept -> xer::result<binary_stream>;
auto stropen(std::u8string_view text, const char* mode) noexcept -> xer::result<text_stream>;
auto stropen(std::u8string& text, const char* mode) noexcept -> xer::result<text_stream>;
```

### 設計方針

これらの open 関数は、次のようになるよう設計されています。

* バイナリストリームとテキストストリームをオープン時点で区別する
* 背後のコンテナの通常の所有権を暗黙に移譲しない
* 借用されたストレージであることを API 形状で明示する

---

## テキストエンコーディングの選択

テキストストリームでは、`<xer/stdio.h>` は明示的なエンコーディング選択を提供します。

少なくとも、公開エンコーディング列挙は次のとおりです。

```cpp
enum class encoding_t {
    utf8,
    cp932,
    auto_detect,
};
```

### 意味

* `utf8` は UTF-8 テキストを意味します
* `cp932` は CP932 テキストを意味します
* `auto_detect` は、サポートするエンコーディング間で入力側の自動判定を行うことを意味します

### 重要な注意

* xer のテキスト I/O はロケール中心ではありません
* エンコーディングはストリームオープンモデルの一部です
* `auto_detect` は入力用であり、一般的な書き込み側の振る舞いを表すものではありません

これは、従来のロケール駆動の C テキスト I/O との最も明確な違いの一つです。

---

## バイナリ I/O

バイナリストリームに対して、このヘッダーはバイト指向操作を提供します。

少なくとも、次のような関数を含みます。

```cpp
fread
fwrite
fgetb
fputb
```

### このグループの役割

これらの関数は次を提供します。

* ブロック入出力
* 1 バイト入出力
* バイト単位のバイナリストリーム操作

### 設計方針

バイナリデータは、テキストではなく生のバイト指向データとして扱います。

そのため、`fgetc` と `fputc` は 1 バイトのバイナリ I/O インターフェイスではありません。
代わりに、xer はその役割に `fgetb` と `fputb` を使用します。

### EOF の扱い

逐次入力関数は、入力ストリームを使い切った状態を `error_t::end_of_file` として報告します。
これは、`fread`、`fgetb`、`fgetc`、`fgets` などで新しいデータを読めない場合に適用されます。

部分読み取りは成功のままです。
たとえば、`fread` が要求されたバイト数より少なくても 1 バイト以上を読み取った場合は、実際に読み取ったバイト数を返します。
`stream_get_contents` は、内容を収集する際に `end_of_file` を自然な終了条件として扱います。

---

## テキスト I/O

テキストストリームに対して、このヘッダーは文字指向および文字列指向の操作を提供します。

少なくとも、次のような関数を含みます。

```cpp
fgetc
getchar
ungetc

fputc
putchar

fgets
gets
fputs
puts
```

### このグループの役割

これらの関数は次を提供します。

* 1 文字のテキスト入出力
* 文字列指向のテキスト入出力
* 標準ストリーム向けの便利操作
* `ungetc` による限定的な押し戻しサポート

### 設計方針

テキストストリームは内部的に xer のテキストモデルを中心に正規化されます。

特に次の点が重要です。

* 1 文字入力は `char32_t` を中心にします
* 文字列指向テキスト処理は UTF-8 `char8_t` 文字列を中心にします

実際に見えるオーバーロード集合は実装に依存しますが、概念的なモデルは同じです。

---

## 書式付き入出力

`<xer/stdio.h>` は書式付き I/O 機能も提供します。

少なくとも、次のファミリを含みます。

```cpp
fprintf
sprintf
snprintf
printf
fscanf
sscanf
scanf
```

### このグループの役割

これらの関数は、C に慣れたユーザーにも近づきやすいスタイルで、親しみのある書式付き I/O を提供します。

### 設計方針

名前は標準ライブラリに似ていますが、周辺設計は xer 独自です。

* ストリーム型は明示的です
* テキストモデルは UTF-8 指向です
* 通常の失敗は、該当する箇所では xer スタイルの結果処理で報告します
* C の厳密なソースレベル再現よりも、xer のストリーム抽象との統合を優先します

### printf 書式の詳細

# xer printf Format Specifiers

## Scope

This document describes the format strings used by the xer printf family.

Target functions:

```cpp
printf
fprintf
sprintf
snprintf
```

---

## Basic Policy

xer printf-style functions are inspired by C printf, but they are not strict source-compatible reimplementations.

- format strings are UTF-8 strings
- fixed text in the format string is copied as UTF-8
- conversion specifications start with `%`
- ordinary failure is reported through `xer::result`
- xer-specific extensions may exist

A format string may contain ordinary UTF-8 text and conversion specifications.
Ordinary text is copied to the output as-is.

---

## Conversion Specification Syntax

A conversion specification begins with `%`.

The currently supported structure is:

```text
%[position$][flags][width][.precision][length]conversion
```

The positional form is optional.
When it is used, the first argument is numbered `1`.

Examples:

```cpp
xer::printf(u8"%@ %@\n", first, second);
xer::printf(u8"%2$@ %1$@\n", first, second);
```

---

## Flags

The following flags are recognized:

```text
- + space # 0
```

Their meanings follow the usual printf-style interpretation where applicable.
For conversions where a flag has no meaningful effect, it may be ignored.

---

## Width and Precision

A field width may be specified as a decimal integer or by `*`.

A precision may be specified with `.` followed by a decimal integer or by `*`.

Both width and precision may use positional arguments.

Examples:

```cpp
xer::printf(u8"%10@\n", value);
xer::printf(u8"%.*@\n", precision, value);
xer::printf(u8"%2$*1$@\n", width, value);
```

Width is counted in UTF-8 code units in the current implementation.
It is not a display-cell width calculation.

---

## Length Modifiers

The following length modifiers are parsed:

```text
hh h l ll j z t L
```

They are accepted as part of the printf-style grammar.
The actual effect depends on the conversion and on xer's internal argument normalization.

For floating-point conversions, `L` is used when constructing the intermediate narrow format passed to `std::snprintf`.

---

## Supported C-Style Conversions

The following C-style conversion specifiers are supported:

```text
%d %i
%u
%o
%x %X
%c
%s
%p
%e %E
%f %F
%g %G
%a %A
%%
```

`%%` outputs a literal percent sign and does not consume an argument.

---

## xer Generic Display Conversion: `%@`

`%@` is xer's generic display specifier.

It is intended for diagnostics, examples, tracing, and simple output where precise base, padding, or precision control is not the main concern.
When precise formatting is required, ordinary printf-style conversions should be used instead.

### Argument Conversion Rules

Arguments passed to `%@` are normalized to UTF-8 text according to the following rules:

1. `char8_t`, `char8_t*`, `std::u8string`, and `std::u8string_view` are treated directly as UTF-8.
2. `char16_t*`, `std::u16string`, and `std::u16string_view` are converted from UTF-16 to UTF-8.
3. `char32_t*`, `std::u32string`, and `std::u32string_view` are converted from UTF-32 to UTF-8.
4. `wchar_t*`, `std::wstring`, and `std::wstring_view` are converted according to the width of `wchar_t`.
5. `std::string` and `std::string_view` are treated as UTF-8 byte strings.
6. `bool` is formatted as `true` or `false`.
7. `nullptr` is formatted as `null`.
8. Other stream-insertable types are formatted through `std::ostringstream` and the resulting narrow string is treated as UTF-8 bytes.

Invalid UTF-16 or UTF-32 scalar data may be represented by the replacement character in diagnostic-oriented conversions.

### xer Types

The following xer types are intended to be printable through `%@`:

```cpp
xer::error_t
xer::error<Detail>
xer::result<T, Detail>
```

These types provide stream insertion support so that `%@` can display them through the generic stream-based route.

### Notes on `std::ostringstream`

xer does not use iostreams as its primary public I/O model.
However, `%@` may use `std::ostringstream` internally as a practical interoperability mechanism.
This keeps user-facing xer formatted I/O based on `xer::printf` and related functions while allowing types that support `operator<<` to be displayed conveniently.

---

## Error Handling

Format errors, unsupported argument kinds, missing arguments, and out-of-range width or precision arguments are reported through `xer::result`.

The exact error category may be refined as the implementation evolves, but invalid format usage is generally treated as an ordinary formatting failure rather than as undefined behavior.

---

## Implementation Notes

This document is intended to describe the user-visible printf-family behavior.
When implementation details in `xer/bits/printf_format.h` change, this document should be kept in sync.

### scanf 書式の詳細

# xer scanf Format Specifiers

## Scope

This document describes the format strings used by the xer scanf family.

Target functions:

```cpp
scanf
fscanf
sscanf
```

The printf family is documented separately in `stdio_printf_format.md`.

---

## Basic Policy

xer scanf-style functions are inspired by C scanf, but they are not strict source-compatible reimplementations.

- format strings are UTF-8 strings
- input text is read as xer text and is processed as Unicode scalar values where appropriate
- ordinary fixed text in the format string must match the input
- ASCII whitespace in the format string matches zero or more ASCII whitespace characters in the input
- conversion specifications start with `%`
- ordinary failure is reported through `xer::result`
- match failure returns the number of successful assignments already completed
- xer-specific extensions may exist

A format string may contain ordinary UTF-8 text, whitespace, control tokens, and conversion specifications.

---

## Function Result

The scanf family returns the number of successful assignments.

```cpp
auto result = xer::sscanf(input, format, &a, &b);
```

On success, the returned value is the number of output arguments that were assigned.

If input matching fails after some assignments have already succeeded, the function returns the partial assignment count as a successful result.
This follows the general scanf-style model where an ordinary mismatch is not necessarily a format error.

If the format string is invalid, a type mismatch is detected, input decoding fails, or another ordinary runtime error occurs, the function returns failure through `xer::result`.

---

## Format String Structure

A scanf format string consists of the following kinds of items:

```text
ordinary UTF-8 literal text
ASCII whitespace
conversion specifications beginning with %
xer control tokens such as %@
```

Ordinary literal text must match the input exactly.

ASCII whitespace in the format string consumes zero or more ASCII whitespace characters in the input.
Consecutive whitespace in the format string is treated as a single whitespace-matching item.

---

## Conversion Specification Syntax

A conversion specification begins with `%`.

The currently supported structure is:

```text
%[position$][*][width][length]conversion
```

The positional form is optional.
When it is used, the first output argument is numbered `1`.

Examples:

```cpp
xer::sscanf(u8"10 abc", u8"%d %s", &value, &text);
xer::sscanf(u8"10 abc", u8"%2$s %1$d", &value, &text);
```

---

## Positional Arguments

A conversion may specify an output argument position:

```text
%1$d
%2$s
```

Argument positions are one-based.

When positional arguments are used, the format string is treated as positional.
Sequential and positional argument selection must not be mixed in the same format string, except through the xer `%@` control token rules described below.

Examples:

```cpp
int number = 0;
std::u8string text;

xer::sscanf(u8"hello 123", u8"%2$s %1$d", &number, &text);
```

Here `%2$s` stores into the second output argument and `%1$d` stores into the first output argument.

---

## Assignment Suppression

A conversion may suppress assignment by using `*` after `%` or after the optional positional prefix.

```text
%*d
%*s
```

The input is still matched and consumed, but no output argument is assigned and the assignment count is not incremented.

Example:

```cpp
int value = 0;
xer::sscanf(u8"10 20", u8"%*d %d", &value);
```

This stores `20` in `value`.

---

## Field Width

A field width may be specified as a positive decimal integer.

```text
%3s
%2d
%1c
```

The width limits the number of input characters considered by the conversion.

For `%s` and `%[...]`, the width limits the number of Unicode scalar values collected, not the number of UTF-8 code units.

A width of `0` is not accepted as an explicit field width.
When no field width is present, the conversion reads as much as its own matching rule allows.

---

## Length Modifiers

The following length modifiers are parsed:

```text
hh h l ll j z t L
```

They are accepted as part of the scanf-style grammar.
Their effect is applied to the intermediate value used by numeric conversions.
The actual output type is still determined by the pointer type passed by the caller.

For `%[...]`, length modifiers are currently invalid.

---

## Whitespace Handling Around Conversions

For most conversions, leading ASCII whitespace in the input is skipped before matching.

The following conversions skip leading ASCII whitespace:

```text
%d %u %o %x %X
%f %F %e %E %g %G
%s
```

The following conversions do not skip leading whitespace automatically:

```text
%c
%[...]
%%
```

This follows the usual scanf-style distinction: `%c` and scansets read the next input character according to their own matching rule.

---

## Supported Conversions

The following conversion specifiers are supported:

```text
%d
%u
%o
%x %X
%f %F
%e %E
%g %G
%c
%s
%[...]
%%
```

The `%@` token is also supported as an xer-specific control token.
It is described separately below.

---

## Integer Conversions

### `%d`

`%d` reads a signed decimal integer.

It accepts an optional leading sign followed by decimal digits.

### `%u`

`%u` reads an unsigned decimal integer.

### `%o`

`%o` reads an unsigned octal integer.

### `%x` and `%X`

`%x` and `%X` read an unsigned hexadecimal integer.

The implementation accepts hexadecimal digits using either lowercase or uppercase letters.

### Output Targets

Integer conversions can be stored into ordinary integer scalar targets when the target type is compatible with the intermediate value.

The implementation first parses into an intermediate integer value and then stores into the caller-provided output object.
If the destination pointer type is not compatible with the conversion result, the scan operation reports an error.

---

## Floating-Point Conversions

The following floating-point conversions are supported:

```text
%f %F
%e %E
%g %G
```

They read a floating-point lexeme and store the value into a floating-point target.

The accepted input form follows the implementation's current floating parser.
It includes ordinary decimal forms and exponent forms used by typical scanf-style input.

---

## Character Conversion: `%c`

`%c` reads one input character and stores it as a character-like value.

Unlike `%s`, `%c` does not skip leading whitespace automatically.

In the current implementation, `%c` accepts a field width only when the width is `1` or omitted.
A larger width is treated as invalid.

Typical output targets include:

```cpp
char32_t
char16_t
wchar_t
char8_t
char
signed char
unsigned char
```

The input is read as a Unicode scalar value and then stored into the destination character type.
When the destination is a single-byte character type, the caller is responsible for using it only for values that make sense for that type.

---

## String Conversion: `%s`

`%s` reads a non-empty sequence of non-whitespace characters.

Before matching, leading ASCII whitespace is skipped.
The conversion then collects characters until one of the following occurs:

- end of input
- ASCII whitespace
- field width is reached

The collected text is stored as UTF-8 internally and can be assigned to supported string targets.

Supported string targets include:

```cpp
std::u8string
std::u16string
std::u32string
std::wstring
```

The input text is UTF-8 in the xer text model.
When the destination is `std::u16string`, `std::u32string`, or `std::wstring`, the collected UTF-8 text is converted to the corresponding character-string representation.

For `std::wstring`, conversion follows the width of `wchar_t`:

- when `wchar_t` is effectively UTF-16, UTF-16 code units are produced
- when `wchar_t` is effectively UTF-32, UTF-32 code units are produced

---

## Scanset Conversion: `%[...]`

`%[...]` reads a non-empty sequence of characters that match a scanset.

Unlike `%s`, a scanset does not skip leading whitespace automatically.
The first input character must match the scanset for the conversion to succeed.

The collected text is stored as UTF-8 internally and can be assigned to the same string targets as `%s`:

```cpp
std::u8string
std::u16string
std::u32string
std::wstring
```

### Basic Form

```text
%[abc]
```

This matches one or more characters from the set `a`, `b`, and `c`.

### Negated Form

```text
%[^,]
```

A leading `^` negates the scanset.
This example reads characters until a comma is encountered.

### Including `]`

If `]` appears immediately after `[` or after `[^`, it is treated as a member of the scanset.

Examples:

```text
%[]x]
%[^]x]
```

### Ranges

ASCII ranges are supported.

```text
%[a-z]
%[0-9]
```

Ranges are interpreted over ASCII byte values.
For non-ASCII characters, each UTF-8 code point is handled as an individual scanset item rather than as part of a range.

---

## Literal Percent: `%%`

`%%` matches one literal percent sign in the input.

It does not assign to an output argument and does not increment the assignment count.

---

## xer Control Token: `%@`

`%@` is an xer-specific scanf control token.

It does not read input by itself.
Instead, it controls argument selection for the following conversion specification.

The main purpose is to make a following conversion use a specific output argument while keeping the conversion itself written in the ordinary form.

### Sequential Form

```text
%@ %d
```

In sequential mode, `%@` marks the following conversion as controlled by the current argument-selection flow.
This form is mainly useful as a building block for the same mechanism that also supports positional control.

### Positional Form

```text
%1$@ %d
```

The positional form applies the specified argument position to the following conversion.

Example:

```cpp
int a = 0;
int b = 0;

xer::sscanf(u8"10 20", u8"%2$d %1$@ %d", &a, &b);
```

The behavior is:

```text
%2$d   reads 10 into the second output argument
%1$@   selects the first output argument for the next conversion
%d     reads 20 into the first output argument
```

After the call:

```text
a == 20
b == 10
```

### Restrictions

A `%@` control token must be followed by a conversion specification.
A format string ending with pending `%@` is invalid.

Two consecutive control tokens are invalid.

When positional control is used, the format's argument-selection mode rules still apply.
Mixing incompatible sequential and positional forms is treated as an invalid format.

---

## Generic Stream-Extraction Storage

When a destination type is not one of the explicitly supported scalar, character, or string target categories, the implementation may store through a generic stream-extraction route.

Conceptually, the intermediate scanned value is first converted to UTF-8 text, then to a narrow byte string, and then read through:

```cpp
std::istringstream stream(text);
stream >> value;
```

This route is intended for types that naturally support `operator>>`.

Special string and wide-string targets such as `std::u16string`, `std::u32string`, and `std::wstring` are not handled through this generic route; they are handled explicitly from UTF-8 text.

---

## Assignment Count

The returned assignment count is incremented only when a conversion succeeds and actually assigns to a non-null output pointer.

The count is not incremented for:

- `%%`
- suppressed assignments such as `%*d`
- output arguments passed as `nullptr`
- control tokens such as `%@`

---

## Null Output Pointers

If an output pointer is `nullptr`, the conversion still reads and consumes input normally, but the value is discarded.

A successful conversion with a null output pointer does not increment the assignment count.

This allows callers to ignore selected values without changing the input-matching behavior.

---

## Match Failure vs Error

xer scanf-style functions distinguish ordinary match failure from errors.

### Match Failure

A match failure occurs when the input does not match the next literal or conversion.
In this case, the function returns the assignment count already completed.

Example:

```cpp
int a = 0;
int b = 0;

const auto result = xer::sscanf(u8"10 xx", u8"%d %d", &a, &b);
```

The first conversion succeeds, the second conversion fails to match, and the returned count is `1`.

### Error

An error is reported through `xer::result` failure.

Typical error cases include:

- invalid format syntax
- unsupported conversion syntax
- incompatible argument-selection mode
- invalid UTF-8 input where decoding is required
- type mismatch between a conversion and an output target
- invalid use of field width or length modifier

---

## Encoding Notes

xer scanf-style input works in xer's text model.

For `sscanf`, the input is a UTF-8 string.
For `fscanf` and `scanf`, the source is a `text_stream`, whose external encoding is handled by the stream layer and whose characters are read as xer text characters.

Collected string values are stored internally as UTF-8 before being assigned to the destination string type.

---

## Examples

### Basic scanning

```cpp
int value = 0;
std::u8string text;

const auto result = xer::sscanf(u8"123 hello", u8"%d %s", &value, &text);
```

After success:

```text
value == 123
text == u8"hello"
```

### Reading UTF-8 text into wide string targets

```cpp
std::u16string a;
std::u32string b;
std::wstring c;

xer::sscanf(u8"猫 犬 鳥", u8"%s %s %s", &a, &b, &c);
```

Each `%s` reads UTF-8 input and stores it in the destination string type.

### Reading a scanset

```cpp
std::u8string field;

xer::sscanf(u8"abc,rest", u8"%[^,]", &field);
```

This stores `u8"abc"` in `field`.

---

## Implementation Notes

This document is intended to describe the user-visible scanf-family behavior.
When implementation details in `xer/bits/scanf_format.h` or `xer/bits/scanf.h` change, this document should be kept in sync.

---

## CSV サポート

`<xer/stdio.h>` は CSV 向けヘルパーも含みます。

```cpp
fgetcsv
fputcsv
```

### このグループの役割

これらの関数は、xer ストリーム上で便利な CSV 入出力を提供します。

CSV はテキスト指向フォーマットであり、次との統合によって特に有用になります。

* 明示的なテキストエンコーディング
* 明示的なストリーム所有権
* UTF-8 指向文字列

### 設計方針

これらの関数は単なる文字列ヘルパーではありません。
ストリームと書式付きテキストレコードを操作するため、自然に I/O 層に属します。

---

## 位置とストリーム状態のヘルパー

少なくとも、`<xer/stdio.h>` は次のようなヘルパーを提供します。

```cpp
fseek
ftell
fgetpos
fsetpos
feof
ferror
clearerr
```

また、関連する公開型は次のとおりです。

```cpp
enum seek_origin_t { seek_set, seek_cur, seek_end };
using fpos_t = std::uint64_t;
```

### このグループの役割

これらの機能は次を提供します。

* バイトまたは位置指向のストリーム移動
* ストリーム状態の検査
* ストリームエラー状態の制御
* 明示的なテキストストリーム位置処理

### バイナリとテキストの位置指定

基本的な意図は次のとおりです。

* `fseek` / `ftell` は `binary_stream` の通常の位置ヘルパーです
* `fgetpos` / `fsetpos` は `text_stream` の主要な位置ヘルパーです

これは、テキストストリームがデコードやバッファリングの後で単純なバイトオフセットにきれいに対応するとは限らないことを反映しています。

---

## 巻き戻し

`<xer/stdio.h>` は、両方のストリーム種別に対して `rewind` を提供します。

```cpp
auto rewind(binary_stream& stream) noexcept -> xer::result<void>;
auto rewind(text_stream& stream) noexcept -> xer::result<void>;
```

C 標準ライブラリ関数と異なり、xer の `rewind` は `xer::result<void>` を返すため、無効なストリームやシーク失敗を明示的に報告できます。

テキストストリームでは、巻き戻し時に押し戻し文字、先読みバイト、途中のデコード状態もクリアします。
ストリームが `encoding_t::auto_detect` で開かれていた場合、具体的なエンコーディングは未決定状態に戻ります。

---

## ストリーム全体の便利操作

`<xer/stdio.h>` は、ストリーム全体の便利操作を提供します。

```cpp
auto stream_get_contents(
    binary_stream& stream,
    std::uint64_t length = std::numeric_limits<std::uint64_t>::max())
    -> xer::result<std::vector<std::byte>>;

auto stream_get_contents(text_stream& stream)
    -> xer::result<std::u8string>;

auto stream_put_contents(
    binary_stream& stream,
    std::span<const std::byte> contents)
    -> xer::result<void>;

auto stream_put_contents(
    text_stream& stream,
    std::u8string_view contents)
    -> xer::result<void>;
```

### 目的

`stream_get_contents` と `stream_put_contents` は、すでに開いている xer ストリームから読み取る、またはそこへ書き込むための簡潔なヘルパーです。

これらは `file_get_contents` と `file_put_contents` のストリームレベル版です。

ファイル名ではなくストリームに対して動作するため、ファイル、一時ファイル、メモリストリーム、文字列ストリーム、プロセスパイプ、適用可能な場合のソケット由来ストリームなど、xer がサポートする任意のストリーム入出力元・宛先で使用できます。

### バイナリ `stream_get_contents`

```cpp
auto stream_get_contents(
    binary_stream& stream,
    std::uint64_t length = std::numeric_limits<std::uint64_t>::max())
    -> xer::result<std::vector<std::byte>>;
```

このオーバーロードは、`stream` の現在位置からバイナリデータを読み取ります。

最大で `length` バイトを読み取り、EOF に到達した場合はそこで停止します。

`length` が 0 の場合、関数は成功し、空のバイトベクタを返します。

### オフセット引数が無い理由

xer は、意図的に `stream_get_contents` にオフセット引数を提供しません。

ストリームにはすでに現在位置があります。呼び出し側が開始位置を選びたい場合は、`stream_get_contents` を呼ぶ前に `fseek`、`fsetpos`、またはその他の適切な位置指定関数を明示的に使用してください。

これにより、PHP の `file_get_contents` と `stream_get_contents` で offset と length の引数順が異なるという混乱も避けられます。

xer での規則は単純です。

* `file_get_contents` は内部でファイルを開くため、オフセットを取ることがあります
* `stream_get_contents` はストリームの現在位置から読み取ります

### テキスト `stream_get_contents`

```cpp
auto stream_get_contents(text_stream& stream)
    -> xer::result<std::u8string>;
```

このオーバーロードは、`stream` の現在位置から EOF までテキストを読み取ります。

返される文字列は UTF-8 テキストです。

外部バイト列をどのようにデコードするかは、ストリーム自身のエンコーディング状態によって制御されます。

テキストモードの `stream_get_contents` は、`offset` や `length` 引数を提供しません。バイトオフセット、デコード済み文字、行末処理、エンコーディング状態が曖昧になり得るためです。

### バイナリ `stream_put_contents`

```cpp
auto stream_put_contents(
    binary_stream& stream,
    std::span<const std::byte> contents)
    -> xer::result<void>;
```

このオーバーロードは、`contents` 内の全バイトを `stream` の現在位置へ書き込みます。

正確な配置の振る舞いは、ストリームの現在位置とオープンモードによって決まります。

たとえば、ストリームが追記モードで開かれている場合、書き込みはそのストリームの追記動作に従います。

### テキスト `stream_put_contents`

```cpp
auto stream_put_contents(
    text_stream& stream,
    std::u8string_view contents)
    -> xer::result<void>;
```

このオーバーロードは、`contents` 内の UTF-8 テキストを `stream` の現在位置へ書き込みます。

その UTF-8 テキストを外部へどのようにエンコードするかは、ストリームのエンコーディングによって決まります。

### ファイル便利関数との関係

`file_get_contents` と `file_put_contents` は、これらのストリームレベルヘルパーを包むファイルオープン用の便利ラッパーです。

概念的には次のようになります。

```cpp
auto stream = xer::fopen(filename, "r");
return xer::stream_get_contents(*stream);
```

または次のようになります。

```cpp
auto stream = xer::fopen(filename, "w");
return xer::stream_put_contents(*stream, contents);
```

ストリームレベル関数は再利用可能な読み書きロジックを持ち、ファイルレベル関数はファイルを開き、バイナリ `offset` 引数などのファイル固有オプションを適用します。

### エラー処理

これらの関数は xer の通常の失敗モデルに従います。

成功時は次のようになります。

* `stream_get_contents` は読み取ったデータを返します
* `stream_put_contents` は空の成功値を返します

失敗時は、`xer::result` によってエラーを返します。

代表的な失敗条件は次のとおりです。

* 要求された操作に対してストリームが読み取り可能または書き込み可能でない
* 読み取りまたは書き込みに失敗する
* テキストのデコードまたはエンコードに失敗する
* 結果を収集する途中でメモリ確保に失敗する

### 例

```cpp
std::u8string buffer;

auto stream = xer::stropen(buffer, "w+");
if (!stream.has_value()) {
    return 1;
}

const auto written = xer::stream_put_contents(
    *stream,
    std::u8string_view(u8"hello xer"));

if (!written.has_value()) {
    return 1;
}

const auto rewound = xer::rewind(*stream);
if (!rewound.has_value()) {
    return 1;
}

const auto text = xer::stream_get_contents(*stream);
if (!text.has_value()) {
    return 1;
}
```

---

## クローズとフラッシュ

このヘッダーは、次のような操作も提供します。

```cpp
fclose
fflush
tmpfile
```

また、明示的にエンコードされたテキストストリーム向けのテキスト指向一時ファイルオーバーロード、または同等のヘルパーも提供します。

### このグループの役割

これらの関数は次をサポートします。

* 明示的なクローズ
* 明示的なフラッシュ制御
* 一時ストリーム作成

### 設計方針

ストリームのデストラクタは自動クリーンアップを行いますが、それでも明示的な close と flush 操作は重要です。

* 呼び出し側が決定的なリソース解放を望むことがあります
* 呼び出し側が破棄前にエラーを明示的に観測したいことがあります
* 明示的なフラッシュは通常のストリーム制御の一部です

---

## ファイルエントリ操作

`<xer/stdio.h>` は、次のようなファイルエントリ操作も提供します。

```cpp
file_exists
is_file
is_dir
is_readable
is_writable

remove
rename
mkdir
rmdir
copy
touch

fileatime
filemtime
filectime

chdir
getcwd
realpath

file_get_contents
file_put_contents
```

### このグループの役割

これらの関数は、開いているストリームオブジェクトではなく、ファイルシステムエントリに対して動作します。

ストリームやファイル処理と操作上近いため、ここにまとめられています。

### 設計方針

これらの関数はストリームオブジェクトそのものとは意図的に分離されています。

通常は、生のネイティブパス文字列ではなく `xer::path` に対して動作します。

これは、パス値を内部的に UTF-8 文字列で表し、正規化された区切り文字として `/` を使う xer 独自のパスモデルと整合します。

このグループの一部の関数は単純な述語であり、別の関数は実際のファイルシステム操作を行います。

`file_exists`、`is_file`、`is_dir`、`is_readable`、`is_writable` などの述語関数は `bool` を返します。

通常失敗し得る操作は `xer::result` を返します。

### ファイル時刻ヘルパー

`fileatime`、`filemtime`、`filectime` は、POSIX エポックからの秒数としてファイル時刻フィールドを返します。これらはプラットフォーム通常のパス状態取得操作を使い、そのプラットフォームの通常の stat 風操作がシンボリックリンクをたどる場合はそれに従うことがあります。

`filectime` は `xer::stat::ctime` と同じ ctime フィールドを返します。そのフィールドのプラットフォーム固有の意味は `xer::stat` で文書化されています。

```cpp
auto fileatime(const path& filename) -> xer::result<time_t>;
auto filemtime(const path& filename) -> xer::result<time_t>;
auto filectime(const path& filename) -> xer::result<time_t>;
```

### `touch`

```cpp
auto touch(
    const path& filename,
    time_t mtime = -1,
    time_t atime = -1) -> xer::result<void>;
```

`touch` は対象の変更時刻とアクセス時刻を変更します。対象が存在しない場合は、空の通常ファイルを作成します。

負の `mtime` は現在時刻を使用することを意味します。負の `atime` は、解決済みの `mtime` をアクセス時刻としても使用することを意味します。有限でない時刻値は不正な引数として拒否されます。

---

## カレントワーキングディレクトリ操作

`<xer/stdio.h>` はカレントワーキングディレクトリのヘルパーを提供します。

```cpp
auto chdir(const path& target) -> xer::result<void>;
auto getcwd() -> xer::result<path>;
```

### `chdir`

`chdir` はプロセス全体のカレントワーキングディレクトリを変更します。

```cpp
auto chdir(const path& target) -> xer::result<void>;
```

引数は `xer::path` です。

成功時、関数は空の成功値を返します。
失敗時は、`xer::result` によってエラーを返します。

カレントワーキングディレクトリはプロセス全体の状態であるため、複数のコンポーネントやスレッドがカレントディレクトリに依存する可能性があるプログラムでは、この関数を慎重に使用してください。

### `getcwd`

`getcwd` はカレントワーキングディレクトリを返します。

```cpp
auto getcwd() -> xer::result<path>;
```

返される値は `xer::path` です。

パスは xer の内部 UTF-8 表現へ変換され、正規化された区切り文字として `/` を使用します。

結果は、呼び出し時点のプロセス全体のカレントワーキングディレクトリのスナップショットです。

---

## `realpath`

```cpp
auto realpath(const path& filename) -> xer::result<path>;
```

### 目的

`realpath` は、存在するファイルシステムエントリの正規化された絶対パスを返します。

これは、プラットフォームのパス正規化機構を通じて実際のファイルシステムに問い合わせます。

### 振る舞い

対象パスは存在していなければなりません。

相対パス要素は解決されます。
シンボリックリンクやその他のファイルシステムレベルの間接参照は、基盤プラットフォームの振る舞いに従って解決されます。

POSIX 風環境では、振る舞いはプラットフォームの `realpath` 機能に従います。
Windows では、実装は Windows のパス正規化機能を使用し、その結果を xer のパス表現へ戻します。

### 戻り値

成功時、`realpath` は `xer::path` を返します。

返されるパスは次の性質を持ちます。

* 絶対パスである
* 存在するファイルシステムエントリを参照する
* xer の UTF-8 パス表現へ変換される
* 内部区切り文字として `/` を使用する

失敗時は、`xer::result` によってエラーを返します。

代表的な失敗条件は次のとおりです。

* 対象パスが存在しない
* 呼び出し側にパスへアクセスする権限がない
* ネイティブパス変換に失敗する
* プラットフォームのパス正規化に失敗する

### 字句的なパス操作との違い

`realpath` は純粋に字句的なパス操作ではありません。

これは実際のファイルシステムに依存し、シンボリックリンク、マウント済みボリューム、権限、既存エントリなどのファイルシステム状態を観測することがあります。

純粋に字句的なパス操作には、`basename`、`parent_path`、`extension`、`stem`、`is_absolute`、`is_relative` などのパスヘルパーを使用してください。

### 例

```cpp
const auto resolved = xer::realpath(xer::path(u8"."));
if (!resolved.has_value()) {
    return 1;
}
```

成功後、`resolved` にはカレントディレクトリの正規化された絶対パスが入ります。

---

## ファイル全体の便利操作

`<xer/stdio.h>` は、PHP に着想を得たファイル全体の便利操作を提供します。

```cpp
auto file_get_contents(
    const path& filename,
    std::uint64_t offset = 0,
    std::uint64_t length = std::numeric_limits<std::uint64_t>::max())
    -> xer::result<std::vector<std::byte>>;

auto file_get_contents(
    const path& filename,
    encoding_t encoding)
    -> xer::result<std::u8string>;

auto file_put_contents(
    const path& filename,
    std::span<const std::byte> contents)
    -> xer::result<void>;

auto file_put_contents(
    const path& filename,
    std::u8string_view contents,
    encoding_t encoding)
    -> xer::result<void>;
```

### 目的

`file_get_contents` と `file_put_contents` は、手動でストリームを開かずにファイル全体を読み書きするための簡潔なヘルパーです。

これらは `stream_get_contents` と `stream_put_contents` を包むファイルオープン用の便利ラッパーです。再利用可能な読み書きの振る舞いはストリームレベルヘルパーに属し、ファイルレベルヘルパーはさらに対象ファイルを開き、ファイル固有オプションを適用します。

同名の PHP 関数に着想を得ていますが、その振る舞いは xer のストリームおよびエンコーディングモデルに従います。

### バイナリとテキストの選択

オーバーロード集合は、`encoding_t` 引数の有無によってバイナリまたはテキストの振る舞いを選択します。

* エンコーディングが指定されていない場合、ファイルは `binary_stream` として扱われます
* エンコーディングが指定されている場合、ファイルは `text_stream` として扱われます

これにより、別個のモードフラグを導入せずに、呼び出し箇所を明示的に保てます。

### バイナリ `file_get_contents`

```cpp
auto file_get_contents(
    const path& filename,
    std::uint64_t offset = 0,
    std::uint64_t length = std::numeric_limits<std::uint64_t>::max())
    -> xer::result<std::vector<std::byte>>;
```

このオーバーロードはファイルをバイナリとして開き、その内容を `std::vector<std::byte>` として返します。

省略可能な `offset` と `length` 引数はバイト単位です。

`offset` がファイルサイズより大きい場合、関数は `error_t::invalid_argument` を返します。

`offset` がファイルサイズとちょうど等しい場合、関数は成功し、空のバイトベクタを返します。

`length` が 0 の場合、関数は成功し、空のバイトベクタを返します。

### テキスト `file_get_contents`

```cpp
auto file_get_contents(
    const path& filename,
    encoding_t encoding)
    -> xer::result<std::u8string>;
```

このオーバーロードはファイルをテキストとして開き、その内容を UTF-8 テキストとして返します。

指定したエンコーディングは、外部ファイルバイト列をどのようにデコードするかを制御します。

`encoding_t::auto_detect` は、この入力側操作では有効です。

テキストモードの `file_get_contents` は、`offset` や `length` 引数を提供しません。バイトオフセット、デコード済み文字、行末処理、エンコーディング状態が曖昧になり得るためです。

### バイナリ `file_put_contents`

```cpp
auto file_put_contents(
    const path& filename,
    std::span<const std::byte> contents)
    -> xer::result<void>;
```

このオーバーロードはファイルをバイナリとして開き、`contents` 内の全バイトを書き込みます。

既存のファイル内容は置き換えられます。

### テキスト `file_put_contents`

```cpp
auto file_put_contents(
    const path& filename,
    std::u8string_view contents,
    encoding_t encoding)
    -> xer::result<void>;
```

このオーバーロードはファイルをテキストとして開き、`contents` 内の UTF-8 テキストを指定した出力エンコーディングで書き込みます。

`encoding_t::auto_detect` は書き込みでは不正であり、`error_t::invalid_argument` になります。

### PHP スタイルのフラグを提供しない理由

xer は、これらの関数に PHP スタイルの `flags` 引数を意図的に提供しません。

特に、追記動作やロック動作を `file_put_contents` の内部に隠しません。

ファイルロックが必要な場合、呼び出し側は `flock` などの外側の操作で明示的に行ってください。

追記形式の出力が必要な場合、呼び出し側は追記モードでストリームを開き、`fwrite`、`fputs`、または `stream_put_contents` で書き込むなど、ストリーム API を直接使用してください。

### エラー処理

これらの関数は xer の通常の失敗モデルに従います。

成功時は次のようになります。

* `file_get_contents` は読み取ったデータを返します
* `file_put_contents` は空の成功値を返します

失敗時は、`xer::result` によってエラーを返します。

代表的な失敗条件は次のとおりです。

* ファイルを開けない
* シークに失敗する
* 読み取りまたは書き込みに失敗する
* `offset` が不正である
* テキスト出力に `encoding_t::auto_detect` が使用されている
* テキストのデコードまたはエンコードに失敗する

### 例

```cpp
const auto text = xer::file_get_contents(
    xer::path(u8"sample.txt"),
    xer::encoding_t::utf8);

if (!text.has_value()) {
    return 1;
}

const auto written = xer::file_put_contents(
    xer::path(u8"copy.txt"),
    *text,
    xer::encoding_t::utf8);

if (!written.has_value()) {
    return 1;
}
```

---

## ネイティブハンドルアクセス

このヘッダーは、ネイティブハンドルアクセスに関連するサポートを公開する場合があります。

### 役割

これは、呼び出し側が xer ストリーム抽象をより低レベルのプラットフォーム機能またはランタイム機能へ橋渡しする必要がある場合のために存在します。

### 設計方針

このようなサポートは中心的なものではなく補助的なものです。
通常のユーザー向け抽象はネイティブハンドルではなく、`binary_stream` または `text_stream` のままです。

---

## 内部設計方針

`<xer/stdio.h>` は公開ヘッダーですが、その概念設計は次の考え方に依存しています。

* ストリームオブジェクトは軽量な公開ハンドルである
* 内部状態はそれらのハンドルの背後に隠されている
* バイナリストリームとテキストストリームは、関数ポインタベースの内部ディスパッチを使う場合がある
* テキストストリーム状態は、バッファリング、エンコーディング解決、マルチバイト中間状態を含む場合がある

これらの実装上の考え方は、内部構造そのものが公開抽象ではないとしても、公開 API の形を理解するうえで重要です。

---

## xer のテキストモデルとの関係

`<xer/stdio.h>` は、xer の全体的なテキストモデルと最も強く結び付いたヘッダーの一つです。

特に次の点が重要です。

* UTF-8 は主要な公開文字列表現です
* 必要に応じて、個々のテキスト文字には `char32_t` を使用します
* テキストストリームは UTF-8 と CP932 を明示的にサポートします
* 自動テキスト入力判定は限定的かつ明示的です

そのため、`<xer/stdio.h>` は、より広いエンコーディング関連方針とあわせて読む必要があります。

---

## 他のヘッダーおよび方針との関係

`<xer/stdio.h>` は次とあわせて理解してください。

* `policy_project_outline.md`
* `policy_stdio.md`
* `policy_encoding.md`
* `header_path.md`
* `header_stdlib.md`

おおまかな境界は次のとおりです。

* `<xer/path.h>` は字句的なパス表現とネイティブパス変換を扱います
* `<xer/stdio.h>` はストリーム I/O とファイルエントリ操作を扱います
* `<xer/stdlib.h>` はマルチバイト変換と関連するユーティリティ機能を扱います
* エンコーディング方針は、テキストストリームの背後にあるより広いテキストエンコーディングモデルを説明します

---

## ドキュメント上の注意

このヘッダーを生成ドキュメントで扱う場合、通常は次を説明すれば十分です。

* xer が `binary_stream` と `text_stream` を区別すること
* テキストエンコーディングがロケール駆動ではなく明示的であること
* ストリームオブジェクトがムーブ専用 RAII 型であること
* 低レベル I/O と、書式付き I/O や CSV などの高レベル機能の両方を含むこと
* パス指向のファイルエントリ操作がこのヘッダーの一部であること
* `realpath` がファイルシステム依存であり、字句的なパス操作とは異なること
* `stream_get_contents` は現在のストリーム位置から読み取り、意図的にオフセット引数を提供しないこと
* `file_get_contents` と `file_put_contents` は、エンコーディング引数の有無によってバイナリ/テキスト動作が選択される便利 API であること
* `file_get_contents` と `file_put_contents` はストリームレベルの内容ヘルパーを包むファイルオープン用ラッパーであること

関数ごとの詳細な意味は、リファレンスマニュアルまたは生成された API セクションで説明してください。

---

## 例として示す価値のある話題

このヘッダーには、特に次のような例が適しています。

* バイナリファイルを開いてバイトを読み取る
* UTF-8 テキストストリームを開いてテキストを読み取る
* `puts` または `fputs` でテキストを書き込む
* `fgetpos` / `fsetpos` を使用する
* `tmpfile` を使用する
* CSV を読み書きする
* `rename`、`remove`、`copy` を実行する
* `chdir` と `getcwd` でカレントワーキングディレクトリを変更し、復元する
* `realpath` で既存パスを正規化する
* `stream_get_contents` と `stream_put_contents` で、すでに開いているストリームを読み書きする
* `file_get_contents` と `file_put_contents` でファイル全体を読み書きする

これらは `examples/` 以下の実行可能な例の候補として適しています。

---

## 例

```cpp
#include <xer/stdio.h>

auto main() -> int
{
    if (!xer::puts(u8"hello").has_value()) {
        return 1;
    }

    return 0;
}
```

この例は、基本的な xer スタイルを示しています。

* xer のテキスト I/O を直接使用する
* UTF-8 指向テキストを扱う
* `xer::result` を明示的に確認する

---

## 関連項目

* `policy_project_outline.md`
* `policy_stdio.md`
* `policy_encoding.md`
* `header_path.md`
* `header_stdlib.md`


---

---

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
xer::image::point
xer::image::pointf
xer::image::size
xer::image::sizef
xer::image::rect
xer::image::rectf
```

行列と色値の抽出は、現時点では意図的に後回しです。これらの挿入形式は診断用であり、安定した直列化文法として固定するためのものではありません。

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

---

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

---

# `<xer/dirent.h>`

## 目的

`<xer/dirent.h>` は xer のディレクトリストリーム操作を提供します。

このヘッダーは、ディレクトリを開く、エントリ名を読む、ディレクトリストリームを巻き戻す、閉じる、といった PHP/POSIX 風のディレクトリ走査機能を扱います。

目的は POSIX `dirent.h` を正確に再現することではありません。
代わりに、xer は次のものを使う C++23 向けの小さなディレクトリストリーム API を提供します。

- パス名には `xer::path`
- ディレクトリエントリ名には UTF-8 文字列
- 通常の失敗には `xer::result`
- ディレクトリストリームにはムーブ専用の RAII ハンドル

---

## 主なエンティティ

少なくとも、`<xer/dirent.h>` は次のエンティティを提供します。

```cpp
class xer::dir;

auto xer::opendir(const path& dirname) noexcept -> result<dir>;
auto xer::closedir(dir& directory) noexcept -> result<void>;
auto xer::readdir(dir& directory) noexcept -> result<std::u8string>;
auto xer::rewinddir(dir& directory) noexcept -> result<void>;
````

---

## 設計上の役割

このヘッダーはディレクトリストリーム走査のために存在します。

ディレクトリストリームは通常のファイルストリームではなく、状態を持つ走査ハンドルであるため、`<xer/stdio.h>` から分離されています。
名前は POSIX や PHP で見慣れたものですが、API は xer 独自のパス、文字列、エラー処理モデルに合わせて調整されています。

---

## `xer::dir`

`xer::dir` はムーブ専用のディレクトリストリームハンドルです。

内部でネイティブディレクトリストリームハンドルを所有し、破棄時に自動的に閉じます。

```cpp
class xer::dir;
```

### 基本性質

* ムーブ専用
* コピー不可
* RAII ベース
* 開いているディレクトリストリームまたは空/閉じた状態を表す

### 明示的なクローズ

デストラクタはディレクトリストリームを自動的に閉じますが、デストラクタからの失敗は観測できません。

呼び出し側がクローズエラーを観測する必要がある場合は、`xer::closedir` を明示的に呼び出してください。

---

## `xer::opendir`

```cpp
auto opendir(const path& dirname) noexcept -> result<dir>;
```

`opendir` は指定されたパスに対するディレクトリストリームを開きます。

基礎となるディレクトリ API を呼び出す前に、パスは xer の UTF-8 `xer::path` 表現からプラットフォームネイティブのパス表現に変換されます。

### 返り値

成功時は、開いている `xer::dir` を返します。

失敗時は、`xer::result` の失敗を返します。

### 注意

返されるディレクトリストリームは、スナップショットに近い走査ハンドルです。
ディレクトリを読んでいる間に内容が変更された場合、観測される動作はプラットフォームおよびファイルシステム依存です。

---

## `xer::closedir`

```cpp
auto closedir(dir& directory) noexcept -> result<void>;
```

`closedir` はディレクトリストリームを閉じます。

この関数を呼び出した後、`xer::dir` オブジェクトは閉じたものとして扱われます。

### 返り値

成功時は空の成功値を返します。

失敗時は `xer::result` の失敗を返します。

### 注意

すでに閉じられている、または空の `xer::dir` に対する `closedir` 呼び出しは、何もしない成功として扱われます。

`xer::dir` のデストラクタもディレクトリストリームを閉じますが、明示的な `closedir` は呼び出し側がクローズエラーを観測したい場合に有用です。

---

## `xer::readdir`

```cpp
auto readdir(dir& directory) noexcept -> result<std::u8string>;
```

`readdir` はディレクトリストリームから次のエントリ名を読み取ります。

返される文字列は UTF-8 のディレクトリエントリ名です。

### 返り値

成功時は、次のエントリ名を返します。

ディレクトリストリームの終端に達した場合は、次のエラーで失敗を返します。

```cpp
error_t::end_of_file
```

その他の失敗は、通常どおり `xer::result` で報告されます。

### 重要な注意

`readdir` が返すのはエントリ名だけです。

フルパスは返しません。

たとえば、ディレクトリに次のファイルが含まれる場合、

```text
example.txt
```

`readdir` は次を返します。

```text
example.txt
```

次ではありません。

```text
directory/example.txt
```

特殊エントリ `"."` と `".."` は除外されません。
これは PHP/POSIX 風の動作により近いものです。
これらのエントリが不要な呼び出し側は、明示的に読み飛ばしてください。

エントリ順はプラットフォームおよびファイルシステム依存です。
自分でソートしない限り、コードは特定の順序に依存してはいけません。

---

## `xer::rewinddir`

```cpp
auto rewinddir(dir& directory) noexcept -> result<void>;
```

`rewinddir` はディレクトリストリームを先頭に巻き戻します。

この関数が成功した後、以降の `readdir` 呼び出しはディレクトリストリームの先頭から再びエントリを読みます。

### 返り値

成功時は空の成功値を返します。

失敗時は `xer::result` の失敗を返します。

### 注意

巻き戻し後の順序も、依然としてプラットフォームおよびファイルシステム依存です。
xer はディレクトリエントリの安定した順序を保証しません。

---

## ディレクトリ終端の扱い

xer では、ディレクトリストリームの終端到達を次のように表します。

```cpp
error_t::end_of_file
```

典型的な使い方は次のとおりです。

```cpp
for (;;) {
    auto entry = xer::readdir(directory);
    if (!entry.has_value()) {
        if (entry.error().code == xer::error_t::end_of_file) {
            break;
        }

        return 1;
    }

    // *entry を使う。
}
```

これにより、ディレクトリ終端を通常の成功読み取りから分離しつつ、通常の `xer::result` 失敗経路を使えます。

---

## パス処理との関係

`<xer/dirent.h>` はディレクトリパスに `xer::path` を使います。

`path` オブジェクトは、xer の正規化された内部形式で UTF-8 パスを保持します。
`opendir` が呼び出されると、基礎となるディレクトリ API に渡す前に、パスはプラットフォームネイティブ表現に変換されます。

`readdir` が返す名前は UTF-8 文字列へ変換されます。

---

## 他のヘッダーとの関係

`<xer/dirent.h>` は次のヘッダーと関係します。

* `<xer/path.h>`
* `<xer/stdio.h>`
* `<xer/error.h>`

大まかな境界は次のとおりです。

* `<xer/path.h>` は字句的なパス表現とパスユーティリティを扱います。
* `<xer/stdio.h>` は通常のファイルストリームとファイル関連操作を扱います。
* `<xer/dirent.h>` はディレクトリストリーム走査を扱います。

---

## ドキュメント上の注意

このヘッダーを文書化するときに最も重要な点は次のとおりです。

* `xer::dir` はムーブ専用の RAII ディレクトリストリームハンドルであること
* `readdir` はフルパスではなくエントリ名を返すこと
* `"."` と `".."` は除外されないこと
* ディレクトリ終端は `error_t::end_of_file` で表されること
* エントリ順はファイルシステム依存であること
* 走査中の変更結果はプラットフォーム依存であること

---

## 例

```cpp
#include <xer/dirent.h>
#include <xer/error.h>
#include <xer/stdio.h>

auto main() -> int
{
    auto directory = xer::opendir(u8".");
    if (!directory.has_value()) {
        return 1;
    }

    for (;;) {
        auto entry = xer::readdir(*directory);
        if (!entry.has_value()) {
            if (entry.error().code == xer::error_t::end_of_file) {
                break;
            }

            return 1;
        }

        if (*entry == u8"." || *entry == u8"..") {
            continue;
        }

        if (!xer::puts(*entry).has_value()) {
            return 1;
        }
    }

    if (!xer::closedir(*directory).has_value()) {
        return 1;
    }

    return 0;
}
```

---

## 関連項目

* `<xer/path.h>`
* `<xer/stdio.h>`
* `<xer/error.h>`
* `policy_path.md`
* `policy_examples.md`

---

> **未訳:** この節の日本語版はまだ最新ではありません。
> そのため、暫定的に英語版の内容を掲載しています。
> 
> Header: `xer/socket.h`
> Reason: Japanese fragment was translated from a different English source hash.

# `<xer/socket.h>`

## Purpose

`<xer/socket.h>` provides a small socket API for TCP and UDP networking.

The API is intentionally low-level enough to stay understandable, but it wraps platform differences and reports ordinary failure through `xer::result`.

---

## Main Role

This header provides:

- a move-only RAII socket handle
- IPv4 and IPv6 socket creation
- TCP connection, bind, listen, and accept operations
- UDP send/receive operations
- reliable fixed-size send/receive helpers for stream sockets
- length-prefixed message send/receive helpers for stream sockets
- conversion of sockets to xer binary or text streams

---

## Main Types

```cpp
enum class socket_family;
enum class socket_type;
struct socket_address;
struct socket_recvfrom_result;
class socket;
```

### `socket_family`

```cpp
ipv4
ipv6
```

### `socket_type`

```cpp
tcp
udp
```

### `socket_address`

`socket_address` stores a textual address and a port number.

```cpp
std::u8string address;
std::uint16_t port;
```

### `socket_recvfrom_result`

`socket_recvfrom_result` stores the number of bytes read and the remote endpoint address.

---

## Socket Handle

`socket` is a move-only RAII type.

Important operations include:

```cpp
auto is_open() const noexcept -> bool;
auto family() const noexcept -> socket_family;
auto type() const noexcept -> socket_type;
auto close() noexcept -> int;
auto release() noexcept -> native_socket_t;
auto native_handle() const noexcept -> native_socket_t;
```

The destructor closes the socket if it is still open.

---

## Socket Operations

```cpp
auto socket_create(socket_family family, socket_type type) noexcept -> xer::result<socket>;
auto socket_close(socket& s) noexcept -> xer::result<void>;
auto socket_connect(socket& s, std::u8string_view host, std::uint16_t port) noexcept -> xer::result<void>;
auto socket_bind(socket& s, std::uint16_t port) noexcept -> xer::result<void>;
auto socket_bind(socket& s, std::u8string_view address, std::uint16_t port) noexcept -> xer::result<void>;
auto socket_getsockname(socket& s) noexcept -> xer::result<socket_address>;
auto socket_listen(socket& s, int backlog = 16) noexcept -> xer::result<void>;
auto socket_accept(socket& s) noexcept -> xer::result<socket>;
auto socket_send(socket& s, std::span<const std::byte> data) noexcept -> xer::result<std::size_t>;
auto socket_recv(socket& s, std::span<std::byte> data) noexcept -> xer::result<std::size_t>;
auto socket_send_all(socket& s, std::span<const std::byte> data) noexcept -> xer::result<void>;
auto socket_recv_exact(socket& s, std::span<std::byte> data) noexcept -> xer::result<void>;
auto socket_send_message(socket& s, std::span<const std::byte> data) noexcept -> xer::result<void>;
auto socket_recv_message(socket& s, std::size_t max_size) noexcept -> xer::result<std::vector<std::byte>>;
auto socket_sendto(socket& s, std::u8string_view host, std::uint16_t port, std::span<const std::byte> data) noexcept -> xer::result<std::size_t>;
auto socket_recvfrom(socket& s, std::span<std::byte> data) noexcept -> xer::result<socket_recvfrom_result>;
```

### `socket_bind(socket&, port)`

```cpp
auto socket_bind(socket& s, std::uint16_t port) noexcept -> xer::result<void>;
```

Binds a socket to the specified port on all local interfaces for the socket family.

For an IPv4 socket, this uses the wildcard IPv4 address.
For an IPv6 socket, this uses the wildcard IPv6 address.

### `socket_bind(socket&, address, port)`

```cpp
auto socket_bind(socket& s, std::u8string_view address, std::uint16_t port) noexcept -> xer::result<void>;
```

Binds a socket to the specified local address and port.

This overload is useful when a server must listen only on a specific interface, such as `127.0.0.1` for a local helper process.
The address must be compatible with the socket family.
For example, an IPv4 socket can bind to `127.0.0.1`, while an IPv6 socket can bind to `::1`.

### `socket_send` and `socket_recv`

```cpp
auto socket_send(socket& s, std::span<const std::byte> data) noexcept -> xer::result<std::size_t>;
auto socket_recv(socket& s, std::span<std::byte> data) noexcept -> xer::result<std::size_t>;
```

`socket_send` and `socket_recv` perform a single send or receive operation.
They may transfer fewer bytes than requested, especially for stream sockets.
The returned size reports the number of bytes actually transferred.

### `socket_send_all`

```cpp
auto socket_send_all(socket& s, std::span<const std::byte> data) noexcept -> xer::result<void>;
```

Sends all bytes in `data` unless an error occurs.

This function repeatedly calls `socket_send` until the entire span has been sent.
It is intended for connected stream sockets such as TCP sockets.
An empty span succeeds without sending anything.

### `socket_recv_exact`

```cpp
auto socket_recv_exact(socket& s, std::span<std::byte> data) noexcept -> xer::result<void>;
```

Receives exactly `data.size()` bytes unless an error occurs or the peer closes the connection before enough bytes are received.

This function repeatedly calls `socket_recv` until the span has been filled.
It is intended for connected stream sockets such as TCP sockets.
An empty span succeeds without receiving anything.

### `socket_send_message`

```cpp
auto socket_send_message(socket& s, std::span<const std::byte> data) noexcept -> xer::result<void>;
```

Sends a length-prefixed message.

The function first sends a 4-byte unsigned big-endian payload length, followed by all bytes in `data`.
The payload size must fit in `std::uint32_t`; otherwise the function returns `error_t::length_error`.

An empty message is valid and sends only a zero length field.
The function uses `socket_send_all` internally, so the whole frame is sent unless an error occurs.

### `socket_recv_message`

```cpp
auto socket_recv_message(socket& s, std::size_t max_size) noexcept -> xer::result<std::vector<std::byte>>;
```

Receives one length-prefixed message sent in the same frame format used by `socket_send_message`.

The function first receives a 4-byte unsigned big-endian payload length.
If the payload length is greater than `max_size`, the function returns `error_t::length_error` without reading or discarding the payload.
In that case, callers should normally close the connection because the stream is no longer positioned at the next frame.

If the payload length is accepted, the function allocates a `std::vector<std::byte>` of that size, receives exactly that many payload bytes, and returns the vector.
An empty message is valid and returns an empty vector.

If the peer closes the connection before the complete length field or payload has been received, the function returns `error_t::network_error`.

---

## Length-Prefixed Message Example

A message sent by `socket_send_message` has the following frame format:

```text
uint32 big-endian payload_size
payload bytes
```

For example, a 5-byte payload `hello` is sent as a 4-byte length field followed by the five payload bytes.

```cpp
constexpr std::array<std::byte, 5> hello = {
    std::byte {'h'},
    std::byte {'e'},
    std::byte {'l'},
    std::byte {'l'},
    std::byte {'o'},
};

auto sent = xer::socket_send_message(client, hello);
```

The receiver can read the frame with `socket_recv_message`:

```cpp
auto body = xer::socket_recv_message(server, 1024 * 1024);
```

The second argument is the maximum accepted payload size.
It prevents an untrusted length field from causing an unbounded allocation.

---

## Example

The following example binds a TCP server socket to the loopback address and exchanges fixed-size messages.

```cpp
#include <array>
#include <bit>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <thread>

#include <xer/socket.h>

namespace
{
    auto bytes_to_text(std::span<const std::byte> bytes) -> std::string
    {
        std::string text(bytes.size(), '\0');
        std::memcpy(text.data(), bytes.data(), bytes.size());
        return text;
    }
}

auto main() -> int
{
    constexpr auto port = std::uint16_t{39080};

    auto server_result = xer::socket_create(xer::socket_family::ipv4, xer::socket_type::tcp);
    if (!server_result) {
        std::cerr << "socket_create failed\n";
        return 1;
    }

    auto server = std::move(*server_result);

    if (auto result = xer::socket_bind(server, u8"127.0.0.1", port); !result) {
        std::cerr << "socket_bind failed\n";
        return 1;
    }

    if (auto result = xer::socket_listen(server); !result) {
        std::cerr << "socket_listen failed\n";
        return 1;
    }

    auto worker = std::thread([&server] {
        auto accepted_result = xer::socket_accept(server);
        if (!accepted_result) {
            return;
        }

        auto peer = std::move(*accepted_result);

        auto request = std::array<std::byte, 4>{};
        if (auto result = xer::socket_recv_exact(peer, request); !result) {
            return;
        }

        std::cout << "server received: " << bytes_to_text(request) << '\n';

        constexpr auto response = std::array{
            std::byte{'p'},
            std::byte{'o'},
            std::byte{'n'},
            std::byte{'g'},
        };

        (void) xer::socket_send_all(peer, response);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    auto client_result = xer::socket_create(xer::socket_family::ipv4, xer::socket_type::tcp);
    if (!client_result) {
        worker.join();
        return 1;
    }

    auto client = std::move(*client_result);

    if (auto result = xer::socket_connect(client, u8"127.0.0.1", port); !result) {
        worker.join();
        return 1;
    }

    constexpr auto request = std::array{
        std::byte{'p'},
        std::byte{'i'},
        std::byte{'n'},
        std::byte{'g'},
    };

    if (auto result = xer::socket_send_all(client, request); !result) {
        worker.join();
        return 1;
    }

    auto response = std::array<std::byte, 4>{};
    if (auto result = xer::socket_recv_exact(client, response); !result) {
        worker.join();
        return 1;
    }

    std::cout << "client received: " << bytes_to_text(response) << '\n';

    worker.join();
    return 0;
}
```

---

## Stream Conversion

Sockets can be converted into xer streams:

```cpp
auto socket_open(socket&& s) noexcept -> xer::result<binary_stream>;
auto socket_open(socket&& s, encoding_t encoding) noexcept -> xer::result<text_stream>;
```

The binary stream form is suitable for byte-oriented protocols.
The text stream form is suitable for UTF-8 or CP932 text-oriented communication.

---

## Notes

- Network-related ordinary failures are represented mainly with `error_t::network_error`.
- The API does not use command shells or external utilities.
- Host names and textual addresses are accepted as UTF-8 strings and converted to ordinary narrow strings for resolver APIs.
- `socket_send` and `socket_recv` may transfer only part of the supplied buffer.
- Use `socket_send_all` and `socket_recv_exact` when a fixed amount of data must be transferred over a connected stream socket.
- `socket_open` transfers ownership from the socket object into the resulting stream.

---

# `<xer/tk.h>`

## 目的

`<xer/tk.h>` は、xer の初期 Tcl/Tk 連携層を提供します。

このヘッダーの第一の目的は、C++コードから Tcl インタープリターを作成・制御し、C++の呼び出し可能オブジェクトを Tcl コマンドとして登録し、その基盤を Tk ベースの GUI 機能に利用できるようにすることです。

この設計では、意図的に `Tk_Main` を避けています。`Tk_Main` はプロセス寿命の多くを所有し、main 関数が終了するとプロセス全体を終了させる可能性があります。xer ではその代わりに、インタープリター作成、初期化、スクリプト評価、コマンド登録、変数アクセス、イベントループ補助を通常の C++ API として公開します。

---

## 主なエンティティ

少なくとも、`<xer/tk.h>` は次のエンティティを提供します。

```cpp
namespace xer::tk {

using result_code_t = int;
using eval_flag_t = int;
using var_flag_t = int;
using event_flag_t = int;

inline constexpr result_code_t result_ok;
inline constexpr result_code_t result_error;
inline constexpr result_code_t result_return;
inline constexpr result_code_t result_break;
inline constexpr result_code_t result_continue;

inline constexpr eval_flag_t eval_direct;

inline constexpr var_flag_t var_none;
inline constexpr var_flag_t var_global_only;
inline constexpr var_flag_t var_namespace_only;
inline constexpr var_flag_t var_leave_error_msg;
inline constexpr var_flag_t var_append_value;
inline constexpr var_flag_t var_list_element;

inline constexpr event_flag_t event_all;
inline constexpr event_flag_t event_window;
inline constexpr event_flag_t event_file;
inline constexpr event_flag_t event_timer;
inline constexpr event_flag_t event_idle;
inline constexpr event_flag_t event_dont_wait;

struct error_detail;
class interpreter;
class obj;
class photo_image;

using photo_composite_rule_t = int;
using photo_image_block = Tk_PhotoImageBlock;

inline constexpr photo_composite_rule_t photo_composite_overlay;
inline constexpr photo_composite_rule_t photo_composite_set;

struct photo_size {
    int width;
    int height;
};

auto find_executable() -> xer::result<void, error_detail>;
auto init(interpreter& interp) -> xer::result<void, error_detail>;
auto get_result(interpreter& interp) -> std::u8string;
auto reset_result(interpreter& interp) noexcept -> void;

auto eval(interpreter& interp, std::u8string_view script,
          eval_flag_t flags = eval_direct)
    -> xer::result<std::u8string, error_detail>;

auto make_string_obj(std::u8string_view value)
    -> xer::result<obj, error_detail>;
auto make_int_obj(int value) -> obj;
auto make_list_obj(std::span<const std::u8string_view> values)
    -> xer::result<obj, error_detail>;
auto make_list_obj(std::initializer_list<std::u8string_view> values)
    -> xer::result<obj, error_detail>;

auto set_var(interpreter& interp,
             std::u8string_view name,
             std::u8string_view value,
             var_flag_t flags = var_global_only)
    -> xer::result<void, error_detail>;

auto set_var(interpreter& interp,
             std::u8string_view name1,
             std::u8string_view name2,
             std::u8string_view value,
             var_flag_t flags = var_global_only)
    -> xer::result<void, error_detail>;

auto set_var(interpreter& interp,
             std::u8string_view name,
             obj& value,
             var_flag_t flags = var_global_only)
    -> xer::result<void, error_detail>;

auto set_var(interpreter& interp,
             std::u8string_view name1,
             std::u8string_view name2,
             obj& value,
             var_flag_t flags = var_global_only)
    -> xer::result<void, error_detail>;

auto get_var(interpreter& interp,
             std::u8string_view name,
             var_flag_t flags = var_global_only)
    -> xer::result<std::u8string, error_detail>;

auto get_var(interpreter& interp,
             std::u8string_view name1,
             std::u8string_view name2,
             var_flag_t flags = var_global_only)
    -> xer::result<std::u8string, error_detail>;

template <class F>
auto create_command(interpreter& interp,
                    std::u8string_view name,
                    F&& callable)
    -> xer::result<void, error_detail>;

auto to_native_handle(interpreter& interp) noexcept -> Tcl_Interp*;
auto to_native_handle(obj& value) noexcept -> Tcl_Obj*;
auto to_native_handle(photo_image image) noexcept -> Tk_PhotoHandle;

auto find_photo(interpreter& interp, const char8_t* name)
    -> xer::result<photo_image, error_detail>;

auto photo_get_size(photo_image image) noexcept -> photo_size;
auto photo_blank(photo_image image) noexcept -> void;

auto photo_expand(interpreter& interp, photo_image image, int width, int height)
    -> xer::result<void, error_detail>;

auto photo_set_size(interpreter& interp, photo_image image, int width, int height)
    -> xer::result<void, error_detail>;

auto photo_get_image(photo_image image, photo_image_block* block)
    -> xer::result<void, error_detail>;

auto photo_put_block(interpreter& interp,
                     photo_image image,
                     photo_image_block* block,
                     int x,
                     int y,
                     int width,
                     int height,
                     photo_composite_rule_t rule = photo_composite_set)
    -> xer::result<void, error_detail>;

auto photo_put_zoomed_block(interpreter& interp,
                            photo_image image,
                            photo_image_block* block,
                            int x,
                            int y,
                            int width,
                            int height,
                            int zoom_x,
                            int zoom_y,
                            int subsample_x,
                            int subsample_y,
                            photo_composite_rule_t rule = photo_composite_set)
    -> xer::result<void, error_detail>;

template <class F>
auto main(F&& callback) -> xer::result<void>;

auto main_loop() -> void;
auto do_one_event(event_flag_t flags = event_all) -> int;

}
```

Tcl/Tk 層の発展に伴い、対応する呼び出し可能オブジェクトの引数型と戻り値型の正確な集合は拡張される可能性があります。

---

## 名前空間

公開 Tcl/Tk 連携 API はすべて次の名前空間に配置されます。

```cpp
namespace xer::tk
```

初期公開 API では、独立した `xer::tcl` 名前空間は提供しません。Tcl は Tk 連携層で使われるスクリプト基盤として扱います。

将来のバージョンで Tcl 専用の公開ヘッダーを提供する場合は、その時点で独立した名前空間を再検討できます。

---

## 結果コードとフラグ

Tcl は `TCL_OK`、`TCL_ERROR`、`TCL_GLOBAL_ONLY` などの C マクロを通じて結果コードとフラグを公開します。

xer では、通常の利用者がそれらのマクロを直接参照する必要はありません。代わりに、`<xer/tk.h>` は次のような xer 名の定数を提供します。

```cpp
xer::tk::result_ok
xer::tk::result_error
xer::tk::result_break
xer::tk::var_global_only
xer::tk::eval_direct
xer::tk::event_all
```

これらの定数は、Tcl/Tk が使用するネイティブの整数値を保ちながら、公開 API の語彙を `xer::tk` の下に置きます。

---

## エラー詳細

```cpp
struct error_detail {
    result_code_t result_code;
};
```

`error_detail` は、失敗した操作に関連付けられた Tcl/Tk の結果コードを保持します。

`eval` では、保存されるコードは `Tcl_EvalObjEx` が `result_ok` 以外を返したときの戻り値です。null の Tcl 戻り値によって失敗する操作では、`result_error` が使用されます。

現在の Tcl 結果文字列は `error_detail` の内部には保存されません。呼び出し側が現在のインタープリター結果テキストを必要とする場合は、次を呼び出せます。

```cpp
auto get_result(interpreter& interp) -> std::u8string;
```

これにより、エラー詳細を小さく保ち、実際に必要な場合以外は Tcl 結果テキストのコピーを避けられます。

---

## `interpreter`

```cpp
class interpreter;
```

`interpreter` は、`Tcl_Interp*` を包むムーブ専用の RAII ラッパーです。

### 作成

```cpp
auto interpreter::create() -> xer::result<interpreter, error_detail>;
```

`interpreter::create` は、必要であれば Tcl 実行可能ファイル情報を初期化し、その後で新しい Tcl インタープリターを作成します。

通常のコードでは、この静的メンバー関数を通じてインタープリターを作成するべきです。

```cpp
auto interp = xer::tk::interpreter::create();
```

### 寿命

インタープリターは、ネイティブの `Tcl_Interp*` ハンドルを所有します。

`interpreter` オブジェクトが破棄されると、ネイティブインタープリターも削除されます。この型はムーブ専用であり、所有権が明示的に保たれるようにしています。

### 妥当性

```cpp
auto valid() const noexcept -> bool;
```

`valid()` は、ラッパーが現在ネイティブインタープリターハンドルを所有しているかどうかを返します。

---

## 初期化

```cpp
auto init(interpreter& interp) -> xer::result<void, error_detail>;
```

`init` は、指定されたインタープリターに対して Tcl と Tk の両方を初期化します。

この関数は `Tcl_Init` を呼び出し、その後で `Tk_Init` を呼び出します。xer は、`xer::tk` 層では `Tcl_Init` だけを呼び出す公開 API を意図的に提供していません。

呼び出し側が Tcl のみの初期化動作を直接必要とする場合は、`to_native_handle` でネイティブハンドルを取得し、Tcl C API を明示的に呼び出せます。

---

## ネイティブハンドルへの脱出口

```cpp
auto to_native_handle(interpreter& interp) noexcept -> Tcl_Interp*;
```

`to_native_handle` は、背後にある `Tcl_Interp*` を返します。

この関数は、Tcl/Tk C API を直接使うための脱出口です。`get` のような通常のアクセサ名には意図的にしていません。ネイティブハンドルを使うと、xer の抽象化の一部を迂回するためです。

`const interpreter&` 用のオーバーロードはありません。Tcl インタープリターハンドルは状態を変更する操作で使われることが多く、const オーバーロードを用意しても意味のある const 安全性は得られません。

---

## Photo Image 補助

`<xer/tk.h>` は、`Tk_FindPhoto`、`Tk_PhotoGetImage`、`Tk_PhotoPutBlock`、および関連するサイズ操作など、Tk photo image API の薄いラッパーを提供します。

これらの補助は、一般的な画像処理層ではなく、意図的に Tk 連携層の一部です。純粋な画像処理とフレームバッファ操作は `<xer/image.h>` に属し、`<xer/tk.h>` は Tk photo image への橋渡しだけを扱います。

### `photo_image`

```cpp
class photo_image;
```

`photo_image` は、既存の Tk photo image に対する非所有ハンドルです。

`photo_image` 値はデフォルト構築できません。これは `find_photo` によってのみ作成されるため、通常の xer コードでは null photo ハンドルを表しません。

この型は背後の Tk image を所有しません。呼び出し側は、その `photo_image` ハンドルが参照する Tcl/Tk photo image の寿命がハンドルより長く続くことを保証しなければなりません。Tcl/Tk 側で image が削除された場合、既存の `photo_image` はネイティブ Tk の寿命規則により無効になります。

ネイティブへの脱出口は次のとおりです。

```cpp
auto to_native_handle(photo_image image) noexcept -> Tk_PhotoHandle;
```

### Photo Image の検索

```cpp
auto find_photo(interpreter& interp, const char8_t* name)
    -> xer::result<photo_image, error_detail>;
```

`find_photo` は、既存の Tk photo image を名前で検索します。

`name` 引数は、ヌル終端 UTF-8 文字列です。これは、`const char*` の image 名を受け取るネイティブ Tk API に従っています。`eval` と違い、この関数は `std::u8string_view` を受け取りません。ネイティブ API が明示的な長さを持つオブジェクトではなく、ヌル終端文字列を要求するためです。

`name` が null の場合、この関数は `error_t::invalid_argument` で失敗します。指定された名前の image が存在しない、または photo image ではない場合、この関数は `error_t::not_found` で失敗します。

### サイズ操作

```cpp
struct photo_size {
    int width;
    int height;
};

auto photo_get_size(photo_image image) noexcept -> photo_size;

auto photo_expand(interpreter& interp, photo_image image, int width, int height)
    -> xer::result<void, error_detail>;

auto photo_set_size(interpreter& interp, photo_image image, int width, int height)
    -> xer::result<void, error_detail>;
```

`photo_get_size` は現在の Tk photo サイズを返します。

`photo_expand` は、photo image を少なくとも要求されたサイズまで拡張します。Tcl/Tk 側で photo image に明示的な `-width` または `-height` が設定されている場合、Tk はその明示的な寸法を拡張せず保持することがあります。

`photo_set_size` は、photo image の明示的なサイズを設定します。両方の寸法に 0 を渡すと、ネイティブ Tk API と同じ方法で明示的なサイズを解除するために使えます。

負の寸法は、Tk を呼び出す前に `error_t::invalid_argument` として拒否されます。

### クリア操作とブロック操作

```cpp
using photo_image_block = Tk_PhotoImageBlock;
using photo_composite_rule_t = int;

inline constexpr photo_composite_rule_t photo_composite_overlay;
inline constexpr photo_composite_rule_t photo_composite_set;

auto photo_blank(photo_image image) noexcept -> void;

auto photo_get_image(photo_image image, photo_image_block* block)
    -> xer::result<void, error_detail>;

auto photo_put_block(interpreter& interp,
                     photo_image image,
                     photo_image_block* block,
                     int x,
                     int y,
                     int width,
                     int height,
                     photo_composite_rule_t rule = photo_composite_set)
    -> xer::result<void, error_detail>;

auto photo_put_zoomed_block(interpreter& interp,
                            photo_image image,
                            photo_image_block* block,
                            int x,
                            int y,
                            int width,
                            int height,
                            int zoom_x,
                            int zoom_y,
                            int subsample_x,
                            int subsample_y,
                            photo_composite_rule_t rule = photo_composite_set)
    -> xer::result<void, error_detail>;
```

`photo_blank` は photo image を消去します。

`photo_get_image` は、現在の Tk photo image に対する `photo_image_block` 記述子を埋めます。このブロックは Tk が管理するメモリを指すため、Tk の寿命規則に従って扱う必要があります。

`photo_put_block` は、ブロックを photo image に書き込みます。

`photo_put_zoomed_block` は、整数ズームとサブサンプリングを適用しながらブロックを書き込みます。

ブロックポインターは null であってはいけません。座標、幅、高さは負であってはいけません。ズーム係数とサブサンプル係数は正でなければなりません。不正な引数は、Tk を呼び出す前に拒否されます。

### xer 画像機能との関係

photo block 補助は低水準の Tk ラッパーです。必要なときにコードがネイティブ Tk のブロックレイアウトを使えるように、`Tk_PhotoImageBlock` を `photo_image_block` という別名で意図的に公開しています。

Tk photo image と xer の image 型またはフレームバッファ型の間の高水準変換は、これらの補助の上に別途構築するべきです。通常の画像処理アルゴリズム自体は、Tcl/Tk に依存するべきではありません。

---

## Tcl オブジェクト

```cpp
class obj;

auto make_string_obj(std::u8string_view value)
    -> xer::result<obj, error_detail>;
auto make_int_obj(int value) -> obj;
auto make_list_obj(std::span<const std::u8string_view> values)
    -> xer::result<obj, error_detail>;
auto make_list_obj(std::initializer_list<std::u8string_view> values)
    -> xer::result<obj, error_detail>;

auto to_native_handle(obj& value) noexcept -> Tcl_Obj*;
```

`obj` は、`Tcl_Obj*` に対する RAII ラッパーです。ラップされたオブジェクトに対する Tcl 参照を 1 つ所有します。構築とコピーでは Tcl 参照カウントを増やし、破棄では参照カウントを減らし、ムーブでは Tcl 参照カウントを変えずに所有参照を移動します。

`obj` には、`Tcl_Obj*` からの公開コンストラクターはありません。通常のコードでは、`make_string_obj`、`make_int_obj`、`make_list_obj` などのファクトリ関数を通じてオブジェクトを作成するべきです。

`to_native_handle(obj&)` は借用された `Tcl_Obj*` を返し、Tcl 参照カウントを変更しません。返されたポインターを呼び出し側が解放してはいけません。

`make_list_obj` は本物の Tcl リストオブジェクトを作成します。値を手作業で結合した文字列ではなく Tcl リストとして振る舞わせる必要がある場合に有用です。

---

## Tcl 結果の扱い

```cpp
auto get_result(interpreter& interp) -> std::u8string;
auto reset_result(interpreter& interp) noexcept -> void;
```

`get_result` は、現在の Tcl インタープリター結果を UTF-8 テキストとして返します。内部では、`Tcl_GetObjResult` で結果オブジェクトを取得し、`Tcl_GetStringFromObj` でテキストに変換します。

`reset_result` は、現在のインタープリター結果をクリアします。

---

## スクリプト評価

```cpp
auto eval(interpreter& interp,
          std::u8string_view script,
          eval_flag_t flags = eval_direct)
    -> xer::result<std::u8string, error_detail>;
```

`eval` は、`Tcl_EvalObjEx` を使って Tcl スクリプトテキストを評価します。

スクリプトはまず Tcl オブジェクトに変換されます。成功時には、現在の Tcl 結果が `std::u8string` として返されます。失敗時には、返されるエラー詳細が Tcl 結果コードを保持します。

既定の評価フラグは `eval_direct` です。

---

## 変数アクセス

`<xer/tk.h>` は、Tcl オブジェクト API に基づく変数アクセスを提供します。

```cpp
auto set_var(interpreter& interp,
             std::u8string_view name,
             std::u8string_view value,
             var_flag_t flags = var_global_only)
    -> xer::result<void, error_detail>;

auto get_var(interpreter& interp,
             std::u8string_view name,
             var_flag_t flags = var_global_only)
    -> xer::result<std::u8string, error_detail>;
```

これらの関数は、`Tcl_ObjSetVar2` と `Tcl_ObjGetVar2` を使用します。

UTF-8 テキスト値に加えて、`set_var` は `xer::tk::obj` も受け取れます。これにより、呼び出し側はリストオブジェクトなどの Tcl オブジェクトを、手作業で結合した文字列に変換せずに代入できます。

配列変数形式も提供されます。

```cpp
auto set_var(interpreter& interp,
             std::u8string_view name1,
             std::u8string_view name2,
             std::u8string_view value,
             var_flag_t flags = var_global_only)
    -> xer::result<void, error_detail>;

auto get_var(interpreter& interp,
             std::u8string_view name1,
             std::u8string_view name2,
             var_flag_t flags = var_global_only)
    -> xer::result<std::u8string, error_detail>;
```

既定の変数フラグは `var_global_only` です。

たとえば、Tcl リストは次のように代入できます。

```cpp
auto list = xer::tk::make_list_obj({u8"first value", u8"second"});
if (!list.has_value()) {
    return 1;
}

if (!xer::tk::set_var(interp, u8"argv", *list).has_value()) {
    return 1;
}
```

その後 Tcl コードでは、`llength $argv` や `lindex $argv 0` などの通常のリスト操作を使用できます。

---

## コマンド登録

```cpp
template <class F>
auto create_command(interpreter& interp,
                    std::u8string_view name,
                    F&& callable)
    -> xer::result<void, error_detail>;
```

`create_command` は、C++の呼び出し可能オブジェクトを Tcl オブジェクトコマンドとして登録します。

コマンド名は UTF-8 テキストです。呼び出し可能オブジェクトは、関数オブジェクト、関数ポインター、ラムダ式のいずれでも構いません。

簡単な例は次のとおりです。

```cpp
auto created = xer::tk::create_command(
    interp,
    u8"add",
    [](int a, int b) -> int {
        return a + b;
    });
```

登録後、Tcl スクリプトは次のように呼び出せます。

```tcl
add 10 20
```

初期コマンドブリッジは、実用的な一部の引数型と戻り値型をサポートします。

対応するコールバック引数型には次が含まれます。

- `Tcl_Obj*`
- `xer::tk::obj`
- `bool`
- `int`
- `long`
- `long long`
- `unsigned int`
- `unsigned long`
- `unsigned long long`
- `float`
- `double`
- `long double`
- `std::u8string`
- `std::u8string_view`

対応するコールバック戻り値型には次が含まれます。

- `void`
- `bool`
- `int`
- `long`
- `long long`
- `unsigned int`
- `unsigned long`
- `unsigned long long`
- `float`
- `double`
- `long double`
- `std::u8string`
- `std::u8string_view`
- `const char8_t*`
- `Tcl_Obj*`
- `xer::tk::obj`
- `xer::result<T, xer::tk::error_detail>`
- `xer::result<T>` のうち実装が対応するもの
- `xer::tk::result_code_t`

未対応の場合、利用者は `Tcl_Obj*` またはネイティブ Tcl/Tk API にフォールバックできます。

### `std::u8string_view` コールバック引数

C++コマンドコールバックが `std::u8string_view` を受け取る場合、そのビューは対応する `Tcl_Obj` の文字列表現を参照します。
このビューは、コールバックの実行中に限って有効です。

値を保存する、キャプチャする、あとで返す、またはコールバックが戻った後で使用する必要がある場合、コールバックはそれを `std::u8string` などの所有オブジェクトにコピーしなければなりません。

コールバックから `std::u8string_view` を返す場合は話が異なります。xer は、コマンドハンドラーが戻る前に、参照されたテキストを Tcl インタープリター結果へコピーします。
返されるビューは、コマンドハンドラーが Tcl 結果の設定を終えるまで有効であれば十分です。

---

## Tcl/Tk main 補助

```cpp
template <class F>
auto main(F&& callback) -> xer::result<void>;
```

`xer::tk::main` は Tcl/Tk 実行ブロックであり、C++のグローバル `main` 関数を置き換えるものではありません。これはプログラムのメインスレッドから呼び出しても、利用者が管理する別スレッドから呼び出しても構いません。

コールバックは、基本的に次の形を持つべきです。

```cpp
auto callback(xer::tk::interpreter& interp) -> xer::result<void>;
```

この補助は、通常のセットアップ手順を実行します。実行可能ファイルの検出、インタープリター作成、Tcl/Tk 初期化、Tcl 起動変数の設定、コールバック呼び出し、そしてコールバックが成功した場合の `main_loop()` です。イベントループが戻った後、インタープリターは RAII によって破棄されます。

この補助は、Tcl スクリプトが通常の名前を使えるように Tcl 変数を設定します。

- `argc`
- `argv`
- `argv0`
- `env`

`argv` は本物の Tcl リストオブジェクトとして設定されます。`env` は Tcl 配列変数として設定されます。

---

## イベントループ

```cpp
auto main_loop() -> void;
auto do_one_event(event_flag_t flags = event_all) -> int;
```

`main_loop` は、`Tk_MainLoop` によって Tk のメインイベントループを実行します。

`do_one_event` は、`Tcl_DoOneEvent` によってイベントを 1 つ処理します。

xer は `Tk_Main` を使用しません。`Tk_Main` はプロセス寿命を制御しすぎるためです。プログラムは、インタープリターを明示的に作成・初期化し、その後で望むスレッド上でイベントループを実行するべきです。

---

## スレッド方針

xer は、利用者が選んだスレッドで Tcl/Tk を実行できるようにすることを目指します。

ただし、インタープリターはスレッドに属するものとして扱います。通常は、同じスレッドで作成、初期化、使用、破棄するべきです。

この規則は、評価、変数アクセス、コマンド登録、そのインタープリターに関連するイベントループ使用、破棄などの通常操作に適用されます。

つまり、xer は Tcl/Tk をメインスレッド以外で実行するという考え方をサポートしますが、1 つのインタープリターを任意のスレッドから自由に呼び出せるようにはしません。

安全なパターンは、ワーカースレッド内でインタープリターを作成し、そのワーカースレッド内だけで使用し、そのスレッドが終了する前に破棄させることです。

xer は現在、スレッド間呼び出し補助、イベント投稿補助、Tcl/Tk 用のスレッド安全なディスパッチキューを提供していません。

---

## 後回しにしている項目

次の項目は、初期 Tcl/Tk 層では意図的に後回しにしています。

* widget ラッパークラス
* `%w` などのコールバック置換値
* スレッド間呼び出し補助
* 完全なイベントディスパッチ抽象化
* TomMath 連携
* Tcl オブジェクト型の完全な網羅
* Tcl 専用の公開ヘッダーと名前空間分離

---

## 例

```cpp
#include <xer/stdio.h>
#include <xer/tk.h>

auto main() -> int
{
    auto interp = xer::tk::interpreter::create();
    if (!interp.has_value()) {
        return 1;
    }

    const auto command = xer::tk::create_command(
        *interp,
        u8"add",
        [](int a, int b) -> int {
            return a + b;
        });
    if (!command.has_value()) {
        return 1;
    }

    const auto result = xer::tk::eval(*interp, u8"add 10 20");
    if (!result.has_value()) {
        return 1;
    }

    if (!xer::printf(u8"%@\n", *result).has_value()) {
        return 1;
    }

    return 0;
}
```

この例では、Tcl インタープリターを作成し、C++の呼び出し可能オブジェクトを Tcl コマンドとして登録しています。GUI widget を使っていないため、Tk は初期化していません。

---

## 関連項目

* `policy_tk.md`
* `policy_project_outline.md`
* `policy_result_arguments.md`
* `header_cmdline.md`

---

# `<xer/stdint.h>`

## 目的

`<xer/stdint.h>` は、固定幅整数機能と、それに密接に関連する数値ユーティリティを xer で提供します。

その役割は C 標準ライブラリの `<stdint.h>` に近いものですが、単に整数 typedef を再公開するだけに限られません。
むしろ、xer 全体の設計に合う実用的な整数向けヘルパーの置き場所としても機能します。

このヘッダーは、特に次のものを提供するため重要です。

- 固定幅整数型の別名
- ポインターサイズ整数型
- 対応環境で利用できる任意の 128 ビット整数別名
- コンパイル時数値ヘルパー定数
- 型付き整数定数を便利に書くための整数リテラルサフィックス

---

## 主な役割

`<xer/stdint.h>` の主な役割は、xer の残りの部分で使う、安定した明示的な整数語彙を提供することです。

具体的には、次のことを簡単かつ明確にします。

- 明示的なサイズを持つ整数型でコードを書く
- ポインターサイズ整数など、実装サイズに依存する整数型を参照する
- 整数の限界値やビット幅を統一された xer らしい形で表現する
- ソースコード中で型付き整数リテラルを直接書く

これにより、このヘッダーは基礎的な型ヘッダーとしても、整数を多用するコード向けの実用ユーティリティヘッダーとしても有用になります。

---

## 主な要素

少なくとも、`<xer/stdint.h>` は次の種類の要素を提供します。

- 固定幅符号付き整数型
- 固定幅符号なし整数型
- 必要に応じた最小幅整数型および高速整数型
- ポインターサイズ整数型
- 最大幅整数型
- 利用可能な場合の任意の 128 ビット整数型
- コンパイル時ヘルパー定数
- ユーザー定義整数リテラルサフィックス

正確に公開される集合は実装とプロジェクト方針に従いますが、これらが意図している公開カテゴリです。

---

## 固定幅整数型

`<xer/stdint.h>` は、少なくとも次のような、なじみのある固定幅整数型を提供します。

```cpp
int8_t
int16_t
int32_t
int64_t

uint8_t
uint16_t
uint32_t
uint64_t
```

### これらの型の役割

これらの型は、整数値の幅が明示的に重要な場合に使います。

典型的な用途には次のものがあります。

* バイナリ形式
* プロトコル定義
* パックされたデータ構造
* 明示的なサイズ期待を伴う算術
* 通常の `int` の幅が実装定義であることに依存したくないクロスプラットフォームコード

### 注記

これらの名前は、C や C++ から来た利用者にとって素直でなじみやすいものを意図しています。

---

## ポインターサイズ整数型と最大幅整数型

このヘッダーは、ポインターサイズや実用上の最大幅に関連する整数型も提供します。

```cpp
intptr_t
uintptr_t
intmax_t
uintmax_t
```

### これらの型の役割

これらは、コードで次のことが必要な場合に有用です。

* 適切な場面でポインターを安全に整数形式へ変換する
* 実用上もっとも広い整数カテゴリについて考える
* 実装に適応する汎用数値コードを書く

### 注記

これらの型は、低レベルコードや実装補助コードで特に有用です。

---

## 任意の 128 ビット整数型

実装が `__int128` をサポートしている場合、xer は次を提供することがあります。

```cpp
int128_t
uint128_t
```

### これらの型の役割

これらの型は次の場合に有用です。

* 64 ビット範囲では不十分である
* 中間算術でオーバーフローを避けたい
* より大きな整数定数の型を明示したい

### 利用可能性

これらの型は実装依存です。

必要な基礎整数型をコンパイラとターゲットがサポートしている場合にのみ利用できます。

したがって、ドキュメントでは普遍的に保証されるものではなく、任意の機能として説明するべきです。

---

## コンパイル時数値ヘルパー

`<xer/stdint.h>` は、次のようなヘルパー定数も提供することがあります。

```cpp
min_of<T>
max_of<T>
bit_width_of<T>
```

### `min_of<T>`

`min_of<T>` は、整数型 `T` の最小値を表します。

### `max_of<T>`

`max_of<T>` は、整数型 `T` の最大値を表します。

### `bit_width_of<T>`

`bit_width_of<T>` は、`T` のビット幅を表します。

### これらのヘルパーの役割

これらのヘルパーは、整数型メタデータを、簡潔で読みやすく、xer として一貫した形で参照できるようにするためにあります。

特に次の場面で有用です。

* コンパイル時チェック
* 汎用数値ユーティリティ
* 範囲に敏感なコード
* ドキュメント例

### 設計方針

これらのヘルパーは単純なコンパイル時機能を意図しており、大きな抽象化層ではありません。

---

## 整数リテラルサフィックス

`<xer/stdint.h>` の利用者向け機能の中で特に目に見えやすいものの一つが、整数リテラルサフィックス群です。

少なくとも、xer は次のようなリテラルサフィックスを提供することがあります。

```cpp
_i8   _i16   _i32   _i64
_u8   _u16   _u32   _u64
_i128 _u128
```

これらは通常、次の名前空間下に置かれます。

```cpp
xer::literals::integer_literals
```

### これらのサフィックスの役割

これらのサフィックスにより、型付き整数定数を直接、読みやすく書けます。

例です。

```cpp
using namespace xer::literals::integer_literals;

constexpr auto x = 123_i32;
constexpr auto y = 255_u8;
```

これにより、意図した整数型が重要なコードで明瞭さが高まります。

### 設計方針

これらのリテラルサフィックスは、次の性質を意図しています。

* 明示的である
* 読みやすい
* テストや例で便利である
* コンパイル時文脈で有用である

明示性と型の明確さを重視する xer のようなプロジェクトでは、特に相性のよい機能です。

---

## 対応するリテラル形式

サフィックスの背後にあるリテラル解析機能は、少なくとも次のテキスト形式をサポートすることがあります。

* 10 進
* 8 進
* 16 進
* `0b...` による 2 進
* `'` による桁区切り

### 例

```cpp
using namespace xer::literals::integer_literals;

constexpr auto a = 123_i32;
constexpr auto b = 0xff_u32;
constexpr auto c = 0b1010_u8;
constexpr auto d = 1'000'000_i64;
```

### 注記

これらの機能は、特に次の用途で有用です。

* テスト
* バイナリやビット指向のコード
* 意図した数値型を例の中で明確に保ちたい場合

---

## 範囲チェック

整数リテラル機能の重要な設計点は、範囲チェックが明示的に行われることです。

### 意味

リテラルサフィックスが特定の変換先型を要求する場合、そのリテラルはその型に収まるべきです。

収まらない場合は、値を暗黙に縮小するのではなく、プログラムがコンパイルに失敗するべきです。

### これが重要な理由

これにより、型付き整数リテラルを信頼できるものにし、隠れた切り捨てを避けられます。

また、意外な暗黙動作よりも明示的な失敗を好む、xer のより広い設計方針とも一致します。

---

## 他のヘッダーとの関係

`<xer/stdint.h>` は次とあわせて理解してください。

* `policy_project_outline.md`
* `header_arithmetic.md`

大まかな境界は次のとおりです。

* `<xer/stdint.h>` は整数型、数値限界値やヘルパー、型付き整数リテラルを提供する
* `<xer/arithmetic.h>` は、明示的な数値型の上に構築される算術および比較ヘルパーを提供する

これにより `<xer/stdint.h>` は基礎となり、`<xer/arithmetic.h>` はより高水準の数値操作を扱います。

---

## xer の数値設計との関係

`<xer/stdint.h>` は基本的な型ヘッダーに見えますが、xer の数値設計では重要な役割を持ちます。

特に、次のことを明示する助けになります。

* 整数値の正確な型
* 定数の意図した幅
* 数値コードの範囲前提
* より広い整数型の実装依存の利用可能性

これは、型混在算術ヘルパーや明示的な範囲チェックも提供するライブラリでは特に価値があります。

---

## ドキュメント上の注記

このヘッダーを生成ドキュメントで扱う場合、通常は次の点を説明すれば十分です。

* 固定幅整数型と関連する別名を提供すること
* 対応環境では任意の 128 ビット別名を提供することがあること
* 単純なコンパイル時整数ヘルパーを含むこと
* 型付き整数定数のためのユーザー定義リテラルサフィックスを提供すること

リテラルサフィックスの詳細な解析規則や境界事例は、詳細リファレンスまたは生成された API 節に属します。

---

## 例として示すのに適した話題

このヘッダーでは、次のような例が特に適しています。

* 固定幅整数型で値を宣言する
* `bit_width_of<T>` を使う
* `_i32` や `_u64` で型付き整数リテラルを書く
* コンパイル時に範囲安全な定数式の使用を示す

これらは `examples/` 以下の実行可能サンプルのよい候補です。

---

## 例

```cpp
#include <xer/stdint.h>

using namespace xer::literals::integer_literals;

auto main() -> int
{
    constexpr auto x = 123_i32;
    constexpr auto y = 255_u16;

    static_assert(std::same_as<decltype(x), const xer::int32_t>);
    static_assert(std::same_as<decltype(y), const xer::uint16_t>);

    return 0;
}
```

この例は、通常の使い方を示しています。

* 明示的な xer 整数型を使う
* 型付き整数リテラルサフィックスを使う
* 整数幅をコード自体の中で見える状態に保つ

---

## 関連項目

* `policy_project_outline.md`
* `header_arithmetic.md`


---

## 整数リテラルサフィックス

整数リテラルサフィックスは次の名前空間で提供されます。

```cpp
xer::literals::integer_literals
```

固定幅サフィックスには次のものがあります。

```cpp
_i8   _i16   _i32   _i64
_u8   _u16   _u32   _u64
_i128 _u128  // when supported
```

最小幅サフィックスには次のものがあります。

```cpp
_il8   _il16   _il32   _il64
_ul8   _ul16   _ul32   _ul64
```

最小幅サフィックスは、対応する `int_leastN_t` または `uint_leastN_t` 型を生成します。正確な保存幅より、保証される最小範囲が重要な場合に有用です。

---

## 数値限界値ヘルパー

`min_of<T>` と `max_of<T>` は、共有の数値限界値ヘルパーを通じて実装されています。そのため、`<xer/stdfloat.h>` のような浮動小数点機能からも再利用できます。

`bit_width_of<T>` は、整数向けのビット幅問い合わせとして `<xer/stdint.h>` から引き続き利用できます。

---

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

---

# `<xer/arithmetic.h>`

## 目的

`<xer/arithmetic.h>` は、算術演算と比較の補助関数を提供します。

このヘッダーの目的は、組み込み演算子に別名を付けることではありません。符号付き整数型と符号無し整数型の混在、明示的な失敗処理との統合、範囲外の結果の可視化、xer 独自の数値規則に沿った比較など、通常の C++ 算術で起こりやすい問題を避けるための数値ユーティリティ層を提供します。

そのため、このヘッダーは xer の数値設計の中核に位置します。

---

## 主な役割

`<xer/arithmetic.h>` の主な役割は、次のような算術演算と比較演算を提供することです。

- 明示的であること
- 予測しやすいこと
- 生の組み込み演算子より安全に連鎖しやすいこと
- xer のエラーモデルと整合していること

特に、次の用途を扱いやすくします。

- 符号付き整数型と符号無し整数型を混在させた整数演算
- 範囲を意識した明示的な算術演算
- ジェネリックコードで使いやすい比較補助関数
- `min`, `max`, `clamp`, `in_range` などの実用的なユーティリティ

---

## 主な関数群

`<xer/arithmetic.h>` には、大きく分けて次の機能群があります。

- 整数算術補助関数
- 比較補助関数
- 浮動小数点数の近似比較
- 範囲・境界補助関数
- 絶対値補助関数
- 平方・立方補助関数
- 同じ設計に基づく浮動小数点数および複素数のサポート

---

## 整数算術補助関数

このヘッダーは、少なくとも次の整数向け算術補助関数を提供します。

```cpp
add
uadd
sub
usub
mul
umul
div
udiv
mod
umod
```

### このグループの役割

これらの関数は、C++ の通常の算術変換から自動的に継承される動作ではなく、明示的に設計された算術演算を提供します。

これは、符号付き整数型と符号無し整数型を混在させる場合に特に重要です。

### 設計方針

このグループの設計目標は次のとおりです。

- 数学的に意味がある場合は、符号付き・符号無しの混在入力を許す
- 結果が対象の結果領域に収まらない場合は明示的に失敗を返す
- 暗黙の狭小化や予期しないラップアラウンドを避ける
- 除算と剰余の動作を明示する

---

## `add`, `sub`, `mul`

### 符号付き領域の変種

次の補助関数は、概念的に符号付きの結果領域で動作します。

```cpp
add
sub
mul
```

整数値に対しては、通常、次を返します。

```cpp
xer::result<std::int64_t>
```

### 意味

- `add(a, b)` は加算を行います。
- `sub(a, b)` は減算を行います。
- `mul(a, b)` は乗算を行います。

### エラー処理

結果が意図した符号付き結果領域で表現できない場合、これらの関数は失敗を返します。

これにより、オーバーフローや範囲外の状況が明示されます。

---

## `uadd`, `usub`, `umul`

### 符号無し領域の変種

次の補助関数は、概念的に符号無しの結果領域で動作します。

```cpp
uadd
usub
umul
```

整数値に対しては、通常、次を返します。

```cpp
xer::result<std::uint64_t>
```

### 意味

- `uadd(a, b)` は符号無し結果領域で加算を行います。
- `usub(a, b)` は符号無し結果領域で減算を行います。
- `umul(a, b)` は符号無し結果領域で乗算を行います。

### エラー処理

数学的に選ばれた結果が意図した符号無し結果領域で表現できない場合、これらの関数は失敗を返します。

たとえば、`usub` や `umul` で負の結果になる場合はエラーです。

---

## `div`, `udiv`, `mod`, `umod`

このヘッダーは、除算と剰余の補助関数も提供します。

```cpp
div
udiv
mod
umod
```

### `div`

`div` は符号付き結果領域で除算を行います。

整数入力では、次の規則に従います。

- 商は 0 方向に丸められます。
- 0 除算はエラーです。
- 範囲外の結果はエラーです。
- 剰余出力もサポートされる場合があります。

### `udiv`

`udiv` は符号無し結果領域で除算を行います。

整数入力では、次の規則に従います。

- 0 除算はエラーです。
- 範囲外の結果はエラーです。
- 剰余出力もサポートされる場合があります。

### `mod`

`mod` は `div` と同じ規則系に基づく符号付き剰余を返します。

### `umod`

`umod` は `udiv` と同じ規則系に基づく符号無し剰余を返します。

### 重要性

これらの補助関数は、商と剰余の動作を明示し、組み込み演算子の動作に任せきるのではなく、xer 独自の算術ポリシーと整合させます。

---

## 比較補助関数

`<xer/arithmetic.h>` は、少なくとも次の比較補助関数を提供します。

```cpp
eq
ne
lt
le
gt
ge
```

### このグループの役割

これらの関数は、xer の数値規則に合う形で明示的な比較を提供します。

特に次の場合に有用です。

- 異なる整数型が混在する
- ジェネリックコードで組み込み比較演算子に直接依存したくない
- 算術補助関数全体で一貫した比較層を使いたい

### 戻り値型

比較補助関数は通常 `bool` を返します。

比較は通常の失敗を表す操作ではありません。比較可能な入力に対しては、結果は `true` または `false` のどちらかです。

### `xer::result<bool>` を使わない理由

これらの比較補助関数は、通常の範囲外エラーやオーバーフローを報告するための API ではありません。比較の意味は型と値から決まり、失敗を通常の制御フローとして返す必要はありません。

比較が成り立たない型の組み合わせは、原則としてコンパイル時の問題として扱われます。

---

## 浮動小数点数の近似比較

このヘッダーは、浮動小数点数の近似比較を行う補助関数を提供します。

### `is_close`

`is_close` は、2つの浮動小数点値が指定した許容範囲内で十分近いかを判定します。

概念的には次のような用途に使います。

```cpp
xer::is_close(a, b, tolerance)
```

`a` と `b` の差が許容値以下であれば `true` を返します。

この関数は、丸め誤差を含む計算結果を比較するための実用的な補助関数です。厳密な等価比較の代替として、数値計算やテストで使うことを想定しています。

### 丸め余裕

浮動小数点演算では、数学的には等しい値でも、丸め誤差によって完全には一致しないことがあります。

`is_close` は、そのような場合に「十分近い」ことを明示的に判定するための関数です。

### 無効値

`NaN` などの無効値は、近似比較の対象として特別な注意が必要です。具体的な扱いは実装の仕様に従いますが、通常の用途では有限値同士の比較を想定します。

### `xer::result` 引数

`<xer/arithmetic.h>` は、xer の中でも特別に `xer::result` を引数として扱う関数を持つ場合があります。これは、算術演算を連鎖させるための実用上の例外です。

### 名前

近似比較の関数名は、意味が曖昧になりにくいように `is_close` としています。

---

## 範囲・境界補助関数

このヘッダーは、範囲や境界を扱う補助関数を提供します。

```cpp
in_range
min
max
clamp
```

### `in_range`

`in_range<T>(value)` は、`value` が型 `T` の表現可能範囲に収まるかどうかを判定します。

これは、明示的なキャストや代入の前に、値が安全に収まるかを確認したい場合に有用です。

### `min` と `max`

`min` と `max` は、値の大小関係に基づいて小さい方または大きい方を返します。

xer の比較規則に沿うことで、異なる数値型が混在する場合でも、組み込み演算子の暗黙変換に起因する驚きを減らします。

### `clamp`

`clamp(value, low, high)` は、`value` を `[low, high]` の範囲に収めます。

- `value < low` なら `low`
- `value > high` なら `high`
- それ以外なら `value`

を返します。

---

## 絶対値補助関数

このヘッダーは、絶対値を扱う補助関数を提供します。

```cpp
abs
uabs
```

### `abs`

`abs` は、符号付き領域で絶対値を返します。

結果が表現できない場合は失敗を返します。たとえば、2の補数表現で最小の負数を正に反転できない場合が該当します。

### `uabs`

`uabs` は、絶対値を符号無し領域で返します。

符号付き最小値のように、符号付き領域では表現できない絶対値も、符号無し領域では表現できる場合があります。

### 重要性

絶対値は単純に見えますが、整数型では最小値の扱いに落とし穴があります。これらの補助関数は、その問題を明示的に扱うために用意されています。

---

## 平方・立方補助関数

このヘッダーは、平方と立方を計算する補助関数も提供します。

```cpp
sq
cb
```

### `sq`

`sq(value)` は `value * value` を表します。

### `cb`

`cb(value)` は `value * value * value` を表します。

### 連鎖利用

これらの関数は、算術補助関数の結果を扱いやすくし、式の意図を読み取りやすくするための小さなユーティリティです。

---

## `xer::result` の受け入れ

### このヘッダーが特別である理由

通常の xer の公開 API は、`xer::result` を引数として受け取ることを避けます。失敗は呼び出し側で明示的に処理するのが基本です。

しかし算術補助関数では、演算を安全に連鎖したい場面が多いため、`xer::result` を受け取る特別な設計が採用される場合があります。

### 意味

前段の演算が失敗している場合、後続の演算もその失敗を伝播します。前段が成功している場合は、その値を使って演算を続けます。

### 重要な境界

この設計は算術補助関数のための実用的な例外です。すべての公開 API が `xer::result` 引数を受け取るべきだという意味ではありません。

---

## 浮動小数点数のサポート

### 一般方針

浮動小数点数についても、整数と同じく、明示的で読みやすい補助関数を提供します。

ただし、浮動小数点数ではオーバーフロー、無限大、NaN、丸め誤差など、整数とは異なる性質があります。そのため、整数演算と完全に同じ意味にはなりません。

### 重要性

同じ名前の補助関数で整数と浮動小数点数の両方を扱えると、ジェネリックコードで使いやすくなります。一方で、比較やエラー処理ではそれぞれの型の性質を尊重します。

---

## 複素数のサポート

### 一般方針

複素数についても、可能な範囲で同じ算術補助関数の設計に含めます。

加算、減算、乗算、除算などは自然に定義できます。

### 比較が異なる理由

複素数には自然な大小関係がありません。そのため、`lt` や `gt` のような順序比較は通常の実数とは同じようには扱えません。

等価性と順序性は別の問題として扱います。

---

## 算術と比較の関係

### 意味

算術補助関数と比較補助関数は、どちらも xer の数値規則を支える層です。

算術は値を生成し、比較は値の関係を判定します。どちらも、型変換や範囲の扱いをできるだけ明示的にします。

### 重要性

この関係をそろえることで、数値 API 全体の予測可能性が高まります。

---

## 他のヘッダーとの関係

`<xer/arithmetic.h>` は次のヘッダーやポリシーと関係します。

- `<xer/error.h>`
- `<xer/quantity.h>`
- `<xer/interval.h>`
- `<xer/cyclic.h>`
- `policy_arithmetic.md`
- `policy_project_outline.md`

---

## ドキュメント上の注意

このヘッダーを生成リファレンスで扱う場合は、次の点を説明するのが重要です。

- 組み込み演算子の単なる別名ではないこと
- 符号付き・符号無しの混在を明示的に扱うこと
- 範囲外やオーバーフローを `xer::result` で表すこと
- 比較補助関数は通常 `bool` を返すこと
- `xer::result` 引数を受け取るのは算術連鎖のための限定的な例外であること

---

## 例として示す価値がある題材

このヘッダーでは、次のような例が適しています。

- `add` や `sub` による範囲安全な整数演算
- `uabs` による安全な絶対値取得
- `is_close` による浮動小数点数の近似比較
- `in_range` による代入前チェック
- `min`, `max`, `clamp` の異種数値型での利用

---

## 例

```cpp
#include <xer/arithmetic.h>

#include <cstdint>

auto main() -> int
{
    const auto sum = xer::add(std::int64_t{40}, std::uint64_t{2});
    if (!sum.has_value()) {
        return 1;
    }

    if (*sum != 42) {
        return 1;
    }

    if (!xer::is_close(1.0, 1.0 + 1e-9, 1e-8)) {
        return 1;
    }

    return 0;
}
```

---

## 関連項目

- `policy_arithmetic.md`
- `policy_project_outline.md`
- `header_error.md`
- `header_quantity.md`
- `header_interval.md`
- `header_cyclic.md`

---

# `<xer/math.h>`

## 目的

`<xer/math.h>` は、xer の軽量な実数数学ヘルパーを提供します。

初期の対象範囲は、意図的に実用的な範囲に限定されています。これは数値解析ライブラリの代替ではありません。振る舞いとエラー処理が明示的な、小さくてよく使うヘルパーを提供します。

---

## 型

### `vec`

```cpp
template<class T, std::size_t N = 2>
struct vec;
```

小さな位置ベクトルまたは数学的なベクトルを表します。

提供されるのは `N == 2`、`N == 3`、`N == 4` のみです。`<xer/math.h>` は任意次元の汎用ベクトル型を提供することを目的としていないため、主テンプレートは意図的に定義されていません。

各特殊化は、名前付きの座標メンバーを提供します。

| 型 | メンバー |
|---|---|
| `vec<T, 2>` | `x`, `y` |
| `vec<T, 3>` | `x`, `y`, `z` |
| `vec<T, 4>` | `x`, `y`, `z`, `w` |

`vec<T>` は `vec<T, 2>` と同じです。

各特殊化は、範囲チェックを行わない添字アクセスも提供します。

```cpp
auto operator[](std::size_t index) noexcept -> T&;
auto operator[](std::size_t index) const noexcept -> const T&;
```

`operator[]` は範囲チェックを行いません。範囲外の添字を渡した場合の動作は未定義です。

チェック付きアクセスには `at` を使用します。

```cpp
auto at(std::size_t index) noexcept
    -> xer::result<std::reference_wrapper<T>>;

auto at(std::size_t index) const noexcept
    -> xer::result<std::reference_wrapper<const T>>;
```

`index` が範囲外の場合、`at` は `error_t::out_of_range` を返します。

### `polar`

```cpp
template<class T, std::size_t N = 2>
struct polar;
```

極座標を表します。

現在提供されるのは `polar<T, 2>` のみです。

```cpp
template<class T>
struct polar<T, 2> {
    T r;
    cyclic<T> theta;
};
```

`theta` は τrad で表された循環角です。`1` は 1 回転を意味します。

---

## 提供される関数

### 三角関数

```cpp
template<std::floating_point T>
auto sin(T theta) noexcept -> T;

template<std::floating_point T>
auto sin(cyclic<T> theta) noexcept -> T;

template<std::floating_point T>
auto cos(T theta) noexcept -> T;

template<std::floating_point T>
auto cos(cyclic<T> theta) noexcept -> T;

template<std::floating_point T>
auto tan(T theta) noexcept -> T;

template<std::floating_point T>
auto tan(cyclic<T> theta) noexcept -> T;
```

xer の角度単位を使って通常の三角関数を計算します。

スカラー版は `theta` を τrad 値として解釈します。`1` は 1 回転を意味します。`cyclic<T>` 版は、循環角の正規化済みの値を使用します。

例:

```text
sin(0.25) == 1
cos(0.5) == -1
tan(0.125) == 1
```

双曲線関数は、意図的に `<xer/math.h>` では提供しません。それらの関数が必要な場合は、C++ 標準ライブラリを直接使用してください。

### 逆三角関数

```cpp
template<std::floating_point T>
auto asin(T value) noexcept -> T;

template<std::floating_point T>
auto acos(T value) noexcept -> T;

template<std::floating_point T>
auto atan(T value) noexcept -> T;

template<std::floating_point T>
auto atan2(T y, T x) noexcept -> T;
```

逆三角関数を計算し、τrad のスカラー値を返します。

典型的な戻り値の範囲は、対応する標準ライブラリ関数の戻り値をラジアンから τrad に変換したものに従います。

| 関数 | 典型的な戻り値の範囲 |
|---|---|
| `asin` | `[-0.25, 0.25]` |
| `acos` | `[0, 0.5]` |
| `atan` | `(-0.25, 0.25)` |
| `atan2` | `[-0.5, 0.5]` |

定義域エラーについては、これらの関数は基になる標準ライブラリ関数の振る舞いに従います。たとえば不正な浮動小数点入力に対して NaN を返します。これらの関数は `xer::result` を返しません。

### `to_polar`

```cpp
template<std::floating_point T>
auto to_polar(vec<T, 2> v) noexcept -> polar<T, 2>;
```

2 次元直交座標ベクトルを極座標に変換します。

返される半径は `std::hypot(v.x, v.y)` で計算されます。返される角度は `std::atan2(v.y, v.x)` で計算され、τrad の循環値に変換されます。

### `to_cartesian`

```cpp
template<std::floating_point T>
auto to_cartesian(polar<T, 2> p) noexcept -> vec<T, 2>;
```

2 次元極座標を直交座標ベクトルに変換します。

座標成分は次のように計算されます。

```text
x = r * cos(theta * τ)
y = r * sin(theta * τ)
```

### `dot`

```cpp
template<class T, std::size_t N>
auto dot(vec<T, N> a, vec<T, N> b) noexcept -> T;
```

2 つのベクトルの内積を計算します。

`T` は算術型でなければなりません。`N` は対応している `vec` の次元、つまり `2`、`3`、`4` のいずれかでなければなりません。

この関数は、対応する成分どうしの積の総和を返します。整数ベクトルの場合、戻り値も整数です。

### `length`

```cpp
template<class T, std::size_t N>
auto length(vec<T, N> v) noexcept -> std::common_type_t<T, double>;
```

ベクトルのユークリッド長を計算します。

`T` は算術型でなければなりません。`N` は対応している `vec` の次元、つまり `2`、`3`、`4` のいずれかでなければなりません。

戻り値の型は `std::common_type_t<T, double>` です。そのため、整数ベクトルでも浮動小数点の長さが得られます。

### `distance`

```cpp
template<class T, std::size_t N>
auto distance(vec<T, N> a, vec<T, N> b) noexcept -> std::common_type_t<T, double>;
```

2 つのベクトル間のユークリッド距離を計算します。位置は位置ベクトルとして表されるため、この関数は 2 点間の距離の計算にも使用できます。

`T` は算術型でなければなりません。`N` は対応している `vec` の次元、つまり `2`、`3`、`4` のいずれかでなければなりません。

戻り値の型は `std::common_type_t<T, double>` です。そのため、整数ベクトルでも浮動小数点の距離が得られます。

### `normalize`

```cpp
template<class T, std::size_t N>
auto normalize(vec<T, N> v) noexcept
    -> xer::result<vec<std::common_type_t<T, double>, N>>;
```

`v` と同じ向きで長さが `1` のベクトルを返します。

`T` は算術型でなければなりません。`N` は対応している `vec` の次元、つまり `2`、`3`、`4` のいずれかでなければなりません。

返されるベクトルは、成分型として `std::common_type_t<T, double>` を使用します。そのため、整数ベクトルでも小数成分を失わずに正規化できます。

`v` がゼロベクトルの場合、この関数は `error_t::invalid_argument` を返します。

### `angle`

```cpp
template<class T, std::size_t N>
auto angle(vec<T, N> a, vec<T, N> b) noexcept
    -> xer::result<std::common_type_t<T, double>>;
```

2 つのベクトルのなす角の大きさを τrad で計算します。

`T` は算術型でなければなりません。`N` は対応している `vec` の次元、つまり `2`、`3`、`4` のいずれかでなければなりません。

戻り値の型は `std::common_type_t<T, double>` です。そのため、整数ベクトルでも浮動小数点の角度が得られます。返される値は角度の大きさであるため、`cyclic<T>` ではありません。範囲は `0` から `0.5` で、`0.25` は直角、`0.5` は平角です。

どちらかのベクトルがゼロベクトルの場合、この関数は `error_t::invalid_argument` を返します。

### `rotate`

```cpp
template<class T, std::floating_point Angle>
auto rotate(vec<T, 2> v, cyclic<Angle> theta) noexcept
    -> vec<std::common_type_t<T, Angle, double>, 2>;
```

2 次元ベクトルを原点の周りで回転します。

`T` は算術型でなければなりません。`theta` は τrad で表された循環角です。`1` は 1 回転を意味します。

戻り値の型は、成分型として `std::common_type_t<T, Angle, double>` を使用します。そのため、整数ベクトルでも小数成分を失わずに回転できます。

正の角度は、通常の数学的座標系で反時計回りに回転します。

### `cross`

```cpp
template<class T>
auto cross(vec<T, 3> a, vec<T, 3> b) noexcept -> vec<T, 3>;
```

2 つの 3 次元ベクトルの外積を計算します。

`T` は算術型でなければなりません。この関数は現在、`vec<T, 3>` のみに対応しています。

### `heron`

```cpp
template<std::floating_point T>
auto heron(T a, T b, T c) -> xer::result<T>;
```

ヘロンの公式を使って、3 辺の長さから三角形の面積を計算します。

辺の長さは非負であり、三角形を作れるものでなければなりません。辺の長さが負の場合、または辺の長さが三角不等式に反する場合、この関数は `error_t::invalid_argument` を返します。退化した三角形は受け入れられ、0 を返します。

実装では、辺の長さをソートしたあと、並べ替えた形のヘロンの公式を使用します。これにより、最も直接的な `s * (s - a) * (s - b) * (s - c)` の形と比べて、避けられる桁落ちをいくらか回避できます。

### `quadratic`

```cpp
template<std::floating_point T>
auto quadratic(T a, T b, T c)
    -> xer::result<std::array<std::optional<T>, 2>>;
```

次の二次方程式を解きます。

```text
a * x * x + b * x + c == 0
```

係数 `a` は 0 であってはなりません。`a == 0` の場合、この関数は `error_t::invalid_argument` を返します。

返される配列には、先頭要素から順に相異なる実根が格納されます。

| 結果 | 意味 |
|---|---|
| `{ nullopt, nullopt }` | 実根なし |
| `{ x, nullopt }` | 重解を含む 1 個の実根 |
| `{ x1, x2 }` | 2 個の相異なる実根 |

2 個の実根が返される場合、それらは昇順に並べられます。

### `cubic`

```cpp
template<std::floating_point T>
auto cubic(T a, T b, T c, T d)
    -> xer::result<std::array<std::optional<T>, 3>>;
```

次の三次方程式を解きます。

```text
a * x * x * x + b * x * x + c * x + d == 0
```

係数 `a` は 0 であってはなりません。`a == 0` の場合、この関数は `error_t::invalid_argument` を返します。

返される配列には、先頭要素から順に相異なる実根が格納されます。空の要素は `std::nullopt` で表されます。返される根は昇順に並べられます。

---

## 設計メモ

`quadratic` と `cubic` は実根のみを返します。非実根はエラーではありません。単に返される optional 配列には格納されません。

複素根が必要な場合は、`<xer/complex.h>` を使用してください。

重根は実数関数では 1 回だけ表されます。これにより、交点、衝突時刻、実数領域での制約のような一般的な実用チェックに使いやすい結果になります。

## 座標とベクトルの方針

位置は位置ベクトルとして表します。`<xer/math.h>` は、数学的な座標用の独立した点型を提供しません。

`vec` 型は 2 次元、3 次元、4 次元に限定されています。これにより、この機能が汎用線形代数パッケージになるのではなく、実用的な幾何、グラフィックス、座標変換、単純な物理計算に焦点を合わせたものになります。

---

# `<xer/statistics.h>`

## 目的

`<xer/statistics.h>` は、算術値の範囲に対する小さな記述統計ユーティリティ関数を提供します。

初期の対象範囲は、一般的な記述統計に意図的に限定されています。

- 合計
- 積
- 算術平均
- 幾何平均
- 調和平均
- 中央値
- 分位数
- パーセンタイル
- 最頻値
- 母分散
- 標本分散
- 母標準偏差
- 標本標準偏差

スカラー関数は `xer::result<double>` を返すため、不正な入力は明示的に報告されます。
`mode` は、複数の値が最高頻度を共有できるため、`xer::result<std::vector<double>>` を返します。

---

## 提供される関数

```cpp
template<class Range>
auto mean(Range&& range) -> xer::result<double>;

template<class T>
auto mean(std::initializer_list<T> values) -> xer::result<double>;

template<class Range>
auto sum(Range&& range) -> xer::result<double>;

template<class T>
auto sum(std::initializer_list<T> values) -> xer::result<double>;

template<class Range>
auto product(Range&& range) -> xer::result<double>;

template<class T>
auto product(std::initializer_list<T> values) -> xer::result<double>;

template<class Range>
auto geometric_mean(Range&& range) -> xer::result<double>;

template<class T>
auto geometric_mean(std::initializer_list<T> values) -> xer::result<double>;

template<class Range>
auto harmonic_mean(Range&& range) -> xer::result<double>;

template<class T>
auto harmonic_mean(std::initializer_list<T> values) -> xer::result<double>;

template<class Range>
auto median(Range&& range) -> xer::result<double>;

template<class T>
auto median(std::initializer_list<T> values) -> xer::result<double>;

template<class Range>
auto quantile(Range&& range, double q) -> xer::result<double>;

template<class T>
auto quantile(std::initializer_list<T> values, double q) -> xer::result<double>;

template<class Range>
auto percentile(Range&& range, double p) -> xer::result<double>;

template<class T>
auto percentile(std::initializer_list<T> values, double p) -> xer::result<double>;

template<class Range>
auto mode(Range&& range, double tolerance = 0.0) -> xer::result<std::vector<double>>;

template<class T>
auto mode(std::initializer_list<T> values, double tolerance = 0.0) -> xer::result<std::vector<double>>;

template<class Range>
auto variance(Range&& range) -> xer::result<double>;

template<class T>
auto variance(std::initializer_list<T> values) -> xer::result<double>;

template<class Range>
auto sample_variance(Range&& range) -> xer::result<double>;

template<class T>
auto sample_variance(std::initializer_list<T> values) -> xer::result<double>;

template<class Range>
auto stddev(Range&& range) -> xer::result<double>;

template<class T>
auto stddev(std::initializer_list<T> values) -> xer::result<double>;

template<class Range>
auto sample_stddev(Range&& range) -> xer::result<double>;

template<class T>
auto sample_stddev(std::initializer_list<T> values) -> xer::result<double>;
```

各範囲版は、参照型が `bool` ではない算術型である入力範囲を受け取ります。
初期化子リスト版は、次のような呼び出しが自然に動作するように提供されています。

```cpp
auto value = xer::mean({1.0, 2.0, 3.0});
```

---

## 平均

```cpp
template<class Range>
auto mean(Range&& range) -> xer::result<double>;
```

入力値の算術平均を計算します。

範囲には少なくとも 1 個の値が含まれていなければなりません。
範囲が空の場合、この関数は `error_t::invalid_argument` を返します。

---

## 合計

```cpp
template<class Range>
auto sum(Range&& range) -> xer::result<double>;
```

入力値の合計を計算します。

範囲には少なくとも 1 個の値が含まれていなければなりません。
範囲が空の場合、この関数は `error_t::invalid_argument` を返します。

---

## 積

```cpp
template<class Range>
auto product(Range&& range) -> xer::result<double>;
```

入力値の積を計算します。

範囲には少なくとも 1 個の値が含まれていなければなりません。
範囲が空の場合、この関数は `error_t::invalid_argument` を返します。

---

## 幾何平均

```cpp
template<class Range>
auto geometric_mean(Range&& range) -> xer::result<double>;
```

入力値の幾何平均を計算します。

すべての入力値は有限で非負でなければなりません。
入力値のいずれかが 0 の場合、結果は 0 になります。
入力値のいずれかが負値、NaN、無限大の場合、この関数は `error_t::invalid_argument` を返します。

範囲には少なくとも 1 個の値が含まれていなければなりません。
範囲が空の場合、この関数は `error_t::invalid_argument` を返します。

---

## 調和平均

```cpp
template<class Range>
auto harmonic_mean(Range&& range) -> xer::result<double>;
```

入力値の調和平均を計算します。

すべての入力値は有限で正でなければなりません。
入力値のいずれかが 0、負値、NaN、無限大の場合、この関数は `error_t::invalid_argument` を返します。

範囲には少なくとも 1 個の値が含まれていなければなりません。
範囲が空の場合、この関数は `error_t::invalid_argument` を返します。

---

## 中央値

```cpp
template<class Range>
auto median(Range&& range) -> xer::result<double>;
```

入力値の中央値を計算します。

入力値は内部でコピーされ、ソートされます。
値の個数が奇数の場合、中央の値が返されます。
値の個数が偶数の場合、中央の 2 値の算術平均が返されます。

範囲には少なくとも 1 個の値が含まれていなければなりません。
範囲が空の場合、この関数は `error_t::invalid_argument` を返します。

---

## 分位数

```cpp
template<class Range>
auto quantile(Range&& range, double q) -> xer::result<double>;
```

入力値の分位数を計算します。

入力値は内部でコピーされ、ソートされます。
分位数の比率 `q` は有限で、範囲 `[0.0, 1.0]` 内になければなりません。

補間規則は、ソート済み列に対する線形補間です。

```text
position = q * (n - 1)
result = values[floor(position)] * (1 - fraction)
       + values[ceil(position)]  * fraction
```

ここで `fraction` は `position` の小数部分です。

これは次を意味します。

```text
quantile(values, 0.0) == 最小値
quantile(values, 0.5) == 中央値
quantile(values, 1.0) == 最大値
```

範囲には少なくとも 1 個の値が含まれていなければなりません。
範囲が空の場合、この関数は `error_t::invalid_argument` を返します。

`q` が `[0.0, 1.0]` の外側、NaN、または無限大の場合、この関数は `error_t::invalid_argument` を返します。

---

## パーセンタイル

```cpp
template<class Range>
auto percentile(Range&& range, double p) -> xer::result<double>;
```

入力値のパーセンタイルを計算します。

パーセンタイル値 `p` は有限で、範囲 `[0.0, 100.0]` 内になければなりません。
この関数は `quantile` と同じ補間規則を使用します。

```text
percentile(values, p) == quantile(values, p / 100.0)
```

これは次を意味します。

```text
percentile(values, 0.0)   == 最小値
percentile(values, 50.0)  == 中央値
percentile(values, 100.0) == 最大値
```

範囲には少なくとも 1 個の値が含まれていなければなりません。
範囲が空の場合、この関数は `error_t::invalid_argument` を返します。

`p` が `[0.0, 100.0]` の外側、NaN、または無限大の場合、この関数は `error_t::invalid_argument` を返します。

---

## 最頻値

```cpp
template<class Range>
auto mode(Range&& range, double tolerance = 0.0) -> xer::result<std::vector<double>>;
```

入力値の最頻値を計算します。

複数の値が最高頻度を共有できるため、結果は vector です。
2 回以上現れる値がない場合、結果は空の vector です。
つまり、`mode({1, 2, 3})` はすべての値を最頻値として扱うのではなく、成功して空の vector を返します。

`tolerance` が `0.0` の場合、値はソート後に完全一致でグループ化されます。
`tolerance` が正の場合、現在のグループの先頭値からの距離が `tolerance` 以下であるソート済みの値は、同じグループとして扱われます。
許容幅付きグループで返される代表値は、そのグループ内の値の算術平均です。

例:

```cpp
auto values = std::vector<double>{1.00, 1.02, 1.04, 2.00, 2.04};
auto modes = xer::mode(values, 0.05);
```

最初の 3 値が同じグループになり、返される最頻値はおよそ `1.02` です。

`tolerance` は有限で非負でなければなりません。
`tolerance` が負値、NaN、または無限大の場合、この関数は `error_t::invalid_argument` を返します。

---

## 母分散

```cpp
template<class Range>
auto variance(Range&& range) -> xer::result<double>;
```

母分散を計算します。

母分散は、偏差平方和を `n` で割ります。

```text
sum((x - mean)^2) / n
```

この関数は少なくとも 1 個の値を必要とします。
値が 1 個の場合、母分散は `0` です。

---

## 標本分散

```cpp
template<class Range>
auto sample_variance(Range&& range) -> xer::result<double>;
```

標本分散を計算します。

標本分散は、偏差平方和を `n - 1` で割ります。

```text
sum((x - mean)^2) / (n - 1)
```

この関数は少なくとも 2 個の値を必要とします。
2 個未満の値が渡された場合、この関数は `error_t::invalid_argument` を返します。

---

## 標準偏差

```cpp
template<class Range>
auto stddev(Range&& range) -> xer::result<double>;

template<class Range>
auto sample_stddev(Range&& range) -> xer::result<double>;
```

`stddev` は母標準偏差を計算します。
これは `variance` の平方根です。

`sample_stddev` は標本標準偏差を計算します。
これは `sample_variance` の平方根です。

---

## エラー処理

統計関数は、不正な入力を `xer::result` を通じて報告します。

| 条件 | エラー |
|---|---|
| 空の範囲 | `error_t::invalid_argument` |
| 標本分散 / 標本標準偏差で値が 2 個未満 | `error_t::invalid_argument` |
| 入力内の NaN または無限大 | `error_t::invalid_argument` |
| `geometric_mean` への負の入力 | `error_t::invalid_argument` |
| `harmonic_mean` への 0 または負の入力 | `error_t::invalid_argument` |
| `quantile` の比率が `[0.0, 1.0]` の外側、NaN、または無限大 | `error_t::invalid_argument` |
| `percentile` の値が `[0.0, 100.0]` の外側、NaN、または無限大 | `error_t::invalid_argument` |
| `mode` の `tolerance` が負値、NaN、または無限大 | `error_t::invalid_argument` |
| 中間値または最終値が `double` で表現できる範囲の外側 | `error_t::range_error` |

---

## 数値的振る舞い

`sum`、`product`、`mean`、`geometric_mean`、`harmonic_mean`、`variance`、`sample_variance`、`stddev`、`sample_stddev` は、内部で 1 パスの累積を使用します。
これにより入力範囲で動作でき、入力を 2 回走査する必要を避けられます。

`geometric_mean` は、正の値に対して対数による累積を使用します。
入力値に 0 がある場合、`log(0)` を取らずに結果を 0 にします。

`median`、`quantile`、`percentile`、`mode` はソート済みデータを必要とするため、入力値をコピーします。
入力範囲は受け取れますが、入力値の個数に比例するメモリが必要です。

累積は内部で `long double` を使って行われ、公開されるスカラー結果の型は `double` です。
これにより、初期 API を単純で予測しやすいものに保ちながら、`double` に直接累積するよりも高い中間精度を得ます。

---

## 例

```cpp
#include <iostream>
#include <vector>

#include <xer/statistics.h>

void print_modes(const std::vector<double>& values)
{
    for (const double value : values) {
        std::cout << ' ' << value;
    }
    std::cout << '\n';
}

auto main() -> int
{
    const std::vector<double> values{2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0};

    const auto mean = xer::mean(values);
    const auto sum = xer::sum(values);
    const auto product = xer::product(values);
    const auto median = xer::median(values);
    const auto quantile = xer::quantile(values, 0.25);
    const auto percentile = xer::percentile(values, 75.0);
    const auto variance = xer::variance(values);
    const auto stddev = xer::stddev(values);
    const auto modes = xer::mode(values);

    if (!mean || !sum || !product || !median || !quantile || !percentile ||
        !variance || !stddev || !modes) {
        return 1;
    }

    std::cout << *mean << '\n';
    std::cout << *sum << '\n';
    std::cout << *product << '\n';
    std::cout << *median << '\n';
    std::cout << *quantile << '\n';
    std::cout << *percentile << '\n';
    std::cout << *variance << '\n';
    std::cout << *stddev << '\n';
    print_modes(*modes);

    return 0;
}
```

---

## メモ

`variance` と `stddev` は、意図的に母分散と母標準偏差を意味します。

`n - 1` を使う標本公式が必要な場合は、`sample_variance` と `sample_stddev` を使用してください。
この命名により、分母の選択が明示的になり、曖昧な既定用語に依存することを避けられます。

`<xer/statistics.h>` は、範囲の最小値または最大値を求めるヘルパーを提供しません。
その用途には、`std::ranges::min_element`、`std::ranges::max_element`、`std::min_element`、または `std::max_element` を使用してください。

---

# `<xer/complex.h>`

## 目的

`<xer/complex.h>` は、xer の軽量な複素数数学ヘルパーを提供します。

このヘッダーには、`<xer/math.h>` の一部の関数に対応する複素数版が含まれます。実数のみを扱うコードが複素数機能をインクルードせずに済むように、`<xer/math.h>` とは分離されています。

---

## 提供される関数

### `cquadratic`

```cpp
template<std::floating_point T>
auto cquadratic(T a, T b, T c)
    -> xer::result<std::array<std::complex<T>, 2>>;
```

次の二次方程式を解きます。

```text
a * x * x + b * x + c == 0
```

係数 `a` は 0 であってはなりません。`a == 0` の場合、この関数は `error_t::invalid_argument` を返します。

返される配列には、重複度を含む 2 個の複素根が格納されます。

### `ccubic`

```cpp
template<std::floating_point T>
auto ccubic(T a, T b, T c, T d)
    -> xer::result<std::array<std::complex<T>, 3>>;
```

次の三次方程式を解きます。

```text
a * x * x * x + b * x * x + c * x + d == 0
```

係数 `a` は 0 であってはなりません。`a == 0` の場合、この関数は `error_t::invalid_argument` を返します。

返される配列には、重複度を含む 3 個の複素根が格納されます。

---

## 設計メモ

`c` 接頭辞は複素根版を意味します。たとえば、`cquadratic` は `quadratic` に対応する複素根版です。

現在の関数は実係数を受け取り、複素根を返します。実用上の必要が出てきた場合は、複素係数への対応を後で検討できます。

---

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

---

# `<xer/interval.h>`

## 目的

`<xer/interval.h>` は、境界付き浮動小数点値型を提供します。

主なエンティティは `xer::interval<T, Min, Max>` です。これは、固定された閉区間内に制約された有限スカラー値を格納する軽量な値型です。

既定の区間は `[0, 1]` です。この区間は、色成分、アルファ値、正規化比率、不透明度、明るさ、ゲインなど、境界を持つ制御値に有用です。

---

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

#include <stdexcept>

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

---

# `<xer/color.h>`

## 目的

`<xer/color.h>` は、色体系の値型と色変換関数を提供します。

このヘッダーの目的は、軽量な xer らしい形で、実用的な数式ベースの色表現と変換を支援することです。

初期段階でサポートする色体系は次のとおりです。

- RGB
- グレースケール
- CMY
- HSV
- CIE 1931 XYZ
- CIE 1976 L*a*b*
- CIE 1976 L*u*v*

このヘッダーは、完全なカラーマネジメントシステムを目指していません。ICC プロファイル、色順応、分光データ、名前付き色、カラーパレット管理などは扱いません。

---

## 主なエンティティ

`<xer/color.h>` は、少なくとも次のクラステンプレートを提供します。

```cpp
template <std::floating_point T>
struct basic_rgb;

template <std::floating_point T>
struct basic_gray;

template <std::floating_point T>
struct basic_cmy;

template <std::floating_point T>
struct basic_hsv;

template <std::floating_point T>
struct basic_xyz;

template <std::floating_point T>
struct basic_lab;

template <std::floating_point T>
struct basic_luv;
```

通常用途向けに、`float` を使う別名も提供します。

```cpp
using rgb = basic_rgb<float>;
using gray = basic_gray<float>;
using cmy = basic_cmy<float>;
using hsv = basic_hsv<float>;
using xyz = basic_xyz<float>;
using lab = basic_lab<float>;
using luv = basic_luv<float>;
```

公開ヘッダーは次のとおりです。

```cpp
#include <xer/color.h>
```

実装は次の内部ヘッダーで提供されます。

```cpp
#include <xer/bits/color.h>
```

ユーザーコードでは公開ヘッダーをインクルードしてください。

---

## 設計上の役割

`<xer/color.h>` は、一般的な色体系のための小さな値型と変換関数を提供します。

設計は次の考え方に基づきます。

- 色値は公開データメンバーを持つ単純な構造体にする
- 通常の別名は `float` を使う
- 正規化された境界付き成分には `xer::interval<T>` を使う
- 色相には `xer::cyclic<T>` を使う
- XYZ、Lab、Luv のような測色系では生の浮動小数点メンバーを使う
- 変換関数は自由関数にする
- 決定的な算術変換は変換先の値を直接返す

---

## `float` 別名

テンプレート自体は `float`、`double`、`long double` を使えますが、実用的な色処理では通常 `float` を使います。

そのため、通常の別名は `float` を使います。

```cpp
xer::rgb color(0.25f, 0.5f, 0.75f);
```

別の精度が必要な場合は、対応するテンプレートを直接使います。

```cpp
xer::basic_rgb<double> color(0.25, 0.5, 0.75);
```

---

## RGB

## `basic_rgb`

```cpp
template <std::floating_point T>
struct basic_rgb {
    using value_type = T;
    using component_type = interval<T>;

    component_type r;
    component_type g;
    component_type b;
};
```

`basic_rgb<T>` は、正規化された赤、緑、青の成分を持つ RGB 色を表します。

各成分は `interval<T>` で表されるため、`[0, 1]` に保たれます。

```cpp
auto color = xer::rgb(1.25f, 0.5f, -0.25f);

// color.r.value() == 1.0f
// color.g.value() == 0.5f
// color.b.value() == 0.0f
```

### sRGB の仮定

公開型名は `rgb` であり、`srgb` ではありません。

ただし、RGB と XYZ の変換では、D65 白色点の sRGB を仮定します。

つまり、次の意味になります。

- `to_xyz(rgb)` は RGB 成分を非線形 sRGB 成分として扱います。
- `to_rgb(xyz)` は非線形 sRGB 成分を生成します。

RGB を XYZ、Lab、Luv と組み合わせて使う場合は、この仮定に注意してください。

### アルファ

アルファは `basic_rgb` には含まれません。

アルファは、主にグラフィックス、合成、画像処理で有用です。色そのものの一般的な成分ではなく、印刷、塗装、照明、測色などの分野では不要です。

将来アルファ対応が必要になった場合は、`basic_rgba<T>` のような別型として提供するのがよいです。`basic_rgb<T>` に混ぜるべきではありません。

---

## グレースケール

## `basic_gray`

```cpp
template <std::floating_point T>
struct basic_gray {
    using value_type = T;
    using component_type = interval<T>;

    component_type y;
};
```

`basic_gray<T>` は、表示指向のグレースケール値を表します。成分は `interval<T>` で表され、`[0, 1]` に保たれます。

`to_luma_gray` は、非線形 sRGB 成分から単純なルーマを直接計算します。`to_luminance_gray` は、sRGB をデコードして相対輝度を求め、それを表示グレースケール値へ再エンコードします。`to_gray` は `to_luma_gray` の別名です。`to_rgb(gray)` はグレースケール成分を RGB の各成分へ複製します。

---

## CMY

## `basic_cmy`

```cpp
template <std::floating_point T>
struct basic_cmy {
    using value_type = T;
    using component_type = interval<T>;

    component_type c;
    component_type m;
    component_type y;
};
```

`basic_cmy<T>` は、単純な正規化 CMY 色を表します。

各成分は `interval<T>` で表されるため、`[0, 1]` に保たれます。

xer の CMY は RGB の単純な補色モデルです。

概念的には次の関係です。

```text
C = 1 - R
M = 1 - G
Y = 1 - B
```

および:

```text
R = 1 - C
G = 1 - M
B = 1 - Y
```

CMYK や印刷用のカラーマネジメントは扱いません。

---

## HSV

## `basic_hsv`

```cpp
template <std::floating_point T>
struct basic_hsv {
    using value_type = T;
    using hue_type = cyclic<T>;
    using component_type = interval<T>;

    hue_type h;
    component_type s;
    component_type v;
};
```

`basic_hsv<T>` は、色相、彩度、明度を持つ HSV 色を表します。

色相 `h` は循環値なので `cyclic<T>` で表されます。彩度 `s` と明度 `v` は `[0, 1]` の境界付き値なので `interval<T>` で表されます。

---

## XYZ

## `basic_xyz`

```cpp
template <std::floating_point T>
struct basic_xyz {
    using value_type = T;

    T x;
    T y;
    T z;
};
```

`basic_xyz<T>` は CIE 1931 XYZ 色を表します。

XYZ は正規化成分型ではなく、測色値として生の浮動小数点値を保持します。

RGB との変換では、sRGB と D65 白色点を仮定します。

---

## Lab

## `basic_lab`

```cpp
template <std::floating_point T>
struct basic_lab {
    using value_type = T;

    T l;
    T a;
    T b;
};
```

`basic_lab<T>` は CIE 1976 L*a*b* 色を表します。

`l` は明度、`a` と `b` は色度成分です。

Lab は、RGB のような `[0, 1]` 成分ではありません。値域は変換元や白色点に依存するため、生の浮動小数点値として表します。

---

## Luv

## `basic_luv`

```cpp
template <std::floating_point T>
struct basic_luv {
    using value_type = T;

    T l;
    T u;
    T v;
};
```

`basic_luv<T>` は CIE 1976 L*u*v* 色を表します。

`l` は明度、`u` と `v` は色度成分です。

Lab と同様に、正規化区間値ではなく生の浮動小数点値として表します。

---

## コンストラクタ

### RGB

RGB は、赤、緑、青の値から構築できます。

```cpp
xer::rgb color(0.25f, 0.5f, 0.75f);
```

入力値は `interval<T>` の規則に従って `[0, 1]` にクランプされます。`NaN` や無限大は例外で拒否されます。

### CMY

CMY は、シアン、マゼンタ、イエローの値から構築できます。

```cpp
xer::cmy color(0.0f, 0.5f, 1.0f);
```

### HSV

HSV は、色相、彩度、明度から構築できます。

```cpp
xer::hsv color(0.5f, 1.0f, 0.75f);
```

色相は `cyclic<T>` として扱われ、循環正規化されます。彩度と明度は `interval<T>` として扱われます。

### XYZ、Lab、Luv

XYZ、Lab、Luv は、対応する成分値から構築できます。

これらの測色系では、成分は生の浮動小数点値です。正規化成分ではありません。

---

## 変換関数

変換関数は自由関数として提供されます。

---

## RGB と CMY

RGB と CMY の相互変換は、単純な補色関係に基づきます。

```text
C = 1 - R
M = 1 - G
Y = 1 - B
```

および:

```text
R = 1 - C
G = 1 - M
B = 1 - Y
```

---

## RGB と HSV

RGB と HSV の変換は、一般的な数式に基づきます。

HSV の色相は循環値として扱われ、彩度と明度は `[0, 1]` に保たれます。

グレースケールに近い入力では、色相の意味が弱くなる点に注意してください。

---

## RGB と XYZ

RGB と XYZ の変換では、sRGB と D65 白色点を仮定します。

RGB から XYZ への変換では、非線形 sRGB 成分を線形化してから変換します。

XYZ から RGB への変換では、線形 RGB へ変換した後、sRGB エンコードを行います。結果の RGB 成分は `interval<T>` によって `[0, 1]` に収められます。

---

## XYZ と Lab

XYZ と Lab の相互変換は、CIE 1976 L*a*b* の式に基づきます。

白色点は、RGB/XYZ 変換と整合するように扱われます。

---

## XYZ と Luv

XYZ と Luv の相互変換は、CIE 1976 L*u*v* の式に基づきます。

Luv も測色系であり、成分は正規化区間値ではありません。

---

## 直接変換ポリシー

直接変換関数は、必要に応じて中間色空間を経由しても構いません。

たとえば、RGB から Lab への変換は、概念的には RGB から XYZ、XYZ から Lab への変換を組み合わせたものです。

公開 API として重要なのは、呼び出し側が意図する変換を直接書けることです。

---

## エラーと例外のポリシー

決定的な算術変換は、通常、変換先の値を直接返します。

正規化成分では、`interval<T>` が範囲外値をクランプし、`NaN` や無限大を例外で拒否します。

色変換そのものは、通常の失敗を `xer::result` で返す操作としてではなく、値変換として扱います。

---

## サポートする色体系とサポートしない色体系

### サポート対象

現在サポートする色体系は次のとおりです。

- RGB
- グレースケール
- CMY
- HSV
- CIE 1931 XYZ
- CIE 1976 L*a*b*
- CIE 1976 L*u*v*

### サポート対象外

現在サポートしないものは次のとおりです。

- ICC プロファイル
- CMYK
- HSL
- HWB
- スペクトル色
- 名前付き色
- カラーパレット管理
- デバイス依存の詳細なカラーマネジメント

---

## 後回しにしている項目

次の項目は、初期実装では意図的に後回しにしています。

- アルファ付き色型
- ICC プロファイル対応
- 色順応
- CMYK
- HSL や HWB などの追加色体系
- 色差計算
- 名前付き色
- パレット処理

これらは、必要性が明確になってから別の型や関数として追加するのがよいです。

---

## 他のヘッダーとの関係

`<xer/color.h>` は次のヘッダーやポリシーと関係します。

- `<xer/interval.h>`
- `<xer/cyclic.h>`
- `<xer/arithmetic.h>`
- `policy_color.md`
- `policy_interval.md`

`interval` は正規化成分に使われ、`cyclic` は色相に使われます。

---

## ドキュメント上の注意

このヘッダーを説明するときは、次の点を明確にします。

- 色値は単純な構造体であること
- 正規化成分は `interval<T>` を使うこと
- 色相は `cyclic<T>` を使うこと
- RGB/XYZ 変換では sRGB と D65 を仮定すること
- 完全なカラーマネジメントシステムではないこと

---

## 例

```cpp
#include <xer/color.h>

#include <xer/arithmetic.h>

auto main() -> int
{
    const auto color = xer::rgb(1.25f, 0.5f, -0.25f);

    if (color.r.value() != 1.0f) {
        return 1;
    }

    if (color.g.value() != 0.5f) {
        return 1;
    }

    if (color.b.value() != 0.0f) {
        return 1;
    }

    const auto cmy = xer::to_cmy(color);
    const auto rgb = xer::to_rgb(cmy);

    if (!xer::is_close(rgb.r.value(), 1.0f, 1e-6f)) {
        return 1;
    }

    return 0;
}
```

---

## 関連項目

- `policy_color.md`
- `policy_interval.md`
- `header_interval.md`
- `header_cyclic.md`
- `header_arithmetic.md`

---

> **未訳:** この節の日本語版はまだ最新ではありません。
> そのため、暫定的に英語版の内容を掲載しています。
> 
> Header: `xer/quantity.h`
> Reason: Japanese fragment was translated from a different English source hash.

# `<xer/quantity.h>`

## Purpose

`<xer/quantity.h>` provides physical quantity and unit facilities in xer.

Its purpose is to allow quantities with dimensions to be handled in a type-safe and practical way.
This includes:

- preventing meaningless arithmetic between different dimensions
- making unit conversion explicit
- allowing natural notation with unit objects
- keeping the design lightweight and easy to understand

This header is not intended to reproduce an existing quantity library as it is.
Instead, it follows xer's own design priorities.

---

## Main Role

The main role of `<xer/quantity.h>` is to provide a compact framework for:

- dimensions
- units
- quantities
- practical predefined units under `xer::units`

This makes it possible to write code such as:

```cpp
using namespace xer::units;

auto x = 1.5 * km;
auto t = 2.0 * sec;
auto v = x / t;
```

while preserving dimension safety and explicit conversion rules.

---

## Main Entities

At minimum, `<xer/quantity.h>` provides the following entities:

```cpp
template <int L, int M, int T, int I>
struct dimension;

using dimensionless = dimension<0, 0, 0, 0>;

template <typename Dim, typename Scale = std::ratio<1>>
class unit;

template <std::floating_point T, typename Dim>
class quantity;

sq
cb
```

In addition, the public `<xer/quantity.h>` header provides predefined unit objects under the `xer::units` namespace. Internally, the core quantity framework and the predefined unit families are split so that future unit families can be added without expanding the core implementation header.

---

## Header Organization

The public header keeps the traditional include model:

```cpp
#include <xer/quantity.h>
```

This includes the core quantity framework, the predefined SI/common units, and yard-pound units.

Internally, the implementation is organized as follows:

```text
xer/bits/quantity.h
  Core dimension, unit, quantity, arithmetic, and unit-composition support.

xer/bits/units_si.h
  SI base units, selected prefixed units, derived units, conventional metric units,
  and angular units under xer::units.

xer/bits/units_imperial.h
  International yard-pound length and mass units under xer::units.
```

Users normally include only `<xer/quantity.h>`. The split is an implementation and maintenance detail, but it keeps the unit definitions separable from the core quantity machinery.

---

## `dimension`

`dimension` represents the dimension of a physical quantity.

### Basic Shape

```cpp
template <int L, int M, int T, int I>
struct dimension;
```

### Meaning of the Parameters

The template arguments represent exponents of the base dimensions:

* `L`: length
* `M`: mass
* `T`: time
* `I`: electric current

### Examples

Typical examples include:

```cpp
dimension<1, 0, 0, 0>   // length
dimension<0, 1, 0, 0>   // mass
dimension<0, 0, 1, 0>   // time
dimension<0, 0, 0, 1>   // electric current
dimension<1, 0, -1, 0>  // velocity
dimension<1, 1, -2, 0>  // force
```

### Role

`dimension` exists to make dimensional correctness part of the type system.

This prevents invalid combinations such as:

* adding length and time
* comparing mass and electric current directly

---

## `dimensionless`

```cpp
using dimensionless = dimension<0, 0, 0, 0>;
```

### Role

`dimensionless` represents a quantity with no physical dimension.

This is useful for values such as:

* pure ratios
* normalized coefficients
* angular-unit-related scale values when treated dimensionlessly

### Notes

Even dimensionless quantities remain quantities in the type system.
They are not automatically the same thing as raw scalar values.

---

## `quantity<T, Dim>`

`quantity<T, Dim>` is the central value type of the header.

It represents a numeric value together with a dimension.

### Basic Shape

```cpp
template <std::floating_point T, typename Dim>
class quantity;
```

### Role

A `quantity<T, Dim>` stores:

* a numeric value
* a physical dimension

This allows arithmetic to preserve dimensional correctness.

### Stored Value Type

At least in the current design direction, `T` is restricted to floating-point types such as:

* `float`
* `double`
* `long double`

Integer storage is not the primary model.

### Why Floating-Point Storage

This is because:

* unit conversion naturally produces fractional values
* some unit scales are non-rational
* internal normalization to base units is not generally integral
* keeping the design simple is more important than supporting every numeric form initially

---

## Internal Quantity Representation

A `quantity<T, Dim>` stores its value normalized to the base unit system.

### Base Unit System

At minimum, the base dimensions are:

* meter
* kilogram
* second
* ampere

This means the internal system is effectively MKSA.

### Examples

Conceptually:

* `1 km` is stored as `1000 m`
* `1 g` is stored as `0.001 kg`
* `1 msec` is stored as `0.001 sec`

### Why This Matters

Normalizing stored values to base units simplifies:

* arithmetic
* comparison
* conversion between units
* reasoning about mixed units of the same dimension

---

## Construction and Value Retrieval

The intended usage model includes at least the following ideas:

* constructing directly from a base-unit value
* constructing from a scalar multiplied by a unit object
* retrieving the normalized base-unit value
* retrieving the value converted to a specified unit

Typical forms include:

```cpp
auto value() const noexcept -> T;
auto value(unit_type u) const noexcept -> T;
```

The exact signatures may vary, but this is the intended public direction.

### Example

```cpp
using namespace xer::units;

auto x = 1.5 * km;
auto a = x.value();    // base-unit value
auto b = x.value(km);  // value expressed in km
```

---

## Raw Value Access for Dimensionless Quantities

Dimensionless quantities sometimes need to be converted back to raw scalars.

### Design Direction

This should be possible, but it should remain explicit.

### Why Explicit

Implicit conversion from a dimensionless quantity to a raw scalar weakens the type system and makes code less clear.

For that reason, explicit conversion or explicit value retrieval is preferred.

---

## `unit<Dim, Scale>`

`unit<Dim, Scale>` represents a unit.

### Basic Shape

```cpp
template <typename Dim, typename Scale = std::ratio<1>>
class unit;
```

### Role

A unit represents:

* a dimension
* a scale relative to the base unit of that dimension

### Nature of `unit`

The intended design is that `unit` should be primarily type-level information.

That means:

* unit information should ideally be carried by template arguments
* unit objects should remain lightweight
* unnecessary runtime data members should be avoided

In practice, predefined unit objects should behave like empty or near-empty compile-time objects.

---

## Scale Representation

One important design point is that unit scales are **not** limited to rational values.

### Rational-Scale Examples

Many units can be represented naturally with rational scales:

* `mm`
* `cm`
* `km`
* `g`
* `mg`
* `msec`
* `kHz`
* `hPa`

### Non-Rational-Scale Example

Some units, such as `rad` relative to `taurad`, are not naturally representable by a purely rational scale.

### Design Direction

Therefore:

* `std::ratio`-like rational scales are the default
* floating-point-based scale representation may also be necessary for some units
* the template default should not be interpreted as a permanent restriction of the entire design

This point is especially important for understanding `unit<Dim, Scale>` correctly.

---

## Unit Arithmetic

Units may be multiplied and divided.

### Meaning

When units are multiplied or divided:

* dimensions are combined
* scales are combined

This allows natural construction of derived quantities such as:

```cpp
using namespace xer::units;

auto v = 3.0 * m / sec;
auto a = 9.8 * m / sq(sec);
auto f = 2.0 * kg * m / sq(sec);
```

### Why This Matters

This avoids the need to define every composite unit as a separate fixed name.

---

## Square and Cube Helpers for Units and Quantities

`<xer/quantity.h>` provides `sq` and `cb` for units and quantities.

```cpp
sq(unit)
cb(unit)
sq(quantity)
cb(quantity)
```

### Role

These helpers make repeated multiplication easier to read in unit expressions.
For example:

```cpp
using namespace xer::units;

auto acceleration = 9.8 * m / sq(sec);
```

This is equivalent in meaning to:

```cpp
auto acceleration = 9.8 * m / (sec * sec);
```

For quantities, `sq` and `cb` multiply the stored value and combine the dimension exponents accordingly.

### Symbolic Unit Aliases

For common base units, symbolic aliases are also provided under `xer::units`:

```cpp
m²
m³
sec²
sec³
```

These are aliases for the corresponding square or cube unit expressions:

```cpp
m²   // sq(m)
m³   // cb(m)
sec² // sq(sec)
sec³ // cb(sec)
```

They are intended as readable symbolic notation.
The ASCII forms `sq(m)`, `cb(m)`, `sq(sec)`, and `cb(sec)` remain available as the portable spelling.
---

## `xer::units`

Predefined unit objects are provided under the `xer::units` namespace.

### Role

This namespace groups common unit names in one predictable place.

### Basic Direction

Units are intentionally **not** placed directly under `xer`.

This makes it possible to write:

```cpp
using namespace xer::units;
```

only where needed, without polluting the main namespace.

### Examples of Base Units

At minimum, the following base units are provided:

```cpp
xer::units::m
xer::units::kg
xer::units::sec
xer::units::A
```

---

## Predefined Units

The header is expected to provide a practical set of common units.

### Base Units

At minimum:

* `m`
* `kg`
* `sec`
* `A`

### Squared and Cubed Base Units

At minimum:

* `m²`
* `m³`
* `sec²`
* `sec³`

### Selected Prefixed Units

Examples include:

* `mm`
* `cm`
* `km`
* `microm`
* `nm`
* `g`
* `mg`
* `nsec`
* `microsec`
* `msec`
* `mA`
* `kHz`
* `GHz`
* `hPa`

### Selected Derived Units

Examples include:

* `Hz`
* `N`
* `J`
* `W`
* `V`
* `Pa`

### Conventional Units

Examples include:

* `ha`
* `mL`
* `dL`
* `L`
* `kL`
* `cal`
* `kcal`


### Yard-Pound Units

The header also provides a small set of international yard-pound units.

Length units:

* `inch`
* `ft`
* `yd`
* `mile`

Mass units:

* `oz`
* `lb`

These names intentionally use either the ordinary singular unit name (`inch`, `mile`) or the common unit symbol (`ft`, `yd`, `oz`, `lb`). Plural names such as `feet` and `pounds` are not provided.

The conversion factors are exact international definitions:

* `1 inch = 0.0254 m`
* `1 ft = 0.3048 m`
* `1 yd = 0.9144 m`
* `1 mile = 1609.344 m`
* `1 lb = 0.45359237 kg`
* `1 oz = 1/16 lb`

### Aliases

Examples may include:

* `μm`
* `μsec`
* `cc`

The exact set belongs to the detailed unit reference, but these are the main intended categories.

---

## Angular Units

`<xer/quantity.h>` also covers angle-related units in coordination with the broader xer design.

### Important Units

At minimum:

* `taurad`
* `τrad`
* `rad`

### Design Meaning

* `taurad` is the base unit for angle
* `τrad` is an alias of `taurad`
* `rad` is treated as a dimensionless unit for angle

### Why This Matters

This keeps angle quantities compatible with xer's design where one full turn corresponds naturally to the `cyclic` model.

---

## Relationship to `cyclic`

A key design point is that angle quantities and circular values are **not identical concepts**.

### Quantity Side

`quantity` handles ordinary angle quantities, including values such as:

* multiple turns
* negative turns
* values used in ordinary arithmetic

### `cyclic` Side

`cyclic` handles explicitly circular semantics such as:

* clockwise distance
* counterclockwise distance
* shortest difference on a circle

### Design Boundary

So the design direction is:

* store ordinary angles as quantities
* convert to `cyclic` when circular behavior is actually needed

This keeps both abstractions focused.

---

## User-Defined Literals

User-defined literals are not the primary notation model here.

### Preferred Style

The intended style is:

```cpp
1.23f * msec
```

rather than unit suffix literals.

### Why

This keeps the notation:

* explicit
* easy to read
* consistent with the rest of xer
* easier to extend without creating many special literal forms

---

## Relationship to Other Headers

`<xer/quantity.h>` should be understood together with:

* `policy_project_outline.md`
* `policy_quantity.md`
* `header_cyclic.md`
* `header_arithmetic.md`

The rough boundary is:

* `<xer/quantity.h>` handles units, dimensions, and quantities
* `<xer/cyclic.h>` handles explicitly circular values
* `<xer/arithmetic.h>` handles general arithmetic/comparison helpers not specific to physical dimensions

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that it provides dimensions, units, and quantities
* that quantities are normalized internally to base units
* that unit objects live under `xer::units`
* that scale is not conceptually restricted to rational representation only
* that ordinary angular quantities and `cyclic` values are distinct concepts

Detailed unit catalogs and per-operator rules belong in the detailed reference or generated API sections.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* constructing a quantity from a scalar and a unit
* converting a quantity to base units and to another unit
* dividing distance by time to obtain velocity\n* using `sq`, `cb`, `m²`, `m³`, and `sec²` in unit expressions
* using predefined units from `xer::units`
* handling angle quantities with `taurad` or `rad`

These are good candidates for executable examples in `examples/`.

---

## Example

```cpp
#include <xer/quantity.h>

using namespace xer::units;

auto main() -> int
{
    const auto distance = 1.5 * km;
    const auto seconds = 30.0 * sec;
    const auto speed = distance / seconds;

    const auto meters = distance.value();
    if (meters <= 0.0) {
        return 1;
    }

    const auto kilometers = distance.value(km);
    if (kilometers != 1.5) {
        return 1;
    }

    static_cast<void>(speed);
    return 0;
}
```

This example shows the normal xer style:

* use unit objects from `xer::units`
* construct quantities with scalar × unit
* retrieve values explicitly
* keep dimensional meaning in the type system

---

## See Also

* `policy_project_outline.md`
* `policy_quantity.md`
* `header_cyclic.md`
* `header_arithmetic.md`

---

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

---

# `<xer/image.h>`


## 目的

`<xer/image.h>` は、軽量な画像およびフレームバッファ機能を提供します。

このヘッダーの初期目的は、本格的な写真編集や完全な画像ファイル処理ではありません。固定サイズキャンバス、VRAM風のエミュレーション、単純な描画、画像処理、および将来的な Tcl/Tk photo image 連携に向けた、小さなフレームバッファ指向レイヤーです。

純粋な画像処理と描画は `<xer/image.h>` に属します。Tcl/Tk photo 連携は `<xer/tk.h>` に属します。

---

## 名前空間

画像関連の型と関数は `xer::image` 名前空間に配置されます。

主要なフレームバッファ所有型は `xer::image::canvas` です。これにより、`xer::image` を画像ストレージ、描画、画像処理の名前空間として使えます。

---

## 主なエンティティ

少なくとも `<xer/image.h>` は次のエンティティを提供します。

```cpp
namespace xer::image {

struct point;
struct pointf;
struct size;
struct sizef;
struct rect;
struct rectf;

struct filter_pixels_error_detail;

enum class bitmap_glyph_width : std::uint8_t;
struct bitmap_font_range;
struct bitmap_font;
struct text_draw_options;

struct pixel;

struct argb32_policy;
struct rgba32_policy;
struct rgb24_policy;
struct bgr24_policy;

template <std::size_t Width,
          std::size_t Height,
          class Policy = argb32_policy>
class canvas;

template <class Policy = argb32_policy>
using dynamic_canvas = canvas<0, 0, Policy>;

[[nodiscard]] auto bitmap_font_load(const xer::path& filename)
    -> xer::result<bitmap_font, xer::parse_error_detail>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_hline(canvas<Width, Height, Policy>& img,
                int x,
                int y,
                int length,
                pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_hline(canvas<Width, Height, Policy>& img,
                const point& p,
                int length,
                pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_vline(canvas<Width, Height, Policy>& img,
                int x,
                int y,
                int length,
                pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_vline(canvas<Width, Height, Policy>& img,
                const point& p,
                int length,
                pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line(canvas<Width, Height, Policy>& img,
               int x0,
               int y0,
               int x1,
               int y1,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line(canvas<Width, Height, Policy>& img,
               const point& p0,
               const point& p1,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line_aa(canvas<Width, Height, Policy>& img,
                  float x0,
                  float y0,
                  float x1,
                  float y1,
                  pixel color) noexcept -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line_aa(canvas<Width, Height, Policy>& img,
                  float x0,
                  float y0,
                  float x1,
                  float y1,
                  float width,
                  pixel color) noexcept -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line_aa(canvas<Width, Height, Policy>& img,
                  const pointf& p0,
                  const pointf& p1,
                  pixel color) noexcept -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_line_aa(canvas<Width, Height, Policy>& img,
                  const pointf& p0,
                  const pointf& p1,
                  float width,
                  pixel color) noexcept -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_rect(canvas<Width, Height, Policy>& img,
               int x,
               int y,
               int width,
               int height,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_rect(canvas<Width, Height, Policy>& img,
               const point& origin,
               const size& extent,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_rect(canvas<Width, Height, Policy>& img,
               const rect& area,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_rect(canvas<Width, Height, Policy>& img,
               int x,
               int y,
               int width,
               int height,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_rect(canvas<Width, Height, Policy>& img,
               const point& origin,
               const size& extent,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_rect(canvas<Width, Height, Policy>& img,
               const rect& area,
               pixel color) noexcept -> void;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_circle(canvas<Width, Height, Policy>& img,
                 int cx,
                 int cy,
                 int radius,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_circle(canvas<Width, Height, Policy>& img,
                 const point& center,
                 int radius,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_circle(canvas<Width, Height, Policy>& img,
                 int cx,
                 int cy,
                 int radius,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_circle(canvas<Width, Height, Policy>& img,
                 const point& center,
                 int radius,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_circle_aa(canvas<Width, Height, Policy>& img,
                    float cx,
                    float cy,
                    float radius,
                    pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_circle_aa(canvas<Width, Height, Policy>& img,
                    float cx,
                    float cy,
                    float radius,
                    float width,
                    pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_circle_aa(canvas<Width, Height, Policy>& img,
                    const pointf& center,
                    float radius,
                    pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_circle_aa(canvas<Width, Height, Policy>& img,
                    const pointf& center,
                    float radius,
                    float width,
                    pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_circle_aa(canvas<Width, Height, Policy>& img,
                    float cx,
                    float cy,
                    float radius,
                    pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_circle_aa(canvas<Width, Height, Policy>& img,
                    const pointf& center,
                    float radius,
                    pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse(canvas<Width, Height, Policy>& img,
                  int cx,
                  int cy,
                  int radius_x,
                  int radius_y,
                  pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse(canvas<Width, Height, Policy>& img,
                  const point& center,
                  int radius_x,
                  int radius_y,
                  pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_ellipse(canvas<Width, Height, Policy>& img,
                  int cx,
                  int cy,
                  int radius_x,
                  int radius_y,
                  pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_ellipse(canvas<Width, Height, Policy>& img,
                  const point& center,
                  int radius_x,
                  int radius_y,
                  pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_aa(canvas<Width, Height, Policy>& img,
                     float cx,
                     float cy,
                     float radius_x,
                     float radius_y,
                     pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_aa(canvas<Width, Height, Policy>& img,
                     float cx,
                     float cy,
                     float radius_x,
                     float radius_y,
                     float width,
                     pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_aa(canvas<Width, Height, Policy>& img,
                     const pointf& center,
                     float radius_x,
                     float radius_y,
                     pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_aa(canvas<Width, Height, Policy>& img,
                     const pointf& center,
                     float radius_x,
                     float radius_y,
                     float width,
                     pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_ellipse_aa(canvas<Width, Height, Policy>& img,
                     float cx,
                     float cy,
                     float radius_x,
                     float radius_y,
                     pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto fill_ellipse_aa(canvas<Width, Height, Policy>& img,
                     const pointf& center,
                     float radius_x,
                     float radius_y,
                     pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_arc(canvas<Width, Height, Policy>& img,
              int cx,
              int cy,
              int radius,
              float start_angle,
              float sweep_angle,
              pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_arc(canvas<Width, Height, Policy>& img,
              const point& center,
              int radius,
              float start_angle,
              float sweep_angle,
              pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_arc_aa(canvas<Width, Height, Policy>& img,
                 float cx,
                 float cy,
                 float radius,
                 float start_angle,
                 float sweep_angle,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_arc_aa(canvas<Width, Height, Policy>& img,
                 float cx,
                 float cy,
                 float radius,
                 float start_angle,
                 float sweep_angle,
                 float width,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_arc_aa(canvas<Width, Height, Policy>& img,
                 const pointf& center,
                 float radius,
                 float start_angle,
                 float sweep_angle,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_arc_aa(canvas<Width, Height, Policy>& img,
                 const pointf& center,
                 float radius,
                 float start_angle,
                 float sweep_angle,
                 float width,
                 pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_arc(canvas<Width, Height, Policy>& img,
                      int cx,
                      int cy,
                      int radius_x,
                      int radius_y,
                      float start_angle,
                      float sweep_angle,
                      pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_arc(canvas<Width, Height, Policy>& img,
                      const point& center,
                      int radius_x,
                      int radius_y,
                      float start_angle,
                      float sweep_angle,
                      pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_arc_aa(canvas<Width, Height, Policy>& img,
                         float cx,
                         float cy,
                         float radius_x,
                         float radius_y,
                         float start_angle,
                         float sweep_angle,
                         pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_arc_aa(canvas<Width, Height, Policy>& img,
                         float cx,
                         float cy,
                         float radius_x,
                         float radius_y,
                         float start_angle,
                         float sweep_angle,
                         float width,
                         pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_arc_aa(canvas<Width, Height, Policy>& img,
                         const pointf& center,
                         float radius_x,
                         float radius_y,
                         float start_angle,
                         float sweep_angle,
                         pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
auto draw_ellipse_arc_aa(canvas<Width, Height, Policy>& img,
                         const pointf& center,
                         float radius_x,
                         float radius_y,
                         float start_angle,
                         float sweep_angle,
                         float width,
                         pixel color) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto flood_fill(canvas<Width, Height, Policy>& img,
                              int x,
                              int y,
                              pixel color)
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto flood_fill(canvas<Width, Height, Policy>& img,
                              const point& origin,
                              pixel color)
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto mosaic(canvas<Width, Height, Policy>& img,
                          const rect& area,
                          const size& block_size) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto box_blur(canvas<Width, Height, Policy>& img,
                            const rect& area,
                            const size& box_size)
    -> xer::result<void>;

struct filter_pixels_error_detail {
    point first_error_position;
    std::size_t error_count;
};

template <std::size_t Width, std::size_t Height, class Policy, class F>
[[nodiscard]] auto filter_pixels(canvas<Width, Height, Policy>& img,
                                 const rect& area,
                                 F&& filter)
    -> xer::result<void, filter_pixels_error_detail>;

}
```

---

## 幾何型

このヘッダーは、整数座標用と浮動小数点座標用の小さな幾何型を提供します。

```cpp
struct point {
    int x;
    int y;
};

struct pointf {
    float x;
    float y;
};

struct size {
    int width;
    int height;
};

struct sizef {
    float width;
    float height;
};

struct rect {
    point origin;
    size extent;
};

struct rectf {
    pointf origin;
    sizef extent;
};
```

これらの型は、描画APIでスカラー座標と構造化座標の両方を自然に使うためのものです。

整数型は、ピクセル境界に基づく通常の描画に使います。浮動小数点型は、アンチエイリアス描画のようにピクセル中心やサブピクセル位置を扱うAPIで使います。

---

## フィルターエラー詳細

`filter_pixels_error_detail` は、ユーザー指定フィルターが例外を投げた場合の情報を保持します。

```cpp
struct filter_pixels_error_detail {
    point first_error_position;
    std::size_t error_count;
};
```

`first_error_position` は、フィルター呼び出しが最初に失敗したピクセル位置です。

`error_count` は、フィルター呼び出しに失敗したピクセル総数です。

失敗位置は最初の1件だけを保存します。これにより、失敗ピクセルの巨大なリストを確保せずに、呼び出し側に有用な診断位置を返せます。

---

## 論理ピクセル

`xer::image::pixel` は論理色値を表します。

これは物理フレームバッファのストレージ要素とは同じではありません。物理ストレージ形式は canvas policy によって制御されます。

論理表現は32ビット整数のARGBです。

```text
0xAARRGGBB
```

概念的な形は次のとおりです。

```cpp
struct pixel {
    std::uint32_t argb = 0xff000000u;

    constexpr pixel() noexcept = default;
    constexpr explicit pixel(std::uint32_t value) noexcept;
    constexpr pixel(std::uint8_t red,
                    std::uint8_t green,
                    std::uint8_t blue) noexcept;
    constexpr pixel(std::uint8_t alpha,
                    std::uint8_t red,
                    std::uint8_t green,
                    std::uint8_t blue) noexcept;

    constexpr auto alpha() const noexcept -> std::uint8_t;
    constexpr auto red() const noexcept -> std::uint8_t;
    constexpr auto green() const noexcept -> std::uint8_t;
    constexpr auto blue() const noexcept -> std::uint8_t;

    constexpr auto alpha(std::uint8_t value) noexcept -> void;
    constexpr auto red(std::uint8_t value) noexcept -> void;
    constexpr auto green(std::uint8_t value) noexcept -> void;
    constexpr auto blue(std::uint8_t value) noexcept -> void;
};
```

3引数コンストラクタはRGBを表し、アルファ値を `0xff` に設定します。

4引数コンストラクタの順序はARGBです。

```text
alpha, red, green, blue
```

---

## フレームバッファストレージポリシー

canvas policy は、物理フレームバッファのストレージ形式を制御します。

ポリシーは次を提供します。

```cpp
using storage_type = /* physical storage element type */;

static constexpr auto get(const storage_type& value) noexcept -> pixel;
static constexpr auto encode(pixel value) noexcept -> storage_type;
static constexpr auto set(storage_type& dst, pixel value) noexcept -> void;
```

初期ポリシーは次のとおりです。

```cpp
argb32_policy
rgba32_policy
rgb24_policy
bgr24_policy
```

`argb32_policy` は `0xAARRGGBB` を格納するため、論理 `pixel` 表現と直接一致します。

`rgba32_policy` は `0xRRGGBBAA` を格納します。

`rgb24_policy` と `bgr24_policy` は3つの8ビット成分を格納し、アルファ値は保持しません。これらのポリシーを通じて読み出すと、アルファ値が `0xff` の論理ピクセルが返ります。

---

## `canvas`

主要なキャンバス型は次のとおりです。

```cpp
template <std::size_t Width,
          std::size_t Height,
          class Policy = argb32_policy>
class canvas;
```

固定サイズキャンバスが主モデルです。初期用途がVRAMエミュレーションのようなフレームバッファ風の処理だからです。

例:

```cpp
using screen = xer::image::canvas<256, 192>;
using sprite = xer::image::canvas<16, 16>;
using rgba_screen = xer::image::canvas<256, 192, xer::image::rgba32_policy>;
```

動的サイズキャンバスは次の形で表します。

```cpp
canvas<0, 0, Policy>
```

便利な別名は次のとおりです。

```cpp
template <class Policy = argb32_policy>
using dynamic_canvas = canvas<0, 0, Policy>;
```

有効な寸法指定は次の2種類だけです。

```text
Width > 0 && Height > 0
Width == 0 && Height == 0
```

`canvas<0, 192>` や `canvas<256, 0>` のように片方だけ動的な寸法は無効です。

---

## 公開ピクセルアクセス

公開ピクセルAPIは論理ピクセルを使います。

```cpp
auto get_pixel(std::size_t x, std::size_t y) const noexcept -> pixel;
auto get_pixel(const point& p) const noexcept -> pixel;
auto set_pixel(int x, int y, pixel value) noexcept -> void;
auto set_pixel(const point& p, pixel value) noexcept -> void;
auto set_pixel(int x, int y, pixel value, float coverage) noexcept -> void;
auto set_pixel(const point& p, pixel value, float coverage) noexcept -> void;
auto set_pixel_unchecked(std::size_t x,
                         std::size_t y,
                         pixel value) noexcept -> void;
auto set_pixel_unchecked(std::size_t x,
                         std::size_t y,
                         pixel value,
                         float coverage) noexcept -> void;
```

`canvas::at()` は意図的に提供していません。

物理ストレージ要素への参照を返すとフレームバッファレイアウトを露出してしまいます。また、ストレージポリシーがARGBでない場合には不正確です。`pixel` は論理値であり、`Policy::storage_type` は物理値です。

`get_pixel` は、座標がキャンバス内にあることを期待します。

`set_pixel` は符号付き座標を受け取り、座標がキャンバス境界外の場合は何もしません。

coverage付きオーバーロードは、元ピクセルを先ピクセルの上にブレンドします。coverage は `[0.0f, 1.0f]` に丸められます。`0.0f` は先ピクセルを変更しません。`1.0f` は元ピクセルのアルファ値を通常どおり適用します。

`set_pixel_unchecked` は境界チェックを行いません。呼び出し側は `x < width()` かつ `y < height()` を保証しなければなりません。これは、内側の描画ループの外側でクリッピングや境界チェックを済ませたコード向けです。

---

## 基本メンバー関数

`canvas` は基本的なサイズ取得とユーティリティ操作を提供します。

```cpp
auto width() const noexcept -> std::size_t;
auto height() const noexcept -> std::size_t;
auto size() const noexcept -> std::size_t;
auto empty() const noexcept -> bool;
auto contains(int x, int y) const noexcept -> bool;
auto contains(const point& p) const noexcept -> bool;
auto fill(pixel value) noexcept -> void;
auto clear() noexcept -> void;
```

`clear()` はキャンバスを不透明な黒で塗りつぶします。

---

## ビットマップフォント型

`<xer/image.h>` は、XBFファイルから読み込んだ等幅ビットマップフォントのコンパクトな実行時表現を定義します。

```cpp
enum class bitmap_glyph_width : std::uint8_t {
    half,
    full,
};

struct bitmap_font_range {
    char32_t first_code_point {};
    char32_t last_code_point {};
    bitmap_glyph_width glyph_width = bitmap_glyph_width::half;
    std::uint64_t bitmap_offset = 0;
};

struct bitmap_font {
    int half_width = 0;
    int full_width = 0;
    int glyph_height = 0;
    std::vector<bitmap_font_range> ranges {};
    std::vector<std::uint8_t> bitmap {};
};

struct text_draw_options {
    int letter_spacing = 0;
    int line_spacing = 0;
};
```

`bitmap_font` は次を格納します。

- 半角セル幅
- 全角セル幅
- フォント全体で共有されるグリフ高さ
- ソート済みで重なりのないUnicodeコードポイント範囲
- パックされた1bppグリフビットマップバイト列

各範囲は半角セルまたは全角セルのいずれかを選択します。幅の種類はUnicodeコードポイントから推測せず、フォントデータに格納します。

`text_draw_options` は `draw_text` の呼び出しごとのレイアウト制御です。

- `letter_spacing` は描画された各グリフセルの後に加算されます
- `line_spacing` は改行処理時に `glyph_height` へ加算されます

負の間隔値も許可され、グリフセルが重なる場合があります。

---

## ビットマップフォント読み込み

```cpp
[[nodiscard]] auto bitmap_font_load(const xer::path& filename)
    -> xer::result<bitmap_font, xer::parse_error_detail>;
```

`bitmap_font_load` はXBFビットマップフォントファイルを読み込み、検証済みの `bitmap_font` を返します。

XBFは xer のコンパクトなバイナリビットマップフォント形式です。XBFは次を格納します。

- リトルエンディアンの数値フィールド
- 等幅の半角・全角グリフセル
- 共通のグリフ高さ
- Unicodeコードポイント範囲
- パックされた1bppビットマップデータ

ローダーは、成功を返す前に、XBFヘッダー、範囲テーブル、ビットマップ範囲、予約フィールド、コードポイント範囲、および関連オフセットを検証します。

### エラー

XBFの解析開始前にファイルI/Oが失敗した場合、`bitmap_font_load` は下位のファイル関連エラーコードを保持し、理由が `parse_error_reason::none` の空の `parse_error_detail` を返します。

XBFバイト列が不正な場合は、`parse_error_detail` とともに `error_t::invalid_argument` を返します。

XBFでは次のように扱います。

- `offset` はバイナリ入力先頭からのバイトオフセット
- `line` は `0`
- `column` は `0`

XBFローダーは、たとえば次の理由を報告する場合があります。

- `parse_error_reason::invalid_magic`
- `parse_error_reason::unsupported_version`
- `parse_error_reason::invalid_header`
- `parse_error_reason::invalid_range`
- `parse_error_reason::invalid_offset`
- `parse_error_reason::truncated_input`

共通の解析詳細モデルとXBF方針については、`header_parse.md` と `policy_bitmap_font.md` を参照してください。

---

## 描画関数

初期の描画関数は単純なフレームバッファヘルパーです。

```cpp
draw_hline
draw_vline
draw_line
draw_line_aa
draw_rect
fill_rect
```

整数描画座標には `std::size_t` ではなく `int` を使います。描画では負の座標をクリッピングできる方が便利なことが多いためです。

描画操作はキャンバス境界でクリッピングされます。対象領域が完全にキャンバス外にある場合は何も描画しません。

クリッピング後、`draw_hline`、`draw_vline`、`fill_rect` はフレームバッファストレージへ直接書き込みます。各ピクセルごとに `set_pixel` を呼びません。これにより、内側ループを座標からオフセットへの繰り返し計算ではなく、単純なポインタまたはストライド加算にできます。

`draw_line` は単純なBresenham風の整数直線アルゴリズムを使います。生成された各点についてキャンバス境界の確認は行いますが、その確認後は `set_pixel_unchecked` で書き込みます。

`draw_line_aa` は浮動小数点のピクセル中心座標を使い、アンチエイリアス付きのカプセル状ストロークを描画します。幅引数のないオーバーロードは1ピクセル幅のアンチエイリアス直線を描きます。幅付きオーバーロードでは、幅引数は色引数の前に置かれます。`pointf` オーバーロードはスカラー座標オーバーロードと等価です。

`draw_line_aa` は `xer::result<void>` を返します。いずれかの座標が有限でない場合、または `width` が有限でないか0以下の場合、`error_t::invalid_argument` を返します。完全にキャンバス外にある直線は成功したno-opです。

`draw_rect` と `fill_rect` のオーバーロードは、`point` と `size` の組、または単一の `rect` を受け取ります。すでに座標値を個別に持っている呼び出し側のために、スカラー座標オーバーロードも残されています。

---

## ビットマップテキスト描画

```cpp
template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto draw_text(canvas<Width, Height, Policy>& img,
                             int x,
                             int y,
                             std::u8string_view text,
                             const bitmap_font& font,
                             pixel color,
                             const text_draw_options& options = {}) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto draw_text(canvas<Width, Height, Policy>& img,
                             const point& origin,
                             std::u8string_view text,
                             const bitmap_font& font,
                             pixel color,
                             const text_draw_options& options = {}) noexcept
    -> xer::result<void>;
```

`draw_text` は、読み込み済みの `bitmap_font` を使ってUTF-8テキストをキャンバスへ描画します。

原点は最初のグリフセルの左上位置です。ベースライン指向の配置は、初期ビットマップフォントAPIには意図的に含めていません。

### レイアウト規則

- 通常のグリフは読み込み済みビットマップデータから描画されます
- グリフを描画した後、ペン位置はグリフセル幅と `letter_spacing` の分だけ進みます
- `\n`、`\r`、`\r\n` は新しい行を開始します
- 改行は x 位置を元の行原点に戻します
- 改行は y 位置を `glyph_height + line_spacing` だけ進めます
- フォントに存在しないコードポイントは、描画せず、ペンも進めずにスキップされます

欠落グリフ規則は意図的に最小限です。初期APIはフォールバック幅を推測したり、`?` を自動的に代用したりしません。

### クリッピング

描画はキャンバス境界でクリッピングされます。グリフはキャンバス外から開始してもよく、見えているセットピクセルだけが書き込まれます。

### エラー

`draw_text` は次を返します。

- `text` が妥当なUTF-8でない場合は `error_t::encoding_error`
- 指定された `bitmap_font` が要求グリフに対して構造的に使用不能な場合は `error_t::invalid_argument`

空のキャンバスまたは空のテキストは成功したno-opです。

---

## 円、楕円、円弧の描画

曲線形状APIは、整数の1ピクセル描画と浮動小数点のアンチエイリアス描画に分かれます。

- 整数APIは `point` またはスカラー `int` 中心座標を使います
- アンチエイリアスAPIは `pointf` またはスカラー `float` 中心座標を使います
- アンチエイリアス輪郭APIは省略可能な `width` を受け取ります
- すべての曲線形状関数は `[[nodiscard]]` なしで `xer::result<void>` を返します

### 円描画

`draw_circle` は、クリッピングされた1ピクセル幅の円輪郭を描画します。`fill_circle` はクリッピングされた円の内部を塗りつぶし、境界も含みます。

`draw_circle_aa` はアンチエイリアス付き輪郭を描画します。幅引数のないオーバーロードは `1.0f` を使います。幅付きオーバーロードは太い円輪郭に対応します。`fill_circle_aa` は外側境界をアンチエイリアスしながら円を塗りつぶします。

半径の扱いは次のとおりです。

- 負の半径は `error_t::invalid_argument` を返します
- 整数半径0は、見えていれば中心ピクセルだけを書き込みます
- アンチエイリアス輪郭の半径0は、直径が `width` に従う丸い点を描画します
- アンチエイリアス塗りつぶしの半径0は中心点を描画します

アンチエイリアス円描画では、中心座標、半径、幅は有限でなければなりません。`width` は0より大きくなければなりません。

### 楕円描画

`draw_ellipse` は、クリッピングされた1ピクセル幅の楕円輪郭を描画します。`fill_ellipse` はクリッピングされた楕円の内部を塗りつぶし、境界も含みます。

`draw_ellipse_aa` はアンチエイリアス付き楕円輪郭を描画します。幅引数のないオーバーロードは `1.0f` を使います。幅付きオーバーロードは太い楕円輪郭に対応します。`fill_ellipse_aa` は外側境界をアンチエイリアスしながら楕円を塗りつぶします。

半径の扱いは次のとおりです。

- いずれかの半径が負なら `error_t::invalid_argument` を返します
- 両方の整数半径が0なら、見えていれば中心ピクセルだけを書き込みます
- 片方の整数半径だけが0なら、縦線または横線へ退化します
- アンチエイリアス楕円でも同じ退化形を扱います

アンチエイリアス楕円描画では、中心座標、半径、幅は有限でなければなりません。`width` は0より大きくなければなりません。

### 円弧描画

円弧APIの角度は τrad 単位で表します。`0` は右方向を指します。正の sweep 角は数学的な意味で反時計回りに進みます。画像の y 座標は下向きに増えるため、点の式は次のようになります。

概念的には次の座標式です。

```text
x = cx + radius * cos(angle * τ)
y = cy - radius * sin(angle * τ)
```

`sweep_angle` が正の場合は反時計回り、負の場合は時計回りに描画します。絶対値が1回転以上の sweep は完全な円として扱われます。sweep が0の場合は開始点を描画します。

整数円弧描画は負の半径と有限でない角度を拒否します。アンチエイリアス円弧描画は、有限でない中心座標、半径、幅も拒否し、`width <= 0.0f` も拒否します。

### 楕円弧描画

楕円弧の角度は円弧と同じ規則に従います。

```text
x = cx + radius_x * cos(angle * τ)
y = cy - radius_y * sin(angle * τ)
```

1回転以上の sweep は完全な楕円として扱われます。sweep が0の場合は開始点を描画します。両方の半径が0の場合は中心点になります。片方の半径だけが0の場合は、角度に基づくパラメータ化を保ったまま、対応する縦線または横線へ退化します。

アンチエイリアス楕円弧は丸い端点を使い、`width` による太線に対応します。

楕円弧描画は負の半径と有限でない角度を拒否します。アンチエイリアス楕円弧描画は、有限でない中心座標、半径、幅も拒否し、`width <= 0.0f` も拒否します。

### クリッピング、ピクセル、返却値

すべての曲線形状描画はキャンバス境界でクリッピングされます。形状が完全にキャンバス外にある場合は成功したno-opです。

整数の円・楕円描画は、指定された論理 `pixel` を直接書き込みます。アンチエイリアス描画は canvas ピクセルAPIを通じて coverage ブレンドを使います。

これらの描画関数は `xer::result<void>` を返しますが、返却値には意図的に `[[nodiscard]]` を付けていません。これにより、描画コードでの呼び出しを軽く保ちつつ、必要な場所では不正引数を扱えます。

---

## 塗りつぶし

```cpp
template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto flood_fill(canvas<Width, Height, Policy>& img,
                              int x,
                              int y,
                              pixel color)
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto flood_fill(canvas<Width, Height, Policy>& img,
                              const point& origin,
                              pixel color)
    -> xer::result<void>;
```

`flood_fill` は、開始位置を含む4近傍連結領域を置換します。

開始位置の元の論理 `pixel` 値が対象色として使われます。その元の色と論理ARGB値が完全一致する到達可能なすべてのピクセルが `color` に置換されます。

### 連結性

初期実装は4近傍のみを使います。

- 左
- 右
- 上
- 下

斜めに接しているだけでは、2つの領域は連結しているとはみなしません。

### no-opの場合

`flood_fill` は次の場合に成功したno-opです。

- 開始位置がキャンバス外にある
- 置換色が開始位置の元の色と等しい

### 結果

`flood_fill` は `xer::result<void>` を返します。

この操作は再帰探索ではなく、内部の保留位置バッファを使います。そのため、大きな塗りつぶし領域でもコールスタックの深さに依存しません。

---

## 画像処理関数

`mosaic`、`box_blur`、`filter_pixels` はインプレース画像処理操作です。

```cpp
template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto mosaic(canvas<Width, Height, Policy>& img,
                          const rect& area,
                          const size& block_size) noexcept
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto box_blur(canvas<Width, Height, Policy>& img,
                            const rect& area,
                            const size& box_size)
    -> xer::result<void>;

template <std::size_t Width, std::size_t Height, class Policy, class F>
[[nodiscard]] auto filter_pixels(canvas<Width, Height, Policy>& img,
                                 const rect& area,
                                 F&& filter)
    -> xer::result<void, filter_pixels_error_detail>;
```

3つの関数はいずれも `area` をキャンバス境界でクリッピングします。空領域や完全にクリップされた領域は成功したno-opです。

`mosaic` は、クリップ後の領域を `block_size` のブロックに分割します。各ブロックは、そのブロック内ピクセルの平均論理ARGB色で置換されます。右端と下端のブロックは、実際にクリップされたサイズを使います。

`box_blur` は `box_size` を平均化カーネルサイズとして扱います。たとえば `size(3, 3)` は各出力ピクセルの周囲に3x3平均を適用します。ソースサンプルは、クリップされた対象領域内の元ピクセルのコピーから取得されるため、要求領域外のピクセルは結果に影響しません。クリップ領域外に出るカーネル部分は無視されます。

偶数のカーネル寸法にも対応します。この場合、余分なサンプルは現在ピクセルの左側または上側に置かれます。

`mosaic` と `box_blur` は、いずれかのサイズ寸法が正でない場合に `error_t::invalid_argument` を返します。

`filter_pixels` は、呼び出し側が指定したピクセル単位フィルターをクリップ後領域に適用します。各ピクセルについて、フィルターは現在の論理 `pixel` 値を受け取り、置換する論理 `pixel` 値を返します。これにより、グレースケール変換、しきい値処理、チャンネル調整、反転などを、効果ごとの専用関数を増やさずに実現できます。

操作はインプレースで、画像全体の一時バッファは確保しません。フィルターがあるピクセルで例外を投げた場合、そのピクセルは変更されず、処理は次のピクセルへ進みます。1つ以上のピクセルで失敗した場合、関数は `filter_pixels_error_detail` 付きの `error_t::user_error` を返します。正常にフィルターされたピクセルは更新されたままです。

グレースケール風の使用例:

```cpp
auto result = xer::image::filter_pixels(
    img,
    xer::image::rect(xer::image::point(0, 0), xer::image::size(16, 16)),
    [](xer::image::pixel p) -> xer::image::pixel {
        const auto gray = static_cast<std::uint8_t>(
            (static_cast<unsigned>(p.red()) +
             static_cast<unsigned>(p.green()) +
             static_cast<unsigned>(p.blue())) / 3u);

        return xer::image::pixel(p.alpha(), gray, gray, gray);
    });
```

---

## Tcl/Tkとの関係

`<xer/image.h>` はTcl/Tkに依存しません。

Tk photo ブリッジ関数は `<xer/tk.h>` に置くべきです。将来的には、Tk photo image block と `xer::image::canvas` または `xer::image::dynamic_canvas` の間の変換を提供する可能性がありますが、純粋な画像ストレージ、描画、画像処理は `<xer/image.h>` に残ります。

---

## 後回しにしている項目

現在の実装では、次の項目を後回しにしています。

- アフィン変換
- ラスタスクロール
- グレースケール変換
- 画像反転
- ファイル形式の読み込みと保存
- 直接的なTk photo変換ヘルパー

これらは、基本的なフレームバッファ型が安定してから追加できます。

---

## 例

```cpp
#include <xer/image.h>
#include <xer/stdio.h>

auto main() -> int
{
    xer::image::canvas<4, 4> img;

    img.clear();

    // This line intentionally starts outside the canvas.
    // xer clips it to the framebuffer boundary.
    xer::image::draw_hline(
        img,
        -2,
        1,
        4,
        xer::image::pixel(0xffu, 0x00u, 0x00u));

    const auto value = img.get_pixel(0, 1);
    return value.argb == 0xffff0000u ? 0 : 1;
}
```

追加の例:

- `examples/example_image_basic.cpp`
- `examples/example_image_geometry_io.cpp`
- `examples/example_image_effects.cpp`
- `examples/example_image_filter_pixels.cpp`
- `examples/example_image_bitmap_text.cpp`
- `examples/example_image_flood_fill.cpp`
- `examples/example_image_circle.cpp`
- `examples/example_image_curves.cpp`

---

## 関連項目

- `header_iostream.md`
- `header_parse.md`
- `policy_image.md`
- `policy_bitmap_font.md`

---

# `<xer/process.h>`

## 目的

`<xer/process.h>` は子プロセス管理機能を提供します。

初期 API は意図的に小さく、プロセスの直接起動、待機、標準ストリームの接続に重点を置いています。

---

## 主な役割

このヘッダーは次の機能を提供します。

- ムーブ専用のプロセスハンドル
- コマンドシェルを介さない子プロセスの直接起動
- 標準入力、標準出力、標準エラー出力の設定
- `binary_stream` オブジェクトとして公開される任意のパイプ
- プロセスの待機と終了コードの取得

---

## 主な型

```cpp
enum class process_stdio;
struct process_options;
struct process_result;
class process;
struct process_spawn_result;
```

### `process_stdio`

```cpp
inherit
null
pipe
```

- `inherit` は子プロセスのストリームを対応する親プロセスのストリームに接続します。
- `null` は子プロセスのストリームをプラットフォームのヌルデバイスに接続します。
- `pipe` は親側のパイプを作成し、`binary_stream` として表します。

### `process_options`

```cpp
path program;
std::vector<std::u8string> arguments;
process_stdio stdin_mode;
process_stdio stdout_mode;
process_stdio stderr_mode;
```

`arguments` には `argv[0]` を含めません。プログラムパスは別に指定します。

### `process_result`

```cpp
int exit_code;
```

POSIX では、シグナルによる終了は `128 + signal_number` として表されます。

### `process_spawn_result`

```cpp
process proc;
std::optional<binary_stream> stdin_stream;
std::optional<binary_stream> stdout_stream;
std::optional<binary_stream> stderr_stream;
```

任意のストリームは、対応する `process_stdio::pipe` モードが要求された場合にだけ存在します。

---

## 主な関数

```cpp
auto process_spawn(const process_options& options) noexcept -> xer::result<process_spawn_result>;
auto process_wait(process& value) noexcept -> xer::result<process_result>;
```

`process_spawn` は対象プログラムを直接実行し、引数を個別のコマンドライン引数として渡します。
コマンドシェルは起動しません。

`process_wait` は子プロセスを待機し、その終了状態を返します。

---

## プロセスハンドル

`process` はムーブ専用のハンドル型です。

```cpp
auto is_open() const noexcept -> bool;
```

デストラクタはオブジェクトが所有するネイティブハンドルを解放しますが、プロセスの終了は待機しません。
終了コードが必要な場合は、`process_wait` を明示的に呼び出してください。

---

## 注意

- パスには `xer::path` を使い、内部でネイティブパス変換を行います。
- 引数には UTF-8 の `std::u8string` 値を使います。
- Windows では、直接プロセス生成のためのコマンドライン引用処理を内部で行います。
- POSIX では、子プロセスをプラットフォームのプロセス機能で作成し、その後直接実行します。
- パイプストリームはバイナリストリームです。必要であれば、より高水準のテキスト処理を別に重ねられます。

---

# `<xer/cmdline.h>`

## 目的

`<xer/cmdline.h>` は現在のプロセスのコマンドライン引数処理機能を提供します。

このヘッダーの目的は、呼び出し側が `main` の `argc` と `argv` を手作業で引き回さなくても、コマンドライン引数を UTF-8 文字列として利用できるようにすることです。

これは次のような状況で有用です。

* `main` の外で実行されるコード
* 非ローカルオブジェクトの初期化
* メインスレッド以外のスレッドで動くコード
* `argc` と `argv` を明示的に運ぶのが煩雑なユーティリティ関数

---

## 主なエンティティ

少なくとも、`<xer/cmdline.h>` は次のエンティティを提供します。

```cpp
using cmdline_arg =
    std::pair<std::u8string_view, std::u8string_view>;

class cmdline;

auto parse_arg(std::u8string_view value) noexcept -> cmdline_arg;

auto get_cmdline() -> xer::result<cmdline>;
````

---

## `cmdline`

`cmdline` は argv 風の UTF-8 文字列列を所有します。

```cpp
class cmdline;
```

内部では、コマンドライン引数を次の形で保持します。

```cpp
std::vector<std::u8string>
```

クラス自体はオプションを解釈しません。
引数列を所有し公開することだけを担当します。

### 基本操作

```cpp
auto size() const noexcept -> std::size_t;
auto empty() const noexcept -> bool;

auto args() const noexcept -> std::span<const std::u8string>;

auto at(std::size_t index) const -> xer::result<std::u8string_view>;
```

### `size`

`size()` は保持している引数の数を返します。

### `empty`

`empty()` は引数リストが空かどうかを返します。

`get_cmdline` の通常の成功利用では、コマンドラインリストは少なくともプログラム名を含むことが期待されますが、手動で構築された `cmdline` オブジェクトについて呼び出し側はそれに依存すべきではありません。

### `args`

`args()` は保持されている生の UTF-8 引数への span を返します。

返された span とその文字列参照は、`cmdline` オブジェクトが生存し、変更されない限り有効です。

### `at`

`at(index)` は 1 つの生の引数を `std::u8string_view` として返します。

`index` が範囲外の場合、`xer::result` を通じてエラーを返します。

---

## `parse_arg`

```cpp
auto parse_arg(std::u8string_view value) noexcept -> cmdline_arg;
```

`parse_arg` は、xer の単純なコマンドライン規則に従って 1 つの生のコマンドライン引数を解析します。

返り値は次のペアです。

```cpp
{ option_name, value }
```

意味は次のとおりです。

* `first` が空でなければ、その引数はオプションです。
* `first` が空であれば、その引数は通常の値です。
* `second` にはオプション値または通常の値が入ります。

---

## 対応する引数形式

xer は単純なロングオプション形式だけを認識します。

対応するオプション形式は次のとおりです。

```text
--option
--option=value
```

通常の値も受け付けます。

```text
value
```

`-x` のようにハイフン 1 つで始まる形式はオプションとして扱いません。
通常の値として扱います。

### 例

```text
--name        -> { "name", "" }
--name=       -> { "name", "" }
--name=value  -> { "name", "value" }
value         -> { "", "value" }
-name         -> { "", "-name" }
--            -> { "", "--" }
--=value      -> { "", "--=value" }
```

`--name` と `--name=` は意図的に同じ扱いです。

「値なし」と「空の値」を区別するには、より複雑な表現が必要になります。xer の初期コマンドラインヘルパーは、その複雑さを意図的に避けています。

---

## 短いオプションを特別扱いしない理由

`parse_arg` はハイフン 1 つで始まる引数をオプションとして扱いません。

たとえば、

```text
-x
```

は次のように解析されます。

```text
{ "", "-x" }
```

これは意図的です。

初期のコマンドラインモデルは次だけをサポートします。

* `--option`
* `--option=value`
* 通常の値

これにより規則を単純に保ち、この段階でより大きなコマンドラインパーサーを導入することを避けています。

---

## `get_cmdline`

```cpp
auto get_cmdline() -> xer::result<cmdline>;
```

`get_cmdline` は現在のプロセスのコマンドライン引数を取得し、`cmdline` オブジェクトとして返します。

返される引数は UTF-8 文字列です。

### Windows での動作

Windows では、実装は次を使って生のコマンドラインを取得します。

```cpp
GetCommandLineW
```

そして次で分割します。

```cpp
CommandLineToArgvW
```

これにより、`__wargv` のような CRT 固有のグローバルに依存することを避けます。

コマンドラインアクセスは C ランタイムライブラリのリンク方法の詳細に依存すべきではないため、この選択は意図的です。

得られた UTF-16 文字列は UTF-8 に変換されます。

### Linux での動作

Linux では、実装は次を読みます。

```text
/proc/self/cmdline
```

このファイルには、現在のプロセスのコマンドライン引数が NUL 区切りのバイト文字列として含まれます。

そのバイト文字列は、xer の Linux テキスト前提に従って UTF-8 として解釈されます。
引数が有効な UTF-8 でない場合、`get_cmdline` は失敗します。

通常でない環境では、理論上 `/proc/self/cmdline` の読み取りが失敗する場合があります。
その場合、`get_cmdline` は `xer::result` を通じてエラーを返します。

---

## ビューの寿命

`cmdline::at` と `parse_arg` は `std::u8string_view` 値を返します。

これらのビューは背後のテキストを所有しません。

`cmdline` オブジェクトから得たビューの場合、参照先データが有効なのは `cmdline` オブジェクトが生存し、変更されない間だけです。

例:

```cpp
const auto line = xer::get_cmdline();
if (!line.has_value()) {
    return 1;
}

const auto raw = line->at(1);
if (!raw.has_value()) {
    return 1;
}

const auto parsed = xer::parse_arg(*raw);
```

ここで、`parsed.first` と `parsed.second` は `line` が所有する文字列を参照しています。

---

## エラー処理

`<xer/cmdline.h>` は xer の通常の失敗モデルに従います。

`parse_arg` 自体は失敗しません。
これは単純なビューベースのパーサーであり、通常の `cmdline_arg` を返します。

`cmdline::at` は要求されたインデックスが範囲外の場合に失敗することがあります。

`get_cmdline` は、プラットフォーム固有のコマンドライン取得が失敗した場合、またはコマンドラインデータを xer の UTF-8 表現に変換できない場合に失敗することがあります。

典型的な失敗条件には次のものがあります。

* 範囲外の引数アクセス
* プラットフォームのコマンドライン取得失敗
* `/proc/self/cmdline` の読み取り失敗
* Linux のコマンドラインバイト文字列に含まれる無効な UTF-8
* Windows の UTF-16 コマンドライン文字列から UTF-8 への変換失敗

---

## `main` との関係

通常の C および C++ でコマンドライン引数を受け取る方法は `main` です。

```cpp
auto main(int argc, char** argv) -> int;
```

xer はこの方法を否定しません。

ただし、`<xer/cmdline.h>` は、明示的な `argc` / `argv` の伝播が不便または利用できない場合のために存在します。

つまり、`get_cmdline` は現在のプロセスのための便利機能であり、`main` 引数のすべての用途を置き換えるものではありません。

---

## プロセス処理との関係

`<xer/cmdline.h>` は現在のプロセスのコマンドラインを扱います。

`<xer/process.h>` は子プロセスの作成と管理を扱います。

これらは関連する話題ですが、意図的に分離されています。

* `cmdline.h` は現在のプロセスがどのように起動されたかを観測します。
* `process.h` は子プロセスを起動し制御します。

この分離により、それぞれのヘッダーの焦点を保っています。

---

## 設計上の役割

`<xer/cmdline.h>` は意図的に小さく作られています。

完全なコマンドラインオプションパーサーではありません。

特に、現在は次の機能を提供しません。

* `-x` のような短いオプションの解析
* `-abc` のような短いオプションのグループ化
* `--` のようなオプション終端の扱い
* 自動的な型変換
* 必須オプション検証
* ヘルプテキスト生成
* サブコマンド処理

初期機能は、小さな argv 取得機能と単純なロングオプション解析機能だけです。

---

## 例

```cpp
#include <xer/cmdline.h>
#include <xer/stdio.h>

auto main() -> int
{
    const auto line = xer::get_cmdline();
    if (!line.has_value()) {
        return 1;
    }

    for (std::size_t i = 1; i < line->size(); ++i) {
        const auto raw = line->at(i);
        if (!raw.has_value()) {
            return 1;
        }

        const auto parsed = xer::parse_arg(*raw);

        if (!parsed.first.empty()) {
            if (!xer::printf(
                    u8"option %@ = %@\n",
                    parsed.first,
                    parsed.second)
                     .has_value()) {
                return 1;
            }
        } else {
            if (!xer::printf(u8"value %@\n", parsed.second).has_value()) {
                return 1;
            }
        }
    }

    return 0;
}
```

---

## ドキュメント上の注意

このヘッダーを生成ドキュメントで使う場合、通常は次の点を説明すれば十分です。

* `cmdline` は UTF-8 コマンドライン引数を所有すること
* `get_cmdline` は `main` のパラメータを使わずに現在のプロセス引数を取得すること
* Windows では `GetCommandLineW` と `CommandLineToArgvW` を使うこと
* Linux では `/proc/self/cmdline` を読むこと
* `parse_arg` は単純な `--option` と `--option=value` 形式だけを認識すること
* ハイフン 1 つで始まる引数は通常の値として扱われること

詳細なコマンドラインパーサー動作を暗示すべきではありません。
このヘッダーは意図的に完全なオプション解析フレームワークではありません。

---

## 例として示す価値のある題材

次のような例は、このヘッダーに特に適しています。

* 生のコマンドライン引数を列挙する
* `--option` と `--option=value` を解析する
* 通常の値とオプションを分けて扱う
* `-x` が値として扱われることを示す

これらは `examples/` の実行可能例に適した候補です。

---

## 関連項目

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `policy_process.md`
* `header_process.md`

---

# `<xer/time.h>`

## 目的

`<xer/time.h>` は、xer の時刻関連機能を提供します。

このヘッダーの目的は、C標準ライブラリの `time.h` をそのまま再現することでも、公開APIの中心に `std::chrono` を置くことでもありません。
かわりに、C形式の時刻処理が持つ近づきやすさを保ちながら、xer 独自の設計に沿った、より単純な時刻ライブラリを提供します。

このヘッダーは、次のような作業を対象としています。

- 現在時刻の取得
- スカラー時刻値と要素分解時刻の相互変換
- 時刻値のテキスト書式化
- 単純な時刻差の計算

---

## 主な役割

`<xer/time.h>` の主な役割は、次の考え方に基づく、実用的で明示的な時刻モデルを提供することです。

- `time_t` は単純で算術演算に向いている
- 通常の失敗は `xer::result` で報告する
- 要素分解時刻は `tm` 構造体で表す
- 秒未満の精度を明示的に扱う
- 書式化はUTF-8指向とする

これにより、このヘッダーは、`std::chrono` にありがちな重めの式表現を避けつつ、単純な時刻処理を行いたいコードに適したものになります。

---

## 主なエンティティ

少なくとも、`<xer/time.h>` は次のエンティティを提供します。

```cpp
using time_t = double;
using clock_t = std::clock_t;

struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
    int tm_microsec;
};

auto time() -> xer::result<time_t>;
auto clock() -> xer::result<clock_t>;
auto difftime(time_t left, time_t right) noexcept -> double;

auto gmtime(time_t value) -> xer::result<tm>;
auto localtime(time_t value) -> xer::result<tm>;
auto mktime(const tm& value) -> xer::result<time_t>;

auto ctime(time_t value) -> std::u8string;
auto ctime(const tm& value) -> std::u8string;
auto strftime(std::u8string_view format, const tm& value) -> xer::result<std::u8string>;
```

正確な補助関数群は将来拡張される可能性がありますが、これが現在の中核的な公開形状です。

---

## `xer::time_t`

`xer::time_t` は、xer における中心的なスカラー時刻型です。

### 基本形

```cpp
using time_t = double;
```

### 意味

`xer::time_t` の単位は秒です。

その解釈は次のとおりです。

* 整数部: 秒単位の整数部分
* 小数部: 秒未満の部分

### `double` を使う理由

この設計により、時刻値は次の性質を持ちます。

* 算術的に扱いやすい
* 軽量である
* 実用上のマイクロ秒レベル精度に適している
* 重い duration 形式の抽象よりも通常のコードで扱いやすい

### 精度の方向性

実用上の目標はマイクロ秒レベルの処理ですが、公開設計として、厳密な固定小数点表現や保証されたナノ秒精度を約束するものではありません。

---

## エポック

xer は `xer::time_t` のエポックをPOSIXエポックに固定します。

### 意味

```text
1970-01-01 00:00:00 UTC
```

は、`xer::time_t` 値として次に対応します。

```text
0.0
```

### なぜ重要か

これにより、従来のCの `time_t` におけるエポック解釈の実装定義の曖昧さを避け、xer 全体で安定した規則を持てます。

---

## 対応範囲

少なくとも初期設計では、エポックより前の時刻は未対応です。

### 意味

* 負の `time_t` 値は未対応
* エポックより前の要素分解時刻は未対応
* そのような入力は失敗になる

これは初期段階における意図的な単純化です。

---

## `xer::tm`

`xer::tm` は、xer の要素分解時刻構造体です。

### 基本形

```cpp
struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
    int tm_microsec;
};
```

### Cの `struct tm` との関係

この構造体はCの `struct tm` を基礎にしていますが、次のメンバーを追加しています。

```cpp
tm_microsec
```

### `tm_microsec`

`tm_microsec` は、1秒未満のマイクロ秒部分を保持します。

意図された範囲は次のとおりです。

```text
0 .. 999999
```

### なぜ重要か

これにより、馴染みのある要素分解時刻モデルを捨てることなく、秒未満の精度を明示できます。

---

## `time()`

```cpp
auto time() -> xer::result<time_t>;
```

### 目的

`time()` は、現在の暦時刻を `xer::time_t` として返します。

### 設計方針

実際には失敗することはまれですが、xer はこれも通常の失敗しうる操作として扱い、`xer::result` で失敗を報告します。

これにより、このヘッダーはライブラリの他の部分と一貫します。

---

## `clock()`

```cpp
auto clock() -> xer::result<clock_t>;
```

### 目的

`clock()` は、基盤となるC機能の形式に沿った、プロセッサ時間またはクロック関連情報を返します。

### 設計方針

`time()` と同様に、xer はこれを通常の失敗しうる操作として扱い、失敗を明示的に報告します。

---

## `difftime`

```cpp
auto difftime(time_t left, time_t right) noexcept -> double;
```

### 目的

`difftime` は、2つの時刻値の差を返します。

### 意味

結果は秒単位のスカラー差です。

### 補足

この部分は、役割が構造変換ではなく算術演算であるため、ヘッダーの中でも単純な部分のひとつです。

---

## `gmtime`

```cpp
auto gmtime(time_t value) -> xer::result<tm>;
```

### 目的

`gmtime` は、スカラー時刻値をUTCの要素分解時刻に変換します。

### 振る舞い

* 小数部は `tm_microsec` に反映される
* 負の値は未対応であり、失敗になる

### 役割

これは、スカラー値からUTCの要素分解時刻へ変換する主要な入口です。

---

## `localtime`

```cpp
auto localtime(time_t value) -> xer::result<tm>;
```

### 目的

`localtime` は、スカラー時刻値をローカル時刻の要素分解時刻に変換します。

### 振る舞い

* 小数部は `tm_microsec` に反映される
* 負の値は未対応であり、失敗になる
* ローカル時刻への変換失敗は明示的に報告される

### 役割

これは `gmtime` に対応するローカル時刻版です。

---

## `mktime`

```cpp
auto mktime(const tm& value) -> xer::result<time_t>;
```

### 目的

`mktime` は、要素分解時刻値をスカラー時刻に変換します。

### 振る舞い

* `tm_microsec` は小数部に寄与する
* 範囲外の `tm_microsec` はエラーになる
* エポックより前の結果はエラーになる

### 役割

これは、`tm` から `time_t` へ戻す逆変換の入口です。

---

## `ctime`

xer は `ctime` を2つのオーバーロードで提供します。

```cpp
auto ctime(time_t value) -> std::u8string;
auto ctime(const tm& value) -> std::u8string;
```

### 目的

`ctime` は、時刻値を人間が読めるUTF-8テキストに変換します。

### 設計方針

xer では、従来Cの `ctime` と `asctime` が担っていた役割を、`ctime` という名前の下に統合しています。

つまり、次のようになります。

* `ctime(time_t)` はスカラー時刻値を書式化する
* `ctime(const tm&)` は要素分解時刻値を書式化する

### 重要な補足

* 返却型は `std::u8string`
* 静的な内部バッファは使わない
* 要素分解時刻版は生ポインターではなく `const tm&` を受け取る

これは、馴染みのある関数名を保ちながら、xer のC++形式の再設計を反映しています。

---

## `strftime`

```cpp
auto strftime(std::u8string_view format, const tm& value) -> xer::result<std::u8string>;
```

### 目的

`strftime` は、要素分解時刻値を、書式文字列に従って書式化します。

### 書式文字列モデル

書式文字列はUTF-8指向であり、次を使います。

```cpp
std::u8string_view
```

つまり、次のものを含めることができます。

* ASCIIの変換指定
* 固定のUTF-8テキスト

### 例

次のような書式は受け入れ可能であることを意図しています。

```text
%Y年%m月%d日 %H時%M分%S秒
```

### 返却モデル

結果は `std::u8string` として返されます。

ただし、返却は `xer::result` 経由です。

### xer固有の秒未満拡張

基盤となるCライブラリへ委譲される変換指定に加えて、xer は次の秒未満拡張を提供します。

* `%f`: マイクロ秒をちょうど6桁の10進数で出力する
* `%L`: ミリ秒をちょうど3桁の10進数で出力する

これらの指定子は `tm_microsec` に基づきます。対応するのは単純な `%f` と `%L` の形だけです。`%3f` のような幅、フラグ、修飾子付きの形式は拒否されます。

### 現在の設計上の制限

少なくとも現在の段階では、次の制限があります。

* 高度なロケール動作は優先事項ではない
* 高度なタイムゾーン拡張は後回し

---

## UTF-8指向の書式化

`<xer/time.h>` の重要な特徴のひとつは、書式化がUTF-8テキストを中心に設計されていることです。

### なぜ重要か

これにより、このヘッダーは xer のより広い公開テキストモデルと整合します。

* 公開テキストAPIはUTF-8指向である
* `std::u8string` は標準の所有テキスト型である
* `std::u8string_view` は標準の非所有テキスト入力型である

これは、よりロケール中心または狭文字だけを前提とする解釈とは異なる、xer の設計上の特徴です。

---

## エラー処理

`<xer/time.h>` は、xer の通常の失敗モデルに従います。

### 意味

失敗しうる操作は、`xer::result` を通じて失敗を報告します。

典型的な例は次のとおりです。

* 時刻情報の取得
* スカラー時刻と要素分解時刻の相互変換
* 無効な要素分解時刻入力
* 未対応のエポック前入力
* 該当する場合の無効な書式処理

### 典型的なエラーカテゴリ

詳細なエラー分類は実装に属しますが、設計意図としては特に次のようなカテゴリと関係します。

* `runtime_error`
* `invalid_argument`

---

## 後回しにしている機能

少なくとも初期段階では、次の機能は意図的に後回し、または単純化されています。

* エポック前の時刻対応
* `strftime` における高度なタイムゾーン機能
* 高度なロケール制御
* 高度なタイムゾーン機能
* C形式の静的バッファ動作
* `asctime` の個別公開

これは意図的な単純化であり、偶然の抜けではありません。

---

## 他のヘッダーとの関係

`<xer/time.h>` は、次の文書とあわせて理解してください。

* `policy_project_outline.md`
* `policy_time.md`
* `header_error.md`

大まかな境界は次のとおりです。

* `<xer/time.h>` は時刻取得、時刻変換、時刻書式化を扱う
* `<xer/error.h>` は、失敗しうる時刻操作で使うエラー/結果モデルを提供する

このヘッダーは、その領域内ではおおむね自己完結していますが、xer の通常のエラーモデルに直接依存します。

---

## xer全体の設計との関係

`<xer/time.h>` は、プロジェクト全体にわたる重要な設計判断をいくつか反映しています。

* 明示的な失敗報告を優先する
* Cに馴染みのある利用者にとって近づきやすいAPIを保つ
* ロケールへの不要な依存を避ける
* 公開テキスト出力にはUTF-8を使う
* 公開設計の中心を `std::chrono` に置かない

これにより、このヘッダーは xer の一般的な哲学を示す分かりやすい例のひとつになっています。

---

## ドキュメント上の注意

このヘッダーを生成ドキュメントで使う場合、通常は次の点を説明すれば十分です。

* `xer::time_t` は `double` ベースであり、秒単位で測る
* エポックはPOSIXエポックに固定されている
* `xer::tm` は馴染みのある要素分解時刻構造体に `tm_microsec` を追加したものである
* `ctime` は `std::u8string` を返す
* `strftime` はUTF-8書式文字列を使い、`xer::result<std::u8string>` を返す

詳細な書式規則や変換時の境界ケースは、詳細リファレンスまたは生成API節に属します。

---

## 例として示す価値がある主題

次のような例は、このヘッダーに特に適しています。

* `time()` による現在時刻の取得
* `gmtime` または `localtime` によるスカラー時刻値の変換
* `mktime` による逆変換
* `ctime` による書式化
* `strftime` による書式化

これらは `examples/` の実行可能な例として良い候補です。

---

## 例

```cpp
#include <xer/time.h>

auto main() -> int
{
    const auto now = xer::time();
    if (!now.has_value()) {
        return 1;
    }

    const auto utc = xer::gmtime(*now);
    if (!utc.has_value()) {
        return 1;
    }

    const auto text = xer::strftime(u8"%Y-%m-%d %H:%M:%S.%f", *utc);
    if (!text.has_value()) {
        return 1;
    }

    return 0;
}
```

この例は、通常の xer のスタイルを示しています。

* 時刻を明示的に取得する
* 明示的に変換する
* 明示的に書式化する
* 失敗しうる各段階で `xer::result` を確認する

---

## 関連項目

* `policy_project_outline.md`
* `policy_time.md`
* `header_error.md`

---

# `<xer/version.h>`

## 目的

`<xer/version.h>` は、xer のコンパイル時バージョン情報を提供します。

このヘッダーの目的は、ライブラリのバージョンを次の用途で、単純かつ明示的な形で利用できるようにすることです。

- ソースレベルの条件分岐
- 診断と報告
- ビルド時の確認
- 必要に応じた生成ドキュメントやサンプル出力

このヘッダーは、意図的に小さく、焦点を絞ったものです。

---

## 主な役割

`<xer/version.h>` の主な役割は、xer ライブラリのバージョンを問い合わせるための、安定した公開手段を提供することです。

具体的には、次のことを簡単にできるようにします。

- メジャー、マイナー、パッチの各バージョン番号を確認する
- α版を表す記号など、現在のサフィックスを確認する
- 完全なバージョン文字列を取得する
- バージョン情報をマクロとインライン定数の両方で利用できるようにする

これにより、このヘッダーはプリプロセッサ文脈でも通常の C++ コードでも有用になります。

---

## 主な要素

少なくとも、`<xer/version.h>` は次のような要素を提供します。

```cpp
#define XER_VERSION_MAJOR <major>
#define XER_VERSION_MINOR <minor>
#define XER_VERSION_PATCH <patch>
#define XER_VERSION_SUFFIX "<suffix>"
#define XER_VERSION_STRING "<version>"

inline constexpr int version_major;
inline constexpr int version_minor;
inline constexpr int version_patch;
inline constexpr std::string_view version_suffix;
inline constexpr std::string_view version_string;
```

正確な値はリリースごとに当然変わりますが、これが意図している公開形状です。

---

## プリプロセッサマクロ

`<xer/version.h>` は、バージョン情報用のプリプロセッサマクロを提供します。

### 目的

マクロ形式は、通常の C++ 定数評価より前にバージョン処理を行う必要がある場合や、プリプロセッサによる分岐を行いたい場合のためにあります。

### 主なマクロ

少なくとも、次のマクロが想定されています。

```cpp id="gkdm5v"
XER_VERSION_MAJOR
XER_VERSION_MINOR
XER_VERSION_PATCH
XER_VERSION_SUFFIX
XER_VERSION_STRING
```

### 意味

* `XER_VERSION_MAJOR`: メジャーバージョン番号
* `XER_VERSION_MINOR`: マイナーバージョン番号
* `XER_VERSION_PATCH`: パッチバージョン番号
* `XER_VERSION_SUFFIX`: `a3` などのサフィックス
* `XER_VERSION_STRING`: `1.2.3-beta` などの完全なバージョン文字列

### 注記

これらのマクロは、次のような場面で有用です。

* 条件付きコンパイル
* 生成されるバナー
* 単純なコンパイル時チェック
* 他の生成物へのバージョン情報の埋め込み

---

## インライン定数

`<xer/version.h>` は、マクロレベルのバージョン情報に対応するインライン C++ 定数も提供します。

### 目的

インライン定数形式は、通常の C++ コードでマクロに頼らずにバージョン情報を使えるようにするためにあります。

### 主な定数

少なくとも、次の定数が想定されています。

```cpp id="0t63o7"
inline constexpr int version_major;
inline constexpr int version_minor;
inline constexpr int version_patch;
inline constexpr std::string_view version_suffix;
inline constexpr std::string_view version_string;
```

### 意味

これらの定数はマクロ値に直接対応しますが、C++ コードではより自然に扱える形になっています。

### 注記

マクロと定数の両方を提供することには、次の利点があります。

* マクロはプリプロセッサ処理では引き続き実用的である
* 定数は型安全な通常コードに適している

---

## バージョン構成要素

xer のバージョンはいくつかの構成要素に分かれています。

### メジャーバージョン

メジャーバージョンは、大規模な互換性変更やリリース系列の変更を表します。

### マイナーバージョン

マイナーバージョンは、同じメジャー系列内での、より小さな機能系列の進行を表します。

### パッチバージョン

パッチバージョンは、より小さな修正や保守上の進行を表します。

### サフィックス

サフィックスは、α段階表記など、追加のリリース状態情報を表します。

たとえば、次のような表記があります。

```text id="uh0aq6"
1.2.3-beta
```

これは次のように解釈できます。

* major: `0`
* minor: `2`
* patch: `0`
* suffix: `a3`

### 完全なバージョン文字列

完全なバージョン文字列は、これらの要素を組み合わせた、人間が読みやすい単一のバージョン識別子です。

---

## 設計方針

`<xer/version.h>` は意図的に単純です。

### 小さくしている理由

バージョン情報は重要ですが、理解しやすく利用しやすい状態を保つべきです。

そのため、このヘッダーでは次のものを避けます。

* 過度な抽象化
* 必要以上に技巧的なコンパイル時メタプログラミング
* 不必要に複雑な解析機能

主な目標は、ライブラリのバージョンを明確に公開することだけです。

---

## 想定される用途

`<xer/version.h>` の典型的な用途には、次のものがあります。

* 現在の xer バージョンをログや診断に表示する
* 生成ドキュメントにバージョンを埋め込む
* テストコードでライブラリバージョンを確認する
* ソースコード内で単純な条件処理を行う

### 具体的な場面

このヘッダーは、特に次の場面で自然に使えます。

* ドキュメント生成スクリプトや関連サンプル
* テスト出力
* リリース確認コード
* バージョン差異が重要な場合の機能ゲート処理

---

## ドキュメントとの関係

`<xer/version.h>` は、生成ドキュメントと特に関係があります。

### 理由

リファレンスマニュアルは、すでに対象バージョンを明示的に記録しています。
たとえば、リファレンスマニュアルが特定の対象バージョンを記録する場合、その情報を一貫させるための自然な正本は `<xer/version.h>` です。

### 設計方針

長期的には、生成ドキュメントは `<xer/version.h>` の実際の内容と一致していることが望ましく、これによりバージョンのずれを最小化できます。

---

## 他のヘッダーとの関係

`<xer/version.h>` は、ほかの公開ヘッダーからはおおむね独立しています。

ただし、次の文書とあわせて理解してください。

* `policy_project_outline.md`
* `public_headers.md`

大まかな関係は次のとおりです。

* `<xer/version.h>` はバージョンメタデータを提供する
* ほかのヘッダーは機能的なライブラリ機能を提供する

このヘッダーはライブラリのほかの部分の振る舞いを定義しませんが、そのライブラリのリリース状態を識別します。

---

## ドキュメント上の注記

このヘッダーを生成ドキュメントで扱う場合、通常は次の点を説明すれば十分です。

* コンパイル時バージョン情報を提供すること
* マクロ形式とインライン定数形式の両方が利用できること
* サフィックスが公開バージョンモデルの一部であること
* `version_string` / `XER_VERSION_STRING` が、人間が読める完全なバージョンを提供すること

詳細なリリース方針の解釈は、ヘッダーごとの概要ではなく、リリースノートや上位のプロジェクト文書に属します。

---

## 例として示すのに適した話題

このヘッダーでは、次のような例が特に適しています。

* `version_string` を読む
* `version_major` を確認する
* 現在のバージョンを表示する
* テストコードで既知のリリース系列の期待値と比較する

これらは `examples/` 以下の実行可能サンプルのよい候補です。

---

## 例

```cpp id="yrg8u0"
#include <xer/version.h>

auto main() -> int
{
    if (xer::version_major < 0) {
        return 1;
    }

    if (xer::version_string.empty()) {
        return 1;
    }

    return 0;
}
```

この例は、通常の使い方を示しています。

* バージョンヘッダーを直接インクルードする
* 通常の C++ コードでインライン定数を使う
* このヘッダーを、単純なライブラリメタデータの提供元として扱う

---

## 関連項目

* `policy_project_outline.md`
* `public_headers.md`
