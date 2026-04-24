# Policy for Socket Handling

## Overview

XER provides socket handling through `<xer/socket.h>`.

The purpose is to offer a small, practical networking layer that fits XER's error-handling and stream model without attempting to become a full networking framework.

---

## Basic Policy

- Sockets are represented by a move-only RAII `socket` type.
- TCP and UDP are supported in the initial implementation.
- IPv4 and IPv6 are represented explicitly through `socket_family`.
- Ordinary failures are returned as `xer::result`.
- Network failures that are not mapped more precisely may use `error_t::network_error`.

---

## Scope

The initial socket API covers:

- socket creation
- TCP connect, bind, listen, and accept
- UDP send-to and receive-from
- socket address reporting
- conversion of sockets to XER streams

Advanced networking features are deferred.

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

---

## Summary

- `<xer/socket.h>` provides a small socket layer
- it uses RAII and `xer::result`
- it supports TCP, UDP, IPv4, and IPv6
- it integrates with XER streams
- it does not attempt to become a complete networking framework in the initial stage
