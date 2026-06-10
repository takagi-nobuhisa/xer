<!-- xer-reference-source-sha256: 0db914ddfbe4dd079e5598755e18f09832bab6291e3da351e5e49254bb3d238a -->

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

## API の選び方

クイックガイドとして、次の表を利用できます。

| 目的 | 関数 |
|---|---|
| 入力テキストを MeCab トークン列に解析する | `mecab_parse` |
| 生の surface 文字列と feature 文字列を読む | `mecab_token::surface`, `mecab_token::feature` |
| 分割済みの IPADIC 風 feature フィールドへアクセスする | `mecab_token::features` |
| 実用的な文節風の範囲を得る | `mecab_split_phrases` |
| トークン列を空白なしの仮名へ変換する | `mecab_to_kana` |
| トークン列を文節空白付きの仮名へ変換する | `mecab_kana_wakati` |
| トークン列を文節空白付きのローマ字へ変換する | `mecab_romaji_wakati` |
| トークン列を日本語点字分かち書きへ変換する | `mecab_braille_wakati` |
| トークン列を情報処理点字分かち書きへ変換する | `mecab_ip_braille_wakati` |
| 入力テキストを直接点字へ変換する | `mecab_braille_translate` |
| 入力テキストを直接情報処理点字へ変換する | `mecab_ip_braille_translate` |

同じ入力に対して繰り返し処理する場合は、まず次のようにします。

```cpp
auto tokens = xer::ja::mecab_parse(text);
```

そのうえで、`*tokens` をトークン列向け補助関数へ渡して再利用します。
直接変換ラッパーは、1 回だけ点字変換したい場合の便利 API です。

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

## コード例

次のコード例は、API の主なレイヤーに対応しています。

| コード例 | 主な API |
|---|---|
| `example_mecab_parse_basic.cpp` | `mecab_parse` |
| `example_mecab_split_phrases_basic.cpp` | `mecab_split_phrases` |
| `example_mecab_kana_wakati_basic.cpp` | `mecab_kana_wakati` |
| `example_mecab_romaji_wakati_basic.cpp` | `mecab_romaji_wakati` |
| `example_mecab_braille_wakati_basic.cpp` | `mecab_braille_wakati` |
| `example_mecab_braille_japanese_wakati.cpp` | 日本語点字向け分かち書き |
| `example_mecab_braille_translate_basic.cpp` | `mecab_braille_translate` |
| `example_mecab_ip_braille_translate_basic.cpp` | `mecab_ip_braille_translate` |

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

## よく使う処理パターン

### まずトークンを確認する

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

辞書の詳細、品詞情報、独自処理が必要な場合はこの形を使います。

### 一度だけ解析して複数の出力を派生させる

```cpp
const auto tokens = xer::ja::mecab_parse(text);
if (!tokens) {
    return;
}

const auto kana = xer::ja::mecab_kana_wakati(*tokens);
const auto romaji = xer::ja::mecab_romaji_wakati(*tokens);
const auto braille = xer::ja::mecab_braille_wakati(*tokens);
```

このパターンにより、同じテキストに対して MeCab を何度も起動することを避けられます。

### 1 回だけ点字へ変換する場合は直接ラッパーを使う

```cpp
const auto braille = xer::ja::mecab_braille_translate(text);
```

プログラムが最終的な点字結果だけを必要とし、トークン列や文節範囲を確認しない場合は、直接ラッパーを使います。

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
- `<xer/ctype.h>`
- `<xer/braille.h>`
