<!-- xer-reference-source-sha256: 53bc5bef4e34b99e8e8b3c92c89526966fea738f5099cc569fdf61af6a4d0118 -->
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
