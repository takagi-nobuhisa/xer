<!-- xer-reference-source-sha256: 24d8c78f770fd62252a7508dc3cdafdd253c0bb9bfd19df02921469e6ccf593e -->
# `<xer/ja.h>`

## 目的

`<xer/ja.h>` は、日本語固有の xer 機能をまとめる便利なアンブレラヘッダーです。

次をインクルードします。

```cpp
#include <xer/furigana.h>
#include <xer/kansuji.h>
#include <xer/mecab.h>
#include <xer/bits/ja_is.h>
#include <xer/bits/ja_kanji.h>
#include <xer/bits/ja_to.h>
```

これらのヘッダーが提供する API は `xer::ja` 名前空間に置かれます。

---

## 名前空間ポリシー

日本語固有 API は `xer::ja` に集約します。

これにより、メインの `xer` 名前空間は言語非依存のユーティリティに集中させつつ、v1.0.0 より前から深い日本語対応を提供できます。

例:

```cpp
xer::ja::to_kansuji(2026, xer::ja::k十);
xer::ja::from_kansuji(u8"二千二十六");
xer::ja::to_furigana(u8"学校", u8"がっこう", xer::ja::ruby_html);
xer::ja::is_hiragana(U'あ');
xer::ja::is_katakana(U'ア');
xer::ja::is_kanji(U'漢');
xer::ja::is_name_kanji(U'凜');
xer::ja::jis_kanji_level_of(U'亜');
xer::ja::to_hiragana(u8"カタカナ");
xer::ja::to_katakana(u8"ひらがな");
xer::ja::normalize_kana(u8"ｶﾞｷﾞｸﾞｹﾞｺﾞ");
xer::ja::mecab_parse(u8"私は猫です。");
```

---

## 文字分類

`xer::ja::is_hiragana` は、コードポイントが実用的なひらがなかどうかを調べます。Unicode の Hiragana ブロックと長音記号 `U+30FC` を受け付けます。

`xer::ja::is_katakana` は、コードポイントが実用的なカタカナかどうかを調べます。全角カタカナ、半角カタカナ、Katakana Phonetic Extensions、Kana Supplement/Extended ブロック、長音記号を含みます。

`xer::ja::is_kana` は、コードポイントが実用的なひらがなまたは実用的なカタカナかどうかを調べます。

`xer::ja::is_kanji` は、コードポイントが一般的な CJK 統合漢字または CJK 互換漢字の範囲に属するかどうかを調べます。Unicode では多くの漢字/hanzi/hanja コードポイントが共有されるため、この関数は言語固有の用法を識別しようとはしません。

`xer::ja::is_japanese_punctuation` は、日本語テキストで一般的に使われる句読点かどうかを調べます。

`xer::ja::is_japanese` は、コードポイントが仮名、漢字、または日本語句読点かどうかを調べます。

`xer::ja::contains_hiragana`、`xer::ja::contains_katakana`、`xer::ja::contains_kana`、`xer::ja::contains_kanji`、`xer::ja::contains_japanese` は、UTF-8 文字列が少なくとも 1 つの該当コードポイントを含むかどうかを調べます。空入力は `false` を返します。不正な UTF-8 入力は `encoding_error` を返します。

`xer::ja::is_all_hiragana`、`xer::ja::is_all_katakana`、`xer::ja::is_all_kana` は、UTF-8 文字列内のすべてのコードポイントが実用的なひらがな、カタカナ、または仮名テキストに属するかどうかを調べます。空入力は `false` を返します。不正な UTF-8 入力は `encoding_error` を返します。

`contains_*` と `is_all_*` 述語は、対応するコードポイント述語と同じ実用的な文字集合を使います。`is_all_*` の仮名述語は、空白、句読点、漢字、ラテン文字を受け付けません。

```cpp
xer::ja::is_hiragana(U'あ'); // true
xer::ja::is_katakana(U'ア'); // true
xer::ja::is_kana(U'ｱ'); // true
xer::ja::is_kanji(U'漢'); // true
xer::ja::is_japanese_punctuation(U'。'); // true
xer::ja::is_japanese(U'日'); // true

xer::ja::contains_hiragana(u8"abcあ"); // true
xer::ja::contains_katakana(u8"abcア"); // true
xer::ja::contains_kana(u8"abcｱ"); // true
xer::ja::contains_kanji(u8"abc漢"); // true
xer::ja::contains_japanese(u8"hello日本語"); // true
xer::ja::contains_japanese(u8""); // false

xer::ja::is_all_hiragana(u8"こんにちはー"); // true
xer::ja::is_all_katakana(u8"コンニチハー"); // true
xer::ja::is_all_kana(u8"こんにちはコンニチハー"); // true
xer::ja::is_all_kana(u8""); // false
xer::ja::is_all_kana(u8"こんにちは。"); // false
```

これらの関数は内部実装ヘッダー `<xer/bits/ja_is.h>` で定義され、`<xer/ja.h>` から利用できます。

---

## 漢字分類テーブル

`xer::ja::name_kanji_class_of` は、コードポイントの実用的な日本語人名用漢字クラスを返します。

```cpp
namespace xer::ja {

enum class name_kanji_class {
    none = 0,
    name = 1,
    jouyou = 2,
    kyouiku = 3,
};

}
```

このクラスは包含関係に従って順序付けられています。教育漢字は常用漢字の部分集合として扱われ、常用漢字は日本語の名に使える漢字の部分集合として扱われます。

```cpp
xer::ja::is_name_kanji(U'凜'); // true
xer::ja::is_jouyou_kanji(U'鬱'); // true
xer::ja::is_kyouiku_kanji(U'日'); // true
```

`xer::ja::jis_kanji_level_of` は JIS 漢字水準を返します。

```cpp
namespace xer::ja {

enum class jis_kanji_level {
    none = 0,
    level_1 = 1,
    level_2 = 2,
    level_3 = 3,
    level_4 = 4,
};

}
```

便利な述語も用意されています。

```cpp
xer::ja::is_jis_level_1_kanji(U'亜'); // true
xer::ja::is_jis_level_2_kanji(U'弌'); // true
xer::ja::is_jis_level_3_kanji(U'俱'); // true
xer::ja::is_jis_level_4_kanji(U'㐆'); // true
```

実装は基本 CJK 統合漢字範囲についてコンパクトな 1 バイトフラグを保持し、その範囲外の対応コードポイントには疎なフォールバックテーブルを使います。下位 2 ビットは `name_kanji_class` を保持し、ビット 2..4 は `jis_kanji_level` を保持します。

これらの関数は内部実装ヘッダー `<xer/bits/ja_kanji.h>` で定義され、`<xer/ja.h>` から利用できます。

---

## 仮名変換

`xer::ja::to_hiragana` は、UTF-8 テキスト内の全角カタカナをひらがなへ変換します。

```cpp
const auto text = xer::ja::to_hiragana(u8"カタカナとヴ");
// text == u8"かたかなとゔ"
```

`xer::ja::to_katakana` は、UTF-8 テキスト内のひらがなを全角カタカナへ変換します。

```cpp
const auto text = xer::ja::to_katakana(u8"ひらがなとゔ");
// text == u8"ヒラガナトヴ"
```

`xer::ja::normalize_kana` は、ひらがな/カタカナの文字種を保ちながら、実用的な仮名表記を正規化します。半角カタカナを全角カタカナへ変換し、合成済み仮名が存在する場合は分離された濁点または半濁点を合成します。

```cpp
const auto text = xer::ja::normalize_kana(u8"ｶﾞｷﾞｸﾞｹﾞｺﾞ ﾊﾟﾋﾟﾌﾟﾍﾟﾎﾟ");
// text == u8"ガギグゲゴ パピプペポ"

const auto text2 = xer::ja::normalize_kana(u8"がハ゜");
// text2 == u8"がパ"
```

これらの関数は無関係なコードポイントを変更せず、不正な UTF-8 入力を `encoding_error` として報告できるように `xer::result<std::u8string>` を返します。

これらの関数は内部実装ヘッダー `<xer/bits/ja_to.h>` で定義され、`<xer/ja.h>` から利用できます。

---

## 注意

`<xer/ja.h>` は独自の実装層を定義しません。これはインクルード専用の便利ヘッダーです。

`<xer/kansuji.h>`、`<xer/furigana.h>`、`<xer/mecab.h>` のような個別ヘッダーは、呼び出し側がより小さな構成要素だけをインクルードしたい場合にも利用できます。仮名変換は現在 `<xer/ja.h>` からのみ利用できます。
