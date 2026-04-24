// XER_EXAMPLE_BEGIN: socket_tcp_binary_stream
//
// This example creates a localhost TCP connection and opens both endpoints
// as binary streams.
//
// Expected output:
// server received: ping

#include <array>
#include <cstddef>
#include <expected>
#include <span>
#include <utility>

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

    auto bound = xer::socket_bind(*listener, 0);
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

    auto client_stream = xer::socket_open(std::move(pair->client));
    if (!client_stream) {
        return 1;
    }

    auto server_stream = xer::socket_open(std::move(pair->server));
    if (!server_stream) {
        return 1;
    }

    constexpr std::array<std::byte, 4> message = {
        std::byte {'p'},
        std::byte {'i'},
        std::byte {'n'},
        std::byte {'g'},
    };

    for (std::byte value : message) {
        auto written = xer::fputb(value, *client_stream);
        if (!written) {
            return 1;
        }
    }

    std::array<std::byte, 4> buffer {};
    for (std::byte& value : buffer) {
        auto read = xer::fgetb(*server_stream);
        if (!read) {
            return 1;
        }
        value = *read;
    }

    if (!byte_matches(buffer[0], 'p') || !byte_matches(buffer[1], 'i') || !byte_matches(buffer[2], 'n') ||
        !byte_matches(buffer[3], 'g')) {
        return 1;
    }

    if (!xer::puts(u8"server received: ping")) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: socket_tcp_binary_stream
