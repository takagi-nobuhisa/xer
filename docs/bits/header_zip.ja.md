<!-- xer-reference-source-sha256: e2069a27116bf949ceac1a79a350761d1dbbcc46fce6d6996b130be6f00bc3e6 -->
# `<xer/zip.h>`

## 目的

`<xer/zip.h>` は xer の ZIP アーカイブ読み書き機能を提供します。

ZIP は技術的にはアーカイブ形式ですが、実用上は馴染みのある圧縮・展開形式でもあります。xer では、初期 ZIP API を、将来シリアライズ済みデータパッケージ、同梱リソース、通常のファイル交換を支えられる小さな圧縮・アーカイブユーティリティとして扱います。

初期 API は意図的に小さくしています。逐次読み取り、名前検索、単純なアーカイブ作成、エントリ全体の読み取り、単純な展開ヘルパーをサポートします。コメントと ZIP64 対応は後回しです。

---

## 外部依存

`<xer/zip.h>` は zlib の開発用ヘッダーと zlib ライブラリを必要とします。

公開ヘッダーは、利用可能な場合に `__has_include` で `<zlib.h>` を確認し、ヘッダーが見つからなければコンパイル時診断を出します。このヘッダーを使うプログラムは、典型的な Unix 系環境での `-lz` のように、zlib とリンクする必要もあります。

プロジェクトのテストランナーは `xer/zip.h` を `zip` feature として検出し、zlib が利用可能な場合には対応するテストとコード例を zlib とリンクします。Visual Studio 2026 の clang-cl または MSVC cl.exe では、xer のテストとコード例は vcpkg manifest mode により `vcpkg_installed\x64-windows` にインストールされた zlib を使用します。

---

## 主な役割

`<xer/zip.h>` の主な役割は、次のことを可能にすることです。

- ZIP アーカイブを開く
- エントリのメタデータを逐次読む
- 正確な名前でエントリを検索する
- エントリ名、サイズ、圧縮方式名を取得する
- エントリデータを読んで展開する
- 1つのエントリまたはすべてのエントリをファイルシステムへ展開する
- ZIP アーカイブを作成する
- メモリ上のバイト列または元ファイルを deflate エントリとして追加する
- 書き込み側を明示的に確定し、最終化エラーを報告できるようにする
- アーカイブ終端を xer の通常のエラーモデルで報告する

この設計では、アーカイブ終端に `std::optional` を返すことを避けています。エントリ列の末尾に到達したことは、次のように報告されます。

```cpp
error_t::end_of_file
```

これにより、`zip_read` は xer のほかの逐次入力操作と整合します。

---

## 主なエンティティ

少なくとも、`<xer/zip.h>` は次の型と関数を提供します。

```cpp
class zip_archive;
class zip_entry;

auto zip_open(std::u8string_view filename) -> xer::result<zip_archive>;

auto zip_create(std::u8string_view filename) -> xer::result<zip_archive>;

auto zip_read(zip_archive& archive) -> xer::result<zip_entry>;

auto zip_entry_name(const zip_entry& entry) -> xer::result<std::u8string>;

auto zip_entry_filesize(const zip_entry& entry) -> xer::result<std::uint64_t>;

auto zip_entry_compressed_size(const zip_entry& entry)
    -> xer::result<std::uint64_t>;

auto zip_entry_compression_method(const zip_entry& entry)
    -> xer::result<std::u8string>;

auto zip_entry_read(zip_archive& archive, const zip_entry& entry)
    -> xer::result<std::vector<std::byte>>;

auto zip_locate_name(zip_archive& archive, std::u8string_view entry_name)
    -> xer::result<zip_entry>;

auto zip_entry_read_by_name(
    zip_archive& archive,
    std::u8string_view entry_name) -> xer::result<std::vector<std::byte>>;

auto zip_entry_extract(
    zip_archive& archive,
    const zip_entry& entry,
    std::u8string_view target_filename) -> xer::result<void>;

auto zip_entry_extract_to(
    zip_archive& archive,
    const zip_entry& entry,
    std::u8string_view destination_dir) -> xer::result<void>;

auto zip_extract_to(
    zip_archive& archive,
    std::u8string_view destination_dir) -> xer::result<void>;

auto zip_add_from_bytes(
    zip_archive& archive,
    std::u8string_view entry_name,
    std::span<const std::byte> data) -> xer::result<void>;

auto zip_add_file(
    zip_archive& archive,
    std::u8string_view source_path,
    std::u8string_view entry_name) -> xer::result<void>;

auto zip_commit(zip_archive& archive) -> xer::result<void>;
```

通常のケースでは失敗が想定されないメタデータアクセサーを含め、すべての公開操作は `xer::result` を返します。これにより API の対称性を保ち、将来の検証、変換、バックエンド変更の余地を残します。

---

## `zip_archive`

```cpp
class zip_archive;
```

`zip_archive` はムーブ専用のアーカイブハンドルです。

読み取り時には、基盤となるバイナリストリームと中央ディレクトリの読み取り位置を所有します。書き込み時には、出力ストリームと保留中の中央ディレクトリレコードを所有します。破棄時には基盤ストリームを自動的に閉じます。コピーは無効です。

呼び出し側は通常、読み取りには `zip_open`、書き込みには `zip_create` を使ってこのオブジェクトを取得します。

書き込み側を使う場合、呼び出し側は明示的に `zip_commit` を呼び出すべきです。破棄時にストリームを閉じることはできますが、最終化エラーを `xer::result` として報告することはできません。

---

## `zip_entry`

```cpp
class zip_entry;
```

`zip_entry` は、中央ディレクトリから読まれた1つのアーカイブエントリのメタデータを格納します。

これは少なくとも次の情報を含む軽量な値オブジェクトです。

- エントリ名
- 非圧縮サイズ
- 圧縮サイズ
- 圧縮方式識別子
- フラグ
- ローカルヘッダーオフセット

呼び出し側は内部表現の詳細に依存せず、公開されている `zip_entry_*` 関数を使うべきです。

---

## `zip_open`

```cpp
auto zip_open(std::u8string_view filename) -> xer::result<zip_archive>;
```

### 目的

`zip_open` は ZIP アーカイブを読み取り用に開きます。

### 入力モデル

ファイル名は UTF-8 パス文字列です。内部では、ファイルを開く前に xer のパス処理を通して変換されます。

### 対応するアーカイブ

初期実装は、単一ディスク中央ディレクトリを持つ通常の非 ZIP64 アーカイブに対応します。

次のものは `error_t::invalid_argument` として拒否されます。

- 不正な ZIP ファイル
- end-of-central-directory レコードの欠落
- マルチディスクアーカイブ
- ZIP64 アーカイブ
- 整合しない中央ディレクトリ範囲

### 戻り値モデル

成功すると、開かれた `zip_archive` を返します。

失敗すると、`xer::result` のエラー情報を返します。ファイルを開く際の失敗は通常のファイルエラーモデルで報告されます。形式レベルの失敗は、一般に `error_t::invalid_argument` として報告されます。

---

## `zip_create`

```cpp
auto zip_create(std::u8string_view filename) -> xer::result<zip_archive>;
```

### 目的

`zip_create` は ZIP アーカイブを書き込み用に開きます。

返されたアーカイブは書き込み側です。`zip_commit` が中央ディレクトリを書き込んでストリームを閉じるまで、完全な ZIP ファイルではありません。

### 出力モデル

初期の書き込み側は、通常の非 ZIP64 単一ディスクアーカイブを作成します。エントリ名は ZIP の UTF-8 フラグを設定した UTF-8 名として格納されます。

---

## `zip_read`

```cpp
auto zip_read(zip_archive& archive) -> xer::result<zip_entry>;
```

### 目的

`zip_read` は、アーカイブの中央ディレクトリから次のエントリメタデータを読みます。

### 逐次モデル

アーカイブは現在の中央ディレクトリ位置を保持します。各成功呼び出しは、その位置を次のエントリへ進めます。

これにより、エントリ一覧全体をメモリ上に構築せずに済みます。これは多数のエントリを持つ大きなアーカイブで重要です。

### アーカイブ終端

それ以上エントリがない場合、この関数は次を返します。

```cpp
error_t::end_of_file
```

これは空の optional 値ではなく、エラー結果です。

### 未対応のエントリメタデータ

初期実装は、暗号化エントリ、マルチディスクエントリ参照、ZIP64 サイズのエントリを `error_t::invalid_argument` として拒否します。

---

## `zip_entry_name`

```cpp
auto zip_entry_name(const zip_entry& entry) -> xer::result<std::u8string>;
```

`zip_entry_name` はエントリ名を UTF-8 文字列として返します。

初期実装は、有効な UTF-8 であるエントリ名を受け入れます。ZIP の UTF-8 名フラグが設定されていて、格納された名前が有効な UTF-8 でない場合、その操作は `zip_read` の実行中に `error_t::encoding_error` で失敗します。

CP437 名変換はまだ実装されていません。そのため、非 UTF-8 名は推測されずに拒否されます。

---

## `zip_entry_filesize`

```cpp
auto zip_entry_filesize(const zip_entry& entry) -> xer::result<std::uint64_t>;
```

`zip_entry_filesize` は、エントリの非圧縮サイズをバイト単位で返します。

この名前は PHP の `zip_entry_filesize` の語彙に従いつつ、C++ 整数型を `xer::result` 経由で返します。

---

## `zip_entry_compressed_size`

```cpp
auto zip_entry_compressed_size(const zip_entry& entry)
    -> xer::result<std::uint64_t>;
```

`zip_entry_compressed_size` は、エントリの圧縮サイズをバイト単位で返します。

この関数名は PHP の `zip_entry_compressedsize` という綴りではなく snake_case を使います。これは C++ API であり、正確な互換性が必要ない箇所では読みやすさを優先するためです。

---

## `zip_entry_compression_method`

```cpp
auto zip_entry_compression_method(const zip_entry& entry)
    -> xer::result<std::u8string>;
```

`zip_entry_compression_method` は、圧縮方式名の文字列を返します。

初期実装は、stored エントリに対して次を返します。

```text
store
```

また、deflated エントリに対して次を返します。

```text
deflate
```

その他の方式識別子に対しては次を返します。

```text
unknown
```

未対応方式のデータ読み取りは `error_t::invalid_argument` で失敗します。

---

## `zip_entry_read`

```cpp
auto zip_entry_read(zip_archive& archive, const zip_entry& entry)
    -> xer::result<std::vector<std::byte>>;
```

### 目的

`zip_entry_read` は1つのエントリ本体を読んで展開します。

そのエントリは、同じアーカイブから取得されたものでなければなりません。初期 API は、別アーカイブ由来の使用を検証しようとはしません。

### 対応する圧縮方式

初期実装は次に対応します。

- stored エントリ
- deflated エントリ

stored エントリはそのまま返されます。deflated エントリは zlib による raw deflate で展開されます。

### 出力モデル

成功すると、この関数は展開済みのエントリバイト列を含む `std::vector<std::byte>` を返します。

これは意図的に所有権を持つバイトベクターです。大きなエントリを扱う用途が必要になれば、ストリーミング形式のエントリ読み取りを後で追加できます。

---

## `zip_locate_name`

```cpp
auto zip_locate_name(zip_archive& archive, std::u8string_view entry_name)
    -> xer::result<zip_entry>;
```

### 目的

`zip_locate_name` は、名前が `entry_name` と正確に一致するエントリをアーカイブ中央ディレクトリから検索します。

この関数は中央ディレクトリを走査し、最初に一致したエントリを返します。`zip_read` が使う逐次位置は変更しないため、呼び出し側は直接検索と後続の逐次読み取りを混在できます。

### 失敗モデル

要求された名前を持つエントリが存在しない場合、この関数は次を返します。

```cpp
error_t::not_found
```

不正な中央ディレクトリデータは、`zip_read` と同様に `error_t::invalid_argument` として報告されます。

---

## `zip_entry_read_by_name`

```cpp
auto zip_entry_read_by_name(
    zip_archive& archive,
    std::u8string_view entry_name) -> xer::result<std::vector<std::byte>>;
```

### 目的

`zip_entry_read_by_name` は、名前でエントリを見つけ、展開済みの本体を読みます。

これは次の処理を行う便利関数です。

```cpp
auto entry = zip_locate_name(archive, entry_name);
auto body = zip_entry_read(archive, *entry);
```

見つからないエントリは `error_t::not_found` として報告されます。未対応の圧縮方式や不正なエントリデータは、`zip_entry_read` と同じ方法で報告されます。

---

## `zip_entry_extract`

```cpp
auto zip_entry_extract(
    zip_archive& archive,
    const zip_entry& entry,
    std::u8string_view target_filename) -> xer::result<void>;
```

### 目的

`zip_entry_extract` は1つのエントリ本体を読み、指定されたファイルシステムパスへ書き込みます。

`target_filename` の親ディレクトリは必要に応じて作成されます。エントリ名が `/` で終わる場合、対象パスはファイルではなくディレクトリとして作成されます。

この関数は、エントリ名を展開先パスとして解釈しません。呼び出し側が正確な出力ファイル名をすでに選んでいる場合に適しています。

---

## `zip_entry_extract_to`

```cpp
auto zip_entry_extract_to(
    zip_archive& archive,
    const zip_entry& entry,
    std::u8string_view destination_dir) -> xer::result<void>;
```

### 目的

`zip_entry_extract_to` は、エントリ名を相対パスとして使い、1つのエントリを展開先ディレクトリ配下へ展開します。

### パス安全性

エントリ名が空、絶対パス、ドライブ相対、NUL コード単位を含む、中間に空の要素を含む、または `.` / `..` 要素を含む場合、そのエントリ名は `error_t::invalid_argument` として拒否されます。これにより、`../outside.txt` や `/tmp/outside.txt` のような通常のパストラバーサルを防ぎます。

名前が `/` で終わるディレクトリエントリはディレクトリを作成します。ファイルエントリは不足している親ディレクトリを作成し、その後に展開済みバイト列を書き込みます。

---

## `zip_extract_to`

```cpp
auto zip_extract_to(
    zip_archive& archive,
    std::u8string_view destination_dir) -> xer::result<void>;
```

### 目的

`zip_extract_to` は、すべてのエントリを展開先ディレクトリ配下へ展開します。

この関数は中央ディレクトリを直接走査し、`zip_read` が使う逐次位置を変更しません。これにより、呼び出し側はアーカイブを展開した後でも、同じ位置から後続の逐次走査を行えます。

すべてのエントリ名には、`zip_entry_extract_to` と同じパス安全性規則が適用されます。いずれかのエントリが安全でない、または展開できない場合、この関数はエラーを返します。そのエラーより前に書き込まれたエントリはロールバックされません。

---

## `zip_add_from_bytes`

```cpp
auto zip_add_from_bytes(
    zip_archive& archive,
    std::u8string_view entry_name,
    std::span<const std::byte> data) -> xer::result<void>;
```

### 目的

`zip_add_from_bytes` は、メモリ上の1つのバイト列を ZIP アーカイブ書き込み側へ追加します。

エントリは raw deflate で圧縮され、ローカルファイルヘッダーとともに書き込まれます。中央ディレクトリレコードは `zip_commit` が呼ばれるまでメモリ上に保持されます。

### 制限

初期の書き込み側は ZIP64 に対応しません。そのため、エントリ名、圧縮サイズ、非圧縮サイズ、ローカルヘッダーオフセット、エントリ数は、通常の ZIP フィールドに収まらなければなりません。

エントリ名は空でない有効な UTF-8 文字列でなければなりません。非 UTF-8 名は `error_t::encoding_error` で失敗します。

---

## `zip_add_file`

```cpp
auto zip_add_file(
    zip_archive& archive,
    std::u8string_view source_path,
    std::u8string_view entry_name) -> xer::result<void>;
```

### 目的

`zip_add_file` は元ファイルを読み、その内容を1つの ZIP エントリとして追加します。

これは `file_get_contents` と `zip_add_from_bytes` の便利ラッパーです。初期実装は、圧縮前に元ファイル全体をメモリへ読み込みます。大きなファイルを扱う用途が必要になれば、ストリーミング形式の file-to-ZIP 出力を後で追加できます。

---

## `zip_commit`

```cpp
auto zip_commit(zip_archive& archive) -> xer::result<void>;
```

### 目的

`zip_commit` は ZIP アーカイブ書き込み側を最終化します。

中央ディレクトリと end-of-central-directory レコードを書き込み、ストリームを flush して閉じます。最終化時にエラーが発生する可能性があるため、呼び出し側は書き込み側アーカイブではこれを必須手順として扱うべきです。

`zip_commit` 後に書き込み操作を呼び出すと、`error_t::invalid_argument` で失敗します。

---

## エラー処理

`<xer/zip.h>` は xer の通常の失敗モデルに従います。

つまり、次のようになります。

- 通常の失敗は `xer::result` で報告される
- アーカイブ終端は `error_t::end_of_file` として報告される
- 不正なアーカイブと不正な操作順序は `error_t::invalid_argument` として報告される
- 不正なエントリ名エンコーディングは `error_t::encoding_error` として報告される
- ストリーム失敗は通常のファイルエラーまたは I/O エラーを通じて報告される

初期実装は、詳細な ZIP 解析位置を提供しません。位置付き診断が後で有用になれば、エラー詳細型を別途追加できます。

---

## 後回しにしている項目と制限事項

次の項目は意図的に後回しにしています。

- エントリコメントとアーカイブコメント
- ZIP64
- マルチディスクアーカイブ
- 暗号化エントリ
- CP437 ファイル名変換
- ストリーミング形式のエントリ本体読み取り
- stored エントリ書き込みオプション
- データ記述子指向の書き込み対応
- 公開オプションとしての CRC 検証
- ストリーミング形式の file-to-ZIP 書き込み

最初の目標は、xer の `xer::result` と逐次 EOF モデルに合う、小さく予測しやすい PHP 風の ZIP リーダー・ライターです。
