#include "serde_query.h"
#include "serde_value.h"

namespace serde::query {
    using query_size_t = u16;
    static const u8 QUERY_MARKER = 'Q';

    u64 length(query_t const& query) noexcept {
        auto const values_total_size = std::accumulate(
                query.values().cbegin(),
                query.values().cend(),
                0,
                [](ssize_t count, value_t const& v) {
                    return count + serde::value::length(v);
                }
        );
        return u64(1) + sizeof(u16) + query.query().size() + values_total_size;
    }

    vec<u8> as_bytes(query_t const& q) noexcept {
        std::vector<u8> buffer;
        buffer.reserve(length(q));
        buffer.push_back(QUERY_MARKER);

        {   // zapamiętaj rozmiar zapytania w postaci bajtów
            auto n = static_cast<query_size_t>(q.query().size());
            auto const ptr = reinterpret_cast<u8 const *>(&n);
            std::copy(ptr, ptr + sizeof(query_size_t), std::back_inserter(buffer));
        }
        // kopiuj zapytanie
        std::copy(std::begin(q.query()), std::end(q.query()), std::back_inserter(buffer));

        // i poszczególne wartości
        for (auto const& v: q.values()) {
            auto bytes = serde::value::as_bytes(v);
            std::copy(std::begin(bytes), std::end(bytes), std::back_inserter(buffer));
        }

        // tak na wszelki wypadek
        buffer.shrink_to_fit();
        return buffer;
    }

    std::optional<query_t> from_bytes(std::span<u8> bytes) noexcept {
        // pobierz marker i sprawdź czy jest właściwy
        if (bytes.empty() || bytes[0] != QUERY_MARKER) return {};
        bytes = bytes.subspan(1);

        // odczyt rozmiaru zapytania (query)
        if (bytes.size() < sizeof(query_size_t)) return {};
        query_size_t query_size{};
        ::memcpy(&query_size, bytes.data(), sizeof(query_size_t));
        bytes = bytes.subspan(sizeof(query_size_t));

        // odczyt zapytania w postaci stringa
        if (bytes.size() < query_size) return {};
        std::string query{share::as_string(bytes, query_size)};
        bytes = bytes.subspan(query_size);

        // odczyt wartości zapytania
        std::vector<value_t> values{};
        while (!bytes.empty()) {
            auto v = serde::value::from_bytes(bytes);
            if (!v) return {};
            bytes = bytes.subspan(serde::value::length(*v));
            values.push_back(std::move(*v));
        }

        return query_t{share::trim(query), values};
    }
}
