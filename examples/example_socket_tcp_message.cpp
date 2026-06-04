// XER_EXAMPLE_BEGIN: socket_tcp_message
//
// This example creates a localhost TCP connection and sends a length-prefixed
// message with socket_send_message. The receiver reads the 4-byte big-endian
// length field and then reads the message body with socket_recv_exact.
//
// Expected output:
// server received: hello

#include <array>
#include <cstddef>
#include <cstdint>
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

[[nodiscard]] auto decode_message_size(std::span<const std::byte, 4> header) noexcept -> std::uint32_t
{
    return (static_cast<std::uint32_t>(std::to_integer<unsigned char>(header[0])) << 24) |
           (static_cast<std::uint32_t>(std::to_integer<unsigned char>(header[1])) << 16) |
           (static_cast<std::uint32_t>(std::to_integer<unsigned char>(header[2])) << 8) |
           static_cast<std::uint32_t>(std::to_integer<unsigned char>(header[3]));
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

    std::array<std::byte, 4> header {};
    auto received_header = xer::socket_recv_exact(pair->server, header);
    if (!received_header) {
        return 1;
    }

    if (decode_message_size(std::span<const std::byte, 4>(header)) != hello.size()) {
        return 1;
    }

    std::array<std::byte, 5> body {};
    auto received_body = xer::socket_recv_exact(pair->server, body);
    if (!received_body) {
        return 1;
    }

    if (!byte_matches(body[0], 'h') || !byte_matches(body[1], 'e') || !byte_matches(body[2], 'l') ||
        !byte_matches(body[3], 'l') || !byte_matches(body[4], 'o')) {
        return 1;
    }

    if (!xer::puts(u8"server received: hello")) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: socket_tcp_message
