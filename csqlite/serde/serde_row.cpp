#include "serde_row.h"
#include <numeric>
#include "serde_field.h"

namespace serde::row {
    using total_size_t = u64;
    using fields_number_t = u16;

    static const u8 ROW_MARKER = 'R';

    /// Obliczenie ile bajtów zajmie zserializowany obiekt.
    /// \param row - obiekt dla którego wyznaczamy liczbę bajtów.
    /// \return liczba bajtów po zserializowaniu obiektu.
    int length(row_t const& row) noexcept {
        auto const content_size = std::accumulate(
                row.cbegin(),
                row.cend(),
                total_size_t{},
                [](ssize_t const count, field_t const& f) {
                    return count + serde::field::length(f);
                });
        return 1 + int(sizeof(fields_number_t) + content_size);
    }

    /// Zamiana obiektu 'row_t' na odpowiednie bajty (serializacja).
    /// \param row - obiekt do serializacji.
    /// \return wektor bajtów reprezentujących przekazany obiekt.
    vec<u8> as_bytes(row_t const& row) noexcept {
        auto total_size = static_cast<total_size_t>(serde::row::length(row));

        vec<u8> buffer;
        buffer.reserve(total_size);
        buffer.push_back(ROW_MARKER);
        {   // zapamiętaj liczbę pól
            auto const n = static_cast<fields_number_t>(row.size());
            auto ptr = reinterpret_cast<u8 const *>(&n);
            copy(ptr, ptr + sizeof(fields_number_t), back_inserter(buffer));
        }
        for (auto it = row.cbegin(); it != row.cend(); it++) {
            auto data = serde::field::as_bytes(*it);
            copy(begin(data), end(data), back_inserter(buffer));
        }
        buffer.shrink_to_fit();
        return buffer;
    }

    /// Utworzenie obiektu 'row_t' z przysłanych bajtów (deserializacja).
    /// \param bytes - ciąg bajów reprezentujących obiekt 'row_t'.
    /// \return utworzony obiekt jeśli wszystko się powiodło, nullopt w przeciwnym przypadku.
    std::optional<row_t> from_bytes(std::span<u8> bytes) noexcept {
        // pobierz marker i sprawdź czy jest właściwy
        if (bytes.empty() || bytes[0] != ROW_MARKER) return {};
        bytes = bytes.subspan(1);

        // pobierz liczbę pól w wierszu
        if (bytes.size() < sizeof(fields_number_t)) return {};
        fields_number_t n;
        memcpy(&n, bytes.data(), sizeof(fields_number_t));
        bytes = bytes.subspan(sizeof(fields_number_t));

        // pobierz wszystkie pola
        row_t row{};
        for (int i = 0; i < n; i++) {
            auto f = serde::field::from_bytes(bytes);
            if (!f) return {};
            bytes = bytes.subspan(serde::field::length(*f));
            row.add(*f);
        }
        return row;
    }
}
