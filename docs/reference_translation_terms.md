# リファレンスマニュアル対訳表

対象バージョン: **v0.6.0**

この文書は、`docs/xer_reference_manual_en.md` と `docs/xer_reference_manual_ja.md` の表記ゆれを抑えるための対訳表です。

## プロジェクト名と表記

| 英語表記 | 日本語表記 | 備考 |
|---|---|---|
| xer C++ Utility Library | xer C++ Utility Library | 正式名称。 |
| xer | xer | 略称。常に小文字。 |
| xer | 使用しない | マクロ名や `XER_INCLUDE` など、コード上の識別子は除く。 |

## 基本用語

| 英語 | 日本語 | 備考 |
|---|---|---|
| reference manual | リファレンスマニュアル | 文書名では「xer C++ Utility Library リファレンスマニュアル」。 |
| target version | 対象バージョン | 冒頭表記に使用。 |
| header | ヘッダー | `<xer/error.h>` など。 |
| public header | 公開ヘッダー | `docs/public_headers.md` と対応。 |
| fragment | 断片 | `docs/bits/header_*.md` など。 |
| API | API | 原則として英字表記。 |
| public API | 公開API |  |
| implementation detail | 実装詳細 |  |
| helper | ヘルパー |  |
| utility | ユーティリティ |  |
| facility | 機能 | 文脈により「機構」も可。 |
| policy | 方針 | policy文書名では原題を維持。 |

## エラー処理

| 英語 | 日本語 | 備考 |
|---|---|---|
| ordinary failure | 通常の失敗 | 例外的状況ではない失敗。 |
| failure | 失敗 |  |
| error | エラー |  |
| error code | エラーコード |  |
| error category | エラーカテゴリ |  |
| source location | ソース位置 | `std::source_location` の説明で使用。 |
| diagnostic | 診断 |  |
| diagnostic context | 診断用コンテキスト |  |
| assertion | アサーション |  |
| assertion failure | アサーション失敗 |  |
| exception | 例外 |  |
| sentinel value | 番兵値 |  |
| explicit | 明示的 |  |
| type-safe | 型安全 |  |

## 型と値

| 英語 | 日本語 | 備考 |
|---|---|---|
| result type | 結果型 | `xer::result` そのものはコード表記。 |
| success value | 成功値 |  |
| return type | 返却型 | 本プロジェクトでは関数の返却値の型は後置形式。 |
| return value | 返却値 |  |
| payload | ペイロード |  |
| structured detail | 構造化された詳細情報 |  |
| source storage | 元のストレージ | viewの寿命説明など。 |
| owning | 所有する |  |
| non-owning | 非所有 |  |
| view | ビュー |  |
| span | スパン | `std::span` はコード表記。 |
| overload | オーバーロード |  |
| overload set | オーバーロード集合 |  |

## 文字列・Unicode

| 英語 | 日本語 | 備考 |
|---|---|---|
| string | 文字列 |  |
| string view | 文字列ビュー |  |
| text | テキスト |  |
| byte | バイト |  |
| code unit | コード単位 |  |
| UTF-8 code unit | UTF-8コード単位 |  |
| code point | コードポイント |  |
| Unicode scalar value | Unicodeスカラー値 |  |
| grapheme cluster | 書記素クラスタ |  |
| extended grapheme cluster | 拡張書記素クラスタ |  |
| normalization | 正規化 |  |
| Normalization Form C | 正規化形式C | 初出では NFC も併記。 |
| NFC | NFC |  |
| malformed input | 不正な入力 |  |
| invalid UTF-8 | 不正なUTF-8 |  |
| well-formed | 整形式 |  |
| locale-independent | ロケール非依存 |  |
| C locale | Cロケール |  |
| character classification | 文字分類 |  |
| character conversion | 文字変換 |  |
| case conversion | 大文字小文字変換 |  |
| uppercase | 大文字 |  |
| lowercase | 小文字 |  |
| printable | 印字可能 |  |

## 日本語処理

| 英語 | 日本語 | 備考 |
|---|---|---|
| Japanese-specific | 日本語固有 |  |
| kana | 仮名 | ひらがな・カタカナを含む総称。 |
| Hiragana | ひらがな | 文字名では Hiragana を残す場合もある。 |
| Katakana | カタカナ | 文字名では Katakana を残す場合もある。 |
| Kanji | 漢字 |  |
| Kansuji | 漢数字 | ヘッダー名や関数名は `kansuji` のまま。 |
| Daiji | 大字 |  |
| furigana | ふりがな | 関数名は `furigana` のまま。 |
| ruby | ルビ | HTML要素では `ruby`。 |
| romaji | ローマ字 | 識別子は `romaji` のまま。 |
| wakachi-gaki | 分かち書き |  |
| yōon | 拗音 | 必要に応じて初出で英語を併記。 |
| sokuon | 促音 |  |
| syllabic n | 撥音「ん」 |  |
| prolonged sound mark | 長音記号 |  |
| dakuten | 濁点 |  |
| handakuten | 半濁点 |  |
| fullwidth | 全角 |  |
| halfwidth | 半角 |  |
| fullwidth Katakana | 全角カタカナ |  |
| halfwidth Katakana | 半角カタカナ |  |
| MeCab-derived reading | MeCab由来の読み |  |
| bunsetsu-like | 文節風 | 厳密な文節でない場合。 |
| particle reading | 助詞の読み |  |

## バイナリ・ハッシュ

| 英語 | 日本語 | 備考 |
|---|---|---|
| binary data | バイナリデータ |  |
| byte sequence | バイト列 |  |
| byte order | バイト順 |  |
| endian | エンディアン |  |
| little endian | リトルエンディアン |  |
| big endian | ビッグエンディアン |  |
| checksum | チェックサム |  |
| additive checksum | 加算チェックサム |  |
| XOR checksum | XORチェックサム |  |
| CRC | CRC |  |
| hash | ハッシュ |  |
| digest | ダイジェスト |  |
| hexadecimal | 16進 |  |
| Base64 | Base64 |  |
| URL-safe Base64 | URLセーフBase64 |  |
| padding | パディング |  |
| unpadded | 非パディング |  |
| streaming | ストリーミング |  |

## 入出力・環境

| 英語 | 日本語 | 備考 |
|---|---|---|
| stream | ストリーム |  |
| text stream | テキストストリーム |  |
| binary stream | バイナリストリーム |  |
| file path | ファイルパス |  |
| current working directory | カレントディレクトリ |  |
| environment variable | 環境変数 |  |
| process | プロセス |  |
| child process | 子プロセス |  |
| executable | 実行ファイル |  |
| command line | コマンドライン |  |
| standard input | 標準入力 |  |
| standard output | 標準出力 |  |
| standard error | 標準エラー |  |

## 翻訳運用ルール

- コード、識別子、マクロ名、ファイル名、ヘッダー名は翻訳しない。
- `xer` は常に小文字で表記する。
- `XER_VERSION_STRING`、`XER_ENABLE_LOG`、`XER_INCLUDE` などの識別子はそのまま表記する。
- 日本語版の `docs/bits/header_*.ja.md` が存在しない場合、または対応する英語版の内容ハッシュと対応しない場合は、日本語版リファレンスマニュアルでは未訳であることを明示して英語版を使用する。
- 新旧判定にはファイルのタイムスタンプを使わない。`git clone` や `git checkout` でタイムスタンプは容易に変わるため、翻訳元の内容ハッシュで判定する。
- 日本語版の断片には、翻訳元となる英語版断片の内容ハッシュを次の形式で記録する。

```md
<!-- xer-reference-source-sha256: <sha256> -->
```

- このハッシュは生成対象には表示しない。生成時にコメントを取り除く。
