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

    /// Zwraca string zapytania.
    [[nodiscard]]
    std::string const& query() const noexcept {
        return query_;
    }
    /// Dodanie kolejną wartość zapytania.
    query_t& add(value_t v) noexcept {
        values_.push_back(v);
        return *this;
    }
    /// Zamiana ciągu bajtów na obiekt 'query_t'.
    static query_t deserialize(std::span<u8> data) noexcept {
        // odczyt całkowitego rozmiaru (u32)
        u32 total_size{};
        ::memcpy(&total_size, data.data(), sizeof(u32));
        data = data.subspan(sizeof(u32));
        // odczyt rozmiaru zapytania (query)
        u16 query_size{};
        ::memcpy(&query_size, data.data(), sizeof(u16));
        data = data.subspan(sizeof(u16));
        // odczyt zapytania w postaci stringa
        std::string query{share::as_string(data, query_size)};
        data = data.subspan(query_size);
        // odczyt wartości zapytania
        std::vector<value_t> values{};
        while (!data.empty()) {
            auto [v, n] = std::move(value_t::deserialize(data));
            values.push_back(std::move(v));
            data = data.subspan(n);
        }

        return query_t{share::trim(query), values};
    }

    /// Zamiana (serializacja) 'query_t' na bajty (w wektorze).
    /// u32 - całkowity rozmiar query+values w bajtach,
    /// u16 - rozmiar stringa query
    /// bajty stringa query (w ilości określonej powyżej)
    /// dalsze bajty to values
    [[nodiscard]] std::vector<u8>
    bytes() const noexcept {
        // lepiej policzyć niż później realokować wektor
        auto const values_bytes_count = std::accumulate(
                values_.cbegin(), values_.cend(),
                0,
                [](ssize_t count, value_t const& v) {
                    return count + v.size_ext();
                }
        );
        std::cout << "obliczony rozmiar parametrów: " << values_bytes_count << '\n';
        std::vector<u8> buffer;
        u32 content_size = sizeof(u32) + sizeof(u16) + query_.size() + values_bytes_count;
        std::cout << fmt::format("query size: {}\n", query_.size());
        std::cout << "prognozowany rozmiar: " << content_size << '\n';
        buffer.reserve(content_size);
        {   // zapamiętaj rozmiar kontentu w postaci bajtów
            auto const ptr = reinterpret_cast<u8 const *>(&content_size);
            std::copy(ptr, ptr + sizeof(u32), std::back_inserter(buffer));
        }
        {   // zapamiętaj rozmiar query w postaci bajtów
            u16 n = static_cast<u16>(query_.size());
            auto const ptr = reinterpret_cast<u8 const *>(&n);
            std::copy(ptr, ptr + sizeof(u16), std::back_inserter(buffer));
        }
        // kopiuj zapytanie
        std::copy(std::begin(query_), std::end(query_), std::back_inserter(buffer));
        // i poszczególne wartości
        for (auto const& item: values_) {
            auto bytes = item.bytes();
            std::copy(std::begin(bytes), std::end(bytes), std::back_inserter(buffer));
        }

        // tak na wszelki wypadek
        buffer.shrink_to_fit();
        std::cout << "rozmiar wynikowy: " << buffer.size() << '\n';

        return buffer;
    }

    /// Zaprzyjaźniony operator wysyłania do strumienia reprezentacji tekstowej.
    friend std::ostream& operator<<(std::ostream& s, query_t const& q) {
        s << fmt::format("{}\n", q.query_);
        for (auto const& v: q.values_) {
            s << fmt::format("\t{}\n", v.as_str());
        }
        return s;
    }
};