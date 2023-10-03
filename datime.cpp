#include "datime.h"
#include "share.h"
using namespace std;

datime_t::datime_t(std::string const &text) {
    if (auto components = split_date_time(text); components.has_value()) {
        auto [date, time] = std::move(components.value());
        if (auto dtc = split_date(date); dtc) {
            if (auto tmc = split_time(time); tmc) {
                auto [year, month, day] = std::move(dtc.value());
                auto [hour, min, sec] = std::move(tmc.value());

                auto const utc = tp_from_components(
                        {year, month, day},
                        {hour, min, sec});
                tp_ = utc;
//                tp_ = std::chrono::system_clock::from_time_t(utc);
//                    tp_ = tms2tp(utc);
            }
        }
    }
}

datime_t datime_t::add_days(int const n) const noexcept {
    auto const days = chrono::floor<chrono::days>(tp_);
    chrono::hh_mm_ss const hms{tp_ - days};
    auto added = days + chrono::days(n);
    auto const secs = chrono::floor<chrono::seconds>(added)
                      + chrono::hours{hms.hours()}
                      + chrono::minutes{hms.minutes()}
                      + chrono::seconds{hms.seconds()};
    return datime_t{secs.time_since_epoch().count()};
}

optional<date_components_t> datime_t::split_date(std::string const &text) noexcept {
    auto components = share::split(text, DATE_DELIMITER);
    if (components.size() == 3)
        if (int year; share::to_int(components[0], year))
            if (int month; share::to_int(components[1], month))
                if (int day; share::to_int(components[2], day))
                    return std::move(make_tuple(year, month, day));
    return {};
}

optional<time_components_t> datime_t::split_time(std::string const &text) noexcept {
    auto components = share::split(text, TIME_DELIMITER);
    if (components.size() == 3)
        if (int hour; share::to_int(components[0], hour))
            if (int min; share::to_int(components[1], min))
                if (int sec; share::to_int(components[2], sec))
                    return std::move(make_tuple(hour, min, sec));
    return {};
}

optional<pair<string, string>> datime_t::split_date_time(std::string const &text) noexcept {
    auto components = share::split(text, DATE_TIME_DELIMITER);
    if (components.size() == 2)
        return std::move(make_pair(std::move(components[0]), std::move(components[1])));
    return {};
}


