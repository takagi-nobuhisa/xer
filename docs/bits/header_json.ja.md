<!-- xer-reference-source-sha256: f8230759c9623c181491073db3fd3b392425efc159011f3263451e33715f4c33 -->
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
