#pragma once
#include "socket.h"
#include <optional>
#include <vector>
#include <string_view>

class message {
public:
    static std::optional<std::string> read_str(socket_t const& s) noexcept;
    static std::optional<std::vector<char>> read(socket_t const& s) noexcept;
    static bool write(socket_t const& s, std::vector<char> const& v) noexcept;
    static bool write_str(socket_t const& s, std::string_view str) noexcept;
};