#pragma once
#include <vector>
#include <string>
#include <optional>
#include <iterator>
#include <variant>
#include <iostream>
#include "value.h"
#include "field.h"
#include "../share.h"

using names_t = std::vector<std::string>;
using values_t = std::vector<value_t>;

class row_t final {
    std::vector<field_t> data_{};
public:
    row_t(std::string name, value_t value) {
        data_.emplace_back(std::move(name), std::move(value));
    }
    row_t() = default;
    row_t(row_t const &) = default;
    row_t &operator=(row_t const &) = default;
    row_t(row_t &&) = default;
    row_t &operator=(row_t &&) = default;
    ~row_t() = default;

    [[nodiscard]] bool
    empty() const noexcept {
        return data_.empty();
    }
    [[nodiscard]] size_t size() const noexcept {
        return data_.size();
    }

    auto emplace_back(std::string name, value_t value) {
        data_.emplace_back(std::move(name), std::move(value));
    }
    row_t& add(std::string name, value_t value) noexcept {
        data_.emplace_back(std::move(name), std::move(value));
        return *this;
    }
    row_t& add(std::string name) {
        data_.emplace_back(std::move(name));
        return *this;
    }

    template<typename T>
    row_t &add(std::string name, std::optional<T> value) noexcept {
        return (value)
               ? add(std::move(name), std::move(*value))
               : add(std::move(name));
    }

    /// Wektor par <nazwa, wartość> rozbijany na wektor nazw i wektor wartości
    /// \return std-para wektorów (wektor nazw i wektor wartości).
    [[nodiscard]] std::pair<names_t, values_t> split() const noexcept;

    using iterator = std::vector<field_t>::iterator;
    using const_iterator = std::vector<field_t>::const_iterator;
    iterator begin() { return data_.begin(); }
    iterator end() { return data_.end(); }
    const_iterator cbegin() const { return data_.cbegin(); }
    const_iterator cend() const { return data_.cend(); }

    friend std::ostream &operator<<(std::ostream &s, row_t const &r);
    friend class field_t;
};

std::ostream &operator<<(std::ostream &s, field_t const &f);

