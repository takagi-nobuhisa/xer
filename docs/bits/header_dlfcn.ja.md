<!-- xer-reference-source-sha256: 3ec99dcce0da06bf000a5754961ce311461ffbdc92250beaa9d2ae5ab78fbee2 -->

# `<xer/dlfcn.h>`

## 目的

`<xer/dlfcn.h>` は、共有ライブラリを実行時に読み込み、シンボルを解決するための小規模なクロスプラットフォームAPIを提供します。

APIは、よく知られたPOSIX風の命名規則に従いながら、`xer` 名前空間内に置かれます。POSIXプラットフォームでは `dlopen`, `dlsym`, `dlclose` をラップし、Windowsでは対応する `LoadLibraryW`, `GetProcAddress`, `FreeLibrary` の機能をラップします。

---

## 主な役割

このヘッダーは、次の機能を提供します。

- コピー可能な共有ライブラリハンドル
- 共有ライブラリの実行時読み込み
- 呼び出し側で明示的なキャストを行わずに使える、型付きシンボル検索
- 低水準用途のための生シンボル検索
- 最後の共有ハンドルが解放されたときの自動アンロード

---

## 主な型

```cpp
class shared_library;
```

### `shared_library`

`shared_library` は、軽量でコピー可能な共有ハンドルです。

コピーされた各オブジェクトは、同じネイティブライブラリハンドルを参照します。そのハンドルを参照する最後の `shared_library` オブジェクトが解放されたとき、基底となるライブラリがアンロードされます。

---

## 主な関数

```cpp
auto dlopen(std::string_view path) -> result<shared_library>;
auto dlclose(shared_library& library) noexcept -> void;
auto is_open(const shared_library& library) noexcept -> bool;
auto native_handle(const shared_library& library) noexcept -> void*;

auto dlsym(const shared_library& library, std::string_view name, void*& out) -> result<void>;

template<class Function>
auto dlsym(const shared_library& library, std::string_view name, Function*& out) -> result<void>;
```

### `dlopen`

```cpp
auto dlopen(std::string_view path) -> result<shared_library>;
```

共有ライブラリを読み込み、`shared_library` ハンドルを返します。

POSIXプラットフォームでは、初期実装は次のフラグを使います。

```cpp
RTLD_NOW | RTLD_LOCAL
```

Windowsでは、初期実装は `LoadLibraryW` を使います。

最初のバージョンでは、プラットフォーム固有のフラグを意図的に公開しません。フラグ制御が必要になった場合は、既存APIを変更せずにオーバーロードを追加できます。

### `dlclose`

```cpp
auto dlclose(shared_library& library) noexcept -> void;
```

対象ハンドルが保持する、読み込まれたライブラリへの参照を解放します。

同じネイティブハンドルを参照する別の `shared_library` オブジェクトが残っている場合、ライブラリは引き続き読み込まれた状態を保ちます。

### `is_open`

```cpp
auto is_open(const shared_library& library) noexcept -> bool;
```

ハンドルが現在、読み込まれたライブラリを参照しているかどうかを返します。

### `native_handle`

```cpp
auto native_handle(const shared_library& library) noexcept -> void*;
```

ネイティブライブラリハンドルを生ポインタ値として返します。ハンドルが空の場合は `nullptr` を返します。

### `dlsym`

```cpp
auto dlsym(const shared_library& library, std::string_view name, void*& out) -> result<void>;
```

生のシンボルアドレスを検索し、`out` に格納します。

```cpp
template<class Function>
auto dlsym(const shared_library& library, std::string_view name, Function*& out) -> result<void>;
```

関数シンボルを検索し、`out` に格納します。関数型は出力ポインタから推論されるため、呼び出し側で明示的なキャストを書く必要はありません。

例:

```cpp
using function_type = int(const char*);

function_type* function = nullptr;
auto r = xer::dlsym(library, "puts", function);
```

---

## 設計上の注意

公開ヘッダー名が `<xer/dlfcn.h>` なのは、このAPIが意図的にPOSIXの `dlopen` / `dlsym` / `dlclose` という語彙に従っているためです。

内部実装は `<xer/bits/dlfcn.h>` に置かれています。

これらの関数は `xer` 名前空間にあるため、POSIX風の名前を使ってもグローバルなプラットフォーム関数とは衝突しません。

---

## 制限事項

初期APIでは、`dlopen` のフラグや `LoadLibraryExW` のフラグを公開しません。

既定の動作は、意図的に保守的なものです。

- POSIX: `RTLD_NOW | RTLD_LOCAL` で読み込む
- Windows: `LoadLibraryW` で読み込む

具体的な必要性が生じた場合は、オーバーロードを通じてプラットフォーム固有のフラグ制御を後から追加できます。
