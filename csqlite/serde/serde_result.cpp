#include "serde_result.h"
#include "serde_row.h"
#include <numeric>

namespace serde::result {
    using rows_number_t = u16;
    static const u8 RESULT_MARKER = 'R';

    /// Obliczenie ile bajtów zajmie zserializowany obiekt.
    /// \param result - obiekt dla którego wyznaczamy liczbę bajtów.
    /// \return liczba bajtów po zserializowaniu obiektu.
    int length(result_t const& result) noexcept {
        auto const content_size = accumulate(
                result.cbegin(),
                result.cend(),
                0,
                [](ssize_t const count, row_t const& row) {
                    return count + serde::row::length(row);
                });
        return 1 + int(sizeof(rows_number_t) + content_size);
    }

    /// Zamiana obiektu 'result_t' na odpowiednie bajty (serializacja).
    /// \param result - obiekt do serializacji.
    /// \return wektor bajtów reprezentujących przekazany obiekt.
    vec<u8> as_bytes(result_t const& result) noexcept {
        auto const total_size = length(result);

        vec<u8> buffer;
        buffer.reserve(total_size);
        buffer.push_back(RESULT_MARKER);
        {   // zapamiętaj liczbę wierszy
            auto const n = static_cast<rows_number_t>(result.size());
            auto const ptr = reinterpret_cast<u8 const *>(&n);
            copy(ptr, ptr + sizeof(rows_number_t), back_inserter(buffer));
        }
        for (auto const& row: result) {
            auto data = serde::row::as_bytes(row);
            copy(begin(data), end(data), back_inserter(buffer));
        }
        buffer.shrink_to_fit();
        return buffer;
    }

    /// Utworzenie obiektu 'result_t' z przysłanych bajtów (deserializacja).
    /// \param bytes - ciąg bajów reprezentujących obiekt 'result_t'
    /// \return utworzony obiekt jeśli wszystko się powiodło, nullopt w przeciwnym przypadku.
    std::optional<result_t> from_bytes(std::span<u8> bytes) noexcept {
        // pobierz marker i sprawdź czy jest właściwy
        if (bytes.empty() || bytes[0] != RESULT_MARKER) return {};
        bytes = bytes.subspan(1);

        // pobierz liczbę wierszy w wyniku
        if (bytes.size() < sizeof(rows_number_t)) return {};
        rows_number_t n;
        memcpy(&n, bytes.data(), sizeof(rows_number_t));
        bytes = bytes.subspan(sizeof(rows_number_t));

        // pobierz wszystkie wiersze
        result_t result{};
        for (rows_number_t i = 0; i < n; i++) {
            auto row = serde::row::from_bytes(bytes);
            if (!row) return {};
            bytes = bytes.subspan(serde::row::length(*row));
            result.push_back(*row);
        }
        return result;
    }
}