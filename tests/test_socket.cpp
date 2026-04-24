/**
 * @file tests/test_socket.cpp
 * @brief Tests for socket creation and localhost communication.
 */

#include <array>
#include <cstddef>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <utility>

#include <xer/assert.h>
#include <xer/socket.h>
#include <xer/stdio.h>

namespace {

struct tcp_socket_pair {
    xer::socket client{};
    xer::socket server{};
};

[[nodiscard]] auto byte_array(std::string_view text) -> std::array<std::byte, 16>
{
    std::array<std::byte, 16> result{};
    for (std::size_t i = 0; i < text.size(); ++i) {
        result[i] = static_cast<std::byte>(text[i]);
    }
    return result;
}

[[nodiscard]] auto matches_text(std::span<const std::byte> data, std::string_view text) -> bool
{
    if (data.size() != text.size()) {
        return false;
    }

    for (std::size_t i = 0; i < text.size(); ++i) {
        if (std::to_integer<unsigned char>(data[i]) != static_cast<unsigned char>(text[i])) {
            return false;
        }
    }

    return true;
}

[[nodiscard]] auto create_tcp_socket_pair() -> xer::result<tcp_socket_pair>
{
    auto listener = xer::socket_create(xer::socket_family::ipv4, xer::socket_type::tcp);
    if (!listener.has_value()) {
        return std::unexpected(listener.error());
    }

    auto bound = xer::socket_bind(*listener, 0);
    if (!bound.has_value()) {
        return std::unexpected(bound.error());
    }

    auto local = xer::socket_getsockname(*listener);
    if (!local.has_value()) {
        return std::unexpected(local.error());
    }

    auto listening = xer::socket_listen(*listener);
    if (!listening.has_value()) {
        return std::unexpected(listening.error());
    }

    auto client = xer::socket_create(xer::socket_family::ipv4, xer::socket_type::tcp);
    if (!client.has_value()) {
        return std::unexpected(client.error());
    }

    auto connected = xer::socket_connect(*client, u8"127.0.0.1", local->port);
    if (!connected.has_value()) {
        return std::unexpected(connected.error());
    }

    auto server = xer::socket_accept(*listener);
    if (!server.has_value()) {
        return std::unexpected(server.error());
    }

    return tcp_socket_pair{std::move(*client), std::move(*server)};
}

void test_tcp_socket_create_and_close()
{
    auto s = xer::socket_create(xer::socket_family::ipv4, xer::socket_type::tcp);
    xer_assert(s.has_value());
    xer_assert(s->is_open());
    xer_assert_eq(s->family(), xer::socket_family::ipv4);
    xer_assert_eq(s->type(), xer::socket_type::tcp);

    auto closed = xer::socket_close(*s);
    xer_assert(closed.has_value());
    xer_assert_not(s->is_open());
}

void test_udp_socket_create_and_close()
{
    auto s = xer::socket_create(xer::socket_family::ipv4, xer::socket_type::udp);
    xer_assert(s.has_value());
    xer_assert(s->is_open());
    xer_assert_eq(s->family(), xer::socket_family::ipv4);
    xer_assert_eq(s->type(), xer::socket_type::udp);

    auto closed = xer::socket_close(*s);
    xer_assert(closed.has_value());
    xer_assert_not(s->is_open());
}

void test_socket_getsockname_after_bind_zero()
{
    auto s = xer::socket_create(xer::socket_family::ipv4, xer::socket_type::tcp);
    xer_assert(s.has_value());

    auto bound = xer::socket_bind(*s, 0);
    xer_assert(bound.has_value());

    auto local = xer::socket_getsockname(*s);
    xer_assert(local.has_value());
    xer_assert(local->port != 0);
}

void test_tcp_localhost_send_recv()
{
    auto pair = create_tcp_socket_pair();
    xer_assert(pair.has_value());

    auto ping = byte_array("ping");
    auto sent = xer::socket_send(pair->client, std::span<const std::byte>(ping.data(), 4));
    xer_assert(sent.has_value());
    xer_assert_eq(*sent, static_cast<std::size_t>(4));

    std::array<std::byte, 4> buffer{};
    auto received = xer::socket_recv(pair->server, buffer);
    xer_assert(received.has_value());
    xer_assert_eq(*received, static_cast<std::size_t>(4));
    xer_assert(matches_text(buffer, "ping"));

    auto pong = byte_array("pong");
    sent = xer::socket_send(pair->server, std::span<const std::byte>(pong.data(), 4));
    xer_assert(sent.has_value());
    xer_assert_eq(*sent, static_cast<std::size_t>(4));

    received = xer::socket_recv(pair->client, buffer);
    xer_assert(received.has_value());
    xer_assert_eq(*received, static_cast<std::size_t>(4));
    xer_assert(matches_text(buffer, "pong"));
}

void test_tcp_binary_stream_localhost()
{
    auto pair = create_tcp_socket_pair();
    xer_assert(pair.has_value());

    auto client_stream = xer::socket_open(std::move(pair->client));
    xer_assert(client_stream.has_value());

    auto server_stream = xer::socket_open(std::move(pair->server));
    xer_assert(server_stream.has_value());

    auto ping = byte_array("ping");
    auto written = xer::fwrite(std::span<const std::byte>(ping.data(), 4), *client_stream);
    xer_assert(written.has_value());
    xer_assert_eq(*written, static_cast<std::size_t>(4));

    std::array<std::byte, 4> buffer{};
    auto read = xer::fread(buffer, *server_stream);
    xer_assert(read.has_value());
    xer_assert_eq(*read, static_cast<std::size_t>(4));
    xer_assert(matches_text(buffer, "ping"));
}

void test_tcp_text_stream_localhost()
{
    auto pair = create_tcp_socket_pair();
    xer_assert(pair.has_value());

    auto client_stream = xer::socket_open(std::move(pair->client), xer::encoding_t::utf8);
    xer_assert(client_stream.has_value());

    auto server_stream = xer::socket_open(std::move(pair->server), xer::encoding_t::utf8);
    xer_assert(server_stream.has_value());

    auto written = xer::fputs(u8"hello\n", *client_stream);
    xer_assert(written.has_value());
    xer_assert_eq(*written, static_cast<std::size_t>(6));

    auto line = xer::fgets(*server_stream);
    xer_assert(line.has_value());
    xer_assert(*line == std::u8string(u8"hello\n"));
}

void test_udp_localhost_sendto_recvfrom()
{
    auto receiver = xer::socket_create(xer::socket_family::ipv4, xer::socket_type::udp);
    xer_assert(receiver.has_value());

    auto bound = xer::socket_bind(*receiver, 0);
    xer_assert(bound.has_value());

    auto local = xer::socket_getsockname(*receiver);
    xer_assert(local.has_value());
    xer_assert(local->port != 0);

    auto sender = xer::socket_create(xer::socket_family::ipv4, xer::socket_type::udp);
    xer_assert(sender.has_value());

    auto ping = byte_array("ping");
    auto sent = xer::socket_sendto(*sender, std::span<const std::byte>(ping.data(), 4), u8"127.0.0.1", local->port);
    xer_assert(sent.has_value());
    xer_assert_eq(*sent, static_cast<std::size_t>(4));

    std::array<std::byte, 16> buffer{};
    auto received = xer::socket_recvfrom(*receiver, buffer);
    xer_assert(received.has_value());
    xer_assert_eq(received->size, static_cast<std::size_t>(4));
    xer_assert(matches_text(std::span<const std::byte>(buffer.data(), received->size), "ping"));
    xer_assert(received->address.port != 0);
}

void test_udp_socket_is_not_stream_opened()
{
    auto s = xer::socket_create(xer::socket_family::ipv4, xer::socket_type::udp);
    xer_assert(s.has_value());

    auto stream = xer::socket_open(std::move(*s));
    xer_assert_not(stream.has_value());
    xer_assert_eq(stream.error().code, xer::error_t::invalid_argument);
}

void test_empty_socket_operations_fail()
{
    xer::socket s;

    std::array<std::byte, 4> buffer{};
    auto received = xer::socket_recv(s, buffer);
    xer_assert_not(received.has_value());
    xer_assert_eq(received.error().code, xer::error_t::network_error);

    auto sent = xer::socket_send(s, std::span<const std::byte>(buffer.data(), buffer.size()));
    xer_assert_not(sent.has_value());
    xer_assert_eq(sent.error().code, xer::error_t::network_error);
}

} // namespace

auto main() -> int
{
    test_tcp_socket_create_and_close();
    test_udp_socket_create_and_close();
    test_socket_getsockname_after_bind_zero();
    test_tcp_localhost_send_recv();
    test_tcp_binary_stream_localhost();
    test_tcp_text_stream_localhost();
    test_udp_localhost_sendto_recvfrom();
    test_udp_socket_is_not_stream_opened();
    test_empty_socket_operations_fail();

    return 0;
}
