// XER_EXAMPLE_BEGIN: socket_tcp_text_stream
//
// This example creates a localhost TCP connection and opens both endpoints
// as UTF-8 text streams.
//
// Expected output:
// server received: hello from client

#include <expected>
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

} // namespace

auto main() -> int
{
    auto pair = create_tcp_socket_pair();
    if (!pair) {
        return 1;
    }

    auto client_stream = xer::socket_open(std::move(pair->client), xer::encoding_t::utf8);
    if (!client_stream) {
        return 1;
    }

    auto server_stream = xer::socket_open(std::move(pair->server), xer::encoding_t::utf8);
    if (!server_stream) {
        return 1;
    }

    if (!xer::fputs(u8"hello from client\n", *client_stream)) {
        return 1;
    }

    auto line = xer::fgets(*server_stream, false);
    if (!line) {
        return 1;
    }

    if (!xer::fputs(u8"server received: ", xer_stdout)) {
        return 1;
    }

    if (!xer::puts(*line)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: socket_tcp_text_stream
