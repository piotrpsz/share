#pragma once
#include <utility>
#include <variant>
#include <span>
#include <vector>
#include <fmt/core.h>
#include "share.h"

namespace ppx {
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

        // GETTERS
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
        std::optional<f64> double64_if() const noexcept {
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

        [[nodiscard]] ssize_t
        size_ext() const noexcept {
            switch (data_.index()) {
                case MONOSTATE_INDEX: return 1;
                case INTEGER_INDEX: return 1 + sizeof(i64);
                case DOUBLE_INDEX: return 1 + sizeof(f64);
                case STRING_INDEX: return 1 + sizeof(u16) + std::get<STRING_INDEX>(data_).size();
                case VECTOR_INDEX: return 1 + sizeof(u32) + std::get<VECTOR_INDEX>(data_).size();
            }
            return 0;
        }

        /// Utworzenie obiektu 'value_t' z ciągu wskazanych bajtów. \n
        /// Funkcja konsumuje tyle bajtów ile potrzebuje (dalsze bajty mogą dotyczyć kolejnych obiektów).
        /// \param v - span bajtów, z ktorych odtwarzamy pierwszy obiekt 'value_t'
        /// \return para, którą tworzą utworzony obiekt 'value_t' i liczba skonsumowanych bajtów.
        static std::pair<value_t, int>
        from_bytes(std::span<u8> v) noexcept {
            switch (v[0]) {
                case MONOSTATE_INDEX:
                    return {value_t(), 1};
                case INTEGER_INDEX: {
                    i64 n{};
                    ::memcpy(&n, v.data() + 1, sizeof(i64));
                    return {value_t(n), 1 + sizeof(i64)};
                }
                case DOUBLE_INDEX: {
                    f64 n{};
                    ::memcpy(&n, v.data() + 1, sizeof(f64));
                    return {value_t(n), 1 + sizeof(f64)};
                }
                case STRING_INDEX: {
                    u16 n{};
                    ::memcpy(&n, v.data() + 1, sizeof(u16));
                    std::string str{reinterpret_cast<char *>(v.data() + 1 + sizeof(u16)), n};
                    return {value_t(str), 1 + sizeof(u16) + n};
                }
                case VECTOR_INDEX: {
                    u32 n{};
                    ::memcpy(&n, v.data() + 1, sizeof(u32));
                    std::vector<u8> vec;
                    vec.resize(n);
                    ::memcpy(vec.data(), v.data() + 1 + sizeof(u32), n);
                    return {value_t(vec), 1 + sizeof(u32) + n};
                }
            }
            return {};
        }

        /// Opis obiektu 'value' w postaci tekstowej.
        [[nodiscard]] std::string
        as_str() const noexcept {
            switch (data_.index()) {
                case MONOSTATE_INDEX:
                    return "NULL";
                case INTEGER_INDEX:
                    return fmt::format("int64 ({})", std::get<INTEGER_INDEX>(data_));
                case DOUBLE_INDEX:
                    return fmt::format("double ({})", std::get<DOUBLE_INDEX>(data_));
                case STRING_INDEX:
                    return fmt::format("string ({})", std::get<STRING_INDEX>(data_));
                case VECTOR_INDEX:
                    return fmt::format("blob ({})", share::bytes_as_str(std::get<VECTOR_INDEX>(data_)));
                default:
                    return "?";
            }
        }

        /// Zamiana (serializacja) 'value' na bajty (w wektorze).
        [[nodiscard]] std::vector<u8>
        bytes() const noexcept {
            switch (data_.index()) {
                case MONOSTATE_INDEX:
                    return {MONOSTATE_INDEX};
                case INTEGER_INDEX: {
                    // 1 bajt type + 8 baitów wartości (64 bity)
                    i64 const n = std::get<INTEGER_INDEX>(data_);
                    std::vector<u8> v{INTEGER_INDEX};
                    v.resize(1 + sizeof(i64));
                    ::memcpy(v.data() + 1, reinterpret_cast<void const *>(&n), sizeof(i64));
                    return v;
                }
                case DOUBLE_INDEX: {
                    // 1 bajt type + 8 baitów wartości
                    f64 const n = std::get<DOUBLE_INDEX>(data_);
                    std::vector<u8> v{DOUBLE_INDEX};
                    v.resize(1 + sizeof(f64));
                    ::memcpy(v.data() + 1, reinterpret_cast<void const *>(&n), sizeof(f64));
                    return v;
                }
                case STRING_INDEX: {
                    // 1 bajt type + 2 bajty (u16) size + bajty stringa
                    // maksymalny rozmiar stringa: 65535 (u16, 2 bajty)
                    std::string str = std::get<STRING_INDEX>(data_);
                    u16 n = static_cast<u16>(str.size());
                    std::vector<u8> v{STRING_INDEX};
                    v.resize(1 + sizeof(u16) + n);
                    ::memcpy(v.data() + 1, reinterpret_cast<void const *>(&n), sizeof(u16));
                    ::memcpy(v.data() + 1 + sizeof(u16), str.data(), n);
                    return v;
                }
                case VECTOR_INDEX: {
                    // 1 bajt type + 4 bajy (u32) size + bajty wektora blob
                    // maksymalny rozmiar wektora-blob: 4 294 967 295 (u32, 4 bajty)
                    std::vector<u8> vec = std::get<VECTOR_INDEX>(data_);
                    u32 n = static_cast<u32>(vec.size());
                    std::vector<u8> v{VECTOR_INDEX};
                    v.resize(1 + sizeof(u32) + n);
                    ::memcpy(v.data() + 1, reinterpret_cast<void const *>(&n), sizeof(u32));
                    ::memcpy(v.data() + 1 + sizeof(u32), vec.data(), n);
                    return v;
                }
            }
            return {};
        }

        /// Zaprzyjaźniony operator wysyłania do strumienia reprezentacji tekstowej.
        friend std::ostream& operator<<(std::ostream& s, value_t const& v) {
            s << v.as_str();
            return s;
        }
    };
}
