# リファレンスマニュアル対訳表

対象バージョン: **v0.8.0**

この文書は、`docs/xer_reference_manual_en.md` と `docs/xer_reference_manual_ja.md` の表記ゆれを抑えるための対訳表です。

## プロジェクト名と表記

| 英語表記 | 日本語表記 | 備考 |
|---|---|---|
| xer C++ Utility Library | xer C++ Utility Library | 正式名称。 |
| xer | xer | 略称。常に小文字。 |
| 大文字表記のプロジェクト名 | 使用しない | コード上の識別子に含まれる大文字表記は除く。 |

## 基本用語

| 英語 | 日本語 | 備考 |
|---|---|---|
| reference manual | リファレンスマニュアル | 文書名では「xer C++ Utility Library リファレンスマニュアル」。 |
| target version | 対象バージョン | 冒頭表記に使用。 |
| header | ヘッダー | `<xer/error.h>` など。 |
| public header | 公開ヘッダー | `docs/public_headers.md` と対応。 |
| internal header | 内部ヘッダー | `xer/bits/` 以下のヘッダー。 |
| fragment | 断片 | `docs/bits/header_*.md` など。 |
| API | API | 原則として英字表記。 |
| public API | 公開API |  |
| implementation detail | 実装詳細 |  |
| helper | ヘルパー |  |
| utility | ユーティリティ |  |
| facility | 機能 | 文脈により「機構」も可。 |
| generic value conversion | 汎用値変換 | `xer::to<T>` の説明で使用。 |
| C++ cast expression | C++のキャスト式 | `static_cast` などとの対比。 |
| range-checked conversion | 範囲チェック付き変換 | 数値変換の説明で使用。 |
| policy | 方針 | policy文書名では原題を維持。 |
| scope | 対象範囲 | 「スコープ」よりも原則こちらを使う。 |
| out of scope | 対象外 |  |
| limitation | 制限 |  |
| deferred | 後回し | 将来対応に残す場合。 |
| intentionally | 意図的に | 偶然の未実装でないことを示す場合。 |

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
| invalid argument | 不正な引数 | `error_t::invalid_argument` の説明で使用。 |
| out of range | 範囲外 | `error_t::out_of_range` の説明で使用。 |
| end of file | ファイル終端 | `error_t::end_of_file` の説明で使用。逐次入力では「入力終端」も可。 |
| not found | 見つからない | 検索失敗。EOFとは区別する。 |

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
| range | 範囲 | C++ ranges の文脈。 |
| input range | 入力範囲 | `std::ranges::input_range` 相当。 |
| iterator | 反復子 |  |
| initializer list | 初期化子リスト | `std::initializer_list` はコード表記。 |
| overload | オーバーロード |  |
| overload set | オーバーロード集合 |  |
| scalar | スカラー |  |
| arithmetic type | 算術型 |  |
| floating-point type | 浮動小数点型 |  |
| integral type | 整数型 |  |
| boolean | ブール値 | 型名 `bool` はコード表記。 |

## 文字列・Unicode

| 英語 | 日本語 | 備考 |
|---|---|---|
| string | 文字列 |  |
| string view | 文字列ビュー |  |
| narrow `char` string | `char`文字列 | エンコーディングが曖昧な文字列として説明する場合。 |
| explicitly encoded string | 明示的なエンコードの文字列 | `char8_t` / `char16_t` / `char32_t` / `wchar_t` 文字列など。 |
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
| whitespace | 空白 |  |
| trim | トリム | 関数名は `trim` のまま。説明では「前後の空白を取り除く」も可。 |

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
| braille | 点字 |  |
| braille pattern | 点字パターン | Unicode文字名などでは英語を残す場合もある。 |
| numeric indicator | 数符 | 点字文脈。 |
| alphabetic indicator | 英字符 | 点字文脈。 |
| capital indicator | 大文字符 | 点字文脈。 |
| information-processing braille | 情報処理点字 |  |

## 数値・数学

| 英語 | 日本語 | 備考 |
|---|---|---|
| mathematical helper | 数学ヘルパー |  |
| real-number helper | 実数ヘルパー |  |
| complex-number helper | 複素数ヘルパー |  |
| vector | ベクトル | `vec` はコード表記。 |
| position vector | 位置ベクトル |  |
| Cartesian coordinates | 直交座標 |  |
| polar coordinates | 極座標 |  |
| radius | 半径 | 極座標の `r`。 |
| angle | 角度 |  |
| full turn | 1回転 |  |
| τrad | τrad | 1を1回転とする角度単位。日本語でもそのまま表記。 |
| degree | 度 | 単位名や関数名では `degree` を残す。 |
| radian | ラジアン | 単位名や関数名では `radian` / `rad` を残す。 |
| trigonometric function | 三角関数 |  |
| inverse trigonometric function | 逆三角関数 |  |
| hyperbolic function | 双曲線関数 |  |
| finite | 有限 | 浮動小数点値の説明。 |
| NaN | NaN |  |
| infinity | 無限大 |  |
| epsilon | イプシロン | 許容誤差の文脈では「許容誤差」も可。 |
| tolerance | 許容幅 | `mode` など幅を持たせる場合。比較誤差では「許容誤差」も可。 |
| interpolation | 補間 |  |
| linear interpolation | 線形補間 |  |
| polynomial equation | 多項式方程式 |  |
| quadratic equation | 二次方程式 |  |
| cubic equation | 三次方程式 |  |
| coefficient | 係数 |  |
| root | 根 | 方程式の根。 |
| multiplicity | 重複度 | 根の重複度。 |
| discriminant | 判別式 |  |

## 統計

| 英語 | 日本語 | 備考 |
|---|---|---|
| statistics | 統計 | ヘッダー説明では「統計ユーティリティ」も可。 |
| descriptive statistics | 記述統計 | `<xer/statistics.h>` の説明で使用。 |
| arithmetic range | 算術範囲 | 算術型の値を返す範囲。 |
| sum | 合計 | 関数名は `sum`。 |
| product | 積 | 関数名は `product`。 |
| mean | 平均 | 一般説明。関数名は `mean`。 |
| arithmetic mean | 算術平均 |  |
| geometric mean | 幾何平均 |  |
| harmonic mean | 調和平均 |  |
| median | 中央値 |  |
| quantile | 分位数 |  |
| percentile | パーセンタイル |  |
| mode | 最頻値 |  |
| frequency | 頻度 |  |
| population variance | 母分散 |  |
| sample variance | 標本分散 | 不偏分散として説明する場合は文脈に注意。 |
| population standard deviation | 母標準偏差 |  |
| sample standard deviation | 標本標準偏差 |  |
| empty range | 空範囲 |  |
| highest frequency | 最高頻度 |  |

## 物理量・単位

| 英語 | 日本語 | 備考 |
|---|---|---|
| physical quantity | 物理量 |  |
| quantity | 量 | 型名 `quantity` はコード表記。 |
| dimension | 次元 | 型名 `dimension` はコード表記。 |
| dimensionless | 無次元 | 型名 `dimensionless` はコード表記。 |
| unit | 単位 | 型名 `unit` はコード表記。 |
| unit conversion | 単位変換 |  |
| SI unit | SI単位 |  |
| yard-pound unit | ヤード・ポンド法単位 | `inch`, `ft`, `yd`, `mile`, `oz`, `lb` など。 |
| international yard-pound unit | 国際ヤード・ポンド法単位 | 定義値を説明する場合。 |
| plural alias | 複数形エイリアス | `feet`, `pounds` などを提供しない説明で使用。 |
| scale | スケール | `std::ratio` による倍率。 |
| base dimension | 基本次元 |  |
| length | 長さ | 次元名。 |
| mass | 質量 | 次元名。 |
| time | 時間 | 次元名。 |
| electric current | 電流 | 次元名。 |
| dimension safety | 次元安全性 |  |

## 画像・描画

| 英語 | 日本語 | 備考 |
|---|---|---|
| image | 画像 | 名前空間 `xer::image` はコード表記。 |
| framebuffer | フレームバッファ |  |
| canvas | キャンバス | 型名 `canvas` はコード表記。 |
| dynamic canvas | 動的キャンバス | `dynamic_canvas` はコード表記。 |
| pixel | ピクセル | 型名 `pixel` はコード表記。 |
| alpha | アルファ |  |
| coverage | カバレッジ | アンチエイリアスの被覆率。 |
| anti-aliasing | アンチエイリアス | 関数名の `_aa` はそのまま。 |
| clipping | クリッピング |  |
| draw | 描画 | `draw_*` 関数の説明。 |
| fill | 塗りつぶし | `fill_*` 関数の説明。 |
| line | 線分 |  |
| rectangle | 矩形 |  |
| circle | 円 |  |
| ellipse | 楕円 |  |
| arc | 円弧 |  |
| elliptical arc | 楕円弧 |  |
| polygon | 多角形 |  |
| bitmap font | ビットマップフォント |  |
| glyph | グリフ |  |
| grayscale | グレイスケール |  |
| luminance | 輝度 |  |
| filter | フィルター | `filter_pixels` の説明では「ピクセル変換」も可。 |

## バイナリ・ハッシュ・アーカイブ・シリアライズ

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
| archive | アーカイブ | ZIPなど。 |
| ZIP archive | ZIPアーカイブ |  |
| entry | エントリ | ZIP内の項目。 |
| central directory | セントラルディレクトリ | ZIP文脈。 |
| extraction | 展開 | ZIPエントリの取り出し。 |
| compression | 圧縮 |  |
| deflate | deflate | 方式名は英字。 |
| encrypted | 暗号化された | ZIP制限事項など。 |
| serialization | シリアライズ |  |
| binary serialization | バイナリシリアライズ |  |
| fixed-schema | 固定スキーマ |  |
| archive object | アーカイブオブジェクト | `binary_output_archive` などの説明。 |
| schema | スキーマ |  |

## 入出力・環境

| 英語 | 日本語 | 備考 |
|---|---|---|
| stream | ストリーム |  |
| text stream | テキストストリーム |  |
| binary stream | バイナリストリーム |  |
| file path | ファイルパス |  |
| current working directory | カレントディレクトリ |  |
| directory | ディレクトリ |  |
| directory entry | ディレクトリエントリ |  |
| environment variable | 環境変数 |  |
| process | プロセス |  |
| child process | 子プロセス |  |
| executable | 実行ファイル |  |
| command line | コマンドライン |  |
| standard input | 標準入力 |  |
| standard output | 標準出力 |  |
| standard error | 標準エラー |  |
| socket | ソケット |  |
| exact byte transfer | 固定長送受信 | `socket_send_all` / `socket_recv_exact` の説明で使用。 |
| length-prefixed message | 長さ付きメッセージ | `socket_send_message` / `socket_recv_message` の説明で使用。 |
| message framing | メッセージフレーミング | 長さ付きメッセージ形式など。 |
| endpoint | エンドポイント |  |
| host | ホスト |  |
| port | ポート |  |
| Tcl/Tk | Tcl/Tk |  |

## 診断・ログ

| 英語 | 日本語 | 備考 |
|---|---|---|
| diagnostic print | 診断出力 | `xer_print` の説明で使用。 |
| simple diagnostic print | 簡易診断出力 | `xer_print` の説明で使用。 |
| trace | トレース | `xer_trace` の説明で使用。 |
| log | ログ | `xer_log` の説明で使用。 |
| log record | ログレコード |  |

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
