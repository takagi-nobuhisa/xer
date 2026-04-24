// XER_EXAMPLE_BEGIN: socket_create
//
// This example creates and closes a TCP socket.
//
// Expected output:
// socket created

#include <xer/socket.h>
#include <xer/stdio.h>

auto main() -> int
{
    auto s = xer::socket_create(xer::socket_family::ipv4, xer::socket_type::tcp);
    if (!s.has_value()) {
        return 1;
    }

    if (!xer::puts(u8"socket created").has_value()) {
        return 1;
    }

    if (!xer::socket_close(*s).has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: socket_create
