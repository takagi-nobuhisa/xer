<!-- xer-reference-source-sha256: e5d24a2c1ad5822dc071d67c552a6ad4d565d52ebcece33439dc1a64b8f01a1a -->

# `<xer/diag.h>`

## 目的

`<xer/diag.h>` は、xer の軽量な診断機能を提供します。

このヘッダーは、トレースとログ出力のサポートを1つの公開診断ヘッダーにまとめつつ、両機能で共有するカテゴリとレベルの語彙を共通化します。

---

## 主な要素

少なくとも、`<xer/diag.h>` は次の要素を提供します。

```cpp
using xer::diag_level_t = int;

inline constexpr xer::diag_level_t xer::diag_error;
inline constexpr xer::diag_level_t xer::diag_warning;
inline constexpr xer::diag_level_t xer::diag_info;
inline constexpr xer::diag_level_t xer::diag_debug;
inline constexpr xer::diag_level_t xer::diag_verbose;

enum class xer::diag_category : std::uint32_t;

xer_trace(category, level, object)
xer_log(category, level, message)
xer_log(category, level, format, ...)
```

また、トレース出力ストリームとログ出力ストリーム、およびそれぞれのレベルを設定する関数も提供します。

---

## 設計上の役割

このヘッダーは、診断、開発時のトレース、単純な実行時ログ出力のためのものです。

トレースとログ出力は、次のものを共有します。

* `diag_category`
* `diag_level_t`
* 名前付きレベル定数

それぞれの出力先と現在のレベルは個別に設定されます。

---

## トレース

`xer_trace(category, level, object)` は、次の内容を含む診断行を1行出力します。

* 診断カテゴリ
* 診断レベル
* ソース式のテキスト
* 静的に導出された型名
* xer の `%@` printf変換を通じて整形された値

出力形式の概念は次のとおりです。

```text
[category][level] expression (type) = value
```

`NDEBUG` が定義されている場合、`xer_trace` は何もしない式に展開され、引数を評価しません。

---

## ログ

`xer_log(category, level, message)` は、単純なログレコードを1件書き込みます。

`xer_log(category, level, format, ...)` は、書式付きログレコードを1件書き込みます。メッセージ本体では、`%@` を含む xer のprintf書式規則を使います。

`xer_trace` とは異なり、ログ出力は `NDEBUG` が定義されているだけでは無効になりません。`<xer/diag.h>` をインクルードする前に `XER_ENABLE_LOG` を `0` と定義することで、コンパイル時に無効化できます。

## ログレコード形式

各ログ呼び出しは、CSVデータレコードを1行書き込みます。

列は次のように固定されています。

```text
timestamp,category,level,message
```

ヘッダー行は自動では書き込まれません。

タイムスタンプはローカル時刻を使い、ミリ秒精度です。

```text
YYYY-MM-DD HH:MM:SS.mmm
```

典型的なログレコードは次のようになります。

```csv
2026-04-26 18:42:15.123,io,30,"opened sample.txt"
```

先頭3つのフィールドは xer が生成し、CSVクォートなしで書き込まれます。
メッセージフィールドは常にクォートされたCSVフィールドとして書き込まれます。メッセージ中の二重引用符は、二重にすることでエスケープされます。

## ログのコスト方針

ログ出力は、合理的に軽量であり続けるように設計されています。

* 無効なログメッセージは、書式引数が評価される前にフィルタされる
* 単純なメッセージログ出力では `sprintf` を使わない
* 書式付きメッセージログ出力では、レベル確認が成功した後にだけ `sprintf` を使う
* 秒単位までのタイムスタンプ接頭辞はスレッドごとにキャッシュする
* レベルフィールドは `printf` を使わずに整形する
* メッセージフィールドは、別のクォート済み文字列を構築せず、ストリームへ直接クォート出力する

---

## 出力ストリーム

トレース出力とログ出力は、既定では標準エラーのテキストストリームに出力されます。
対応するストリーム設定関数を使うことで、それぞれを個別に変更できます。

---

## 注意

これらの機能は意図的に小さく保たれています。
完全なログフレームワークではありませんが、xer 自身と、xer を使う小さなプログラムにとって有用な診断基盤を提供します。
