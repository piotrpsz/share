#pragma once
#include <utility>
#include <variant>
#include <span>
#include <vector>
#include <fmt/core.h>
#include "../share.h"

static const char MONOSTATE_INDEX{0};
static const char INTEGER_INDEX{1};
static const char DOUBLE_INDEX{2};
static const char STRING_INDEX{3};
static const char VECTOR_INDEX{4};

class value_t final {
    std::variant<std::monostate, i64, f64, std::string, std::vector<u8>> data_;
public:
    ~value_t() = default;
    value_t(value_t const&) = default;
    value_t(value_t&&) = default;
    value_t& operator=(value_t const&) = default;
    value_t& operator=(value_t&&) = default;

    // CONSTRUCTION
    value_t() : data_{} {}
    value_t(std::integral auto v) : data_{static_cast<i64>(v)} {}
    value_t(std::floating_point auto v) : data_{static_cast<f64>(v)} {}
    value_t(std::string v) : data_{v} {}
    value_t(std::vector<u8> v) : data_{std::move(v)} {}

    auto const& operator()() const noexcept {
        return data_;
    }

    // GETTERS
    [[nodiscard]] int index() const noexcept {
        return data_.index();
    }
    [[nodiscard]] bool is_null() const noexcept {
        return data_.index() == MONOSTATE_INDEX;
    }

    // pobranie wartości bez sprawdzania (użytkownik wie co robi)
    [[nodiscard]]
    i64 int64() const noexcept {
        return std::get<INTEGER_INDEX>(data_);
    }
    [[nodiscard]]
    f64 float64() const noexcept {
        return std::get<DOUBLE_INDEX>(data_);
    }
    [[nodiscard]]
    std::string str() const noexcept {
        return std::get<STRING_INDEX>(data_);
    }
    [[nodiscard]]
    std::vector<u8> vec() const noexcept {
        return std::get<VECTOR_INDEX>(data_);
    }

    // pobranie wartości ze sprawdzeniem (OPCJONALNY WYNIK) -----
    [[nodiscard]]
    std::optional<i64> int64_if() const noexcept {
        if (data_.index() == INTEGER_INDEX)
            return std::get<INTEGER_INDEX>(data_);
        return {};
    }
    [[nodiscard]]
    std::optional<f64> float64_if() const noexcept {
        if (data_.index() == DOUBLE_INDEX)
            return std::get<DOUBLE_INDEX>(data_);
        return {};
    }
    [[nodiscard]]
    std::optional<std::string> str_if() const noexcept {
        if (data_.index() == STRING_INDEX)
            return std::get<STRING_INDEX>(data_);
        return {};
    }
    [[nodiscard]]
    std::optional<std::vector<u8>> vec_if() const noexcept {
        if (data_.index() == VECTOR_INDEX)
            return std::get<VECTOR_INDEX>(data_);
        return {};
    }

    /// Opis obiektu 'value' w postaci tekstowej.
    [[nodiscard]] std::string
    as_str() const noexcept {
        switch (data_.index()) {
            case MONOSTATE_INDEX:
                return "NULL";
            case INTEGER_INDEX:
                return fmt::format("int64{{{}}}", std::get<INTEGER_INDEX>(data_));
            case DOUBLE_INDEX:
                return fmt::format("double{{{}}}", std::get<DOUBLE_INDEX>(data_));
            case STRING_INDEX:
                return fmt::format("string{{{}}}", std::get<STRING_INDEX>(data_));
            case VECTOR_INDEX:
                return fmt::format("blob{{{}}}", share::bytes_as_str(std::get<VECTOR_INDEX>(data_)));
            default:
                return "?";
        }
    }

//    friend class serde;
    /// Zaprzyjaźniony operator wysyłania do strumienia reprezentacji tekstowej.
    friend std::ostream& operator<<(std::ostream& s, value_t const& v) {
        s << v.as_str();
        return s;
    }
};
