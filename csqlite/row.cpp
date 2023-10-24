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

std::string row_t::as_str() const noexcept {
    vector<string> buffer;
    buffer.reserve(data_.size());
    for (auto const& f : data_)
        buffer.push_back(f.as_str());

    return share::join_strings(buffer, ',');
}

ostream& operator<<(ostream& s, row_t const& r) {
    s << r.as_str();
    return s;
}
