#pragma once
#include <iostream>
#include <string>
#include <cstdint>
#include <sstream>
#include <date/date.h>
#include <date/tz.h>
#include <fmt/core.h>
#include <fmt/chrono.h>

using i64 = int64_t;
//using tplocal_t = std::chrono::time_point<date::local_t, std::chrono::duration<i64>>;
using zoned_time_t = date::zoned_time<std::chrono::seconds>;

struct dt_t {
    int y{}, m{}, d{};
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

class daytime_t final {
    date::time_zone const* zone = date::locate_zone("Europe/Warsaw");
    zoned_time_t tp_{};
public:
    /// Data-czas teraz (now).
    daytime_t() {
        auto const now = std::chrono::system_clock::now();
        tp_ = date::make_zoned(zone, std::chrono::floor<std::chrono::seconds>(now));
    }
    /// Data-czas z timestampu (liczba sekund od początku epoki).
    /// \param timestamp - liczba sekund od początku epoki.
    explicit daytime_t(i64 const timestamp) {
        auto const tp = std::chrono::system_clock::from_time_t(timestamp);
        tp_ = date::make_zoned(zone, std::chrono::floor<std::chrono::seconds>(tp));
    }
    explicit daytime_t(zoned_time_t const tp) : tp_{tp} {
    }

    /// Data-czas z tekstu (np. 2023-10-23 11:06:21).
    /// \param str - string z datą i godziną
    explicit daytime_t(std::string const& str) {
        std::stringstream ss{str};
        date::local_time<std::chrono::seconds> tmp;
        date::from_stream(ss, "%F %X", tmp);
        tp_ = make_zoned(zone, tmp);
    }
    /// Data-czas z komponentów.
    explicit daytime_t(dt_t const dt, tm_t const tm)
    : tp_{from_components(dt, tm)}
    {}

    // Kopiowanie i przekazywanie - domyślne
    daytime_t(daytime_t const&) = default;
    daytime_t& operator=(daytime_t const&) = default;
    daytime_t(daytime_t&&) = default;
    daytime_t& operator=(daytime_t&&) = default;
    ~daytime_t() = default;

    /// Sprawdza czy wskazany obiekt określa ten sam punkt w czasie.
    /// \return True jeśli punkty w czasie są takie same, False w przeciwnym przepadku.
    bool operator==(daytime_t const &rhs) const noexcept {
        return timestamp() == rhs.timestamp();
    }
    /// Sprawdza czy wskazany obiekt określa inny punkt w czasie
    /// \return True jeśli punkty w czasie są różne, False w przeciwnym przypadku.
    bool operator!=(daytime_t const &rhs) const noexcept {
        return !operator==(rhs);
    }
    /// Sprawdza czy wskazana data-czas (rhs) jest późniejsza.
    /// \return True jeśli rhs jest późniejszy, False w przeciwnym przypadku,
    bool operator<(daytime_t const& rhs) const noexcept {
        return timestamp() < rhs.timestamp();
    }
    /// Sprawdza czy wskazana data-czas (rhs) jest wcześniejsza.
    /// \return True jeśli rhs jest wcześniejszy, False w przeciwnym przypadku.
    bool operator>(daytime_t const& rhs) const noexcept {
        return timestamp() > rhs.timestamp();
    }

    /// Zwraca różnicę jako liczbę minut.
    [[nodiscard]] i64
    minutes_from(daytime_t const& rhs) const noexcept {
        auto const a = date::floor<std::chrono::minutes>(tp_.get_sys_time());
        auto const b = date::floor<std::chrono::minutes>(rhs.tp_.get_sys_time());
        return (b - a).count();
    }
    /// Obliczenie timestampu (liczba sekund od początku epoki).
    /// \return timestamp
    [[nodiscard]] i64
    timestamp() const noexcept {
        auto const seconds = tp_.get_sys_time().time_since_epoch().count();
        return static_cast<i64>(seconds);
    }

    /// Zmiana czasu na podany.
    daytime_t& set_time(tm_t const tm) noexcept {
        namespace chrono = std::chrono;
        auto t = date::floor<chrono::days>(tp_.get_local_time())
                + chrono::hours(tm.h)
                + chrono::minutes(tm.m)
                + chrono::seconds(tm.s);
        tp_ = make_zoned(zone, t);
        return *this;
    }
    /// Wyzerowanie sekund z ewentualnym zaokrągleniem minut.
    daytime_t& clear_seconds() noexcept {
        namespace chrono = std::chrono;
        auto const days = date::floor<chrono::days>(tp_.get_local_time());
        date::hh_mm_ss hms{tp_.get_local_time() - days};
        auto t = date::floor<chrono::days>(tp_.get_local_time())
                + hms.hours()
                + hms.minutes()
                + chrono::seconds(hms.seconds().count() >= 30 ? 60 : 0);
        tp_ = make_zoned(zone, t);
        return *this;
    }
    /// Wyzerowanie czasu.
    daytime_t& clear_time() noexcept {
        namespace chrono = std::chrono;
        auto t = date::floor<chrono::days>(tp_.get_local_time())
                + chrono::hours(0)
                + chrono::minutes(0)
                + chrono::seconds(0);
        tp_ = make_zoned(zone, t);
        return *this;
    }
    /// Początek dnia dla daty.
    daytime_t& beginning_day() noexcept {
        return clear_time();
    }
    /// Koniec dnia dla daty.
    daytime_t& end_day() noexcept {
        return set_time({23,59,59});
    }
    /// Wyznaczenie składników daty (bez czasu).
    [[nodiscard]] dt_t
    date_components() const noexcept {
        namespace chrono = std::chrono;
        auto days = date::floor<chrono::days>(tp_.get_local_time());
        date::year_month_day ymd{days};
        auto const year = static_cast<int>(ymd.year());
        auto const month = static_cast<int>(static_cast<unsigned>(ymd.month()));
        auto const day = static_cast<int>(static_cast<unsigned>(ymd.day()));
        return {year, month, day};
    }
    /// Sprawdzenie czy to ten sam dzień.
    [[nodiscard]] bool
    is_same_day(daytime_t const& rhs) const noexcept {
        return date_components() == rhs.date_components();
    }
    /// Wyznaczenie składników czasu (bez daty).
    [[nodiscard]] tm_t
    time_components() const noexcept {
        namespace chrono = std::chrono;
        auto const days = date::floor<chrono::days>(tp_.get_local_time());
        date::hh_mm_ss const hms{tp_.get_local_time() - days};
        auto hour = static_cast<int>(hms.hours().count());
        auto min = static_cast<int>(hms.minutes().count());
        auto sec = static_cast<int>(hms.seconds().count());
        return {hour, min, sec};
    }
    /// Wyznaczenie składników daty i czasu.
    [[nodiscard]] std::tuple<dt_t, tm_t>
    components() const noexcept {
        namespace chrono = std::chrono;
        auto const days = date::floor<chrono::days>(tp_.get_local_time());

        date::year_month_day const ymd{days};
        auto const year = static_cast<int>(ymd.year());
        auto const month = static_cast<int>(static_cast<unsigned>(ymd.month()));
        auto const day = static_cast<int>(static_cast<unsigned>(ymd.day()));
        dt_t const dt{year, month, day};

        date::hh_mm_ss const hms{tp_.get_local_time() - days};
        auto hour = static_cast<int>(hms.hours().count());
        auto min = static_cast<int>(hms.minutes().count());
        auto sec = static_cast<int>(hms.seconds().count());
        tm_t const tm{hour, min, sec};

        return {dt, tm};
    }

    /// Utworzenie nowej data-czasu przez dodanie wskazanej liczby dni.
    /// \param n - liczba dnie do dodania (może być ujemna)
    /// \return nowa data-czas
    [[nodiscard]] daytime_t
    add_days(int const n) const noexcept {
        namespace chrono = std::chrono;
        auto const days = date::floor<chrono::days>(tp_.get_local_time());
        date::hh_mm_ss const hms{tp_.get_local_time() - days};
        auto const added = days + chrono::days(n);
        auto const secs = chrono::floor<chrono::seconds>(added)
                + hms.hours()
                + hms.minutes()
                + hms.seconds();
        return daytime_t(make_zoned(zone, secs));
    }

    [[nodiscard]] daytime_t
    next_day() const noexcept {
        return add_days(1);
    }
    [[nodiscard]] daytime_t
    prev_day() const noexcept {
        return add_days(-1);
    }
    [[nodiscard]] int
    week_day() const noexcept {
        namespace chrono = std::chrono;
        auto const wd = date::weekday(chrono::floor<chrono::days>(tp_.get_local_time()));
        return wd.iso_encoding();
    }
    [[nodiscard]] std::tuple<daytime_t, daytime_t>
    week_range() const noexcept {
        auto const today_idx = week_day();
        return {add_days(-today_idx + 1), add_days(7 - today_idx)};
    }

    /// Data-czas w postaci tekstu (LOCAL)
    /// \return string z datą-czasem.
    [[nodiscard]] std::string
    str() const noexcept {
        std::stringstream ss{};
        ss << tp_.get_local_time();
        return ss.str();
    }
private:
    [[nodiscard]] zoned_time_t
    from_components(dt_t const dt, tm_t const tm) noexcept {
        namespace chrono = std::chrono;
        date::year_month_day const ymd = date::year(dt.y) / dt.m / dt.d;
        auto const days = static_cast<date::local_days>(ymd);
        auto t = days + chrono::hours(tm.h) + chrono::minutes(tm.m) + chrono::seconds(tm.s);
        return make_zoned(zone, t);
    }
};
