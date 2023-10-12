#include "socket.h"
#include <iostream>
#include <format>

/// Utworzenie nowego gniazda TCP/IP.
socket_t::socket_t() {
    if (auto retv = WSAStartup(MAKEWORD(2, 2), &wsa_data_); retv != 0) {
        std::cerr << std::format("WSAStartup failed: {}",  retv) << '\n';
        return;
    }
    if (auto const fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); fd != INVALID_SOCKET)
        if (socket_t::setopt(fd, SO_REUSEADDR, 1))
            if (socket_t::setopt(fd, SO_KEEPALIVE, 1)) {
                fd_ = static_cast<int>(fd);
                return;
            }

    std::cerr << std::format("Error at socket: {}",  WSAGetLastError()) << '\n';
}

/// Połączenie z serwerem pod wskazanym adresem IP na wskazanym porcie.
/// \param address - adres IP serwera,
/// \param port - port na którym nawiązać łączność
/// \return True jeśli połączenie się powidło, False w przeciwnym przypadku.
bool socket_t::connect_to(std::string const& address, int const port) const noexcept {
    struct sockaddr_in dest{};
    dest.sin_family = AF_INET;

    if (auto host = gethostbyname(address.c_str()); host) {
        dest.sin_addr.S_un.S_addr = *(unsigned *)host->h_addr_list[0];
        dest.sin_port = htons(port);
        if (::connect(fd_, (struct sockaddr*)&dest, sizeof(dest)) == 0)
            return true;
    }
    std::cerr << std::format("Error at connect: {}\n",  WSAGetLastError());
    return false;
}

/// Podłączenie się do wskazanego portu (na wszystkich interfejsach).
/// \param port - port do którym będziemy nasłuchiwać.
/// \return True jesli udało się podłączyć, False w przeciwnym przypadku.
bool socket_t::bind_to(int const port) const noexcept {
    struct sockaddr_in sin{};
    sin.sin_family = AF_INET;
    sin.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(port);
    if (::bind(fd_, (sockaddr*)&sin, sizeof(sin)) == 0)
        return true;
    std::cerr << std::format("Error at bind: {}\n", WSAGetLastError());
    return false;
}

/// Odczyt adresu IP (+ port) hosta z drugiej strony łącza.
std::string socket_t::peer_address() const noexcept {
    struct sockaddr_in addr{};
    auto n = static_cast<int>(sizeof(addr));
    if (::getpeername(fd_, (sockaddr*)&addr, &n) == 0) {
        auto ptr = ::inet_ntoa(addr.sin_addr);
        return std::format("{}:{}", ptr, addr.sin_port);
    }
    return {};
}

/// Odczyt z gniazda wskazanej liczby bajtów do wskazanego bufora.
/// \param buffer - bufor na odczytane bajty,
/// \param nbytes - liczba bajtów do odczytu (rozmiar bufora).
/// \return True jeśli wszystko poszło dobrze, False w przeciwnym przypadku.
bool socket_t::read_bytes(void* const buffer, u32 const nbytes) const noexcept {
    auto nleft = static_cast<int>(nbytes);
    auto ptr = static_cast<char*>(buffer);

    while (nleft > 0) {
        auto nread = ::recv(fd_, ptr, nleft, 0);
        if (nread < 0) {
            if (errno == EINTR)
                nread = 0;
            else {
               std::cerr << "Error at reading: " << WSAGetLastError() << '\n';
               return false;
            }
        }
        else if (nread == 0) {
            break;  // connection closed
        }
        nleft -= nread;
        ptr += nread;
    }

    return true;
}

/// Zapis do gniazda wskazanej liczby bajtów ze wskazanego bufora.
/// \param buffer - bufor z bajtami do zapisu,
/// \param nbytes - liczba bajtów do zapisu (rozmiar bufora).
/// \return True jeśli wszystko poszło dobrze, False w przeciwnym przypadku.
bool socket_t::write_bytes(void const* buffer, u32 const nbytes) const noexcept {
    auto nleft = static_cast<int>(nbytes);
    auto ptr = static_cast<char const*>(buffer);

    while (nleft > 0) {
        auto nwrite = ::send(fd_, ptr, int(nleft), 0);

        if (nwrite <= 0) {
            if (errno == EINTR)
                nwrite = 0;
            else {
                std::cerr << "Error at writting: " << WSAGetLastError() << '\n';
                return false;
            }
        }
        nleft -= nwrite;
        ptr += nwrite;
    }

    return true;
}
