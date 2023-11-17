#pragma once

#include <string>
#include <optional>
#include <variant>
#include <cstdint>

using i64 = int64_t;
using f64 = double_t;

using value_t = std::variant<std::monostate, bool, i64, f64, std::string>;

class Arg {
    std::string shortcut_{};
    std::string name_{};
    value_t value_{};
    value_t default_{};
    std::string description_{};
    int index_{};
public:
    Arg() = default;
    ~Arg() = default;
    Arg(Arg const&) = default;
    Arg(Arg&&) = default;
    Arg& operator=(Arg const&) = default;
    Arg& operator=(Arg&&) = default;

    // np. -i for ignore case
    Arg& shortcut(std::string v) noexcept {
        shortcut_ = std::move(v);
        return *this;
    }
    // np. --icase for ignore case
    Arg& name(std::string v) noexcept {
        name_ = std::move(v);
        return *this;
    }
    Arg& value(value_t v) noexcept {
        value_ = std::move(v);
        return *this;
    }
    Arg& ordef(value_t v) noexcept {
        default_ = std::move(v);
        return *this;
    }
    Arg& index(int idx) noexcept {
        index_ = idx;
        return *this;
    }
};

