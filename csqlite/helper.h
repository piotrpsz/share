#ifndef BEESOFT_SQLITE_HELPER
#define BEESOFT_SQLITE_HELPER

#include <string>
#include <vector>
#include <iostream>
#include <cstdint>
#include <filesystem>
#include <numeric>

#define LOG_ERROR() \
    auto fn = std::string(__FILE__); \
    if (auto pos = fn.find_last_of('/'); pos != std::string::npos) \
        fn = fn.substr(pos+1); \
    std::cerr << "SQLITE ERROR: " << err_msg() << " (" << err_code() << ") =>" << fn << "::" << __FUNCTION__ \
    << "()." << __LINE__ << "[" << __FILE__ << "]\n";

using u8 = uint8_t;
using u32 = uint32_t;
using i64 = int64_t;
using f64 = double;
namespace fs = std::filesystem;

inline static std::string join(std::vector<std::string> const &vec, std::string const& sep = ", ") noexcept {
    return std::accumulate(
            std::next(vec.cbegin()),
            vec.cend(),
            vec[0],
            [&sep](std::string a, const std::string& b) {
                auto result = std::move(a);
                return result.append(sep).append(b);
            }
    );
}

std::string bytes_as_string(std::vector<u8> const& data) noexcept;

#endif // BEESOFT_SQLITE_HELPER
