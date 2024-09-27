// MIT License
//
// Copyright (c) 2023 Piotr Pszczółkowski
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#pragma once
#include <cstdint>
#include <filesystem>
#include <vector>
#include <string>
#include <numeric>
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
    /// Zamiana liczby całkowitej na tekst, w którym grupy tysięcy \n
    /// oddzielone są separatorem.
    /// \param v - liczba całkowita dowolnego rozmiaru (+/-)
    /// \return tekst reprezentujący przysłaną liczbę.
    static std::string number2str(std::integral auto v, char separator = '\'') noexcept {
        auto const text = std::to_string(static_cast<int64_t>(v));
        ssize_t const size = text.size();
        std::vector<char> buffer;
        buffer.reserve(size + size/3);
        auto offset = 0;

        if (v < 0) {
            offset = 1;
            buffer.push_back('-');
        }
        auto comma_idx = 3 - ((size - offset) % 3);
        if (comma_idx == 3)
            comma_idx = 0;

        for (ssize_t i = offset; i < size; i++) {
            if (comma_idx == 3) {
                buffer.push_back(separator);
                comma_idx = 0;
            }
            comma_idx++;
            buffer.push_back(text[i]);
        }

        return std::string{buffer.data(), buffer.size()};
    }

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
    static inline std::string_view trimv_left(std::string_view sv) noexcept {
        sv.remove_prefix(
                std::distance(
                        sv.cbegin(),
                        std::find_if(sv.cbegin(), sv.cend(), is_not_space)
                ));
        return sv;
    }

    static inline size_t new_line_count(std::string_view sv) noexcept {
        // Lepiej policzyć delimitery niż później realokować wektor.
        return std::accumulate(
                sv.cbegin(),
                sv.cend(),
                ssize_t{0},
                [](ssize_t const count, char const c) {
                    return (c == '\n') ? count + 1 : count;
                }
        );
    }

    /// Usunięcie zamykającyh (końcowych) białych znaków (z prawej strony).
    /// \param s - string z którego należy usunąć białe znaki
    /// \return string bez zamykających białych znaków.
    static std::string trim_right(std::string s) noexcept;
    static std::string_view trimv_right(std::string_view sv) noexcept;

    /// Usunięcie wiodących i zamykających białych znaków (z obu stron).
    /// \param s - string z którego należy usunąć białe znaki
    /// \return string bez wiodących i zamykających białych znaków.
    static std::string trim(std::string s) noexcept;
    static inline std::string_view trimv(std::string_view sv) noexcept {
        return trimv_left(trimv_right(sv));
    }

    /// Podział przysłanego stringa na wektor stringów. \n
    /// Wyodrębnianie stringów składowych odbywa się po napotkaniu delimiter'a.
    /// \param text - string do podziału,
    /// \param delimiter - znak sygnalizujący podział,
    /// \return Wektor stringów.
    static std::vector<std::string> split(std::string const& text, char const delimiter) noexcept;
    static std::vector<std::string_view> splitv(std::string_view text, char const delimiter) noexcept;

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
