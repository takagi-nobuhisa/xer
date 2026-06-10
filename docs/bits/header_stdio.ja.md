<!-- xer-reference-source-sha256: 10aab8ccac4f2419be96f7cb9dba7763f23e7d8fcbebdd6bc2392e65ad16ab40 -->

# `<xer/stdio.h>`

## 目的

`<xer/stdio.h>` は、xer のストリームベースの入出力機能を提供します。

その役割は C 標準ライブラリの `<stdio.h>` と精神的には似ていますが、文字どおりの再実装を目的とするものではありません。
代わりに、明示的なストリーム型、明示的なエンコーディング、xer の通常の失敗モデルを中心に、実用的な I/O を再構成します。

このヘッダーは、次のような主要なユーザー向け経路を提供するため、xer の中でも特に重要な公開ヘッダーの一つです。

- バイナリストリーム I/O
- テキストストリーム I/O
- 書式付き入出力
- ファイルエントリ操作
- CSV 入出力
- ストリーム状態と位置指定
- ストリームの巻き戻し
- ストリーム内容の便利操作
- ファイル全体の内容を扱う便利操作

---

## 主な役割

`<xer/stdio.h>` の主な役割は、次の原則に基づく一貫した I/O モデルを提供することです。

- `FILE*` を公開抽象として直接公開しない
- バイナリ I/O とテキスト I/O を明示的に区別する
- ストリームの生存期間管理に RAII を使用する
- テキストエンコーディングをロケールではなく明示的に扱う
- 通常の失敗を `xer::result` で報告する

これにより、古典的な C スタイル関数名の親しみやすさを保ちながら、現代的な C++ コードからより安全に使いやすいヘッダーになります。

---

## 中核となるストリーム型

少なくとも、`<xer/stdio.h>` は次の公開ストリーム型を提供します。

```cpp
class binary_stream;
class text_stream;
```

これらはこのヘッダーの中心的な抽象です。

### `binary_stream`

`binary_stream` はバイナリ入出力を表します。

これは次の用途に使われます。

* バイナリモードで開いたファイル
* メモリを背後に持つバイナリストリーム
* バイナリ一時ストリーム
* その他のバイト指向ストリーム対象

### `text_stream`

`text_stream` はテキスト入出力を表します。

これは次の用途に使われます。

* 明示的なテキストエンコーディングで開いたファイル
* UTF-8 または CP932 のテキスト入力元
* 文字列を背後に持つテキストストリーム
* テキスト一時ストリーム
* 標準のテキスト指向入出力対象

### 設計方針

これら 2 つのストリーム型は意図的に分離されています。

xer は、モード切り替えを持つ 1 つのストリームクラスとしてこれらをモデル化しません。
代わりに、バイナリ I/O とテキスト I/O の区別を型レベルで明示します。

---

## ムーブ専用 RAII オブジェクト

`binary_stream` と `text_stream` はムーブ専用の RAII オブジェクトです。

### 意味

これは少なくとも次のことを意味します。

* コピーできない
* ムーブできる
* オブジェクトの生存期間を通じてストリームリソースを獲得・解放する
* デストラクタが自動クリーンアップを行う

### なぜ重要か

これにより、ストリーム所有権が明示され、生ハンドル共有に伴う多くの曖昧さを避けられます。

また、明示的な所有権と明示的な失敗処理を好む xer の全体的な設計にも合っています。

---

## ストリームを開く

`<xer/stdio.h>` は、ファイル、メモリ、文字列からストリームを開く関数を提供します。

### ファイルを開く

少なくとも、公開されるファイルオープン形式は次のとおりです。

```cpp
auto fopen(const path& filename, const char* mode) noexcept -> xer::result<binary_stream>;
auto fopen(const path& filename, const char* mode, encoding_t encoding) noexcept -> xer::result<text_stream>;
```

これら 2 つのオーバーロードは、バイナリオープンとテキストオープンを分離します。

### メモリを開く

メモリを背後に持つストリームについては、次のような形式を提供する場合があります。

```cpp
auto memopen(std::span<std::byte> memory, const char* mode) noexcept -> xer::result<binary_stream>;
auto stropen(std::u8string_view text, const char* mode) noexcept -> xer::result<text_stream>;
auto stropen(std::u8string& text, const char* mode) noexcept -> xer::result<text_stream>;
```

### 設計方針

これらの open 関数は、次のようになるよう設計されています。

* バイナリストリームとテキストストリームをオープン時点で区別する
* 背後のコンテナの通常の所有権を暗黙に移譲しない
* 借用されたストレージであることを API 形状で明示する

---

## テキストエンコーディングの選択

テキストストリームでは、`<xer/stdio.h>` は明示的なエンコーディング選択を提供します。

少なくとも、公開エンコーディング列挙は次のとおりです。

```cpp
enum class encoding_t {
    utf8,
    cp932,
    auto_detect,
};
```

### 意味

* `utf8` は UTF-8 テキストを意味します
* `cp932` は CP932 テキストを意味します
* `auto_detect` は、サポートするエンコーディング間で入力側の自動判定を行うことを意味します

### 重要な注意

* xer のテキスト I/O はロケール中心ではありません
* エンコーディングはストリームオープンモデルの一部です
* `auto_detect` は入力用であり、一般的な書き込み側の振る舞いを表すものではありません

これは、従来のロケール駆動の C テキスト I/O との最も明確な違いの一つです。

---

## バイナリ I/O

バイナリストリームに対して、このヘッダーはバイト指向操作を提供します。

少なくとも、次のような関数を含みます。

```cpp
fread
fwrite
fgetb
fputb
```

### このグループの役割

これらの関数は次を提供します。

* ブロック入出力
* 1 バイト入出力
* バイト単位のバイナリストリーム操作

### 設計方針

バイナリデータは、テキストではなく生のバイト指向データとして扱います。

そのため、`fgetc` と `fputc` は 1 バイトのバイナリ I/O インターフェイスではありません。
代わりに、xer はその役割に `fgetb` と `fputb` を使用します。

### EOF の扱い

逐次入力関数は、入力ストリームを使い切った状態を `error_t::end_of_file` として報告します。
これは、`fread`、`fgetb`、`fgetc`、`fgets` などで新しいデータを読めない場合に適用されます。

部分読み取りは成功のままです。
たとえば、`fread` が要求されたバイト数より少なくても 1 バイト以上を読み取った場合は、実際に読み取ったバイト数を返します。
`stream_get_contents` は、内容を収集する際に `end_of_file` を自然な終了条件として扱います。

---

## テキスト I/O

テキストストリームに対して、このヘッダーは文字指向および文字列指向の操作を提供します。

少なくとも、次のような関数を含みます。

```cpp
fgetc
getchar
ungetc

fputc
putchar

fgets
gets
fputs
puts
```

### このグループの役割

これらの関数は次を提供します。

* 1 文字のテキスト入出力
* 文字列指向のテキスト入出力
* 標準ストリーム向けの便利操作
* `ungetc` による限定的な押し戻しサポート

### 設計方針

テキストストリームは内部的に xer のテキストモデルを中心に正規化されます。

特に次の点が重要です。

* 1 文字入力は `char32_t` を中心にします
* 文字列指向テキスト処理は UTF-8 `char8_t` 文字列を中心にします

実際に見えるオーバーロード集合は実装に依存しますが、概念的なモデルは同じです。

---

## 書式付き入出力

`<xer/stdio.h>` は書式付き I/O 機能も提供します。

少なくとも、次のファミリを含みます。

```cpp
fprintf
sprintf
snprintf
printf
fscanf
sscanf
scanf
```

### このグループの役割

これらの関数は、C に慣れたユーザーにも近づきやすいスタイルで、親しみのある書式付き I/O を提供します。

### 設計方針

名前は標準ライブラリに似ていますが、周辺設計は xer 独自です。

* ストリーム型は明示的です
* テキストモデルは UTF-8 指向です
* 通常の失敗は、該当する箇所では xer スタイルの結果処理で報告します
* C の厳密なソースレベル再現よりも、xer のストリーム抽象との統合を優先します

### printf 書式の詳細

<!-- XER_INCLUDE: stdio_printf_format.md -->

### scanf 書式の詳細

<!-- XER_INCLUDE: stdio_scanf_format.md -->

---

## CSV サポート

`<xer/stdio.h>` は CSV 向けヘルパーも含みます。

```cpp
fgetcsv
fputcsv
```

### このグループの役割

これらの関数は、xer ストリーム上で便利な CSV 入出力を提供します。

CSV はテキスト指向フォーマットであり、次との統合によって特に有用になります。

* 明示的なテキストエンコーディング
* 明示的なストリーム所有権
* UTF-8 指向文字列

### 設計方針

これらの関数は単なる文字列ヘルパーではありません。
ストリームと書式付きテキストレコードを操作するため、自然に I/O 層に属します。

---

## 位置とストリーム状態のヘルパー

少なくとも、`<xer/stdio.h>` は次のようなヘルパーを提供します。

```cpp
fseek
ftell
fgetpos
fsetpos
feof
ferror
clearerr
```

また、関連する公開型は次のとおりです。

```cpp
enum seek_origin_t { seek_set, seek_cur, seek_end };
using fpos_t = std::uint64_t;
```

### このグループの役割

これらの機能は次を提供します。

* バイトまたは位置指向のストリーム移動
* ストリーム状態の検査
* ストリームエラー状態の制御
* 明示的なテキストストリーム位置処理

### バイナリとテキストの位置指定

基本的な意図は次のとおりです。

* `fseek` / `ftell` は `binary_stream` の通常の位置ヘルパーです
* `fgetpos` / `fsetpos` は `text_stream` の主要な位置ヘルパーです

これは、テキストストリームがデコードやバッファリングの後で単純なバイトオフセットにきれいに対応するとは限らないことを反映しています。

---

## 巻き戻し

`<xer/stdio.h>` は、両方のストリーム種別に対して `rewind` を提供します。

```cpp
auto rewind(binary_stream& stream) noexcept -> xer::result<void>;
auto rewind(text_stream& stream) noexcept -> xer::result<void>;
```

C 標準ライブラリ関数と異なり、xer の `rewind` は `xer::result<void>` を返すため、無効なストリームやシーク失敗を明示的に報告できます。

テキストストリームでは、巻き戻し時に押し戻し文字、先読みバイト、途中のデコード状態もクリアします。
ストリームが `encoding_t::auto_detect` で開かれていた場合、具体的なエンコーディングは未決定状態に戻ります。

---

## ストリーム全体の便利操作

`<xer/stdio.h>` は、ストリーム全体の便利操作を提供します。

```cpp
auto stream_get_contents(
    binary_stream& stream,
    std::uint64_t length = std::numeric_limits<std::uint64_t>::max())
    -> xer::result<std::vector<std::byte>>;

auto stream_get_contents(text_stream& stream)
    -> xer::result<std::u8string>;

auto stream_put_contents(
    binary_stream& stream,
    std::span<const std::byte> contents)
    -> xer::result<void>;

auto stream_put_contents(
    text_stream& stream,
    std::u8string_view contents)
    -> xer::result<void>;
```

### 目的

`stream_get_contents` と `stream_put_contents` は、すでに開いている xer ストリームから読み取る、またはそこへ書き込むための簡潔なヘルパーです。

これらは `file_get_contents` と `file_put_contents` のストリームレベル版です。

ファイル名ではなくストリームに対して動作するため、ファイル、一時ファイル、メモリストリーム、文字列ストリーム、プロセスパイプ、適用可能な場合のソケット由来ストリームなど、xer がサポートする任意のストリーム入出力元・宛先で使用できます。

### バイナリ `stream_get_contents`

```cpp
auto stream_get_contents(
    binary_stream& stream,
    std::uint64_t length = std::numeric_limits<std::uint64_t>::max())
    -> xer::result<std::vector<std::byte>>;
```

このオーバーロードは、`stream` の現在位置からバイナリデータを読み取ります。

最大で `length` バイトを読み取り、EOF に到達した場合はそこで停止します。

`length` が 0 の場合、関数は成功し、空のバイトベクタを返します。

### オフセット引数が無い理由

xer は、意図的に `stream_get_contents` にオフセット引数を提供しません。

ストリームにはすでに現在位置があります。呼び出し側が開始位置を選びたい場合は、`stream_get_contents` を呼ぶ前に `fseek`、`fsetpos`、またはその他の適切な位置指定関数を明示的に使用してください。

これにより、PHP の `file_get_contents` と `stream_get_contents` で offset と length の引数順が異なるという混乱も避けられます。

xer での規則は単純です。

* `file_get_contents` は内部でファイルを開くため、オフセットを取ることがあります
* `stream_get_contents` はストリームの現在位置から読み取ります

### テキスト `stream_get_contents`

```cpp
auto stream_get_contents(text_stream& stream)
    -> xer::result<std::u8string>;
```

このオーバーロードは、`stream` の現在位置から EOF までテキストを読み取ります。

返される文字列は UTF-8 テキストです。

外部バイト列をどのようにデコードするかは、ストリーム自身のエンコーディング状態によって制御されます。

テキストモードの `stream_get_contents` は、`offset` や `length` 引数を提供しません。バイトオフセット、デコード済み文字、行末処理、エンコーディング状態が曖昧になり得るためです。

### バイナリ `stream_put_contents`

```cpp
auto stream_put_contents(
    binary_stream& stream,
    std::span<const std::byte> contents)
    -> xer::result<void>;
```

このオーバーロードは、`contents` 内の全バイトを `stream` の現在位置へ書き込みます。

正確な配置の振る舞いは、ストリームの現在位置とオープンモードによって決まります。

たとえば、ストリームが追記モードで開かれている場合、書き込みはそのストリームの追記動作に従います。

### テキスト `stream_put_contents`

```cpp
auto stream_put_contents(
    text_stream& stream,
    std::u8string_view contents)
    -> xer::result<void>;
```

このオーバーロードは、`contents` 内の UTF-8 テキストを `stream` の現在位置へ書き込みます。

その UTF-8 テキストを外部へどのようにエンコードするかは、ストリームのエンコーディングによって決まります。

### ファイル便利関数との関係

`file_get_contents` と `file_put_contents` は、これらのストリームレベルヘルパーを包むファイルオープン用の便利ラッパーです。

概念的には次のようになります。

```cpp
auto stream = xer::fopen(filename, "r");
return xer::stream_get_contents(*stream);
```

または次のようになります。

```cpp
auto stream = xer::fopen(filename, "w");
return xer::stream_put_contents(*stream, contents);
```

ストリームレベル関数は再利用可能な読み書きロジックを持ち、ファイルレベル関数はファイルを開き、バイナリ `offset` 引数などのファイル固有オプションを適用します。

### エラー処理

これらの関数は xer の通常の失敗モデルに従います。

成功時は次のようになります。

* `stream_get_contents` は読み取ったデータを返します
* `stream_put_contents` は空の成功値を返します

失敗時は、`xer::result` によってエラーを返します。

代表的な失敗条件は次のとおりです。

* 要求された操作に対してストリームが読み取り可能または書き込み可能でない
* 読み取りまたは書き込みに失敗する
* テキストのデコードまたはエンコードに失敗する
* 結果を収集する途中でメモリ確保に失敗する

### 例

```cpp
std::u8string buffer;

auto stream = xer::stropen(buffer, "w+");
if (!stream.has_value()) {
    return 1;
}

const auto written = xer::stream_put_contents(
    *stream,
    std::u8string_view(u8"hello xer"));

if (!written.has_value()) {
    return 1;
}

const auto rewound = xer::rewind(*stream);
if (!rewound.has_value()) {
    return 1;
}

const auto text = xer::stream_get_contents(*stream);
if (!text.has_value()) {
    return 1;
}
```

---

## クローズとフラッシュ

このヘッダーは、次のような操作も提供します。

```cpp
fclose
fflush
tmpfile
```

また、明示的にエンコードされたテキストストリーム向けのテキスト指向一時ファイルオーバーロード、または同等のヘルパーも提供します。

### このグループの役割

これらの関数は次をサポートします。

* 明示的なクローズ
* 明示的なフラッシュ制御
* 一時ストリーム作成

### 設計方針

ストリームのデストラクタは自動クリーンアップを行いますが、それでも明示的な close と flush 操作は重要です。

* 呼び出し側が決定的なリソース解放を望むことがあります
* 呼び出し側が破棄前にエラーを明示的に観測したいことがあります
* 明示的なフラッシュは通常のストリーム制御の一部です

---

## ファイルエントリ操作

`<xer/stdio.h>` は、次のようなファイルエントリ操作も提供します。

```cpp
file_exists
is_file
is_dir
is_readable
is_writable

remove
rename
mkdir
rmdir
copy
touch

fileatime
filemtime
filectime

chdir
getcwd
realpath

file_get_contents
file_put_contents
```

### このグループの役割

これらの関数は、開いているストリームオブジェクトではなく、ファイルシステムエントリに対して動作します。

ストリームやファイル処理と操作上近いため、ここにまとめられています。

### 設計方針

これらの関数はストリームオブジェクトそのものとは意図的に分離されています。

通常は、生のネイティブパス文字列ではなく `xer::path` に対して動作します。

これは、パス値を内部的に UTF-8 文字列で表し、正規化された区切り文字として `/` を使う xer 独自のパスモデルと整合します。

このグループの一部の関数は単純な述語であり、別の関数は実際のファイルシステム操作を行います。

`file_exists`、`is_file`、`is_dir`、`is_readable`、`is_writable` などの述語関数は `bool` を返します。

通常失敗し得る操作は `xer::result` を返します。

### ファイル時刻ヘルパー

`fileatime`、`filemtime`、`filectime` は、POSIX エポックからの秒数としてファイル時刻フィールドを返します。これらはプラットフォーム通常のパス状態取得操作を使い、そのプラットフォームの通常の stat 風操作がシンボリックリンクをたどる場合はそれに従うことがあります。

`filectime` は `xer::stat::ctime` と同じ ctime フィールドを返します。そのフィールドのプラットフォーム固有の意味は `xer::stat` で文書化されています。

```cpp
auto fileatime(const path& filename) -> xer::result<time_t>;
auto filemtime(const path& filename) -> xer::result<time_t>;
auto filectime(const path& filename) -> xer::result<time_t>;
```

### `touch`

```cpp
auto touch(
    const path& filename,
    time_t mtime = -1,
    time_t atime = -1) -> xer::result<void>;
```

`touch` は対象の変更時刻とアクセス時刻を変更します。対象が存在しない場合は、空の通常ファイルを作成します。

負の `mtime` は現在時刻を使用することを意味します。負の `atime` は、解決済みの `mtime` をアクセス時刻としても使用することを意味します。有限でない時刻値は不正な引数として拒否されます。

---

## カレントワーキングディレクトリ操作

`<xer/stdio.h>` はカレントワーキングディレクトリのヘルパーを提供します。

```cpp
auto chdir(const path& target) -> xer::result<void>;
auto getcwd() -> xer::result<path>;
```

### `chdir`

`chdir` はプロセス全体のカレントワーキングディレクトリを変更します。

```cpp
auto chdir(const path& target) -> xer::result<void>;
```

引数は `xer::path` です。

成功時、関数は空の成功値を返します。
失敗時は、`xer::result` によってエラーを返します。

カレントワーキングディレクトリはプロセス全体の状態であるため、複数のコンポーネントやスレッドがカレントディレクトリに依存する可能性があるプログラムでは、この関数を慎重に使用してください。

### `getcwd`

`getcwd` はカレントワーキングディレクトリを返します。

```cpp
auto getcwd() -> xer::result<path>;
```

返される値は `xer::path` です。

パスは xer の内部 UTF-8 表現へ変換され、正規化された区切り文字として `/` を使用します。

結果は、呼び出し時点のプロセス全体のカレントワーキングディレクトリのスナップショットです。

---

## `realpath`

```cpp
auto realpath(const path& filename) -> xer::result<path>;
```

### 目的

`realpath` は、存在するファイルシステムエントリの正規化された絶対パスを返します。

これは、プラットフォームのパス正規化機構を通じて実際のファイルシステムに問い合わせます。

### 振る舞い

対象パスは存在していなければなりません。

相対パス要素は解決されます。
シンボリックリンクやその他のファイルシステムレベルの間接参照は、基盤プラットフォームの振る舞いに従って解決されます。

POSIX 風環境では、振る舞いはプラットフォームの `realpath` 機能に従います。
Windows では、実装は Windows のパス正規化機能を使用し、その結果を xer のパス表現へ戻します。

### 戻り値

成功時、`realpath` は `xer::path` を返します。

返されるパスは次の性質を持ちます。

* 絶対パスである
* 存在するファイルシステムエントリを参照する
* xer の UTF-8 パス表現へ変換される
* 内部区切り文字として `/` を使用する

失敗時は、`xer::result` によってエラーを返します。

代表的な失敗条件は次のとおりです。

* 対象パスが存在しない
* 呼び出し側にパスへアクセスする権限がない
* ネイティブパス変換に失敗する
* プラットフォームのパス正規化に失敗する

### 字句的なパス操作との違い

`realpath` は純粋に字句的なパス操作ではありません。

これは実際のファイルシステムに依存し、シンボリックリンク、マウント済みボリューム、権限、既存エントリなどのファイルシステム状態を観測することがあります。

純粋に字句的なパス操作には、`basename`、`parent_path`、`extension`、`stem`、`is_absolute`、`is_relative` などのパスヘルパーを使用してください。

### 例

```cpp
const auto resolved = xer::realpath(xer::path(u8"."));
if (!resolved.has_value()) {
    return 1;
}
```

成功後、`resolved` にはカレントディレクトリの正規化された絶対パスが入ります。

---

## ファイル全体の便利操作

`<xer/stdio.h>` は、PHP に着想を得たファイル全体の便利操作を提供します。

```cpp
auto file_get_contents(
    const path& filename,
    std::uint64_t offset = 0,
    std::uint64_t length = std::numeric_limits<std::uint64_t>::max())
    -> xer::result<std::vector<std::byte>>;

auto file_get_contents(
    const path& filename,
    encoding_t encoding)
    -> xer::result<std::u8string>;

auto file_put_contents(
    const path& filename,
    std::span<const std::byte> contents)
    -> xer::result<void>;

auto file_put_contents(
    const path& filename,
    std::u8string_view contents,
    encoding_t encoding)
    -> xer::result<void>;
```

### 目的

`file_get_contents` と `file_put_contents` は、手動でストリームを開かずにファイル全体を読み書きするための簡潔なヘルパーです。

これらは `stream_get_contents` と `stream_put_contents` を包むファイルオープン用の便利ラッパーです。再利用可能な読み書きの振る舞いはストリームレベルヘルパーに属し、ファイルレベルヘルパーはさらに対象ファイルを開き、ファイル固有オプションを適用します。

同名の PHP 関数に着想を得ていますが、その振る舞いは xer のストリームおよびエンコーディングモデルに従います。

### バイナリとテキストの選択

オーバーロード集合は、`encoding_t` 引数の有無によってバイナリまたはテキストの振る舞いを選択します。

* エンコーディングが指定されていない場合、ファイルは `binary_stream` として扱われます
* エンコーディングが指定されている場合、ファイルは `text_stream` として扱われます

これにより、別個のモードフラグを導入せずに、呼び出し箇所を明示的に保てます。

### バイナリ `file_get_contents`

```cpp
auto file_get_contents(
    const path& filename,
    std::uint64_t offset = 0,
    std::uint64_t length = std::numeric_limits<std::uint64_t>::max())
    -> xer::result<std::vector<std::byte>>;
```

このオーバーロードはファイルをバイナリとして開き、その内容を `std::vector<std::byte>` として返します。

省略可能な `offset` と `length` 引数はバイト単位です。

`offset` がファイルサイズより大きい場合、関数は `error_t::invalid_argument` を返します。

`offset` がファイルサイズとちょうど等しい場合、関数は成功し、空のバイトベクタを返します。

`length` が 0 の場合、関数は成功し、空のバイトベクタを返します。

### テキスト `file_get_contents`

```cpp
auto file_get_contents(
    const path& filename,
    encoding_t encoding)
    -> xer::result<std::u8string>;
```

このオーバーロードはファイルをテキストとして開き、その内容を UTF-8 テキストとして返します。

指定したエンコーディングは、外部ファイルバイト列をどのようにデコードするかを制御します。

`encoding_t::auto_detect` は、この入力側操作では有効です。

テキストモードの `file_get_contents` は、`offset` や `length` 引数を提供しません。バイトオフセット、デコード済み文字、行末処理、エンコーディング状態が曖昧になり得るためです。

### バイナリ `file_put_contents`

```cpp
auto file_put_contents(
    const path& filename,
    std::span<const std::byte> contents)
    -> xer::result<void>;
```

このオーバーロードはファイルをバイナリとして開き、`contents` 内の全バイトを書き込みます。

既存のファイル内容は置き換えられます。

### テキスト `file_put_contents`

```cpp
auto file_put_contents(
    const path& filename,
    std::u8string_view contents,
    encoding_t encoding)
    -> xer::result<void>;
```

このオーバーロードはファイルをテキストとして開き、`contents` 内の UTF-8 テキストを指定した出力エンコーディングで書き込みます。

`encoding_t::auto_detect` は書き込みでは不正であり、`error_t::invalid_argument` になります。

### PHP スタイルのフラグを提供しない理由

xer は、これらの関数に PHP スタイルの `flags` 引数を意図的に提供しません。

特に、追記動作やロック動作を `file_put_contents` の内部に隠しません。

ファイルロックが必要な場合、呼び出し側は `flock` などの外側の操作で明示的に行ってください。

追記形式の出力が必要な場合、呼び出し側は追記モードでストリームを開き、`fwrite`、`fputs`、または `stream_put_contents` で書き込むなど、ストリーム API を直接使用してください。

### エラー処理

これらの関数は xer の通常の失敗モデルに従います。

成功時は次のようになります。

* `file_get_contents` は読み取ったデータを返します
* `file_put_contents` は空の成功値を返します

失敗時は、`xer::result` によってエラーを返します。

代表的な失敗条件は次のとおりです。

* ファイルを開けない
* シークに失敗する
* 読み取りまたは書き込みに失敗する
* `offset` が不正である
* テキスト出力に `encoding_t::auto_detect` が使用されている
* テキストのデコードまたはエンコードに失敗する

### 例

```cpp
const auto text = xer::file_get_contents(
    xer::path(u8"sample.txt"),
    xer::encoding_t::utf8);

if (!text.has_value()) {
    return 1;
}

const auto written = xer::file_put_contents(
    xer::path(u8"copy.txt"),
    *text,
    xer::encoding_t::utf8);

if (!written.has_value()) {
    return 1;
}
```

---

## ネイティブハンドルアクセス

このヘッダーは、ネイティブハンドルアクセスに関連するサポートを公開する場合があります。

### 役割

これは、呼び出し側が xer ストリーム抽象をより低レベルのプラットフォーム機能またはランタイム機能へ橋渡しする必要がある場合のために存在します。

### 設計方針

このようなサポートは中心的なものではなく補助的なものです。
通常のユーザー向け抽象はネイティブハンドルではなく、`binary_stream` または `text_stream` のままです。

---

## 内部設計方針

`<xer/stdio.h>` は公開ヘッダーですが、その概念設計は次の考え方に依存しています。

* ストリームオブジェクトは軽量な公開ハンドルである
* 内部状態はそれらのハンドルの背後に隠されている
* バイナリストリームとテキストストリームは、関数ポインタベースの内部ディスパッチを使う場合がある
* テキストストリーム状態は、バッファリング、エンコーディング解決、マルチバイト中間状態を含む場合がある

これらの実装上の考え方は、内部構造そのものが公開抽象ではないとしても、公開 API の形を理解するうえで重要です。

---

## xer のテキストモデルとの関係

`<xer/stdio.h>` は、xer の全体的なテキストモデルと最も強く結び付いたヘッダーの一つです。

特に次の点が重要です。

* UTF-8 は主要な公開文字列表現です
* 必要に応じて、個々のテキスト文字には `char32_t` を使用します
* テキストストリームは UTF-8 と CP932 を明示的にサポートします
* 自動テキスト入力判定は限定的かつ明示的です

そのため、`<xer/stdio.h>` は、より広いエンコーディング関連方針とあわせて読む必要があります。

---

## 他のヘッダーおよび方針との関係

`<xer/stdio.h>` は次とあわせて理解してください。

* `policy_project_outline.md`
* `policy_stdio.md`
* `policy_encoding.md`
* `header_path.md`
* `header_stdlib.md`

おおまかな境界は次のとおりです。

* `<xer/path.h>` は字句的なパス表現とネイティブパス変換を扱います
* `<xer/stdio.h>` はストリーム I/O とファイルエントリ操作を扱います
* `<xer/stdlib.h>` はマルチバイト変換と関連するユーティリティ機能を扱います
* エンコーディング方針は、テキストストリームの背後にあるより広いテキストエンコーディングモデルを説明します

---

## ドキュメント上の注意

このヘッダーを生成ドキュメントで扱う場合、通常は次を説明すれば十分です。

* xer が `binary_stream` と `text_stream` を区別すること
* テキストエンコーディングがロケール駆動ではなく明示的であること
* ストリームオブジェクトがムーブ専用 RAII 型であること
* 低レベル I/O と、書式付き I/O や CSV などの高レベル機能の両方を含むこと
* パス指向のファイルエントリ操作がこのヘッダーの一部であること
* `realpath` がファイルシステム依存であり、字句的なパス操作とは異なること
* `stream_get_contents` は現在のストリーム位置から読み取り、意図的にオフセット引数を提供しないこと
* `file_get_contents` と `file_put_contents` は、エンコーディング引数の有無によってバイナリ/テキスト動作が選択される便利 API であること
* `file_get_contents` と `file_put_contents` はストリームレベルの内容ヘルパーを包むファイルオープン用ラッパーであること

関数ごとの詳細な意味は、リファレンスマニュアルまたは生成された API セクションで説明してください。

---

## 例として示す価値のある話題

このヘッダーには、特に次のような例が適しています。

* バイナリファイルを開いてバイトを読み取る
* UTF-8 テキストストリームを開いてテキストを読み取る
* `puts` または `fputs` でテキストを書き込む
* `fgetpos` / `fsetpos` を使用する
* `tmpfile` を使用する
* CSV を読み書きする
* `rename`、`remove`、`copy` を実行する
* `chdir` と `getcwd` でカレントワーキングディレクトリを変更し、復元する
* `realpath` で既存パスを正規化する
* `stream_get_contents` と `stream_put_contents` で、すでに開いているストリームを読み書きする
* `file_get_contents` と `file_put_contents` でファイル全体を読み書きする

これらは `examples/` 以下の実行可能な例の候補として適しています。

---

## 例

```cpp
#include <xer/stdio.h>

auto main() -> int
{
    if (!xer::puts(u8"hello").has_value()) {
        return 1;
    }

    return 0;
}
```

この例は、基本的な xer スタイルを示しています。

* xer のテキスト I/O を直接使用する
* UTF-8 指向テキストを扱う
* `xer::result` を明示的に確認する

---

## 関連項目

* `policy_project_outline.md`
* `policy_stdio.md`
* `policy_encoding.md`
* `header_path.md`
* `header_stdlib.md`


---
