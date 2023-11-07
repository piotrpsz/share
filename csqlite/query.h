#pragma once
#include "value.h"
#include "../share.h"
#include <string>
#include <vector>
#include <numeric>  // accumulate
#include <iostream>
#include <span>

class query_t final {
    std::string query_{};
    std::vector<value_t> values_{};
public:
    query_t() = default;
    ~query_t() = default;
    query_t(query_t const&) = default;
    query_t(query_t&&) = default;
    query_t& operator=(query_t const&) = default;
    query_t& operator=(query_t&&) = default;

    explicit query_t(std::string str) : query_{std::move(str)} {}
    explicit query_t(std::string str, std::vector<value_t> data) : query_{std::move(str)}, values_{std::move(data)} {}

    [[nodiscard]]
    bool valid() const noexcept;

    /// Zwraca string zapytania.
    [[nodiscard]]
    std::string const& query() const noexcept {
        return query_;
    }

    [[nodiscard]]
    std::vector<value_t> const& values() const noexcept {
        return values_;
    }

    query_t add(std::optional<value_t> v = {}) {
        if (v) values_.push_back(*v);
        else values_.emplace_back();
        return *this;
    }

    /// Zaprzyjaźniony operator wysyłania do strumienia reprezentacji tekstowej.
    friend std::ostream& operator<<(std::ostream& s, query_t const& q) noexcept;
};
