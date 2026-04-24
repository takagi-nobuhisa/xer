# `<xer/socket.h>`

## Purpose

`<xer/socket.h>` provides a small socket API for TCP and UDP networking.

The API is intentionally low-level enough to stay understandable, but it wraps platform differences and reports ordinary failure through `xer::result`.

---

## Main Role

This header provides:

- a move-only RAII socket handle
- IPv4 and IPv6 socket creation
- TCP connection, bind, listen, and accept operations
- UDP send/receive operations
- conversion of sockets to XER binary or text streams

---

## Main Types

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

`socket_address` stores a textual address and a port number.

```cpp
std::u8string address;
std::uint16_t port;
```

### `socket_recvfrom_result`

`socket_recvfrom_result` stores the number of bytes read and the remote endpoint address.

---

## Socket Handle

`socket` is a move-only RAII type.

Important operations include:

```cpp
auto is_open() const noexcept -> bool;
auto family() const noexcept -> socket_family;
auto type() const noexcept -> socket_type;
auto close() noexcept -> int;
auto release() noexcept -> native_socket_t;
auto native_handle() const noexcept -> native_socket_t;
```

The destructor closes the socket if it is still open.

---

## Socket Operations

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

## Stream Conversion

Sockets can be converted into XER streams:

```cpp
auto socket_open(socket&& s) noexcept -> xer::result<binary_stream>;
auto socket_open(socket&& s, encoding_t encoding) noexcept -> xer::result<text_stream>;
```

The binary stream form is suitable for byte-oriented protocols.
The text stream form is suitable for UTF-8 or CP932 text-oriented communication.

---

## Notes

- Network-related ordinary failures are represented mainly with `error_t::network_error`.
- The API does not use command shells or external utilities.
- Host names are accepted as UTF-8 strings and converted to ordinary narrow strings for resolver APIs.
- `socket_open` transfers ownership from the socket object into the resulting stream.
