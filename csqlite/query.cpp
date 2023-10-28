/*------- include files:
-------------------------------------------------------------------*/
#include <iostream>
#include <fmt/core.h>
#include <utility>
#include "query.h"


/// Sprawdzenie poprawności zapytania.
/// Zapytanie jest poprawne jeśli liczba placeholderów ('?') zgadza się
/// z liczbą wartości.
bool query_t::
valid() const noexcept {
    auto const placeholder_count = count_if(query_.cbegin(), query_.cend(), [](char const c) {
        return '?' == c;
    });

    if (std::cmp_not_equal(placeholder_count, values_.size())) {
        std::cerr << fmt::format("The number of placeholders and arguments does not match ({}, {})\n", placeholder_count, values_.size());
        return false;
    }
    return true;
}

query_t query_t::
deserialize(std::span<u8> data) noexcept {
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
        auto [v, n] = value_t::deserialize(data);
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
[[nodiscard]] std::vector<u8> query_t::
serialize() const noexcept {
    // lepiej policzyć niż później realokować wektor
    auto const values_bytes_count = std::accumulate(
            values_.cbegin(), values_.cend(),
            0,
            [](ssize_t count, value_t const& v) {
                return count + v.size_ext();
            }
    );
    std::vector<u8> buffer;
    u32 content_size = sizeof(u32) + sizeof(u16) + query_.size() + values_bytes_count;
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
    return buffer;
}

/// Zaprzyjaźniony operator wysyłania do strumienia reprezentacji tekstowej.
std::ostream& operator<<(std::ostream& s, query_t const& q) noexcept {
    s << fmt::format("{}\n", q.query_);
    for (auto const& v: q.values_) {
        s << fmt::format("\t{}\n", v.as_str());
    }
    return s;
}
