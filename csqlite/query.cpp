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

/// Zaprzyjaźniony operator wysyłania do strumienia reprezentacji tekstowej.
std::ostream& operator<<(std::ostream& s, query_t const& q) noexcept {
    s << fmt::format("{}\n", q.query_);
    for (auto const& v: q.values_) {
        s << fmt::format("\t{}\n", v.as_str());
    }
    return s;
}
