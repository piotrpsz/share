#pragma once
#include <cstdint>
#include <filesystem>
#include <vector>
#include <string>
#include <string_view>
#include <span>
#include <fmt/core.h>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using i64 = int64_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;
using uint = unsigned int;
template<typename T>
    using vec= std::vector<T>;
namespace fs = std::filesystem;

enum class BytesFormat {
    DEC, HEX
};

class share final {
public:
    /// Zamienia ciąg bajtów typu 'u8' na string.
    /// \param data - widok na ciągły zbór bajtów.
    /// \param n - liczba bajtów do użycia.
    /// \return string utworzony ze wskazanej liczby bajtów.
    static std::string as_string(std::span<u8> data, int n) noexcept;
    static std::string as_string(std::span<u8> data) noexcept;

    /// Sprawdzenie czy przysłany znak nie(!) jest białym znakiem.
    /// \param c - znak do sprawdzenia
    /// \return True jeśli NIE jest białym znakiem, False w przeciwnym przypadku.
    static bool is_not_space(char c) noexcept;

    /// Obcięcie wiodących (z początku) białych znaków.
    /// \param s - string z którego należy usunąć białe znaki
    /// \return string bez wiodących białych znaków.
    static std::string trim_left(std::string s) noexcept;

    /// Usunięcie zamykającyh (końcowych) białych znaków (z prawej strony).
    /// \param s - string z którego należy usunąć białe znaki
    /// \return string bez zamykających białych znaków.
    static std::string trim_right(std::string s) noexcept;

    /// Usunięcie wiodących i zamykających białych znaków (z obu stron).
    /// \param s - string z którego należy usunąć białe znaki
    /// \return string bez wiodących i zamykających białych znaków.
    static std::string trim(std::string s) noexcept;

    /// Podział przysłanego stringa na wektor stringów. \n
    /// Wyodrębnianie stringów składowych odbywa się po napotkaniu delimiter'a.
    /// \param text - string do podziału,
    /// \param delimiter - znak sygnalizujący podział,
    /// \return Wektor stringów.
    static std::vector<std::string> split(std::string const& text, char const delimiter) noexcept;
    static std::vector<std::string> split_view(std::string_view text, char const delimiter) noexcept;

    /// Tworzy string będący złączeniem stringów przysłanych w wektorze. \n
    /// Łączone stringi rozdzielone są przysłanym delimiter'em.
    /// \param data - wektor stringów do połączenia,
    /// \param delimiter - znak wstawiany pomiędzy łączonymi stringami (domyślnie przecinek).
    /// \return String jako suma przysłanych stringów.
    static std::string join_strings(std::vector<std::string> const& data, char delimiter = ',') noexcept;

    static inline std::string strview2str(std::string_view sv) noexcept {
        return {sv.data(), sv.size()};
    }

    /// Zamiana tekstu na liczbę typu integer.
    /// \param text - tekstowa reprezentacja liczby,
    /// \param v - referencja do zmiennej typu integer, do której zostanie przekazana wyznaczona wartość,
    /// \param base - system numeryczny użyty w tekście (domyślnie 10)
    /// \return true jeśli wszystko poszło dobrze, w przeciwnym przypadku false.
    static std::optional<int> to_int(std::string_view text, int base = 10);

    /// Utworzenie wektora losowych bajtów.
    /// \param n - oczekiwana liczba bajtów.
    /// \return Wektor losowych bajtów.
    static std::vector<u8> random_bytes(int n) noexcept;

    /// Utworzenie reprezentacji tekstowej bajtów (HEX | DEC).
    /// \param data - wektor bajtów
    /// \param format format prezentacji bajtów - HEX | DEC - domyślnie HEX
    /// \return string z bajtami
    static std::string bytes_as_str(std::vector<u8> const& data, BytesFormat fmt = BytesFormat::HEX) noexcept;

    /// Konwersja tekstu na wektor.
    static std::vector<char> str2vec(std::string_view text) noexcept;

    /// Konwersja wektora na tekst,
    static std::string vec2str(std::span<char> vec) noexcept;

    /// Funkcja opakowująca obiekt funkcyjny, dla której mierzymy czas wykonania.
    /// \param fn - obiekt funkcyjny dla którego mierzymy czas wykonania,
    /// \param args - parametry które należy przekazać do obiektu funkcyjnego,
    /// \param n - liczba wywołań obiektu funkcyjnego (domyślnie 1000).
    /// \return średni czas jednego wywołania obiektu funkcyjnego
    template<typename Fn, typename... Args>
    static inline std::string
    execution_timer(Fn fn, Args &&... args, unsigned n = 1000) {
        auto start = std::chrono::steady_clock::now();
        for (unsigned i = 0; i < n; i++)
            fn(std::forward<Args>(args)...);
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> const elapsed = end - start;
        return fmt::format("{}s", elapsed.count() / n);
    }
};
