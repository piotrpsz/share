#include "serde.h"
#include <variant>
#include <numeric>
#include <fmt/core.h>

using namespace std;


/********************************************************************
 *                                                                  *
 *                        R E S U L T                               *
 *                                                                  *
 ********************************************************************/
namespace tns {
    using total_size_t = u64;
    using rows_number_t = u32;
}


u64 serde::
ser_size(result_t const& v) noexcept {
    auto const content_size = accumulate(
            v.data_.cbegin(),
            v.data_.cend(),
            tns::total_size_t {},
            [](ssize_t const count, row_t const& r) {
                return count + ser_size(r);
            });
    return u64(sizeof(tns::total_size_t) + sizeof(tns::rows_number_t) + content_size);
}

vec<u8> serde::
serialize(result_t const& result) noexcept {
    auto const total_size = static_cast<tns::total_size_t>(ser_size(result));

    vec<u8> buffer;
    buffer.reserve(total_size);
    {   // zapamiętaj całkowity rozmiar
        auto const ptr = reinterpret_cast<u8 const*>(&total_size);
        copy(ptr, ptr + sizeof(tns::total_size_t), back_inserter(buffer));
    }
    {   // zapamiętaj liczbę wierszy
        auto const n = static_cast<tns::rows_number_t>(result.data_.size());
        auto const ptr = reinterpret_cast<u8 const*>(&n);
        copy(ptr, ptr + sizeof(tns::rows_number_t), back_inserter(buffer));
    }
    for (auto const& row : result) {
        auto data = serialize(row);
        copy(begin(data), end(data), back_inserter(buffer));
    }
    buffer.shrink_to_fit();
    return buffer;
}

result_t serde::
deserialize2result(std::span<u8> data) noexcept {
    tns::total_size_t total_size;
    memcpy(&total_size, data.data(), sizeof(tns::total_size_t));
    data = data.subspan(sizeof(tns::total_size_t), data.size() - sizeof(tns::total_size_t));

    tns::rows_number_t n;
    memcpy(&n, data.data(), sizeof(tns::rows_number_t ));
    data = data.subspan(sizeof(tns::rows_number_t));

    result_t result{};
    for (tns::rows_number_t i = 0; i < n; i++) {
        auto row = deserialize2row(data);
        data = data.subspan(ser_size(row));
        result.push_back(row);
    }
    return result;
}


/********************************************************************
 *                                                                  *
 *                            R O W                                 *
 *                                                                  *
 ********************************************************************/
namespace rns {
    using total_size_t = u64;
    using fields_number_t = u16;
}

u64 serde::
ser_size(row_t const& row) noexcept {
     auto const content_size = accumulate(
            row.data_.cbegin(),
            row.data_.cend(),
            rns::total_size_t {},
            [](ssize_t const count, field_t const& f) {
                return count + ser_size(f);
            });
    return u64(sizeof(rns::total_size_t) + sizeof(rns::fields_number_t) + content_size);
}

vec<u8> serde::
serialize(row_t const& row) noexcept {
    auto total_size = static_cast<rns::total_size_t>(ser_size(row));

    vec<u8> buffer;
    buffer.reserve(total_size);
    {   // zapamiętaj całkowity rozmiar
        auto const ptr = reinterpret_cast<u8 const*>(&total_size);
        copy(ptr, ptr + sizeof(rns::total_size_t), back_inserter(buffer));
    }
    {   // zapamiętaj liczbę pól
        auto const n = static_cast<rns::fields_number_t>(row.data_.size());
        auto ptr = reinterpret_cast<u8 const*>(&n);
        copy(ptr, ptr + sizeof(rns::fields_number_t), back_inserter(buffer));
    }
    for (auto const& field : row.data_) {
        auto data = serialize(field);
        copy(begin(data), end(data), back_inserter(buffer));
    }
    buffer.shrink_to_fit();
    return buffer;
}

row_t serde::
deserialize2row(std::span<u8> data) noexcept {
    rns::total_size_t total_size;
    memcpy(&total_size, data.data(), sizeof(rns::total_size_t));
    data = data.subspan(sizeof(rns::total_size_t), data.size() - sizeof(rns::total_size_t));

    rns::fields_number_t n;
    memcpy(&n, data.data(), sizeof(rns::fields_number_t));
    data = data.subspan(sizeof(rns::fields_number_t));

    row_t row{};
    for (int i = 0; i < n; i++) {
        auto f = deserialize2field(data);
        data = data.subspan(ser_size(f));
        row.add(f);
    }
    return row;
}

/********************************************************************
 *                                                                  *
 *                           F I E L D                              *
 *                                                                  *
 ********************************************************************/
namespace fns {
    using total_size_t = u64;
    using name_size_t = u16;
    using value_size_t = u32;
}

u64 serde::
ser_size(field_t const& v) noexcept {
    auto const& [name, value] = v.data_;
    auto const name_size = static_cast<fns::name_size_t>(name.size());
    auto const value_size =  static_cast<fns::value_size_t>(serde::ser_size(value));
    return u64(sizeof(fns::total_size_t)) + sizeof(fns::name_size_t) + name_size + value_size;
}

vec<u8> serde::
serialize(field_t const& v) noexcept {
    auto const& [name, value] = v.data_;
    auto const name_size = static_cast<fns::name_size_t>(name.size());
    auto const value_size =  static_cast<fns::value_size_t>(serde::ser_size(value));
    fns::total_size_t const total_size = sizeof(fns::total_size_t) + sizeof(fns::name_size_t) + name_size + value_size;

    vec<u8> buffer;
    buffer.reserve(total_size);

    {   // zapamiętaj całkowity rozmiar w postaci bajtów
        auto const ptr = reinterpret_cast<u8 const*>(&total_size);
        copy(ptr, ptr + sizeof(total_size), back_inserter(buffer));
    }
    {   // zapamiętaj rozmiar 'name' w postaci bajtów
        auto const ptr = reinterpret_cast<u8 const*>(&name_size);
        copy(ptr, ptr + sizeof(name_size), back_inserter(buffer));
    }
    {   // zapamiętaj 'name'
        copy(begin(name), end(name), back_inserter(buffer));
    }
    {   // zapamiętaj 'value'
        auto data = serde::serialize(value); // value.serialize();
        copy(begin(data), end(data), back_inserter(buffer));
    }

    // tak na wszelki wypadek (nie powinno się zdarzyć)
    buffer.shrink_to_fit();
    return buffer;
}

field_t serde::
deserialize2field(std::span<u8> data) noexcept {
    // pobierz span odpowiadający polu
    fns::total_size_t total_size;
    memcpy(&total_size, data.data(), sizeof(fns::total_size_t));
    data = data.subspan(sizeof(fns::total_size_t), data.size() - sizeof(fns::total_size_t));

    // pobierz rozmiar 'name'
    fns::name_size_t  name_size;
    memcpy(&name_size, data.data(), sizeof(fns::name_size_t));
    data = data.subspan(sizeof(fns::name_size_t));
    // pobierz 'name'
    string name = share::as_string(data, name_size);
    data = data.subspan(name_size);

    return field_t{name, serde::deserialize2value(data)};
}

/********************************************************************
 *                                                                  *
 *                         V A L U E                                *
 *                                                                  *
 ********************************************************************/

namespace vns {
    using str_size_t = u16;
    using vec_size_t = u32;
}
/// Zwraca liczbę bajtów jaką zajmie serializowany obiekt 'value_t',
/// \remark 1-szy bajt oznacza typy pola.
u64 serde::
ser_size(value_t const& v) noexcept {
    switch (v.data_.index()) {
        case MONOSTATE_INDEX:
            // type
            return u64(1);
        case INTEGER_INDEX:
            // typ + wartość
            return u64(1) + sizeof(i64);
        case DOUBLE_INDEX:
            // type + wartość
            return u64(1) + sizeof(f64);
        case STRING_INDEX:
            // typ + rozmiar + wartość
            return u64(1) + sizeof(vns::str_size_t) + get<STRING_INDEX>(v.data_).size();
        case VECTOR_INDEX:
            // typ + rozmiar + wartość
            return u64(1) + sizeof(vns::vec_size_t) + get<VECTOR_INDEX>(v.data_).size();
    }
    return 0;
}

vec<u8> serde::
serialize(value_t const& v) noexcept {
    switch (v.data_.index()) {
        case MONOSTATE_INDEX:
            return {MONOSTATE_INDEX};
        case INTEGER_INDEX: {
            // 1 bajt type + 8 baitów wartości (64 bity)
            i64 const n = v.int64();
            vec<u8> v{INTEGER_INDEX};
            v.resize(1 + sizeof(i64));
            memcpy(v.data() + 1, reinterpret_cast<void const *>(&n), sizeof(i64));
            return v;
        }
        case DOUBLE_INDEX: {
            // 1 bajt type + 8 baitów wartości
            f64 const n = v.float64();
            vec<u8> v{DOUBLE_INDEX};
            v.resize(1 + sizeof(f64));
            memcpy(v.data() + 1, reinterpret_cast<void const *>(&n), sizeof(f64));
            return v;
        }
        case STRING_INDEX: {
            // 1 bajt type + 2 bajty (u16) size + bajty stringa
            // maksymalny rozmiar stringa: 65535 (u16, 2 bajty)
            std::string str = v.str();
            auto n = static_cast<vns::str_size_t>(str.size());
            vec<u8> v{STRING_INDEX};
            v.resize(1 + sizeof(vns::str_size_t) + n);
            memcpy(v.data() + 1, reinterpret_cast<void const *>(&n), sizeof(vns::str_size_t));
            memcpy(v.data() + 1 + sizeof(vns::str_size_t), str.data(), n);
            return v;
        }
        case VECTOR_INDEX: {
            // 1 bajt type + 4 bajy (u32) size + bajty wektora blob
            // maksymalny rozmiar wektora-blob: 4 294 967 295 (u32, 4 bajty)
            std::vector<u8> vec = v.vec();
            auto n = static_cast<vns::vec_size_t>(vec.size());
            std::vector<u8> v{VECTOR_INDEX};
            v.resize(1 + sizeof(vns::vec_size_t) + n);
            ::memcpy(v.data() + 1, reinterpret_cast<void const *>(&n), sizeof(vns::vec_size_t));
            ::memcpy(v.data() + 1 + sizeof(vns::vec_size_t), vec.data(), n);
            return v;
        }
    }
    return {};
}

value_t serde::
deserialize2value(std::span<u8> v) noexcept {
    // typ wartości rozpoznajemy po pierwszym bajcie
    switch (v[0]) {
        case MONOSTATE_INDEX:
            return {};
        case INTEGER_INDEX: {
            i64 n{};
            memcpy(&n, v.data() + 1, sizeof(i64));
            return {n};
        }
        case DOUBLE_INDEX: {
            f64 n{};
            memcpy(&n, v.data() + 1, sizeof(f64));
            return {n};
        }
        case STRING_INDEX: {
            vns::str_size_t n{};
            memcpy(&n, v.data() + 1, sizeof(vns::str_size_t));
            string str{reinterpret_cast<char *>(v.data() + 1 + sizeof(vns::str_size_t)), n};
            return {str};
        }
        case VECTOR_INDEX: {
            vns::vec_size_t n{};
            memcpy(&n, v.data() + 1, sizeof(vns::vec_size_t));
            vec<u8> vc;
            vc.resize(n);
            memcpy(vc.data(), v.data() + 1 + sizeof(vns::vec_size_t), n);
            return {vc};
        }
    }
    return {};
}