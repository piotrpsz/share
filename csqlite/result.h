#pragma once
#include "row.h"

class result_t {
    std::vector<row_t> data_ = {};
public:
    result_t() = default;
    result_t(result_t const&) = default;
    result_t& operator=(result_t const&) = default;
    result_t(result_t &&) = default;
    result_t &operator=(result_t &&) = default;
    ~result_t() = default;

    explicit result_t(row_t row) {
        data_.push_back(std::move(row));
    }
    [[nodiscard]] bool empty() const noexcept {
        return data_.empty();
    }
    [[nodiscard]] size_t size() const noexcept;

    void push_back(row_t row) noexcept {
        data_.push_back(std::move(row));
    }

    using iterator = std::vector<row_t>::iterator;
    using const_iterator = std::vector<row_t>::const_iterator;
    iterator begin()  { return data_.begin(); }
    iterator end() { return data_.end(); }
    const_iterator begin() const{ return data_.cbegin(); }
    const_iterator end() const { return data_.cend(); }
    const_iterator cbegin() const { return data_.cbegin(); }
    const_iterator cend() const { return data_.cend(); }

    friend std::ostream &operator<<(std::ostream &s, row_t const &r);
};

static inline std::ostream &operator<<(std::ostream& s, result_t const& result) {
    for (auto const& row: result)
        std::cout << row << '\n';
    return s;
}

