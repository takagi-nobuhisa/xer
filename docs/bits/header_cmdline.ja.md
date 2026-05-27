<!-- xer-reference-source-sha256: a5e980795d7ffd8331c15a65325c5b28c6688abf3801f2e258bda19bed541168 -->

# `<xer/cmdline.h>`

## 目的

`<xer/cmdline.h>` は現在のプロセスのコマンドライン引数処理機能を提供します。

このヘッダーの目的は、呼び出し側が `main` の `argc` と `argv` を手作業で引き回さなくても、コマンドライン引数を UTF-8 文字列として利用できるようにすることです。

これは次のような状況で有用です。

* `main` の外で実行されるコード
* 非ローカルオブジェクトの初期化
* メインスレッド以外のスレッドで動くコード
* `argc` と `argv` を明示的に運ぶのが煩雑なユーティリティ関数

---

## 主なエンティティ

少なくとも、`<xer/cmdline.h>` は次のエンティティを提供します。

```cpp
using cmdline_arg =
    std::pair<std::u8string_view, std::u8string_view>;

class cmdline;

auto parse_arg(std::u8string_view value) noexcept -> cmdline_arg;

auto get_cmdline() -> xer::result<cmdline>;
````

---

## `cmdline`

`cmdline` は argv 風の UTF-8 文字列列を所有します。

```cpp
class cmdline;
```

内部では、コマンドライン引数を次の形で保持します。

```cpp
std::vector<std::u8string>
```

クラス自体はオプションを解釈しません。
引数列を所有し公開することだけを担当します。

### 基本操作

```cpp
auto size() const noexcept -> std::size_t;
auto empty() const noexcept -> bool;

auto args() const noexcept -> std::span<const std::u8string>;

auto at(std::size_t index) const -> xer::result<std::u8string_view>;
```

### `size`

`size()` は保持している引数の数を返します。

### `empty`

`empty()` は引数リストが空かどうかを返します。

`get_cmdline` の通常の成功利用では、コマンドラインリストは少なくともプログラム名を含むことが期待されますが、手動で構築された `cmdline` オブジェクトについて呼び出し側はそれに依存すべきではありません。

### `args`

`args()` は保持されている生の UTF-8 引数への span を返します。

返された span とその文字列参照は、`cmdline` オブジェクトが生存し、変更されない限り有効です。

### `at`

`at(index)` は 1 つの生の引数を `std::u8string_view` として返します。

`index` が範囲外の場合、`xer::result` を通じてエラーを返します。

---

## `parse_arg`

```cpp
auto parse_arg(std::u8string_view value) noexcept -> cmdline_arg;
```

`parse_arg` は、xer の単純なコマンドライン規則に従って 1 つの生のコマンドライン引数を解析します。

返り値は次のペアです。

```cpp
{ option_name, value }
```

意味は次のとおりです。

* `first` が空でなければ、その引数はオプションです。
* `first` が空であれば、その引数は通常の値です。
* `second` にはオプション値または通常の値が入ります。

---

## 対応する引数形式

xer は単純なロングオプション形式だけを認識します。

対応するオプション形式は次のとおりです。

```text
--option
--option=value
```

通常の値も受け付けます。

```text
value
```

`-x` のようにハイフン 1 つで始まる形式はオプションとして扱いません。
通常の値として扱います。

### 例

```text
--name        -> { "name", "" }
--name=       -> { "name", "" }
--name=value  -> { "name", "value" }
value         -> { "", "value" }
-name         -> { "", "-name" }
--            -> { "", "--" }
--=value      -> { "", "--=value" }
```

`--name` と `--name=` は意図的に同じ扱いです。

「値なし」と「空の値」を区別するには、より複雑な表現が必要になります。xer の初期コマンドラインヘルパーは、その複雑さを意図的に避けています。

---

## 短いオプションを特別扱いしない理由

`parse_arg` はハイフン 1 つで始まる引数をオプションとして扱いません。

たとえば、

```text
-x
```

は次のように解析されます。

```text
{ "", "-x" }
```

これは意図的です。

初期のコマンドラインモデルは次だけをサポートします。

* `--option`
* `--option=value`
* 通常の値

これにより規則を単純に保ち、この段階でより大きなコマンドラインパーサーを導入することを避けています。

---

## `get_cmdline`

```cpp
auto get_cmdline() -> xer::result<cmdline>;
```

`get_cmdline` は現在のプロセスのコマンドライン引数を取得し、`cmdline` オブジェクトとして返します。

返される引数は UTF-8 文字列です。

### Windows での動作

Windows では、実装は次を使って生のコマンドラインを取得します。

```cpp
GetCommandLineW
```

そして次で分割します。

```cpp
CommandLineToArgvW
```

これにより、`__wargv` のような CRT 固有のグローバルに依存することを避けます。

コマンドラインアクセスは C ランタイムライブラリのリンク方法の詳細に依存すべきではないため、この選択は意図的です。

得られた UTF-16 文字列は UTF-8 に変換されます。

### Linux での動作

Linux では、実装は次を読みます。

```text
/proc/self/cmdline
```

このファイルには、現在のプロセスのコマンドライン引数が NUL 区切りのバイト文字列として含まれます。

そのバイト文字列は、xer の Linux テキスト前提に従って UTF-8 として解釈されます。
引数が有効な UTF-8 でない場合、`get_cmdline` は失敗します。

通常でない環境では、理論上 `/proc/self/cmdline` の読み取りが失敗する場合があります。
その場合、`get_cmdline` は `xer::result` を通じてエラーを返します。

---

## ビューの寿命

`cmdline::at` と `parse_arg` は `std::u8string_view` 値を返します。

これらのビューは背後のテキストを所有しません。

`cmdline` オブジェクトから得たビューの場合、参照先データが有効なのは `cmdline` オブジェクトが生存し、変更されない間だけです。

例:

```cpp
const auto line = xer::get_cmdline();
if (!line.has_value()) {
    return 1;
}

const auto raw = line->at(1);
if (!raw.has_value()) {
    return 1;
}

const auto parsed = xer::parse_arg(*raw);
```

ここで、`parsed.first` と `parsed.second` は `line` が所有する文字列を参照しています。

---

## エラー処理

`<xer/cmdline.h>` は xer の通常の失敗モデルに従います。

`parse_arg` 自体は失敗しません。
これは単純なビューベースのパーサーであり、通常の `cmdline_arg` を返します。

`cmdline::at` は要求されたインデックスが範囲外の場合に失敗することがあります。

`get_cmdline` は、プラットフォーム固有のコマンドライン取得が失敗した場合、またはコマンドラインデータを xer の UTF-8 表現に変換できない場合に失敗することがあります。

典型的な失敗条件には次のものがあります。

* 範囲外の引数アクセス
* プラットフォームのコマンドライン取得失敗
* `/proc/self/cmdline` の読み取り失敗
* Linux のコマンドラインバイト文字列に含まれる無効な UTF-8
* Windows の UTF-16 コマンドライン文字列から UTF-8 への変換失敗

---

## `main` との関係

通常の C および C++ でコマンドライン引数を受け取る方法は `main` です。

```cpp
auto main(int argc, char** argv) -> int;
```

xer はこの方法を否定しません。

ただし、`<xer/cmdline.h>` は、明示的な `argc` / `argv` の伝播が不便または利用できない場合のために存在します。

つまり、`get_cmdline` は現在のプロセスのための便利機能であり、`main` 引数のすべての用途を置き換えるものではありません。

---

## プロセス処理との関係

`<xer/cmdline.h>` は現在のプロセスのコマンドラインを扱います。

`<xer/process.h>` は子プロセスの作成と管理を扱います。

これらは関連する話題ですが、意図的に分離されています。

* `cmdline.h` は現在のプロセスがどのように起動されたかを観測します。
* `process.h` は子プロセスを起動し制御します。

この分離により、それぞれのヘッダーの焦点を保っています。

---

## 設計上の役割

`<xer/cmdline.h>` は意図的に小さく作られています。

完全なコマンドラインオプションパーサーではありません。

特に、現在は次の機能を提供しません。

* `-x` のような短いオプションの解析
* `-abc` のような短いオプションのグループ化
* `--` のようなオプション終端の扱い
* 自動的な型変換
* 必須オプション検証
* ヘルプテキスト生成
* サブコマンド処理

初期機能は、小さな argv 取得機能と単純なロングオプション解析機能だけです。

---

## 例

```cpp
#include <xer/cmdline.h>
#include <xer/stdio.h>

auto main() -> int
{
    const auto line = xer::get_cmdline();
    if (!line.has_value()) {
        return 1;
    }

    for (std::size_t i = 1; i < line->size(); ++i) {
        const auto raw = line->at(i);
        if (!raw.has_value()) {
            return 1;
        }

        const auto parsed = xer::parse_arg(*raw);

        if (!parsed.first.empty()) {
            if (!xer::printf(
                    u8"option %@ = %@\n",
                    parsed.first,
                    parsed.second)
                     .has_value()) {
                return 1;
            }
        } else {
            if (!xer::printf(u8"value %@\n", parsed.second).has_value()) {
                return 1;
            }
        }
    }

    return 0;
}
```

---

## ドキュメント上の注意

このヘッダーを生成ドキュメントで使う場合、通常は次の点を説明すれば十分です。

* `cmdline` は UTF-8 コマンドライン引数を所有すること
* `get_cmdline` は `main` のパラメータを使わずに現在のプロセス引数を取得すること
* Windows では `GetCommandLineW` と `CommandLineToArgvW` を使うこと
* Linux では `/proc/self/cmdline` を読むこと
* `parse_arg` は単純な `--option` と `--option=value` 形式だけを認識すること
* ハイフン 1 つで始まる引数は通常の値として扱われること

詳細なコマンドラインパーサー動作を暗示すべきではありません。
このヘッダーは意図的に完全なオプション解析フレームワークではありません。

---

## 例として示す価値のある題材

次のような例は、このヘッダーに特に適しています。

* 生のコマンドライン引数を列挙する
* `--option` と `--option=value` を解析する
* 通常の値とオプションを分けて扱う
* `-x` が値として扱われることを示す

これらは `examples/` の実行可能例に適した候補です。

---

## 関連項目

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `policy_process.md`
* `header_process.md`
