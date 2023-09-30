#pragma once

#include <cstdint>
#include <chrono>
#include <ctime>
#include <format>
#include <vector>
#include <string>
#include <optional>

using u8 = uint8_t;
using i64 = int64_t;

struct dt_t {
    int y, m, d;
};
struct tm_t {
    int h{}, m{}, s{};
};

namespace chrono = std::chrono;
using time_point_t = chrono::time_point<chrono::system_clock>;
using date_components_t = std::tuple<int, int, int>;
using time_components_t = std::tuple<int, int, int>;

class datime_t {
    // Precondition: z punktu widzenia użytkownika data i czas są w formcie "2023-12-21 19:31:44"
    inline static char const DATE_TIME_DELIMITER{' '};
    inline static char const DATE_DELIMITER{'-'};
    inline static char const TIME_DELIMITER{':'};

    time_point_t tp_;

public:
    /// Tworzy obiekt dla aktualnego (now) punktu w czasie.
    datime_t() : tp_{chrono::floor<chrono::seconds>(chrono::system_clock::now())} {}

    /// Tworzy obiekt dla wskazanego timestamp (liczba sekund od początku epoki).
    explicit datime_t(i64 ts) : tp_{std::chrono::system_clock::from_time_t(static_cast<time_t>(ts))} {}

    /// Tworzy obiekt dla daty i czasu przekazanych jako tekst.
    explicit datime_t(std::string const &str);

    /// Tworzy obiekt dla podanych komponentów daty i czasu (LOCAL).
    explicit datime_t(dt_t date, tm_t time = {}) {
        auto const utc = utc_time_from_components(date, time);
        tp_ = std::chrono::system_clock::from_time_t(utc);
    }

    /// Domyślne kopiowanie i przenoszenie.
    datime_t(datime_t const &) = default;

    datime_t &operator=(datime_t const &) = default;

    datime_t(datime_t &&) = default;

    datime_t &operator=(datime_t &&) = default;

    ~datime_t() = default;

    /// Sprawdza czy wskazany obiekt określa ten sam punkt w czasie.
    /// \return True jeśli punkty w czasie są takie same, False w przeciwnym przepadku.
    bool operator==(datime_t const &rhs) const noexcept {
        return tp_ == rhs.tp_;
    }

    /// Sprawdza czy wskazany obiekt określa inny punkt w czasie
    /// \return True jeśli punkty w czasie są różne, False w przeciwnym przypadku.
    bool operator!=(datime_t const &rhs) const noexcept {
        return !operator==(rhs);
    }

    /// Ustawienie czasu w istniejącym obiekcie.
    /// \remark Przekazany czas jest w LOCAL.
    /// \param tm - komponenty czasu (struktura tm_t).
    void set_time(tm_t tm) noexcept {
        auto days = chrono::floor<chrono::days>(tp_);
        tp_ = days
              + chrono::hours{tm.h}
              + chrono::minutes{tm.m}
              + chrono::seconds{tm.s};
    }

    /// Wyzerowanie liczby sekund (minuty zaokrąglamy). Interesuje na tylko godzina i minuty.
    /// \return referencja do tego obiektu.
    datime_t& clear_seconds() noexcept {
        auto const days = chrono::floor<chrono::days>(tp_);
        chrono::hh_mm_ss const hms{tp_ - days};
        tp_ = days
              + chrono::hours{hms.hours()}
              + chrono::minutes{hms.minutes()}
              + chrono::seconds{hms.seconds().count() > 30 ? 60 : 0};
        return *this;
    }

    /// Zwraca komponenty daty i czasu (LOCAL).
    /// \return para(pair) struktur z danymi (dt_t i tm_t).
    [[nodiscard]] std::pair<dt_t, tm_t>
    components() const noexcept {
        auto const ts = timestamp();
        auto const tm = std::localtime(&ts);
        dt_t const date{tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday};
        tm_t const time{tm->tm_hour, tm->tm_min, tm->tm_sec};
        return std::move(std::make_pair(date, time));
    }

    /// Zwaraca date i czas obiektu w lokalnej reprezentacji (uwzgędnia zone).
    /// \return lokalny moment w czasie (time point)
    [[nodiscard]] chrono::time_point<chrono::local_t, chrono::seconds>
    local_time() const noexcept {
        auto zone = chrono::locate_zone("Europe/Warsaw");
        auto local = chrono::zoned_time(zone, tp_).get_local_time();
        return chrono::floor<chrono::seconds>(local);
    }

    /// Wyznacznie nowej daty oddalonej o podaną liczbę dni.
    /// \param n - liczba dni (dodatnia lub ujemna)
    /// \return nowa data oddalona o podaną liczbę dni (data w tym obiekcie pozostaje bez zmian).
    [[nodiscard]] datime_t add_days(int n) const noexcept;

    /// Wyznaczenie daty 'jutra' względem daty tego obiektu.
    /// \return data dnia 'jutrzejszego' względem daty tego obiektu.
    [[nodiscard]] datime_t next_day() const noexcept { return add_days(1); }

    /// Wyznaczenie daty 'wczoraj' względem daty tego obiektu.
    /// \return data dnia 'wczorajszego' zględem daty tego obiektu.
    [[nodiscard]] datime_t prev_day() const noexcept { return add_days(-1); }

    /// Wyznaczenie indeksu dnia w tygodniu dla daty tego obiektu.
    /// \remark 1:poniedziałek, ... , 7:niedziela
    /// \return indeks dnia w tygodniu.
    [[nodiscard]] int week_day() const noexcept {
        auto const wd = std::chrono::weekday{chrono::floor<chrono::days>(tp_)};
        return static_cast<int>(wd.iso_encoding());
    }

    /// Wyznaczenie daty początku i końca tygodnia, w którym zawiera się dzień daty obiektu.
    /// \return data początku i końca tygodnia.
    [[nodiscard]] std::pair<datime_t, datime_t> week_range() const noexcept {
        auto const today_idx = week_day();
        auto start = add_days(-today_idx + 1);
        auto end = add_days(7 - today_idx);
        return std::make_pair(start, end);
    }

    /// Data i czas obiektu (LOCAL) jako tekst (z doładnością do sekund).
    /// \return data i czas jako tekst.
    [[nodiscard]] auto str_local() const noexcept {
        return std::format("{}", local_time());
    }

    /// Data i czas obiektu (UTC) jako tekst (z dokładością do sekund)
    /// \remark jest to natywna dla obiektu reprezentacja bo używamy czasu UTC.
    /// \return data i czas jako tekst.
    [[nodiscard]] auto str_utc() const noexcept {
        return std::format("{}", chrono::floor<chrono::seconds>(tp_));
    }

    /// Wyznacza i zwraca liczbę sekund od początku epoki (1.01.1970).
    /// \return liczba sekund od początku epoki.
    [[nodiscard]] i64 timestamp() const noexcept {
        auto const n = floor<chrono::seconds>(tp_).time_since_epoch().count();
        return static_cast<i64>(n);
    }

private:
    /// Podział tekstu daty i wyznaczenie jej liczbowych wartości.
    /// \param text - string z tekstową reprezentacją daty (np. "2023-09-28").
    /// \return opcjonalne(optional) trzy(tuple) składowe daty (year, month, day) jako liczby.
    static std::optional<date_components_t> split_date(std::string const &text) noexcept;

    /// Podział tekstu czasu i wyznaczenie jego numerycznych wartości.
    /// \param text - string z tekstową reprezentacją czasu (np. "10:21:59")
    /// \return opcjonalny(optional) trzy(tuple) składowe czasu (hour, min, sec) jako liczby .
    static std::optional<time_components_t> split_time(std::string const &text) noexcept;

    /// Podział stringu zawierającego datę i czas na stringi daty i czasu.
    /// \param text - tekst do podziału
    /// \return opcjonalna(optional) para(pair) stringów dla daty i czasu.
    static std::optional<std::pair<std::string, std::string>>
    split_date_time(std::string const &text) noexcept;

    /// Wyznaczenie czasu UTC jako liczby sekund od początki epoki.
    /// \remark Zakłada się że przysłane komponenty dotyczą czasu lokalnego
    /// \param date - struktura zawierąca komponenty daty {year, month, day},
    /// \param time - struktura zawierająca komponenty czasu {hour, mim, sec}
    /// \return UTC timestamp
    static std::time_t utc_time_from_components(dt_t date, tm_t time) noexcept {
        struct std::tm tm{};
        tm.tm_year = date.y - 1900;
        tm.tm_mon = date.m - 1;
        tm.tm_mday = date.d;
        tm.tm_hour = time.h;
        tm.tm_min = time.m;
        tm.tm_sec = time.s;

        auto const local_time = std::mktime(&tm);
        auto const utc_tm = std::gmtime(&local_time);
        return std::mktime(utc_tm);
    }
};
