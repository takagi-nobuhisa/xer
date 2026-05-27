<!-- xer-reference-source-sha256: b56eacb53fce13ff5fbf8b9e028ec48e2f8aa909ba2f20b1ebe972c30ec9a791 -->

# `<xer/time.h>`

## 目的

`<xer/time.h>` は、xer の時刻関連機能を提供します。

このヘッダーの目的は、C標準ライブラリの `time.h` をそのまま再現することでも、公開APIの中心に `std::chrono` を置くことでもありません。
かわりに、C形式の時刻処理が持つ近づきやすさを保ちながら、xer 独自の設計に沿った、より単純な時刻ライブラリを提供します。

このヘッダーは、次のような作業を対象としています。

- 現在時刻の取得
- スカラー時刻値と要素分解時刻の相互変換
- 時刻値のテキスト書式化
- 単純な時刻差の計算

---

## 主な役割

`<xer/time.h>` の主な役割は、次の考え方に基づく、実用的で明示的な時刻モデルを提供することです。

- `time_t` は単純で算術演算に向いている
- 通常の失敗は `xer::result` で報告する
- 要素分解時刻は `tm` 構造体で表す
- 秒未満の精度を明示的に扱う
- 書式化はUTF-8指向とする

これにより、このヘッダーは、`std::chrono` にありがちな重めの式表現を避けつつ、単純な時刻処理を行いたいコードに適したものになります。

---

## 主なエンティティ

少なくとも、`<xer/time.h>` は次のエンティティを提供します。

```cpp
using time_t = double;
using clock_t = std::clock_t;

struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
    int tm_microsec;
};

auto time() -> xer::result<time_t>;
auto clock() -> xer::result<clock_t>;
auto difftime(time_t left, time_t right) noexcept -> double;

auto gmtime(time_t value) -> xer::result<tm>;
auto localtime(time_t value) -> xer::result<tm>;
auto mktime(const tm& value) -> xer::result<time_t>;

auto ctime(time_t value) -> std::u8string;
auto ctime(const tm& value) -> std::u8string;
auto strftime(std::u8string_view format, const tm& value) -> xer::result<std::u8string>;
```

正確な補助関数群は将来拡張される可能性がありますが、これが現在の中核的な公開形状です。

---

## `xer::time_t`

`xer::time_t` は、xer における中心的なスカラー時刻型です。

### 基本形

```cpp
using time_t = double;
```

### 意味

`xer::time_t` の単位は秒です。

その解釈は次のとおりです。

* 整数部: 秒単位の整数部分
* 小数部: 秒未満の部分

### `double` を使う理由

この設計により、時刻値は次の性質を持ちます。

* 算術的に扱いやすい
* 軽量である
* 実用上のマイクロ秒レベル精度に適している
* 重い duration 形式の抽象よりも通常のコードで扱いやすい

### 精度の方向性

実用上の目標はマイクロ秒レベルの処理ですが、公開設計として、厳密な固定小数点表現や保証されたナノ秒精度を約束するものではありません。

---

## エポック

xer は `xer::time_t` のエポックをPOSIXエポックに固定します。

### 意味

```text
1970-01-01 00:00:00 UTC
```

は、`xer::time_t` 値として次に対応します。

```text
0.0
```

### なぜ重要か

これにより、従来のCの `time_t` におけるエポック解釈の実装定義の曖昧さを避け、xer 全体で安定した規則を持てます。

---

## 対応範囲

少なくとも初期設計では、エポックより前の時刻は未対応です。

### 意味

* 負の `time_t` 値は未対応
* エポックより前の要素分解時刻は未対応
* そのような入力は失敗になる

これは初期段階における意図的な単純化です。

---

## `xer::tm`

`xer::tm` は、xer の要素分解時刻構造体です。

### 基本形

```cpp
struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
    int tm_microsec;
};
```

### Cの `struct tm` との関係

この構造体はCの `struct tm` を基礎にしていますが、次のメンバーを追加しています。

```cpp
tm_microsec
```

### `tm_microsec`

`tm_microsec` は、1秒未満のマイクロ秒部分を保持します。

意図された範囲は次のとおりです。

```text
0 .. 999999
```

### なぜ重要か

これにより、馴染みのある要素分解時刻モデルを捨てることなく、秒未満の精度を明示できます。

---

## `time()`

```cpp
auto time() -> xer::result<time_t>;
```

### 目的

`time()` は、現在の暦時刻を `xer::time_t` として返します。

### 設計方針

実際には失敗することはまれですが、xer はこれも通常の失敗しうる操作として扱い、`xer::result` で失敗を報告します。

これにより、このヘッダーはライブラリの他の部分と一貫します。

---

## `clock()`

```cpp
auto clock() -> xer::result<clock_t>;
```

### 目的

`clock()` は、基盤となるC機能の形式に沿った、プロセッサ時間またはクロック関連情報を返します。

### 設計方針

`time()` と同様に、xer はこれを通常の失敗しうる操作として扱い、失敗を明示的に報告します。

---

## `difftime`

```cpp
auto difftime(time_t left, time_t right) noexcept -> double;
```

### 目的

`difftime` は、2つの時刻値の差を返します。

### 意味

結果は秒単位のスカラー差です。

### 補足

この部分は、役割が構造変換ではなく算術演算であるため、ヘッダーの中でも単純な部分のひとつです。

---

## `gmtime`

```cpp
auto gmtime(time_t value) -> xer::result<tm>;
```

### 目的

`gmtime` は、スカラー時刻値をUTCの要素分解時刻に変換します。

### 振る舞い

* 小数部は `tm_microsec` に反映される
* 負の値は未対応であり、失敗になる

### 役割

これは、スカラー値からUTCの要素分解時刻へ変換する主要な入口です。

---

## `localtime`

```cpp
auto localtime(time_t value) -> xer::result<tm>;
```

### 目的

`localtime` は、スカラー時刻値をローカル時刻の要素分解時刻に変換します。

### 振る舞い

* 小数部は `tm_microsec` に反映される
* 負の値は未対応であり、失敗になる
* ローカル時刻への変換失敗は明示的に報告される

### 役割

これは `gmtime` に対応するローカル時刻版です。

---

## `mktime`

```cpp
auto mktime(const tm& value) -> xer::result<time_t>;
```

### 目的

`mktime` は、要素分解時刻値をスカラー時刻に変換します。

### 振る舞い

* `tm_microsec` は小数部に寄与する
* 範囲外の `tm_microsec` はエラーになる
* エポックより前の結果はエラーになる

### 役割

これは、`tm` から `time_t` へ戻す逆変換の入口です。

---

## `ctime`

xer は `ctime` を2つのオーバーロードで提供します。

```cpp
auto ctime(time_t value) -> std::u8string;
auto ctime(const tm& value) -> std::u8string;
```

### 目的

`ctime` は、時刻値を人間が読めるUTF-8テキストに変換します。

### 設計方針

xer では、従来Cの `ctime` と `asctime` が担っていた役割を、`ctime` という名前の下に統合しています。

つまり、次のようになります。

* `ctime(time_t)` はスカラー時刻値を書式化する
* `ctime(const tm&)` は要素分解時刻値を書式化する

### 重要な補足

* 返却型は `std::u8string`
* 静的な内部バッファは使わない
* 要素分解時刻版は生ポインターではなく `const tm&` を受け取る

これは、馴染みのある関数名を保ちながら、xer のC++形式の再設計を反映しています。

---

## `strftime`

```cpp
auto strftime(std::u8string_view format, const tm& value) -> xer::result<std::u8string>;
```

### 目的

`strftime` は、要素分解時刻値を、書式文字列に従って書式化します。

### 書式文字列モデル

書式文字列はUTF-8指向であり、次を使います。

```cpp
std::u8string_view
```

つまり、次のものを含めることができます。

* ASCIIの変換指定
* 固定のUTF-8テキスト

### 例

次のような書式は受け入れ可能であることを意図しています。

```text
%Y年%m月%d日 %H時%M分%S秒
```

### 返却モデル

結果は `std::u8string` として返されます。

ただし、返却は `xer::result` 経由です。

### xer固有の秒未満拡張

基盤となるCライブラリへ委譲される変換指定に加えて、xer は次の秒未満拡張を提供します。

* `%f`: マイクロ秒をちょうど6桁の10進数で出力する
* `%L`: ミリ秒をちょうど3桁の10進数で出力する

これらの指定子は `tm_microsec` に基づきます。対応するのは単純な `%f` と `%L` の形だけです。`%3f` のような幅、フラグ、修飾子付きの形式は拒否されます。

### 現在の設計上の制限

少なくとも現在の段階では、次の制限があります。

* 高度なロケール動作は優先事項ではない
* 高度なタイムゾーン拡張は後回し

---

## UTF-8指向の書式化

`<xer/time.h>` の重要な特徴のひとつは、書式化がUTF-8テキストを中心に設計されていることです。

### なぜ重要か

これにより、このヘッダーは xer のより広い公開テキストモデルと整合します。

* 公開テキストAPIはUTF-8指向である
* `std::u8string` は標準の所有テキスト型である
* `std::u8string_view` は標準の非所有テキスト入力型である

これは、よりロケール中心または狭文字だけを前提とする解釈とは異なる、xer の設計上の特徴です。

---

## エラー処理

`<xer/time.h>` は、xer の通常の失敗モデルに従います。

### 意味

失敗しうる操作は、`xer::result` を通じて失敗を報告します。

典型的な例は次のとおりです。

* 時刻情報の取得
* スカラー時刻と要素分解時刻の相互変換
* 無効な要素分解時刻入力
* 未対応のエポック前入力
* 該当する場合の無効な書式処理

### 典型的なエラーカテゴリ

詳細なエラー分類は実装に属しますが、設計意図としては特に次のようなカテゴリと関係します。

* `runtime_error`
* `invalid_argument`

---

## 後回しにしている機能

少なくとも初期段階では、次の機能は意図的に後回し、または単純化されています。

* エポック前の時刻対応
* `strftime` における高度なタイムゾーン機能
* 高度なロケール制御
* 高度なタイムゾーン機能
* C形式の静的バッファ動作
* `asctime` の個別公開

これは意図的な単純化であり、偶然の抜けではありません。

---

## 他のヘッダーとの関係

`<xer/time.h>` は、次の文書とあわせて理解してください。

* `policy_project_outline.md`
* `policy_time.md`
* `header_error.md`

大まかな境界は次のとおりです。

* `<xer/time.h>` は時刻取得、時刻変換、時刻書式化を扱う
* `<xer/error.h>` は、失敗しうる時刻操作で使うエラー/結果モデルを提供する

このヘッダーは、その領域内ではおおむね自己完結していますが、xer の通常のエラーモデルに直接依存します。

---

## xer全体の設計との関係

`<xer/time.h>` は、プロジェクト全体にわたる重要な設計判断をいくつか反映しています。

* 明示的な失敗報告を優先する
* Cに馴染みのある利用者にとって近づきやすいAPIを保つ
* ロケールへの不要な依存を避ける
* 公開テキスト出力にはUTF-8を使う
* 公開設計の中心を `std::chrono` に置かない

これにより、このヘッダーは xer の一般的な哲学を示す分かりやすい例のひとつになっています。

---

## ドキュメント上の注意

このヘッダーを生成ドキュメントで使う場合、通常は次の点を説明すれば十分です。

* `xer::time_t` は `double` ベースであり、秒単位で測る
* エポックはPOSIXエポックに固定されている
* `xer::tm` は馴染みのある要素分解時刻構造体に `tm_microsec` を追加したものである
* `ctime` は `std::u8string` を返す
* `strftime` はUTF-8書式文字列を使い、`xer::result<std::u8string>` を返す

詳細な書式規則や変換時の境界ケースは、詳細リファレンスまたは生成API節に属します。

---

## 例として示す価値がある主題

次のような例は、このヘッダーに特に適しています。

* `time()` による現在時刻の取得
* `gmtime` または `localtime` によるスカラー時刻値の変換
* `mktime` による逆変換
* `ctime` による書式化
* `strftime` による書式化

これらは `examples/` の実行可能な例として良い候補です。

---

## 例

```cpp
#include <xer/time.h>

auto main() -> int
{
    const auto now = xer::time();
    if (!now.has_value()) {
        return 1;
    }

    const auto utc = xer::gmtime(*now);
    if (!utc.has_value()) {
        return 1;
    }

    const auto text = xer::strftime(u8"%Y-%m-%d %H:%M:%S.%f", *utc);
    if (!text.has_value()) {
        return 1;
    }

    return 0;
}
```

この例は、通常の xer のスタイルを示しています。

* 時刻を明示的に取得する
* 明示的に変換する
* 明示的に書式化する
* 失敗しうる各段階で `xer::result` を確認する

---

## 関連項目

* `policy_project_outline.md`
* `policy_time.md`
* `header_error.md`
