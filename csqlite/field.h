#pragma once
#include "value.h"
#include <span>
#include <fmt/core.h>

class field_t final {
    std::pair<std::string, value_t> data_;
public:
    explicit field_t(const std::string& name) : data_{name, {}} {}
    explicit field_t(std::string name, value_t v) : data_{std::move(name), std::move(v)} {}
    field_t(std::pair<std::string, value_t> data) : data_{std::move(data)} {}

    field_t() = default;
    ~field_t() = default;
    field_t(field_t const&) = default;
    field_t(field_t&&) = default;
    field_t& operator=(field_t const&) = default;
    field_t& operator=(field_t&&) = default;

//    vec<u8> serialize() const noexcept;
//    static field_t deserialize(std::span<u8> data) noexcept;

    /// Zwraca kopię danych (pair).
    std::pair<std::string, value_t> operator()() const noexcept {
        return data_;
    }

    [[nodiscard]] std::string as_str() const noexcept {
        auto const [name, value] = data_;
        return fmt::format("{}:({})", name, value.as_str());
    }

    friend class serde;

    friend std::ostream &operator<<(std::ostream& s, field_t const& f) {
        s << f.as_str();
        return s;
    }
};
