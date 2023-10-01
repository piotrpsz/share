#ifndef BEESOFT_SQLITE_ROW
#define BEESOFT_SQLITE_ROW

#include <vector>
#include <string>
#include <optional>
#include <utility>
#include <iterator>
#include <cstddef>
#include <variant>
#include <iostream>
#include "helper.h"

using value_t = std::variant<std::monostate, i64, f64, std::string, std::vector<u8>>;
using field_t = std::pair<std::string, value_t>;


class row_t {
    std::vector<field_t> data_{};
public:
    row_t() = default;

    row_t(std::string name, value_t value) {
        data_.emplace_back(std::move(name), std::move(value));
    }

    row_t(row_t const &) = default;
    row_t &operator=(row_t const &) = default;
    row_t(row_t &&) = default;
    row_t &operator=(row_t &&) = default;

    [[nodiscard]] bool
    empty() const noexcept {
        return data_.empty();
    }

    auto emplace_back(std::string name, value_t value) {
        data_.emplace_back(std::move(name), std::move(value));
    }

    row_t &add(std::string name, value_t value) noexcept {
        data_.emplace_back(std::move(name), std::move(value));
        return *this;
    }

    template<typename T>
    row_t &add(std::string name, std::optional<T> value) noexcept {
        return (value)
               ? add(std::move(name), std::move(*value))
               : add(std::move(name), std::monostate());
    }

    std::pair<std::vector<std::string>, std::vector<value_t>> split() noexcept;

    using iterator = std::vector<field_t>::iterator;
    using const_iterator = std::vector<field_t>::const_iterator;
    iterator begin() { return data_.begin(); }
    iterator end() { return data_.end(); }
    const_iterator cbegin() const { return data_.cbegin(); }
    const_iterator cend() const { return data_.cend(); }

    friend std::ostream &operator<<(std::ostream &s, row_t const &r);
};

std::ostream &operator<<(std::ostream &s, field_t const &f);


#endif // BEESOFT_SQLITE_ROW
