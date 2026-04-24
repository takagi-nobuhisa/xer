// XER_EXAMPLE_BEGIN: socket_udp
//
// This example sends one UDP datagram to a localhost receiver.
//
// Expected output:
// UDP received: ping

#include <array>
#include <cstddef>
#include <span>

#include <xer/socket.h>
#include <xer/stdio.h>

namespace {

[[nodiscard]] auto byte_matches(std::byte value, char expected) noexcept -> bool
{
    return std::to_integer<unsigned char>(value) == static_cast<unsigned char>(expected);
}

} // namespace

auto main() -> int
{
    auto receiver = xer::socket_create(xer::socket_family::ipv4, xer::socket_type::udp);
    if (!receiver) {
        return 1;
    }

    if (!xer::socket_bind(*receiver, 0)) {
        return 1;
    }

    auto local = xer::socket_getsockname(*receiver);
    if (!local) {
        return 1;
    }

    auto sender = xer::socket_create(xer::socket_family::ipv4, xer::socket_type::udp);
    if (!sender) {
        return 1;
    }

    constexpr std::array<std::byte, 4> message = {
        std::byte {'p'},
        std::byte {'i'},
        std::byte {'n'},
        std::byte {'g'},
    };

    auto sent = xer::socket_sendto(*sender, std::span<const std::byte>(message), u8"127.0.0.1", local->port);
    if (!sent || *sent != message.size()) {
        return 1;
    }

    std::array<std::byte, 16> buffer {};
    auto received = xer::socket_recvfrom(*receiver, buffer);
    if (!received || received->size != message.size()) {
        return 1;
    }

    if (!byte_matches(buffer[0], 'p') || !byte_matches(buffer[1], 'i') || !byte_matches(buffer[2], 'n') ||
        !byte_matches(buffer[3], 'g')) {
        return 1;
    }

    if (!xer::puts(u8"UDP received: ping")) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: socket_udp
