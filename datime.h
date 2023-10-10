#pragma once
#include <cstdint>
#include <chrono>
#include <ctime>
#include <format>
#include <vector>
#include <string>
#include <optional>
#include <iostream>
#include <string_view>

using u8 = uint8_t;
using i64 = int64_t;

struct dt_t {
    int y, m, d;
    bool operator==(dt_t const& rhs) const noexcept {
        return y == rhs.y && m == rhs.m && d == rhs.d;
    }
    bool operator!=(dt_t const& rhs) const noexcept {
        return !operator==(rhs);
    }
    bool operator<(dt_t const& rhs) const noexcept {
        if (y < rhs.y) return true;
        if (m < rhs.m) return true;
        if (d < rhs.d) return true;
        return false;
    }
    bool operator>(dt_t const& rhs) const noexcept {
        if (y > rhs.y) return true;
        if (m > rhs.m) return true;
        if (d > rhs.d) return true;
        return false;
    }

};
struct tm_t {
    int h{}, m{}, s{};
};

namespace chr = std::chrono;
using tp_sec_t = std::chrono::time_point<std::chrono::local_t, std::chrono::seconds>;
using uint = unsigned int;
using date_components_t = std::tuple<int, int, int>;
using time_components_t = std::tuple<int, int, int>;

class datime_t {
    // Precondition: z punktu widzenia użytkownika data i czas są w formcie "2023-12-21 19:31:44"
    inline static char const DATE_TIME_DELIMITER{' '};
    inline static char const DATE_DELIMITER{'-'};
    inline static char const TIME_DELIMITER{':'};

    tp_sec_t tp_;

public:
    /// Tworzy obiekt dla aktualnego (now) punktu w czasie.
    datime_t() {
        auto const zone = chr::locate_zone("Europe/Warsaw");
        auto now = chr::system_clock::now();
        auto local = chr::zoned_time(zone, now).get_local_time();
        tp_ = std::chrono::floor<std::chrono::seconds>(local);
    }
    /// Tworzy obiekt dla wskazanego timestamp (liczba sekund od początku epoki).
    explicit datime_t(i64 const tms) : tp_{tms2tp(tms)} {}
    /// Tworzy obiekt dla daty i czasu przekazanych jako tekst.
    explicit datime_t(std::string const &str);
    /// Tworzy obiekt dla podanych komponentów daty i czasu (LOCAL).
    explicit datime_t(dt_t date, tm_t time = {}) : tp_{tp_from_components(date, time)} {}

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

    bool operator<(datime_t const& rhs) const noexcept {
        return tp_ < rhs.tp_;
    }
    bool operator>(datime_t const& rhs) const noexcept {
        return tp_ > rhs.tp_;
    }

    [[nodiscard]] i64 minutes_from(datime_t const& rhs) const noexcept {
        auto const a =  chr::floor<chr::minutes>(tp_);
        auto const b = chr::floor<chr::minutes>(rhs.tp_);
        return (b - a).count();
    }

    /// Wyznacza i zwraca liczbę sekund od początku epoki (1.01.1970).
    /// \return liczba sekund od początku epoki.
    [[nodiscard]] i64 timestamp() const noexcept {
        auto const n = chr::floor<chr::seconds>(tp_).time_since_epoch().count();
        return static_cast<i64>(n);
    }

    /// Ustawienie czasu w istniejącym obiekcie.
    /// \remark Przekazany czas jest w LOCAL.
    /// \param tm - komponenty czasu (struktura tm_t).
     [[maybe_unused]] datime_t&
     set_time(tm_t tm) noexcept {
        auto days = chr::floor<chr::days>(tp_);
        tp_ = days
              + chr::hours{tm.h}
              + chr::minutes{tm.m}
              + chr::seconds{tm.s};
        return *this;
    }

    /// Wyzerowanie liczby sekund (minuty zaokrąglamy). Interesuje na tylko godzina i minuty.
    /// \return referencja do tego obiektu.
    [[maybe_unused]] datime_t&
    clear_seconds() noexcept {
        auto const days = chr::floor<chr::days>(tp_);
        chr::hh_mm_ss const hms{tp_ - days};
        tp_ = days
              + chr::hours{hms.hours()}
              + chr::minutes{hms.minutes()}
              + chr::seconds{hms.seconds().count() > 30 ? 60 : 0};
        return *this;
    }

    [[maybe_unused]] datime_t&
    clear_time() noexcept {
        auto const days = chr::floor<chr::days>(tp_);
        chr::hh_mm_ss const hms{tp_ - days};
        tp_ = days + chr::hours{0} + chr::minutes{0} + chr::seconds{0};
        return *this;
    }
    [[maybe_unused]] datime_t&
    beginning_day() noexcept {
        return clear_time();
    }
    [[maybe_unused]] datime_t&
    end_day() noexcept {
        return set_time({23, 59, 59});
    }

    [[nodiscard]] std::tuple<int, int, int>
    date_compnents() const noexcept {
        chr::year_month_day const ymd{chr::floor<chr::days>(tp_)};
        auto year = static_cast<int>(ymd.year());
        auto month = static_cast<int>(static_cast<uint>(ymd.month()));
        auto day = static_cast<int>(static_cast<uint>(ymd.day()));
        return {year, month, day};
    }
    [[nodiscard]] bool
    same_day(datime_t const& rhs) const noexcept {
        return date_compnents() == rhs.date_compnents();
    }

    [[nodiscard]] std::tuple<int, int, int>
    time_compnents() const noexcept {
        chr::hh_mm_ss const hms{tp_ - chr::floor<chr::days>(tp_)};
        auto const hour = static_cast<int>(hms.hours().count());
        auto const minutes = static_cast<int>(hms.minutes().count());
        auto const seconds = static_cast<int>(hms.seconds().count());
        return {hour, minutes, seconds};
    }

    /// Zwraca komponenty daty i czasu (LOCAL).
    /// \return para(pair) struktur z danymi (dt_t i tm_t).
    [[nodiscard]] std::pair<dt_t, tm_t>
    components() const noexcept {
        auto const days = chr::floor<chr::days>(tp_);

        chr::year_month_day const ymd{days};
        auto const year = static_cast<int>(ymd.year());
        auto const month = static_cast<int>(static_cast<uint>(ymd.month()));
        auto const day = static_cast<int>(static_cast<uint>(ymd.day()));
        dt_t const date{year, month, day};

        chr::hh_mm_ss const hms{tp_ - days};
        auto const hour = static_cast<int>(hms.hours().count());
        auto const minutes = static_cast<int>(hms.minutes().count());
        auto const seconds = static_cast<int>(hms.seconds().count());
        tm_t const time{hour, minutes, seconds};

        return {date, time};
    }

    /// Wyznacznie nowej daty oddalonej o podaną liczbę dni.
    /// \param n - liczba dni (dodatnia lub ujemna)
    /// \return nowa data oddalona o podaną liczbę dni (data w tym obiekcie pozostaje bez zmian).
    [[nodiscard]] datime_t
    add_days(int n) const noexcept;

    /// Wyznaczenie daty 'jutra' względem daty tego obiektu.
    /// \return data dnia 'jutrzejszego' względem daty tego obiektu.
    [[nodiscard]] datime_t
    next_day() const noexcept { return add_days(1); }

    /// Wyznaczenie daty 'wczoraj' względem daty tego obiektu.
    /// \return data dnia 'wczorajszego' zględem daty tego obiektu.
    [[nodiscard]] datime_t
    prev_day() const noexcept { return add_days(-1); }

    /// Wyznaczenie indeksu dnia w tygodniu dla daty tego obiektu.
    /// \remark 1:poniedziałek, ... , 7:niedziela
    /// \return indeks dnia w tygodniu.
    [[nodiscard]] int
    week_day() const noexcept {
        auto const wd = std::chrono::weekday{chr::floor<chr::days>(tp_)};
        return static_cast<int>(wd.iso_encoding());
    }

    /// Wyznaczenie daty początku i końca tygodnia, w którym zawiera się dzień daty obiektu.
    /// \return data początku i końca tygodnia.
    [[nodiscard]] std::pair<datime_t, datime_t>
    week_range() const noexcept {
        auto const today_idx = week_day();
        return {add_days(-today_idx + 1), add_days(7 - today_idx)};
    }

    /// Data i czas obiektu (LOCAL) jako tekst (z doładnością do sekund).
    /// \return data i czas jako tekst.
    [[nodiscard]] auto str() const noexcept {
        return std::format("{}", tp_);
    }

private:
    /// Podział tekstu daty i wyznaczenie jej liczbowych wartości.
    /// \param text - string z tekstową reprezentacją daty (np. "2023-09-28").
    /// \return opcjonalne(optional) trzy(tuple) składowe daty (year, month, day) jako liczby.
    static std::optional<date_components_t> split_date(std::string const& text) noexcept;

    /// Podział tekstu czasu i wyznaczenie jego numerycznych wartości.
    /// \param text - string z tekstową reprezentacją czasu (np. "10:21:59")
    /// \return opcjonalny(optional) trzy(tuple) składowe czasu (hour, min, sec) jako liczby .
    static std::optional<time_components_t> split_time(std::string const& text) noexcept;

    /// Podział stringu zawierającego datę i czas na stringi daty i czasu.
    /// \param text - tekst do podziału
    /// \return opcjonalna(optional) para(pair) stringów dla daty i czasu.
    static std::optional<std::pair<std::string, std::string>>
    split_date_time(std::string const &text) noexcept;

    /// Wyznaczenie momentu w czasie (time-point) przysłanych komponentów daty i czasu.
    /// \remark Zakłada się że przysłane komponenty dotyczą czasu lokalnego
    /// \param date - struktura zawierąca komponenty daty {y, m, d},
    /// \param time - struktura zawierająca komponenty czasu {h, m, s}
    /// \return lokalny moment w czasie (time-point)
    static tp_sec_t tp_from_components(dt_t date, tm_t time) noexcept {
        chr::year_month_day const ymd = chr::year(date.y) / date.m / date.d;
        auto const days = static_cast<chr::local_days>(ymd);
        return days + chr::hours(time.h) + chr::minutes(time.m) + chr::seconds(time.s);
    }

    /// Wyznacza moment w czasie (time-point) dla przysłanego timestampa.
    /// \param timestamp (liczba sekund od początku epoki).
    /// \return lokalny moment w czasie (time-point) odpowiadanjący timestampowi.
    static tp_sec_t tms2tp(i64 const tms) noexcept {
        // wyznaczamy utc time-point
        auto const utc = std::chrono::system_clock::from_time_t(static_cast<time_t>(tms));
        auto const days_utc = chr::floor<chr::days>(utc);
        // u wydzielamy czas
        chr::hh_mm_ss hms_utc{utc - days_utc};

        // wyznaczamy local time-point
        auto const zone = chr::locate_zone("Europe/Warsaw");
        auto local = chr::zoned_time(zone, utc).get_local_time();

        // do obcietego do dni lokalnego time-pointa dodajemy czas
        return chr::floor<chr::days>(local)
               + chr::hours{hms_utc.hours()}
               + chr::minutes{hms_utc.minutes()}
               + chr::seconds {hms_utc.seconds()};
    }
};
