<!-- xer-reference-source-sha256: dab9dc9faee1ede4827f96322fb6658609cc8ecd5969f8f3a303e6224c5b1ee1 -->

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
auto socket_getsockname(socket& s) noexcept -> xer::result<socket_address>;
auto socket_listen(socket& s, int backlog = 16) noexcept -> xer::result<void>;
auto socket_accept(socket& s) noexcept -> xer::result<socket>;
auto socket_send(socket& s, std::span<const std::byte> data) noexcept -> xer::result<std::size_t>;
auto socket_recv(socket& s, std::span<std::byte> data) noexcept -> xer::result<std::size_t>;
auto socket_sendto(socket& s, std::u8string_view host, std::uint16_t port, std::span<const std::byte> data) noexcept -> xer::result<std::size_t>;
auto socket_recvfrom(socket& s, std::span<std::byte> data) noexcept -> xer::result<socket_recvfrom_result>;
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
- ホスト名は UTF-8 文字列として受け取り、リゾルバ API 用の通常のナロー文字列に変換されます。
- `socket_open` はソケットオブジェクトから結果のストリームへ所有権を移します。
