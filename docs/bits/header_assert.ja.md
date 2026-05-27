<!-- xer-reference-source-sha256: 7ce010e77a96b0cd583f750b50f493a5b5420b1458a8c950d051980963e34f11 -->
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
