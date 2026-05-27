<!-- xer-reference-source-sha256: 9d2702e419b3d317eec09f0cc68dc5c5a36a434e2701f2e5171206af7acbbffa -->

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
