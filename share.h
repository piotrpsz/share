#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <format>
#include <random>
#include <sstream>
#include <chrono>
#include <iostream>
#include <range/v3/all.hpp>

using u8 = uint8_t;
using u32 = uint32_t;
using i64 = int64_t;
using f32 = float;
using f64 = double;
using uint = unsigned int;


enum class BytesFormat {
    DEC, HEX
};

class share final {
public:
    static i64 user_id_;
public:
    /// Sprawdzenie czy przysłany znak nie(!) jest białym znakiem.
    /// \param c - znak do sprawdzenia
    /// \return True jeśli NIE jest białym znakiem, False w przeciwnym przypadku.
    static inline bool is_not_space(char c) noexcept {
        return !std::isspace(c);
    }

    /// Obcięcie wiodących (z początku) białych znaków.
    /// \param s - string z którego należy usunąć białe znaki
    /// \return string bez wiodących białych znaków.
    static inline std::string trim_left(std::string s) noexcept {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), is_not_space));
        return std::move(s);
    }

    /// Usunięcie zamykającyh (końcowych) białych znaków (z prawej strony).
    /// \param s - string z którego należy usunąć białe znaki
    /// \return string bez zamykających białych znaków.
    static inline std::string trim_right(std::string s) noexcept {
        s.erase(std::find_if(s.rbegin(), s.rend(), is_not_space).base(), s.end());
        return std::move(s);
    }

    /// Usunięcie wiodących i zamykających białych znaków (z obu stron).
    /// \param s - string z którego należy usunąć białe znaki
    /// \return string bez wiodących i zamykających białych znaków.
    static inline std::string trim(std::string s) noexcept {
        return trim_left(trim_right(std::move(s)));
    }

    /// Podział przysłanego stringa na wektor stringów. \n
    /// Wyodrębnianie stringów składowych odbywa się po napotkaniu delimiter'a.
    /// \param text - string do podziału,
    /// \param delimiter - znak sygnalizujący podział,
    /// \return Wektor stringów.
    static std::vector<std::string> split(std::string const &text, char const delimiter) noexcept {
        // Lepiej policzyć delimitery niż później realokować wektor.
        auto const n = std::accumulate(
                text.cbegin(),
                text.cend(),
                uint{0},
                [delimiter](uint const count, char const c) {
                    return c == delimiter ? count + 1 : count;
                }
        );

        std::vector<std::string> tokens{};
        tokens.reserve(n + 1);

        std::string token;
        std::istringstream stream(text);
        while (std::getline(stream, token, delimiter))
            if (auto str = trim(token); !str.empty())
                tokens.push_back(str);
        return std::move(tokens);
    }

    /// Tworzy string będący złączeniem stringów przysłanych w wektorze. \n
    /// Łączone stringi rozdzielone są przysłanym delimiter'em.
    /// \param data - wektor stringów do połączenia,
    /// \param delimiter - znak wstawiany pomiędzy łączonymi stringami (domyślnie przecinek).
    /// \return String jako suma przysłanych stringów.
    static inline std::string join_strings(std::vector<std::string> data, char const delimiter = ',') noexcept {
        return std::move(
                std::accumulate(
                        std::next(data.begin()),
                        data.end(),
                        data[0],
                        [delimiter](std::string a, std::string b) {
                            return std::format("{}{}{}", std::move(a), delimiter, std::move(b));
                        }
                ));
    }

    /// Zamiana tekstu na liczbę typu integer.
    /// \param text - tekstowa reprezentacja liczby,
    /// \param v - referencja do zmiennej typu integer, do której zostanie przekazana wyznaczona wartość,
    /// \param base - system numeryczny użyty w tekście (domyślnie 10)
    /// \return true jeśli wszystko poszło dobrze, w przeciwnym przypadku false.
    static bool to_int(std::string_view text, int &v, int base = 10) noexcept {
        auto [ptr, err] = std::from_chars(text.data(), text.data() + text.size(), v, base);
        return err == std::errc{};
    }

    /// Utworzenie wektora losowych bajtów.
    /// \param n - oczekiwana liczba bajtów.
    /// \return Wektor losowych bajtów.
    static std::vector<u8> random_bytes(int const n) noexcept {
        std::random_device rd;
        std::array<int, std::mt19937::state_size> seed_data{};
        generate(begin(seed_data), end(seed_data), ref(rd));
        std::seed_seq seq(begin(seed_data), end(seed_data));

        auto mtgen = std::mt19937{seq};
        auto ud = std::uniform_int_distribution<>{0, 255};

        using namespace ranges;
        auto bytes_collection =
                views::iota(0, n)
                | views::transform([&](u8 i) {
                    return static_cast<u8>(ud(mtgen));
                })
                | ranges::to<std::vector>();

        return std::move(bytes_collection);
    }


    /// Utworzenie reprezentacji tekstowej bajtów (HEX | DEC).
    /// \param wektor bajtów
    /// \param format wyświetlania bajtów (HEX | DEC), domyślnie HEX
    /// \return string z bajtami
    static std::string bytes_as_str(std::vector<u8> const &data, BytesFormat const fmt = BytesFormat::HEX) noexcept {
        using namespace ranges::views;

        if (fmt == BytesFormat::HEX) {
            auto const components =
                    data
                    | transform([](u8 v) { return std::format("0x{:02x}", v); })
                    | ranges::to<std::vector>();
            return join_strings(components, ',');
        }

        auto const components =
                data
                | transform([](u8 v) { return std::format("{}", v); })
                | ranges::to<std::vector>();
        return join_strings(components, ',');
    }

    template<typename Fn, typename... Args>
    static inline std::string execution_timer(Fn fn, Args&&... args, unsigned n = 1000) {
        auto start = std::chrono::steady_clock::now();
        for (auto i = 0; i < n; i++)
            fn(std::forward<Args>(args)...);
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> const elapsed = end - start;
        return std::format("{}s", elapsed.count()/n);
    }
};



