// XER_EXAMPLE_BEGIN: socket_tcp_message
//
// This example creates a localhost TCP connection and exchanges a
// length-prefixed message with socket_send_message and socket_recv_message.
//
// Expected output:
// server received: hello

#include <array>
#include <cstddef>
#include <span>
#include <string_view>

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

[[nodiscard]] auto byte_matches(std::span<const std::byte> data, std::string_view expected) noexcept -> bool
{
    if (data.size() != expected.size()) {
        return false;
    }

    for (std::size_t i = 0; i < expected.size(); ++i) {
        if (std::to_integer<unsigned char>(data[i]) != static_cast<unsigned char>(expected[i])) {
            return false;
        }
    }

    return true;
}

} // namespace

auto main() -> int
{
    auto pair = create_tcp_socket_pair();
    if (!pair) {
        return 1;
    }

    constexpr std::array<std::byte, 5> hello = {
        std::byte {'h'},
        std::byte {'e'},
        std::byte {'l'},
        std::byte {'l'},
        std::byte {'o'},
    };

    auto sent = xer::socket_send_message(pair->client, hello);
    if (!sent) {
        return 1;
    }

    auto received = xer::socket_recv_message(pair->server, 1024);
    if (!received) {
        return 1;
    }

    if (!byte_matches(*received, "hello")) {
        return 1;
    }

    if (!xer::puts(u8"server received: hello")) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: socket_tcp_message
