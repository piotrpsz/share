#include "clap.h"
#include <fmt/core.h>
#include "../share.h"

/// Przypisanie wartości argumentu na podstawie klucza.
/// \param key - klucz argumentu,
/// \param value - wartość argumentu
/// \remark Dopuszczalny jest brak wartości (wstawiany True - jako znak że jest). \n
///         Dopuszczalny jest klucz jako zestaw (po jednym myślniku).
void Clap::add(std::string key, std::string value) noexcept {
    auto is_long = key[0] == '-' && key[1] == '-';

    // Szukamy pełnego dopasowania.
    for (auto& arg: data_)
        if ((is_long && arg.promarker() == key) || (!is_long && arg.marker() == key)) {
            if (value.empty()) arg.value(true);
            else arg.value(value);
            return;
        }

    // Nie ma dokładnego dopasowania.
    // Może być, że mamy zestaw np. -ifr (jeśli nie ma wartości).
    // W zestawie każda literka to marker jakiegos argumentu.
    if (!is_long && value.empty()) {
        auto const subkey{key.substr(1)};
        for (auto const c : subkey) {
            auto ok{false};
            for (auto& arg: data_) {
                if (arg.marker().substr(1)[0] == c) {
                    arg.value(ok = true);
                    break;
                }
            }
            // Nie dopasowano tej literki.
            if (!ok)
                fmt::print(stderr, "unknown parameter: -{} (in {})\n", c, key);
        }
        return;
    }

    // Nie jest to zestaw lub jest wartość (nie dopuszczalna dla zestawu).
    fmt::print(stderr, "unknown parameter: {}\n", key);
}

int Clap::parse(int const argn, char const *const argv[]) noexcept {
    std::string key{}, value{};

    for (int i = 1; i < argn; i++) {
        auto const str{argv[i]};
        auto const is_key = str[0] == '-';
        if (!key.empty()) {
            // jeśli aktualny argument nie jest kluczem
            // to jest wartością dla zapamiętanego klucza
            if (!is_key)
                value = str;

            add(std::move(key), std::move(value));
            key.clear();
            value.clear();

            // zużyliśmy aktualny argument jako wartość
            // więc nie ma już nic do roboty i przechodzimy
            // do następnego argumentu
            if (!is_key)
                continue;
        }

        auto items = share::split(str, '=');
        switch (items.size()) {
            case 1:
                key = items[0];
                break;
            case 2:
                key = items[0];
                value = items[1];
                break;
            default:
                fmt::print(stderr, "invalid parameter: {}\n", str);
        }
    }

    if (!key.empty())
        add(std::move(key), std::move(value));
    return 0;
}

std::ostream& operator<<(std::ostream& s, Clap const& c) noexcept {
    fmt::print("{}\n", c.progname_);
    for (auto const& arg: c.data_)
        fmt::print("\t{}\n", arg.as_str());
    return s;
}
