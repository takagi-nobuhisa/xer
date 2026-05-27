<!-- xer-reference-source-sha256: f3ffc20158865038d73ffe8f75f05c3700c93613e9ab0ebf598a10435103dba7 -->

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
