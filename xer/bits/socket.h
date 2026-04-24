/**
 * @file xer/bits/socket.h
 * @brief Socket facilities for TCP and UDP communication.
 */

#pragma once

#ifndef XER_BITS_SOCKET_H_INCLUDED_
#define XER_BITS_SOCKET_H_INCLUDED_

#include <climits>
#include <cstdint>
#include <expected>
#include <new>
#include <span>
#include <string>
#include <string_view>
#include <utility>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <xer/bits/advanced_encoding.h>
#include <xer/bits/binary_stream.h>
#include <xer/bits/fopen.h>
#include <xer/bits/text_stream.h>
#include <xer/error.h>

namespace xer {

enum class socket_family {
    ipv4,
    ipv6
};

enum class socket_type {
    tcp,
    udp
};

struct socket_address {
    std::u8string address{};
    std::uint16_t port = 0;
};

struct socket_recvfrom_result {
    std::size_t size = 0;
    socket_address address{};
};

namespace detail {

#ifdef _WIN32
using native_socket_t = SOCKET;
inline constexpr native_socket_t invalid_socket_value = INVALID_SOCKET;
#else
using native_socket_t = int;
inline constexpr native_socket_t invalid_socket_value = -1;
#endif

[[nodiscard]] inline auto ensure_socket_runtime() noexcept -> bool {
#ifdef _WIN32
    static bool initialized = []() noexcept -> bool {
        WSADATA data{};
        return WSAStartup(MAKEWORD(2, 2), &data) == 0;
    }();
    return initialized;
#else
    return true;
#endif
}

inline auto close_native_socket(native_socket_t handle) noexcept -> int {
    if (handle == invalid_socket_value) {
        return 0;
    }
#ifdef _WIN32
    return closesocket(handle) == 0 ? 0 : -1;
#else
    return ::close(handle) == 0 ? 0 : -1;
#endif
}

[[nodiscard]] constexpr auto to_native_family(socket_family family) noexcept -> int {
    switch (family) {
    case socket_family::ipv4:
        return AF_INET;
    case socket_family::ipv6:
        return AF_INET6;
    default:
        return -1;
    }
}

[[nodiscard]] constexpr auto to_native_socket_type(socket_type type) noexcept -> int {
    switch (type) {
    case socket_type::tcp:
        return SOCK_STREAM;
    case socket_type::udp:
        return SOCK_DGRAM;
    default:
        return -1;
    }
}

[[nodiscard]] constexpr auto to_native_protocol(socket_type type) noexcept -> int {
    switch (type) {
    case socket_type::tcp:
        return IPPROTO_TCP;
    case socket_type::udp:
        return IPPROTO_UDP;
    default:
        return -1;
    }
}

[[nodiscard]] inline auto to_host_string(std::u8string_view host) -> std::string {
    std::string result;
    result.reserve(host.size());
    for (char8_t ch : host) {
        result.push_back(static_cast<char>(ch));
    }
    return result;
}

[[nodiscard]] inline auto resolve_socket_address(
    std::u8string_view host,
    std::uint16_t port,
    socket_family family,
    socket_type type) noexcept -> result<addrinfo*> {
    const int native_family = to_native_family(family);
    const int native_type = to_native_socket_type(type);
    const int native_protocol = to_native_protocol(type);
    if (native_family < 0 || native_type < 0 || native_protocol < 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const std::string host_text = to_host_string(host);
    const std::string service = std::to_string(static_cast<unsigned int>(port));

    addrinfo hints{};
    hints.ai_family = native_family;
    hints.ai_socktype = native_type;
    hints.ai_protocol = native_protocol;

    addrinfo* info = nullptr;
    if (getaddrinfo(host_text.c_str(), service.c_str(), &hints, &info) != 0 || info == nullptr) {
        return std::unexpected(make_error(error_t::network_error));
    }

    return info;
}

class addrinfo_holder {
public:
    explicit addrinfo_holder(addrinfo* value) noexcept : value_(value) {}
    addrinfo_holder(const addrinfo_holder&) = delete;
    auto operator=(const addrinfo_holder&) -> addrinfo_holder& = delete;
    addrinfo_holder(addrinfo_holder&& other) noexcept : value_(other.value_) { other.value_ = nullptr; }
    auto operator=(addrinfo_holder&& other) noexcept -> addrinfo_holder& {
        if (this != &other) {
            reset();
            value_ = other.value_;
            other.value_ = nullptr;
        }
        return *this;
    }
    ~addrinfo_holder() { reset(); }
    [[nodiscard]] auto get() const noexcept -> addrinfo* { return value_; }

private:
    auto reset() noexcept -> void {
        if (value_ != nullptr) {
            freeaddrinfo(value_);
            value_ = nullptr;
        }
    }
    addrinfo* value_ = nullptr;
};

[[nodiscard]] inline auto sockaddr_to_socket_address(
    const sockaddr* addr,
    socklen_t addr_len,
    socket_address& out) noexcept -> bool {
    char host[NI_MAXHOST] = {};
    char service[NI_MAXSERV] = {};
    if (getnameinfo(
            addr,
            addr_len,
            host,
            static_cast<socklen_t>(sizeof(host)),
            service,
            static_cast<socklen_t>(sizeof(service)),
            NI_NUMERICHOST | NI_NUMERICSERV) != 0) {
        return false;
    }

    out.address.clear();
    for (const char* p = host; *p != '\0'; ++p) {
        out.address.push_back(static_cast<char8_t>(*p));
    }

    unsigned long port_value = 0;
    for (const char* p = service; *p != '\0'; ++p) {
        if (*p < '0' || *p > '9') {
            return false;
        }
        port_value = port_value * 10UL + static_cast<unsigned long>(*p - '0');
        if (port_value > 65535UL) {
            return false;
        }
    }
    out.port = static_cast<std::uint16_t>(port_value);
    return true;
}

} // namespace detail

class socket {
public:
    constexpr socket() noexcept = default;
    constexpr socket(detail::native_socket_t handle, socket_family family, socket_type type) noexcept
        : handle_(handle), family_(family), type_(type) {}
    socket(const socket&) = delete;
    auto operator=(const socket&) -> socket& = delete;
    constexpr socket(socket&& other) noexcept
        : handle_(other.handle_), family_(other.family_), type_(other.type_) {
        other.handle_ = detail::invalid_socket_value;
    }
    auto operator=(socket&& other) noexcept -> socket& {
        if (this != &other) {
            (void)close();
            handle_ = other.handle_;
            family_ = other.family_;
            type_ = other.type_;
            other.handle_ = detail::invalid_socket_value;
        }
        return *this;
    }
    ~socket() { (void)close(); }
    [[nodiscard]] constexpr auto is_open() const noexcept -> bool { return handle_ != detail::invalid_socket_value; }
    [[nodiscard]] constexpr auto family() const noexcept -> socket_family { return family_; }
    [[nodiscard]] constexpr auto type() const noexcept -> socket_type { return type_; }
    auto close() noexcept -> int {
        const detail::native_socket_t old = handle_;
        handle_ = detail::invalid_socket_value;
        return detail::close_native_socket(old);
    }
    auto release() noexcept -> detail::native_socket_t {
        const detail::native_socket_t old = handle_;
        handle_ = detail::invalid_socket_value;
        return old;
    }
    [[nodiscard]] constexpr auto native_handle() const noexcept -> detail::native_socket_t { return handle_; }

private:
    detail::native_socket_t handle_ = detail::invalid_socket_value;
    socket_family family_ = socket_family::ipv4;
    socket_type type_ = socket_type::tcp;
};

[[nodiscard]] inline auto socket_create(socket_family family, socket_type type) noexcept -> result<socket> {
    if (!detail::ensure_socket_runtime()) {
        return std::unexpected(make_error(error_t::network_error));
    }
    const int native_family = detail::to_native_family(family);
    const int native_type = detail::to_native_socket_type(type);
    const int native_protocol = detail::to_native_protocol(type);
    if (native_family < 0 || native_type < 0 || native_protocol < 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }
    const detail::native_socket_t handle = ::socket(native_family, native_type, native_protocol);
    if (handle == detail::invalid_socket_value) {
        return std::unexpected(make_error(error_t::network_error));
    }
    return socket(handle, family, type);
}

[[nodiscard]] inline auto socket_close(socket& s) noexcept -> result<void> {
    if (s.close() < 0) {
        return std::unexpected(make_error(error_t::network_error));
    }
    return {};
}

[[nodiscard]] inline auto socket_connect(socket& s, std::u8string_view host, std::uint16_t port) noexcept -> result<void> {
    if (!s.is_open()) {
        return std::unexpected(make_error(error_t::network_error));
    }
    auto info_result = detail::resolve_socket_address(host, port, s.family(), s.type());
    if (!info_result.has_value()) {
        return std::unexpected(info_result.error());
    }
    detail::addrinfo_holder info(*info_result);
    for (addrinfo* p = info.get(); p != nullptr; p = p->ai_next) {
        if (::connect(s.native_handle(), p->ai_addr, static_cast<socklen_t>(p->ai_addrlen)) == 0) {
            return {};
        }
    }
    return std::unexpected(make_error(error_t::network_error));
}

[[nodiscard]] inline auto socket_bind(socket& s, std::uint16_t port) noexcept -> result<void> {
    if (!s.is_open()) {
        return std::unexpected(make_error(error_t::network_error));
    }
    if (s.family() == socket_family::ipv4) {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(port);
        if (::bind(s.native_handle(), reinterpret_cast<const sockaddr*>(&addr), static_cast<socklen_t>(sizeof(addr))) == 0) {
            return {};
        }
    } else if (s.family() == socket_family::ipv6) {
        sockaddr_in6 addr{};
        addr.sin6_family = AF_INET6;
        addr.sin6_addr = in6addr_any;
        addr.sin6_port = htons(port);
        if (::bind(s.native_handle(), reinterpret_cast<const sockaddr*>(&addr), static_cast<socklen_t>(sizeof(addr))) == 0) {
            return {};
        }
    } else {
        return std::unexpected(make_error(error_t::invalid_argument));
    }
    return std::unexpected(make_error(error_t::network_error));
}

[[nodiscard]] inline auto socket_getsockname(socket& s) noexcept -> result<socket_address> {
    if (!s.is_open()) {
        return std::unexpected(make_error(error_t::network_error));
    }

    sockaddr_storage addr{};
    socklen_t addr_len = static_cast<socklen_t>(sizeof(addr));
    if (::getsockname(s.native_handle(), reinterpret_cast<sockaddr*>(&addr), &addr_len) != 0) {
        return std::unexpected(make_error(error_t::network_error));
    }

    socket_address result{};
    if (!detail::sockaddr_to_socket_address(reinterpret_cast<const sockaddr*>(&addr), addr_len, result)) {
        return std::unexpected(make_error(error_t::network_error));
    }

    return result;
}

[[nodiscard]] inline auto socket_listen(socket& s, int backlog = 16) noexcept -> result<void> {
    if (!s.is_open() || s.type() != socket_type::tcp || backlog < 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }
    if (::listen(s.native_handle(), backlog) != 0) {
        return std::unexpected(make_error(error_t::network_error));
    }
    return {};
}

[[nodiscard]] inline auto socket_accept(socket& s) noexcept -> result<socket> {
    if (!s.is_open() || s.type() != socket_type::tcp) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }
    const detail::native_socket_t handle = ::accept(s.native_handle(), nullptr, nullptr);
    if (handle == detail::invalid_socket_value) {
        return std::unexpected(make_error(error_t::network_error));
    }
    return socket(handle, s.family(), socket_type::tcp);
}

[[nodiscard]] inline auto socket_send(socket& s, std::span<const std::byte> data) noexcept -> result<std::size_t> {
    if (!s.is_open()) {
        return std::unexpected(make_error(error_t::network_error));
    }
    if (data.empty()) {
        return static_cast<std::size_t>(0);
    }
    const int size = data.size() > static_cast<std::size_t>(INT_MAX) ? INT_MAX : static_cast<int>(data.size());
    const int result = ::send(s.native_handle(), reinterpret_cast<const char*>(data.data()), size, 0);
    if (result < 0) {
        return std::unexpected(make_error(error_t::network_error));
    }
    return static_cast<std::size_t>(result);
}

[[nodiscard]] inline auto socket_recv(socket& s, std::span<std::byte> data) noexcept -> result<std::size_t> {
    if (!s.is_open()) {
        return std::unexpected(make_error(error_t::network_error));
    }
    if (data.empty()) {
        return static_cast<std::size_t>(0);
    }
    const int size = data.size() > static_cast<std::size_t>(INT_MAX) ? INT_MAX : static_cast<int>(data.size());
    const int result = ::recv(s.native_handle(), reinterpret_cast<char*>(data.data()), size, 0);
    if (result < 0) {
        return std::unexpected(make_error(error_t::network_error));
    }
    return static_cast<std::size_t>(result);
}

[[nodiscard]] inline auto socket_sendto(
    socket& s,
    std::span<const std::byte> data,
    std::u8string_view host,
    std::uint16_t port) noexcept -> result<std::size_t> {
    if (!s.is_open() || s.type() != socket_type::udp) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }
    auto info_result = detail::resolve_socket_address(host, port, s.family(), socket_type::udp);
    if (!info_result.has_value()) {
        return std::unexpected(info_result.error());
    }
    detail::addrinfo_holder info(*info_result);
    for (addrinfo* p = info.get(); p != nullptr; p = p->ai_next) {
        const int size = data.size() > static_cast<std::size_t>(INT_MAX) ? INT_MAX : static_cast<int>(data.size());
        const int result = ::sendto(
            s.native_handle(),
            reinterpret_cast<const char*>(data.data()),
            size,
            0,
            p->ai_addr,
            static_cast<socklen_t>(p->ai_addrlen));
        if (result >= 0) {
            return static_cast<std::size_t>(result);
        }
    }
    return std::unexpected(make_error(error_t::network_error));
}

[[nodiscard]] inline auto socket_recvfrom(socket& s, std::span<std::byte> data) noexcept -> result<socket_recvfrom_result> {
    if (!s.is_open() || s.type() != socket_type::udp) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }
    sockaddr_storage addr{};
    socklen_t addr_len = static_cast<socklen_t>(sizeof(addr));
    const int size = data.size() > static_cast<std::size_t>(INT_MAX) ? INT_MAX : static_cast<int>(data.size());
    const int result = ::recvfrom(
        s.native_handle(),
        reinterpret_cast<char*>(data.data()),
        size,
        0,
        reinterpret_cast<sockaddr*>(&addr),
        &addr_len);
    if (result < 0) {
        return std::unexpected(make_error(error_t::network_error));
    }
    socket_recvfrom_result output;
    output.size = static_cast<std::size_t>(result);
    if (!detail::sockaddr_to_socket_address(reinterpret_cast<const sockaddr*>(&addr), addr_len, output.address)) {
        return std::unexpected(make_error(error_t::network_error));
    }
    return output;
}

namespace detail {

struct socket_binary_stream_state {
    socket s{};
};

inline auto socket_binary_close(binary_stream_handle_t handle) noexcept -> int {
    auto* const state = reinterpret_cast<socket_binary_stream_state*>(handle);
    if (state == nullptr) {
        return -1;
    }
    const int result = state->s.close();
    delete state;
    return result;
}

inline auto socket_binary_read(binary_stream_handle_t handle, void* s, int n) noexcept -> int {
    if (s == nullptr || n < 0) {
        return -1;
    }
    auto* const state = reinterpret_cast<socket_binary_stream_state*>(handle);
    if (state == nullptr || !state->s.is_open()) {
        return -1;
    }
    if (n == 0) {
        return 0;
    }
    const int result = ::recv(state->s.native_handle(), reinterpret_cast<char*>(s), n, 0);
    return result < 0 ? -1 : result;
}

inline auto socket_binary_write(binary_stream_handle_t handle, const void* s, int n) noexcept -> int {
    if (s == nullptr || n < 0) {
        return -1;
    }
    auto* const state = reinterpret_cast<socket_binary_stream_state*>(handle);
    if (state == nullptr || !state->s.is_open()) {
        return -1;
    }
    if (n == 0) {
        return 0;
    }
    const int result = ::send(state->s.native_handle(), reinterpret_cast<const char*>(s), n, 0);
    return result < 0 ? -1 : result;
}

inline auto socket_binary_seek(binary_stream_handle_t, std::int64_t, int) noexcept -> int { return -1; }
inline auto socket_binary_tell(binary_stream_handle_t) noexcept -> std::int64_t { return -1; }

struct socket_text_stream_state {
    socket s{};
    text_stream_encoding_t encoding = text_stream_encoding_t::utf8;
};

inline auto socket_text_close(text_stream_handle_t handle) noexcept -> int {
    auto* const state = reinterpret_cast<socket_text_stream_state*>(handle);
    if (state == nullptr) {
        return -1;
    }
    const int result = state->s.close();
    delete state;
    return result;
}

[[nodiscard]] inline auto socket_text_read_byte(socket_text_stream_state& state, unsigned char& out) noexcept -> int {
    char byte = 0;
    const int result = ::recv(state.s.native_handle(), &byte, 1, 0);
    if (result < 0) {
        return -1;
    }
    if (result == 0) {
        return 0;
    }
    out = static_cast<unsigned char>(byte);
    return 1;
}

[[nodiscard]] inline auto socket_text_read_utf8_char(socket_text_stream_state& state, char32_t& out) noexcept -> int {
    unsigned char b1 = 0;
    const int r1 = socket_text_read_byte(state, b1);
    if (r1 <= 0) {
        return r1;
    }
    if (b1 <= 0x7fu) {
        out = static_cast<char32_t>(b1);
        return 1;
    }
    std::uint32_t packed = static_cast<std::uint32_t>(b1);
    if (b1 >= 0xc2u && b1 <= 0xdfu) {
        unsigned char b2 = 0;
        if (socket_text_read_byte(state, b2) != 1) {
            return -1;
        }
        packed |= static_cast<std::uint32_t>(b2) << 8;
        out = advanced::packed_utf8_to_utf32(packed);
        return out == advanced::detail::invalid_utf32 ? -1 : 1;
    }
    if (b1 >= 0xe0u && b1 <= 0xefu) {
        unsigned char b2 = 0;
        unsigned char b3 = 0;
        if (socket_text_read_byte(state, b2) != 1 || socket_text_read_byte(state, b3) != 1) {
            return -1;
        }
        packed |= static_cast<std::uint32_t>(b2) << 8;
        packed |= static_cast<std::uint32_t>(b3) << 16;
        out = advanced::packed_utf8_to_utf32(packed);
        return out == advanced::detail::invalid_utf32 ? -1 : 1;
    }
    if (b1 >= 0xf0u && b1 <= 0xf4u) {
        unsigned char b2 = 0;
        unsigned char b3 = 0;
        unsigned char b4 = 0;
        if (socket_text_read_byte(state, b2) != 1 || socket_text_read_byte(state, b3) != 1 || socket_text_read_byte(state, b4) != 1) {
            return -1;
        }
        packed |= static_cast<std::uint32_t>(b2) << 8;
        packed |= static_cast<std::uint32_t>(b3) << 16;
        packed |= static_cast<std::uint32_t>(b4) << 24;
        out = advanced::packed_utf8_to_utf32(packed);
        return out == advanced::detail::invalid_utf32 ? -1 : 1;
    }
    return -1;
}

[[nodiscard]] inline auto socket_text_read_cp932_char(socket_text_stream_state& state, char32_t& out) noexcept -> int {
    unsigned char b1 = 0;
    const int r1 = socket_text_read_byte(state, b1);
    if (r1 <= 0) {
        return r1;
    }
    std::uint16_t packed = static_cast<std::uint16_t>(b1);
    if (advanced::detail::is_cp932_single_byte(packed)) {
        out = advanced::packed_cp932_to_utf32(packed);
        return out == advanced::detail::invalid_utf32 ? -1 : 1;
    }
    if (!advanced::detail::is_cp932_lead_byte(b1)) {
        return -1;
    }
    unsigned char b2 = 0;
    if (socket_text_read_byte(state, b2) != 1) {
        return -1;
    }
    packed |= static_cast<std::uint16_t>(b2) << 8;
    out = advanced::packed_cp932_to_utf32(packed);
    return out == advanced::detail::invalid_utf32 ? -1 : 1;
}

inline auto socket_text_read(text_stream_handle_t handle, char32_t* s, int n) noexcept -> int {
    if (s == nullptr || n < 0) {
        return -1;
    }
    auto* const state = reinterpret_cast<socket_text_stream_state*>(handle);
    if (state == nullptr || !state->s.is_open()) {
        return -1;
    }
    int count = 0;
    while (count < n) {
        char32_t ch = U'\0';
        const int result = state->encoding == text_stream_encoding_t::cp932
                               ? socket_text_read_cp932_char(*state, ch)
                               : socket_text_read_utf8_char(*state, ch);
        if (result < 0) {
            return count == 0 ? -1 : count;
        }
        if (result == 0) {
            return count;
        }
        s[count] = ch;
        ++count;
    }
    return count;
}

[[nodiscard]] inline auto socket_text_write_bytes(
    socket_text_stream_state& state,
    const unsigned char* bytes,
    int size) noexcept -> int {
    if (bytes == nullptr || size < 0) {
        return -1;
    }
    int total = 0;
    while (total < size) {
        const int result = ::send(state.s.native_handle(), reinterpret_cast<const char*>(bytes + total), size - total, 0);
        if (result <= 0) {
            return -1;
        }
        total += result;
    }
    return total;
}

[[nodiscard]] inline auto socket_text_write_utf8_char(socket_text_stream_state& state, char32_t ch) noexcept -> int {
    const std::uint32_t packed = advanced::utf32_to_packed_utf8(ch);
    if (packed == advanced::detail::invalid_packed_utf8) {
        return -1;
    }
    unsigned char bytes[4] = {};
    int size = 1;
    bytes[0] = static_cast<unsigned char>(packed & 0xffu);
    if ((packed & 0xff00u) != 0) {
        bytes[1] = static_cast<unsigned char>((packed >> 8) & 0xffu);
        size = 2;
    }
    if ((packed & 0xff0000u) != 0) {
        bytes[2] = static_cast<unsigned char>((packed >> 16) & 0xffu);
        size = 3;
    }
    if ((packed & 0xff000000u) != 0) {
        bytes[3] = static_cast<unsigned char>((packed >> 24) & 0xffu);
        size = 4;
    }
    return socket_text_write_bytes(state, bytes, size);
}

[[nodiscard]] inline auto socket_text_write_cp932_char(socket_text_stream_state& state, char32_t ch) noexcept -> int {
    const std::int32_t packed_signed = advanced::utf32_to_packed_cp932(ch);
    if (packed_signed == advanced::detail::invalid_packed_cp932) {
        return -1;
    }
    const std::uint16_t packed = static_cast<std::uint16_t>(packed_signed);
    unsigned char bytes[2] = {};
    int size = 1;
    bytes[0] = static_cast<unsigned char>(packed & 0xffu);
    if ((packed & 0xff00u) != 0) {
        bytes[1] = static_cast<unsigned char>((packed >> 8) & 0xffu);
        size = 2;
    }
    return socket_text_write_bytes(state, bytes, size);
}

inline auto socket_text_write(text_stream_handle_t handle, const char32_t* s, int n) noexcept -> int {
    if (s == nullptr || n < 0) {
        return -1;
    }
    auto* const state = reinterpret_cast<socket_text_stream_state*>(handle);
    if (state == nullptr || !state->s.is_open()) {
        return -1;
    }
    int count = 0;
    while (count < n) {
        const int result = state->encoding == text_stream_encoding_t::cp932
                               ? socket_text_write_cp932_char(*state, s[count])
                               : socket_text_write_utf8_char(*state, s[count]);
        if (result < 0) {
            return count == 0 ? -1 : count;
        }
        ++count;
    }
    return count;
}

inline auto socket_text_getpos(text_stream_handle_t) noexcept -> text_stream_pos_t { return -1; }
inline auto socket_text_setpos(text_stream_handle_t, text_stream_pos_t) noexcept -> int { return -1; }
inline auto socket_text_seek_end(text_stream_handle_t) noexcept -> int { return -1; }

} // namespace detail

[[nodiscard]] inline auto socket_open(socket&& s) noexcept -> result<binary_stream> {
    if (!s.is_open() || s.type() != socket_type::tcp) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }
    auto* const state = new (std::nothrow) detail::socket_binary_stream_state();
    if (state == nullptr) {
        return std::unexpected(make_error(error_t::runtime_error));
    }
    state->s = std::move(s);
    return binary_stream(
        reinterpret_cast<binary_stream_handle_t>(state),
        detail::socket_binary_close,
        detail::socket_binary_read,
        detail::socket_binary_write,
        detail::socket_binary_seek,
        detail::socket_binary_tell);
}

[[nodiscard]] inline auto socket_open(socket&& s, encoding_t encoding) noexcept -> result<text_stream> {
    if (!s.is_open() || s.type() != socket_type::tcp || encoding == encoding_t::auto_detect) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }
    auto* const state = new (std::nothrow) detail::socket_text_stream_state();
    if (state == nullptr) {
        return std::unexpected(make_error(error_t::runtime_error));
    }
    state->s = std::move(s);
    switch (encoding) {
    case encoding_t::utf8:
        state->encoding = detail::text_stream_encoding_t::utf8;
        break;
    case encoding_t::cp932:
        state->encoding = detail::text_stream_encoding_t::cp932;
        break;
    case encoding_t::auto_detect:
    default:
        delete state;
        return std::unexpected(make_error(error_t::invalid_argument));
    }
    return text_stream(
        reinterpret_cast<text_stream_handle_t>(state),
        detail::socket_text_close,
        detail::socket_text_read,
        detail::socket_text_write,
        detail::socket_text_getpos,
        detail::socket_text_setpos,
        detail::socket_text_seek_end);
}

} // namespace xer

#endif /* XER_BITS_SOCKET_H_INCLUDED_ */
