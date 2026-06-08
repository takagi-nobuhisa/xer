# Policy for Socket Handling

## Overview

xer provides socket handling through `<xer/socket.h>`.

The purpose is to offer a small, practical networking layer that fits xer's error-handling and stream model without attempting to become a full networking framework.

---

## Basic Policy

- Sockets are represented by a move-only RAII `socket` type.
- TCP and UDP are supported in the initial implementation.
- IPv4 and IPv6 are represented explicitly through `socket_family`.
- Ordinary failures are returned as `xer::result`.
- Network failures that are not mapped more precisely may use `error_t::network_error`.
- The API remains close to ordinary socket operations instead of hiding them behind a large framework.

---

## Scope

The socket API covers:

- socket creation
- TCP connect, bind, listen, and accept
- binding either to all local interfaces or to an explicitly specified local address
- UDP send-to and receive-from
- socket address reporting
- reliable fixed-size send/receive helpers for connected stream sockets
- sending and receiving length-prefixed messages over connected stream sockets
- conversion of sockets to xer streams

Advanced networking features are deferred.

---

## Binding Policy

`socket_bind(socket&, port)` binds to all local interfaces for the socket family.

`socket_bind(socket&, address, port)` binds to the specified local address and port.
This overload is intended for cases where the listening interface matters, such as binding a helper server to `127.0.0.1` so that it is reachable only from the local machine.

The address must match the socket family.
For example, an IPv4 socket should use an IPv4 address such as `127.0.0.1`, and an IPv6 socket should use an IPv6 address such as `::1`.

---

## Stream Send/Receive Policy

`socket_send` and `socket_recv` expose ordinary single-operation socket behavior.
They may transfer fewer bytes than requested.

`socket_send_all` and `socket_recv_exact` are convenience helpers for connected stream sockets.
They repeatedly call the lower-level operations until the requested amount of data has been transferred, or until an error or premature peer close is detected.

These helpers are intended for simple application protocols that exchange fixed-size fields or length-prefixed messages.
They do not define a message format by themselves.

---

## Message Framing Policy

`socket_send_message` sends one length-prefixed binary message.

The frame format is deliberately small and language-neutral:

```text
uint32 big-endian payload_size
payload bytes
```

The length field is exactly four bytes.
The payload length is limited to the range of `std::uint32_t`.
If the input span is larger than that, `socket_send_message` returns `error_t::length_error` without sending a partial frame.

`socket_recv_message` receives the same frame format.
It requires a caller-supplied maximum payload size and returns `error_t::length_error` if the frame length exceeds that limit.
The oversized payload is not read or discarded; callers should normally close the connection after this error.

An empty payload is valid.
The receive helper returns an empty `std::vector<std::byte>` for an empty message.

---

## Stream Integration

A connected socket may be converted to:

- `binary_stream` for byte-oriented protocols
- `text_stream` for text-oriented protocols using an explicit encoding

This allows higher-level I/O functions from `<xer/stdio.h>` to be reused where appropriate.

---

## Deferred Items

At least the following are deferred:

- nonblocking mode
- readiness polling or event loops
- TLS
- detailed socket options
- asynchronous I/O
- name service policy beyond basic resolver use
- a built-in HTTP server

---

## Summary

- `<xer/socket.h>` provides a small socket layer
- it uses RAII and `xer::result`
- it supports TCP, UDP, IPv4, and IPv6
- it supports both wildcard bind and explicit-address bind
- it provides helpers for complete fixed-size send/receive operations
- it provides helpers for sending and receiving length-prefixed messages
- it integrates with xer streams
- it does not attempt to become a complete networking framework
