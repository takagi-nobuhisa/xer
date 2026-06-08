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
- reliable fixed-size send/receive helpers for stream sockets
- length-prefixed message sending for stream sockets
- conversion of sockets to xer binary or text streams

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

Binds a socket to the specified port on all local interfaces for the socket family.

For an IPv4 socket, this uses the wildcard IPv4 address.
For an IPv6 socket, this uses the wildcard IPv6 address.

### `socket_bind(socket&, address, port)`

```cpp
auto socket_bind(socket& s, std::u8string_view address, std::uint16_t port) noexcept -> xer::result<void>;
```

Binds a socket to the specified local address and port.

This overload is useful when a server must listen only on a specific interface, such as `127.0.0.1` for a local helper process.
The address must be compatible with the socket family.
For example, an IPv4 socket can bind to `127.0.0.1`, while an IPv6 socket can bind to `::1`.

### `socket_send` and `socket_recv`

```cpp
auto socket_send(socket& s, std::span<const std::byte> data) noexcept -> xer::result<std::size_t>;
auto socket_recv(socket& s, std::span<std::byte> data) noexcept -> xer::result<std::size_t>;
```

`socket_send` and `socket_recv` perform a single send or receive operation.
They may transfer fewer bytes than requested, especially for stream sockets.
The returned size reports the number of bytes actually transferred.

### `socket_send_all`

```cpp
auto socket_send_all(socket& s, std::span<const std::byte> data) noexcept -> xer::result<void>;
```

Sends all bytes in `data` unless an error occurs.

This function repeatedly calls `socket_send` until the entire span has been sent.
It is intended for connected stream sockets such as TCP sockets.
An empty span succeeds without sending anything.

### `socket_recv_exact`

```cpp
auto socket_recv_exact(socket& s, std::span<std::byte> data) noexcept -> xer::result<void>;
```

Receives exactly `data.size()` bytes unless an error occurs or the peer closes the connection before enough bytes are received.

This function repeatedly calls `socket_recv` until the span has been filled.
It is intended for connected stream sockets such as TCP sockets.
An empty span succeeds without receiving anything.

### `socket_send_message`

```cpp
auto socket_send_message(socket& s, std::span<const std::byte> data) noexcept -> xer::result<void>;
```

Sends a length-prefixed message.

The function first sends a 4-byte unsigned big-endian payload length, followed by all bytes in `data`.
The payload size must fit in `std::uint32_t`; otherwise the function returns `error_t::length_error`.

An empty message is valid and sends only a zero length field.
The function uses `socket_send_all` internally, so the whole frame is sent unless an error occurs.

### `socket_recv_message`

```cpp
auto socket_recv_message(socket& s, std::size_t max_size) noexcept -> xer::result<std::vector<std::byte>>;
```

Receives one length-prefixed message sent in the same frame format used by `socket_send_message`.

The function first receives a 4-byte unsigned big-endian payload length.
If the payload length is greater than `max_size`, the function returns `error_t::length_error` without reading or discarding the payload.
In that case, callers should normally close the connection because the stream is no longer positioned at the next frame.

If the payload length is accepted, the function allocates a `std::vector<std::byte>` of that size, receives exactly that many payload bytes, and returns the vector.
An empty message is valid and returns an empty vector.

If the peer closes the connection before the complete length field or payload has been received, the function returns `error_t::network_error`.

---

## Length-Prefixed Message Example

A message sent by `socket_send_message` has the following frame format:

```text
uint32 big-endian payload_size
payload bytes
```

For example, a 5-byte payload `hello` is sent as a 4-byte length field followed by the five payload bytes.

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

The receiver can read the frame with `socket_recv_message`:

```cpp
auto body = xer::socket_recv_message(server, 1024 * 1024);
```

The second argument is the maximum accepted payload size.
It prevents an untrusted length field from causing an unbounded allocation.

---

## Example

The following example binds a TCP server socket to the loopback address and exchanges fixed-size messages.

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

## Stream Conversion

Sockets can be converted into xer streams:

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
- Host names and textual addresses are accepted as UTF-8 strings and converted to ordinary narrow strings for resolver APIs.
- `socket_send` and `socket_recv` may transfer only part of the supplied buffer.
- Use `socket_send_all` and `socket_recv_exact` when a fixed amount of data must be transferred over a connected stream socket.
- `socket_open` transfers ownership from the socket object into the resulting stream.
