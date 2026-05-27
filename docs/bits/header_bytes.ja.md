<!-- xer-reference-source-sha256: d6333919938a7668a14eee3c8c241492934b28d3153a0762ce8b625f88564d61 -->

# `<xer/bytes.h>`

## 目的

`<xer/bytes.h>` は、バイト列変換ヘルパーを提供します。

このヘッダーの目的は、通常のバイト風ストレージやテキストストレージを、Base64 エンコード、バイナリストリーム、ソケット、プロセスパイプなどのバイト指向 API に渡しやすくすることです。

---

## 主なエンティティ

少なくとも、`<xer/bytes.h>` は次の関数を提供します。

```cpp
auto to_bytes_view(std::string_view value) noexcept
    -> std::span<const std::byte>;

auto to_bytes_view(std::u8string_view value) noexcept
    -> std::span<const std::byte>;

auto to_bytes_view(std::span<const char> value) noexcept
    -> std::span<const std::byte>;

auto to_bytes_view(std::span<const char8_t> value) noexcept
    -> std::span<const std::byte>;

auto to_bytes_view(std::span<const unsigned char> value) noexcept
    -> std::span<const std::byte>;

auto to_bytes_view(std::span<const std::byte> value) noexcept
    -> std::span<const std::byte>;

auto to_bytes(std::string_view value) -> std::vector<std::byte>;
auto to_bytes(std::u8string_view value) -> std::vector<std::byte>;
auto to_bytes(std::span<const char> value) -> std::vector<std::byte>;
auto to_bytes(std::span<const char8_t> value) -> std::vector<std::byte>;
auto to_bytes(std::span<const unsigned char> value) -> std::vector<std::byte>;
auto to_bytes(std::span<const std::byte> value) -> std::vector<std::byte>;
```

---

## `to_bytes_view`

`to_bytes_view` は、渡されたストレージを指す非所有の `std::span<const std::byte>` ビューを作成します。

割り当てやコピーは行いません。返される span は、入力と同じメモリを参照します。

返される値はビューなので、呼び出し側は入力ストレージが返された span より長く生存することを保証する必要があります。

---

## `to_bytes`

`to_bytes` は、渡されたストレージの所有する `std::vector<std::byte>` コピーを作成します。

これは、元の文字列や span の寿命に依存しない独立したバイト列が必要な場合に有用です。

---

## 設計上の注意

これらのヘルパーは文字エンコーディング変換を行いません。

たとえば、`std::u8string_view` を `to_bytes_view` に渡すと、UTF-8 コード単位をバイトとして公開します。テキストを検証、正規化、または別のエンコーディングとして再解釈することはありません。

区別は単純です。

- `to_bytes_view` は非所有であり、コピーしない
- `to_bytes` は所有し、コピーする

---

## 他のヘッダーとの関係

`<xer/bytes.h>` は、特に次のヘッダーと一緒に使うと有用です。

- `<xer/base64.h>`
- `<xer/stdio.h>`
- `<xer/socket.h>`
- `<xer/process.h>`

おおまかな境界は次のとおりです。

- `<xer/bytes.h>` は、バイト風ストレージやテキストストレージを明示的なバイト列に変換する
- `<xer/base64.h>` は、バイト列と Base64 テキストを相互変換する
- `<xer/stdio.h>` は、バイナリストリームとテキストストリームを扱う

---

## 例

```cpp
#include <string_view>

#include <xer/base64.h>
#include <xer/bytes.h>

auto main() -> int
{
    constexpr std::u8string_view text = u8"hello";

    const auto bytes = xer::to_bytes_view(text);
    const auto encoded = xer::base64_encode(bytes);
    if (!encoded.has_value()) {
        return 1;
    }

    return 0;
}
```
