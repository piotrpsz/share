#include "share.h"
#include <random>
#include <sstream>
#include <iostream>
#include <charconv>
#include <range/v3/all.hpp>
#include <fmt/core.h>


/// Zamienia ciąg bajtów typu 'u8' na string.
/// \param data - widok na ciągły zbór bajtów.
/// \param n - liczba bajtów do użycia.
/// \return string utworzony ze wskazanej liczby bajtów.
std::string share::
as_string(std::span<u8> data, int n) noexcept {
    return data
            | ranges::views::transform([](u8 c) {return char(c);})
            | ranges::views::take(n)
            | ranges::to<std::string >();
}

std::string share::
as_string(std::span<u8> data) noexcept {
    return data
            | ranges::views::transform([](u8 c) {return char(c);})
            | ranges::to<std::string>();
}

/// Sprawdzenie czy przysłany znak nie(!) jest białym znakiem.
/// \param c - znak do sprawdzenia
/// \return True jeśli NIE jest białym znakiem, False w przeciwnym przypadku.
bool share::
is_not_space(char c) noexcept {
    return !std::isspace(c);
}

/// Obcięcie wiodących (z początku) białych znaków.
/// \param s - string z którego należy usunąć białe znaki
/// \return string bez wiodących białych znaków.
std::string share::
trim_left(std::string s) noexcept {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), is_not_space));
    return s;
}

/// Usunięcie zamykającyh (końcowych) białych znaków (z prawej strony).
/// \param s - string z którego należy usunąć białe znaki
/// \return string bez zamykających białych znaków.
std::string share::
trim_right(std::string s) noexcept {
    s.erase(std::find_if(s.rbegin(), s.rend(), is_not_space).base(), s.end());
    return s;
}

/// Usunięcie wiodących i zamykających białych znaków (z obu stron).
/// \param s - string z którego należy usunąć białe znaki
/// \return string bez wiodących i zamykających białych znaków.
std::string share::
trim(std::string s) noexcept {
    return trim_left(trim_right(std::move(s)));
}

std::vector<std::string> share::
split_view(std::string_view sv, char const delimiter) noexcept {
    // Lepiej policzyć delimitery niż później realokować wektor.
    auto const n = std::accumulate(
            sv.cbegin(),
            sv.cend(),
            ssize_t {0},
            [delimiter](ssize_t const count, char const c) {
                return (c == delimiter) ? count + 1 : count;
            }
    );

    std::vector<std::string> tokens{};
    tokens.reserve(n + 1);

    auto pos = sv.find(delimiter);
    while (pos != std::string_view::npos) {
        tokens.push_back(trim({sv.data(), sv.data() + pos}));
        sv.remove_prefix(pos + 1);
        pos = sv.find(delimiter);
    }
    if (sv.data() != sv.end())
        tokens.push_back(trim({sv.data(), sv.end()}));

    return tokens;
}

/// Podział przysłanego stringa na wektor stringów. \n
/// Wyodrębnianie stringów składowych odbywa się po napotkaniu delimiter'a.
/// \param text - string do podziału,
/// \param delimiter - znak sygnalizujący podział,
/// \return Wektor stringów.
std::vector<std::string> share::
split(std::string const &text, char const delimiter) noexcept {
    // Lepiej policzyć delimitery niż później realokować wektor.
    auto const n = std::accumulate(
            text.cbegin(),
            text.cend(),
            ssize_t {0},
            [delimiter](ssize_t const count, char const c) {
                return (c == delimiter) ? count + 1 : count;
            }
    );

    std::vector<std::string> tokens{};
    tokens.reserve(n + 1);

    std::string token;
    std::istringstream stream(text);
    while (std::getline(stream, token, delimiter))
        if (auto str = trim(token); !str.empty())
            tokens.push_back(str);
    return tokens;
}

/// Tworzy string będący złączeniem stringów przysłanych w wektorze. \n
/// Łączone stringi rozdzielone są przysłanym delimiter'em.
/// \param data - wektor stringów do połączenia,
/// \param delimiter - znak wstawiany pomiędzy łączonymi stringami (domyślnie przecinek).
/// \return String jako suma przysłanych stringów.
std::string share::
join_strings(std::vector<std::string> const& data, char const delimiter) noexcept {
    return std::accumulate(
            std::next(data.begin()),
            data.end(),
            data[0],
            [delimiter](std::string a, std::string b) {
                return fmt::format("{}{}{}", std::move(a), delimiter, std::move(b));
            }
    );
}

/// Zamiana tekstu na liczbę typu integer.
/// \param text - tekstowa reprezentacja liczby,
/// \param v - referencja do zmiennej typu integer, do której zostanie przekazana wyznaczona wartość,
/// \param base - system numeryczny użyty w tekście (domyślnie 10)
/// \return true jeśli wszystko poszło dobrze, w przeciwnym przypadku false.
std::optional<int> share::
to_int(std::string_view sv, int const base) {
    int value{};
    auto [_, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value, base);

    // Konwersja się udała.
    if (ec == std::errc{})
        return value;

    // Coś poszło nie tak.
    if (ec == std::errc::invalid_argument)
        std::cerr << fmt::format("This is not a number ({}).\n", sv);
    else if (ec == std::errc::result_out_of_range)
        std::cerr << fmt::format("The number is to big ({}).\n", sv);
    return {};
}

/// Utworzenie wektora losowych bajtów.
/// \param n - oczekiwana liczba bajtów.
/// \return Wektor losowych bajtów.
std::vector<u8> share::
random_bytes(int const n) noexcept {
    std::random_device rd;
    std::array<int, std::mt19937::state_size> seed_data{};
    generate(begin(seed_data), end(seed_data), ref(rd));
    std::seed_seq seq(begin(seed_data), end(seed_data));

    auto mtgen = std::mt19937{seq};
    auto ud = std::uniform_int_distribution<>{0, 255};

    return ranges::views::iota(0, n)
            | ranges::views::transform([&](u8) { return static_cast<u8>(ud(mtgen)); })
            | ranges::to<std::vector>();
}

/// Utworzenie reprezentacji tekstowej bajtów (HEX | DEC).
/// \param data - wektor bajtów
/// \param format format prezentacji bajtów - HEX | DEC - domyślnie HEX
/// \return string z bajtami
std::string share::
bytes_as_str(std::vector<u8> const &data, BytesFormat const fmt) noexcept {
    auto const formatter = [fmt] (u8 c) {
        return (fmt == BytesFormat::HEX)
                ? fmt::format("0x{:02x}", c)
                : fmt::format("{}", c);
    };
    auto const components = data
            | ranges::views::transform([formatter](u8 c) { return formatter(c); })
            | ranges::to<std::vector>();

    return join_strings(components, ',');
}

/// Konwersja tekstu na wektor.
std::vector<char> share::
str2vec(std::string_view text) noexcept {
    return text
            | ranges::views::transform([](char c) { return c; })
            | ranges::to<std::vector>();
}

/// Konwersja wektora na tekst,
std::string share::
vec2str(std::span<char> vec) noexcept {
    return vec
            | ranges::views::transform([](char c) { return c; })
            | ranges::to<std::string>();
}
