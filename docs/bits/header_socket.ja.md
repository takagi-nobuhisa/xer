<!-- xer-reference-source-sha256: 0be1a6ee722b0f9fd25d1f98534f5df38c7e557de6de5e6a25d1f48e73642b0c -->

# `<xer/socket.h>`

## 目的

`<xer/socket.h>` は TCP および UDP ネットワーク用の小さなソケット API を提供します。

この API は理解しやすい程度に低水準に保ちつつ、プラットフォーム差を包み込み、通常の失敗を `xer::result` で報告します。

---

## 主な役割

このヘッダーは次の機能を提供します。

- ムーブ専用の RAII ソケットハンドル
- IPv4 および IPv6 ソケットの作成
- TCP の接続、バインド、リッスン、受け入れ操作
- UDP の送受信操作
- ストリームソケット用の固定長送受信ヘルパー
- ストリームソケット用の長さ付きメッセージ送受信ヘルパー
- ソケットから xer のバイナリストリームまたはテキストストリームへの変換

---

## 主な型

```cpp
enum class socket_family;
enum class socket_type;
struct socket_address;
struct socket_recvfrom_result;
class socket;
```

### `socket_family`

```cpp
ipv4
ipv6
```

### `socket_type`

```cpp
tcp
udp
```

### `socket_address`

`socket_address` はテキスト形式のアドレスとポート番号を保持します。

```cpp
std::u8string address;
std::uint16_t port;
```

### `socket_recvfrom_result`

`socket_recvfrom_result` は読み取ったバイト数とリモートエンドポイントアドレスを保持します。

---

## ソケットハンドル

`socket` はムーブ専用の RAII 型です。

重要な操作には次のものがあります。

```cpp
auto is_open() const noexcept -> bool;
auto family() const noexcept -> socket_family;
auto type() const noexcept -> socket_type;
auto close() noexcept -> int;
auto release() noexcept -> native_socket_t;
auto native_handle() const noexcept -> native_socket_t;
```

ソケットがまだ開いている場合、デストラクタはそのソケットを閉じます。

---

## ソケット操作

```cpp
auto socket_create(socket_family family, socket_type type) noexcept -> xer::result<socket>;
auto socket_close(socket& s) noexcept -> xer::result<void>;
auto socket_connect(socket& s, std::u8string_view host, std::uint16_t port) noexcept -> xer::result<void>;
auto socket_bind(socket& s, std::uint16_t port) noexcept -> xer::result<void>;
auto socket_bind(socket& s, std::u8string_view address, std::uint16_t port) noexcept -> xer::result<void>;
auto socket_getsockname(socket& s) noexcept -> xer::result<socket_address>;
auto socket_listen(socket& s, int backlog = 16) noexcept -> xer::result<void>;
auto socket_accept(socket& s) noexcept -> xer::result<socket>;
auto socket_send(socket& s, std::span<const std::byte> data) noexcept -> xer::result<std::size_t>;
auto socket_recv(socket& s, std::span<std::byte> data) noexcept -> xer::result<std::size_t>;
auto socket_send_all(socket& s, std::span<const std::byte> data) noexcept -> xer::result<void>;
auto socket_recv_exact(socket& s, std::span<std::byte> data) noexcept -> xer::result<void>;
auto socket_send_message(socket& s, std::span<const std::byte> data) noexcept -> xer::result<void>;
auto socket_recv_message(socket& s, std::size_t max_size) noexcept -> xer::result<std::vector<std::byte>>;
auto socket_sendto(socket& s, std::u8string_view host, std::uint16_t port, std::span<const std::byte> data) noexcept -> xer::result<std::size_t>;
auto socket_recvfrom(socket& s, std::span<std::byte> data) noexcept -> xer::result<socket_recvfrom_result>;
```

### `socket_bind(socket&, port)`

```cpp
auto socket_bind(socket& s, std::uint16_t port) noexcept -> xer::result<void>;
```

指定したポートに、そのソケットファミリーのすべてのローカルインターフェースでバインドします。

IPv4 ソケットでは、ワイルドカード IPv4 アドレスを使います。
IPv6 ソケットでは、ワイルドカード IPv6 アドレスを使います。

### `socket_bind(socket&, address, port)`

```cpp
auto socket_bind(socket& s, std::u8string_view address, std::uint16_t port) noexcept -> xer::result<void>;
```

指定したローカルアドレスとポートにソケットをバインドします。

この多重定義は、ローカル補助プロセス用の `127.0.0.1` のように、サーバーが特定のインターフェースだけで待ち受ける必要がある場合に有用です。
アドレスはソケットファミリーと互換でなければなりません。
たとえば、IPv4 ソケットは `127.0.0.1` にバインドでき、IPv6 ソケットは `::1` にバインドできます。

### `socket_send` と `socket_recv`

```cpp
auto socket_send(socket& s, std::span<const std::byte> data) noexcept -> xer::result<std::size_t>;
auto socket_recv(socket& s, std::span<std::byte> data) noexcept -> xer::result<std::size_t>;
```

`socket_send` と `socket_recv` は、1回の送信または受信操作を行います。
とくにストリームソケットでは、要求したバイト数より少ないバイト数だけを転送することがあります。
戻り値のサイズは、実際に転送されたバイト数を表します。

### `socket_send_all`

```cpp
auto socket_send_all(socket& s, std::span<const std::byte> data) noexcept -> xer::result<void>;
```

エラーが発生しない限り、`data` のすべてのバイトを送信します。

この関数は、span 全体が送信されるまで `socket_send` を繰り返し呼び出します。
TCP ソケットのような接続済みストリームソケットを対象とします。
空の span は何も送信せずに成功します。

### `socket_recv_exact`

```cpp
auto socket_recv_exact(socket& s, std::span<std::byte> data) noexcept -> xer::result<void>;
```

エラーが発生するか、十分なバイト数を受信する前にピアが接続を閉じない限り、ちょうど `data.size()` バイトを受信します。

この関数は、span 全体が埋まるまで `socket_recv` を繰り返し呼び出します。
TCP ソケットのような接続済みストリームソケットを対象とします。
空の span は何も受信せずに成功します。

### `socket_send_message`

```cpp
auto socket_send_message(socket& s, std::span<const std::byte> data) noexcept -> xer::result<void>;
```

長さ付きメッセージを送信します。

この関数は、まず4バイトの符号なしビッグエンディアンペイロード長を送信し、続けて `data` の全バイトを送信します。
ペイロードサイズは `std::uint32_t` に収まらなければならず、収まらない場合は `error_t::length_error` を返します。

空メッセージは有効で、ゼロの長さフィールドだけを送信します。
この関数は内部で `socket_send_all` を使うため、エラーが発生しない限りフレーム全体を送信します。

### `socket_recv_message`

```cpp
auto socket_recv_message(socket& s, std::size_t max_size) noexcept -> xer::result<std::vector<std::byte>>;
```

`socket_send_message` と同じフレーム形式で送信された長さ付きメッセージを1件受信します。

この関数は、まず4バイトの符号なしビッグエンディアンペイロード長を受信します。
ペイロード長が `max_size` より大きい場合、ペイロードを読んだり読み捨てたりせずに `error_t::length_error` を返します。
その場合、ストリーム位置は次のフレームの先頭ではなくなっているため、呼び出し側は通常その接続を閉じるべきです。

ペイロード長が受け入れられた場合、この関数はそのサイズの `std::vector<std::byte>` を確保し、ちょうどそのバイト数のペイロードを受信して、その vector を返します。
空メッセージは有効で、空の vector を返します。

完全な長さフィールドまたはペイロードを受信する前にピアが接続を閉じた場合、この関数は `error_t::network_error` を返します。

---

## 長さ付きメッセージの例

`socket_send_message` が送信するメッセージは、次のフレーム形式を持ちます。

```text
uint32 big-endian payload_size
payload bytes
```

たとえば、5バイトのペイロード `hello` は、4バイトの長さフィールドに続けて5バイトのペイロードとして送信されます。

```cpp
constexpr std::array<std::byte, 5> hello = {
    std::byte {'h'},
    std::byte {'e'},
    std::byte {'l'},
    std::byte {'l'},
    std::byte {'o'},
};

auto sent = xer::socket_send_message(client, hello);
```

受信側は `socket_recv_message` でフレームを読み取れます。

```cpp
auto body = xer::socket_recv_message(server, 1024 * 1024);
```

第2引数は、受け入れる最大ペイロードサイズです。
信頼できない長さフィールドによって無制限のメモリ確保が発生することを防ぎます。

---

## 例

次の例は、TCP サーバーソケットをループバックアドレスにバインドし、固定長メッセージを交換します。

```cpp
#include <array>
#include <bit>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <thread>

#include <xer/socket.h>

namespace
{
    auto bytes_to_text(std::span<const std::byte> bytes) -> std::string
    {
        std::string text(bytes.size(), '\0');
        std::memcpy(text.data(), bytes.data(), bytes.size());
        return text;
    }
}

auto main() -> int
{
    constexpr auto port = std::uint16_t{39080};

    auto server_result = xer::socket_create(xer::socket_family::ipv4, xer::socket_type::tcp);
    if (!server_result) {
        std::cerr << "socket_create failed\n";
        return 1;
    }

    auto server = std::move(*server_result);

    if (auto result = xer::socket_bind(server, u8"127.0.0.1", port); !result) {
        std::cerr << "socket_bind failed\n";
        return 1;
    }

    if (auto result = xer::socket_listen(server); !result) {
        std::cerr << "socket_listen failed\n";
        return 1;
    }

    auto worker = std::thread([&server] {
        auto accepted_result = xer::socket_accept(server);
        if (!accepted_result) {
            return;
        }

        auto peer = std::move(*accepted_result);

        auto request = std::array<std::byte, 4>{};
        if (auto result = xer::socket_recv_exact(peer, request); !result) {
            return;
        }

        std::cout << "server received: " << bytes_to_text(request) << '\n';

        constexpr auto response = std::array{
            std::byte{'p'},
            std::byte{'o'},
            std::byte{'n'},
            std::byte{'g'},
        };

        (void) xer::socket_send_all(peer, response);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    auto client_result = xer::socket_create(xer::socket_family::ipv4, xer::socket_type::tcp);
    if (!client_result) {
        worker.join();
        return 1;
    }

    auto client = std::move(*client_result);

    if (auto result = xer::socket_connect(client, u8"127.0.0.1", port); !result) {
        worker.join();
        return 1;
    }

    constexpr auto request = std::array{
        std::byte{'p'},
        std::byte{'i'},
        std::byte{'n'},
        std::byte{'g'},
    };

    if (auto result = xer::socket_send_all(client, request); !result) {
        worker.join();
        return 1;
    }

    auto response = std::array<std::byte, 4>{};
    if (auto result = xer::socket_recv_exact(client, response); !result) {
        worker.join();
        return 1;
    }

    std::cout << "client received: " << bytes_to_text(response) << '\n';

    worker.join();
    return 0;
}
```

---

## ストリーム変換

ソケットは xer のストリームに変換できます。

```cpp
auto socket_open(socket&& s) noexcept -> xer::result<binary_stream>;
auto socket_open(socket&& s, encoding_t encoding) noexcept -> xer::result<text_stream>;
```

バイナリストリーム形式はバイト指向のプロトコルに適しています。
テキストストリーム形式は UTF-8 または CP932 のテキスト指向通信に適しています。

---

## 注意

- ネットワーク関連の通常の失敗は主に `error_t::network_error` で表されます。
- この API はコマンドシェルや外部ユーティリティを使用しません。
- ホスト名とテキスト形式のアドレスは UTF-8 文字列として受け取り、リゾルバ API 用の通常のナロー文字列に変換されます。
- `socket_send` と `socket_recv` は、与えられたバッファの一部だけを転送することがあります。
- 接続済みストリームソケットで固定量のデータを転送する必要がある場合は、`socket_send_all` と `socket_recv_exact` を使います。
- `socket_open` はソケットオブジェクトから結果のストリームへ所有権を移します。
