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

        for (auto const& f: data_) {
            auto const& [name, value] = f();
            names.push_back(name);
            values.push_back(value);
        }
    }
    return {std::move(names), std::move(values)};
}

ostream& operator<<(ostream& s, field_t const& f) {
    auto const& [name, value] = f();

    s << name << ":(";
    switch (value.index()) {
        case MONOSTATE_INDEX:
            s << "NULL";
            break;
        case INTEGER_INDEX:
            s << value.int64();
            break;
        case DOUBLE_INDEX:
            s << value.float64();
            break;
        case STRING_INDEX:
            s << value.str();
            break;
        case VECTOR_INDEX:
            s << '[' << share::bytes_as_str(value.vec()) << ']';
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