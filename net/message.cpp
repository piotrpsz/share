#include "message.h"
#include "share.h"
using namespace std;

std::optional<std::string>
message::read_str(socket_t const& s) noexcept {
    if (auto vopt = message::read(s); vopt)
        return share::vec2str(*vopt);
    return {};
}

std::optional<std::vector<char>>
message::read(socket_t const &s) noexcept {
    u32 n{};
    if (s.read_bytes(&n, sizeof(u32)) && n > 0) {
        std::vector<char> vec(n, 0);
        if (s.read_bytes(vec.data(), n))
            return vec;
    }
    return {};
}

bool message::write(socket_t const &s, std::vector<char> const& buffer) noexcept {
    u32 n = buffer.size();
    if (s.write_bytes(&n, sizeof(u32)))
        return s.write_bytes(buffer.data(), n);
    return {};
}

bool message::write_str(socket_t const &s, std::string_view str) noexcept {
    std::vector<char> v(str.cbegin(), str.cend());
    return write(s, v);
}
