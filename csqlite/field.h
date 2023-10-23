#pragma once
#include "value.h"

class field_t final {
public:
    std::pair<std::string, value_t> data_;
public:
    explicit field_t(std::string name) : data_{std::move(name), {}} {}
    explicit field_t(std::string name, value_t v) : data_{std::move(name), std::move(v)} {}
    explicit field_t(std::pair<std::string, value_t> data) : data_{std::move(data)} {}

    field_t() = default;
    ~field_t() = default;
    field_t(field_t const&) = default;
    field_t(field_t&&) = default;
    field_t& operator=(field_t const&) = default;
    field_t& operator=(field_t&&) = default;

    std::pair<std::string, value_t> operator()() const noexcept {
        return data_;
    }
};

