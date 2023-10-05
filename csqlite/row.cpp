#include "row.h"
using namespace std;

pair<names_t, values_t>
row_t::split() const noexcept {
    names_t names{};
    values_t values{};

    if (!data_.empty()) {
        auto const n = data_.size();
        names.reserve(n);
        values.reserve(n);

        for (auto const& [name, value]: data_) {
            names.push_back(name);
            values.push_back(value);
        }
    }
    return {std::move(names), std::move(values)};
}

ostream& operator<<(ostream& s, field_t const& f) {
    auto const& [name, value] = f;

    s << name << ":(";
    switch (value.index()) {
        case 0:
            s << "NULL";
            break;
        case 1:
            s << get<1>(value);
            break;
        case 2:
            s << get<2>(value);
            break;
        case 3:
            s << get<3>(value);
            break;
        case 4:
            s << '[' << share::bytes_as_str(get<4>(value)) << ']';
    }
    s << ')';
    return s;
}

ostream& operator<<(ostream& s, row_t const& r) {
    auto const prev = std::prev(r.cend());
    auto it = r.cbegin();
    for (; it != prev; )
        s << *it++ << ", ";
    s << *it;

    return s;
}