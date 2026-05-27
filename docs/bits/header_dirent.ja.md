<!-- xer-reference-source-sha256: a8a89a507ff40e0797814947854f8eb29369068fb38c3420904a0bb354a4e669 -->
# `<xer/dirent.h>`

## 目的

`<xer/dirent.h>` は xer のディレクトリストリーム操作を提供します。

このヘッダーは、ディレクトリを開く、エントリ名を読む、ディレクトリストリームを巻き戻す、閉じる、といった PHP/POSIX 風のディレクトリ走査機能を扱います。

目的は POSIX `dirent.h` を正確に再現することではありません。
代わりに、xer は次のものを使う C++23 向けの小さなディレクトリストリーム API を提供します。

- パス名には `xer::path`
- ディレクトリエントリ名には UTF-8 文字列
- 通常の失敗には `xer::result`
- ディレクトリストリームにはムーブ専用の RAII ハンドル

---

## 主なエンティティ

少なくとも、`<xer/dirent.h>` は次のエンティティを提供します。

```cpp
class xer::dir;

auto xer::opendir(const path& dirname) noexcept -> result<dir>;
auto xer::closedir(dir& directory) noexcept -> result<void>;
auto xer::readdir(dir& directory) noexcept -> result<std::u8string>;
auto xer::rewinddir(dir& directory) noexcept -> result<void>;
````

---

## 設計上の役割

このヘッダーはディレクトリストリーム走査のために存在します。

ディレクトリストリームは通常のファイルストリームではなく、状態を持つ走査ハンドルであるため、`<xer/stdio.h>` から分離されています。
名前は POSIX や PHP で見慣れたものですが、API は xer 独自のパス、文字列、エラー処理モデルに合わせて調整されています。

---

## `xer::dir`

`xer::dir` はムーブ専用のディレクトリストリームハンドルです。

内部でネイティブディレクトリストリームハンドルを所有し、破棄時に自動的に閉じます。

```cpp
class xer::dir;
```

### 基本性質

* ムーブ専用
* コピー不可
* RAII ベース
* 開いているディレクトリストリームまたは空/閉じた状態を表す

### 明示的なクローズ

デストラクタはディレクトリストリームを自動的に閉じますが、デストラクタからの失敗は観測できません。

呼び出し側がクローズエラーを観測する必要がある場合は、`xer::closedir` を明示的に呼び出してください。

---

## `xer::opendir`

```cpp
auto opendir(const path& dirname) noexcept -> result<dir>;
```

`opendir` は指定されたパスに対するディレクトリストリームを開きます。

基礎となるディレクトリ API を呼び出す前に、パスは xer の UTF-8 `xer::path` 表現からプラットフォームネイティブのパス表現に変換されます。

### 返り値

成功時は、開いている `xer::dir` を返します。

失敗時は、`xer::result` の失敗を返します。

### 注意

返されるディレクトリストリームは、スナップショットに近い走査ハンドルです。
ディレクトリを読んでいる間に内容が変更された場合、観測される動作はプラットフォームおよびファイルシステム依存です。

---

## `xer::closedir`

```cpp
auto closedir(dir& directory) noexcept -> result<void>;
```

`closedir` はディレクトリストリームを閉じます。

この関数を呼び出した後、`xer::dir` オブジェクトは閉じたものとして扱われます。

### 返り値

成功時は空の成功値を返します。

失敗時は `xer::result` の失敗を返します。

### 注意

すでに閉じられている、または空の `xer::dir` に対する `closedir` 呼び出しは、何もしない成功として扱われます。

`xer::dir` のデストラクタもディレクトリストリームを閉じますが、明示的な `closedir` は呼び出し側がクローズエラーを観測したい場合に有用です。

---

## `xer::readdir`

```cpp
auto readdir(dir& directory) noexcept -> result<std::u8string>;
```

`readdir` はディレクトリストリームから次のエントリ名を読み取ります。

返される文字列は UTF-8 のディレクトリエントリ名です。

### 返り値

成功時は、次のエントリ名を返します。

ディレクトリストリームの終端に達した場合は、次のエラーで失敗を返します。

```cpp
error_t::not_found
```

その他の失敗は、通常どおり `xer::result` で報告されます。

### 重要な注意

`readdir` が返すのはエントリ名だけです。

フルパスは返しません。

たとえば、ディレクトリに次のファイルが含まれる場合、

```text
example.txt
```

`readdir` は次を返します。

```text
example.txt
```

次ではありません。

```text
directory/example.txt
```

特殊エントリ `"."` と `".."` は除外されません。
これは PHP/POSIX 風の動作により近いものです。
これらのエントリが不要な呼び出し側は、明示的に読み飛ばしてください。

エントリ順はプラットフォームおよびファイルシステム依存です。
自分でソートしない限り、コードは特定の順序に依存してはいけません。

---

## `xer::rewinddir`

```cpp
auto rewinddir(dir& directory) noexcept -> result<void>;
```

`rewinddir` はディレクトリストリームを先頭に巻き戻します。

この関数が成功した後、以降の `readdir` 呼び出しはディレクトリストリームの先頭から再びエントリを読みます。

### 返り値

成功時は空の成功値を返します。

失敗時は `xer::result` の失敗を返します。

### 注意

巻き戻し後の順序も、依然としてプラットフォームおよびファイルシステム依存です。
xer はディレクトリエントリの安定した順序を保証しません。

---

## ディレクトリ終端の扱い

xer では、ディレクトリストリームの終端到達を次のように表します。

```cpp
error_t::not_found
```

典型的な使い方は次のとおりです。

```cpp
for (;;) {
    auto entry = xer::readdir(directory);
    if (!entry.has_value()) {
        if (entry.error().code == xer::error_t::not_found) {
            break;
        }

        return 1;
    }

    // *entry を使う。
}
```

これにより、ディレクトリ終端を通常の成功読み取りから分離しつつ、通常の `xer::result` 失敗経路を使えます。

---

## パス処理との関係

`<xer/dirent.h>` はディレクトリパスに `xer::path` を使います。

`path` オブジェクトは、xer の正規化された内部形式で UTF-8 パスを保持します。
`opendir` が呼び出されると、基礎となるディレクトリ API に渡す前に、パスはプラットフォームネイティブ表現に変換されます。

`readdir` が返す名前は UTF-8 文字列へ変換されます。

---

## 他のヘッダーとの関係

`<xer/dirent.h>` は次のヘッダーと関係します。

* `<xer/path.h>`
* `<xer/stdio.h>`
* `<xer/error.h>`

大まかな境界は次のとおりです。

* `<xer/path.h>` は字句的なパス表現とパスユーティリティを扱います。
* `<xer/stdio.h>` は通常のファイルストリームとファイル関連操作を扱います。
* `<xer/dirent.h>` はディレクトリストリーム走査を扱います。

---

## ドキュメント上の注意

このヘッダーを文書化するときに最も重要な点は次のとおりです。

* `xer::dir` はムーブ専用の RAII ディレクトリストリームハンドルであること
* `readdir` はフルパスではなくエントリ名を返すこと
* `"."` と `".."` は除外されないこと
* ディレクトリ終端は `error_t::not_found` で表されること
* エントリ順はファイルシステム依存であること
* 走査中の変更結果はプラットフォーム依存であること

---

## 例

```cpp
#include <xer/dirent.h>
#include <xer/error.h>
#include <xer/stdio.h>

auto main() -> int
{
    auto directory = xer::opendir(u8".");
    if (!directory.has_value()) {
        return 1;
    }

    for (;;) {
        auto entry = xer::readdir(*directory);
        if (!entry.has_value()) {
            if (entry.error().code == xer::error_t::not_found) {
                break;
            }

            return 1;
        }

        if (*entry == u8"." || *entry == u8"..") {
            continue;
        }

        if (!xer::puts(*entry).has_value()) {
            return 1;
        }
    }

    if (!xer::closedir(*directory).has_value()) {
        return 1;
    }

    return 0;
}
```

---

## 関連項目

* `<xer/path.h>`
* `<xer/stdio.h>`
* `<xer/error.h>`
* `policy_path.md`
* `policy_examples.md`
