#include "row.h"
#include <format>
using namespace std;

std::pair<std::vector<std::string>, std::vector<value_t>>
row_t::split() noexcept {
    if (data_.empty())
        return make_pair(vector<string>{}, vector<value_t>{});

    auto const n = data_.size();
    vector<string> names{};
    vector<value_t> values{};
    names.reserve(n);
    values.reserve(n);

    for (auto [name, value] : data_) {
        names.push_back(std::move(name));
        values.push_back(std::move(value));
    }

    return make_pair(std::move(names), std::move(values));
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
            s << '[' << bytes_as_string(get<4>(value)) << ']';
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