// XER_EXAMPLE_BEGIN: socket_tcp_exact
//
// This example creates a localhost TCP connection, binds the listener to
// 127.0.0.1 explicitly, and exchanges fixed-size binary messages with
// socket_send_all and socket_recv_exact.
//
// Expected output:
// server received: ping
// client received: pong

#include <array>
#include <cstddef>
#include <span>

#include <xer/socket.h>
#include <xer/stdio.h>

namespace {

struct tcp_socket_pair {
    xer::socket client;
    xer::socket server;
};

[[nodiscard]] auto create_tcp_socket_pair() -> xer::result<tcp_socket_pair>
{
    auto listener = xer::socket_create(xer::socket_family::ipv4, xer::socket_type::tcp);
    if (!listener) {
        return std::unexpected(listener.error());
    }

    auto bound = xer::socket_bind(*listener, u8"127.0.0.1", 0);
    if (!bound) {
        return std::unexpected(bound.error());
    }

    auto local = xer::socket_getsockname(*listener);
    if (!local) {
        return std::unexpected(local.error());
    }

    auto listening = xer::socket_listen(*listener);
    if (!listening) {
        return std::unexpected(listening.error());
    }

    auto client = xer::socket_create(xer::socket_family::ipv4, xer::socket_type::tcp);
    if (!client) {
        return std::unexpected(client.error());
    }

    auto connected = xer::socket_connect(*client, u8"127.0.0.1", local->port);
    if (!connected) {
        return std::unexpected(connected.error());
    }

    auto server = xer::socket_accept(*listener);
    if (!server) {
        return std::unexpected(server.error());
    }

    return tcp_socket_pair {std::move(*client), std::move(*server)};
}

[[nodiscard]] auto byte_matches(std::byte value, char expected) noexcept -> bool
{
    return std::to_integer<unsigned char>(value) == static_cast<unsigned char>(expected);
}

} // namespace

auto main() -> int
{
    auto pair = create_tcp_socket_pair();
    if (!pair) {
        return 1;
    }

    constexpr std::array<std::byte, 4> ping = {
        std::byte {'p'},
        std::byte {'i'},
        std::byte {'n'},
        std::byte {'g'},
    };

    auto sent_ping = xer::socket_send_all(pair->client, std::span<const std::byte>(ping));
    if (!sent_ping) {
        return 1;
    }

    std::array<std::byte, 4> server_buffer {};
    auto received_ping = xer::socket_recv_exact(pair->server, server_buffer);
    if (!received_ping) {
        return 1;
    }

    if (!byte_matches(server_buffer[0], 'p') || !byte_matches(server_buffer[1], 'i') ||
        !byte_matches(server_buffer[2], 'n') || !byte_matches(server_buffer[3], 'g')) {
        return 1;
    }

    if (!xer::puts(u8"server received: ping")) {
        return 1;
    }

    constexpr std::array<std::byte, 4> pong = {
        std::byte {'p'},
        std::byte {'o'},
        std::byte {'n'},
        std::byte {'g'},
    };

    auto sent_pong = xer::socket_send_all(pair->server, std::span<const std::byte>(pong));
    if (!sent_pong) {
        return 1;
    }

    std::array<std::byte, 4> client_buffer {};
    auto received_pong = xer::socket_recv_exact(pair->client, client_buffer);
    if (!received_pong) {
        return 1;
    }

    if (!byte_matches(client_buffer[0], 'p') || !byte_matches(client_buffer[1], 'o') ||
        !byte_matches(client_buffer[2], 'n') || !byte_matches(client_buffer[3], 'g')) {
        return 1;
    }

    if (!xer::puts(u8"client received: pong")) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: socket_tcp_exact
