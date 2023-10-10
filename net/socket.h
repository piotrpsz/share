#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include <optional>
#include "../share.h"

class socket_t {
    WSADATA wsa_data_{};
    SOCKET fd_{};
public:
    socket_t();
    ~socket_t() { close(); }
    explicit socket_t(SOCKET fd) : fd_{fd} {}
    socket_t(socket_t const&) = delete;
    socket_t& operator=(socket_t const&) = delete;
    socket_t(socket_t&&) = default;
    socket_t& operator=(socket_t&&) = default;

    void close() noexcept {
        if (fd_ != INVALID_SOCKET) {
            ::closesocket(fd_);
            fd_ = INVALID_SOCKET;
        }
        WSACleanup();
    }
    [[nodiscard]] bool connect_to(std::string const& address, int port) const noexcept;
    [[nodiscard]] bool bind_to(int port) const noexcept;
    [[nodiscard]] bool listen() const noexcept {
        return ::listen(fd_, 5) == 0;
    }
    [[nodiscard]] std::optional<SOCKET> accept() const noexcept {
        if (auto cfd = ::accept(fd_, nullptr, nullptr); cfd != INVALID_SOCKET)
            return static_cast<int>(cfd);
        return {};
    }

    bool read_bytes(void* buffer, u32 nbytes) const noexcept;
    bool write_bytes(void const* buffer, u32 nbytes) const noexcept;
    [[nodiscard]] std::string peer_address() const noexcept;
private:
    static bool setopt(SOCKET const fd, int const option, int flag) noexcept {
        return ::setsockopt(fd, SOL_SOCKET, option, (char*)&flag, sizeof(flag));
    }

};

