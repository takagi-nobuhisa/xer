<!-- xer-reference-source-sha256: ac78a32a5ad1ff32e3e99a09460718ff82ae8c87bf6bcc6414b2ac9d633831a2 -->

# `<xer/diagnostics.h>`

## 目的

`<xer/diagnostics.h>` は、xer の軽量な診断機能を提供します。

このヘッダーは、トレースとログ出力のサポートを1つの公開診断ヘッダーにまとめつつ、両機能で共有するカテゴリとレベルの語彙を共通化します。

---

## 主な要素

少なくとも、`<xer/diagnostics.h>` は次の要素を提供します。

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

また、トレース出力ストリームとログ出力ストリーム、およびそれぞれのレベルを設定する関数も提供します。

---

## 設計上の役割

このヘッダーは、診断、開発時のトレース、単純な実行時ログ出力のためのものです。

単純な表示、トレース、ログ出力は同じ公開診断ヘッダーを共有します。トレースとログ出力は、次のものを共有します。

* `diag_category`
* `diag_level_t`
* 名前付きレベル定数

それぞれの出力先と現在のレベルは個別に設定されます。

---


## 単純表示

`xer_print(expression)` は、単純診断表示ストリームへ1行を書き込みます。
式は1回だけ評価され、ソース式のテキストがラベルとして使われます。

```cpp
int value = 42;
xer_print(value);
```

概念的な出力は次のとおりです。

```text
value = 42
```

`xer_print(label, expression)` は、明示的な UTF-8 ラベル付きで1行を書き込みます。

```cpp
xer_print(u8"answer", value);
```

概念的な出力は次のとおりです。

```text
answer: 42
```

値は xer の `%@` printf変換を通じて整形されます。`result<T>` の場合、成功した結果は格納値を表示し、失敗した結果は `error(name)` を表示します。`result<void>` の成功結果は `ok` を表示します。

`xer_print` は、コード例、簡単な確認、軽量な診断を目的としています。`NDEBUG` によって無効化されません。

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

`xer_trace` とは異なり、ログ出力は `NDEBUG` が定義されているだけでは無効になりません。`<xer/diagnostics.h>` をインクルードする前に `XER_ENABLE_LOG` を `0` と定義することで、コンパイル時に無効化できます。

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

単純表示の出力先は、既定では標準出力のテキストストリームです。トレース出力とログ出力は、既定では標準エラーのテキストストリームに出力されます。
対応するストリーム設定関数を使うことで、それぞれを個別に変更できます。

---

## 注意

これらの機能は意図的に小さく保たれています。
完全なログフレームワークではありませんが、xer 自身と、xer を使う小さなプログラムにとって有用な診断基盤を提供します。
