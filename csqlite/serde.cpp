#include "serde.h"
#include <variant>

using namespace std;

/********************************************************************
 *                                                                  *
 *                         F I E L D                                *
 *                                                                  *
 ********************************************************************/
namespace fns {
    using total_size_t = u64;
    using name_size_t = u16;
    using value_size_t = u32;
}

ssize_t serde::
ser_size(field_t const& v) noexcept {
    auto const& [name, value] = v.data_;
    auto const name_size = static_cast<fns::name_size_t>(name.size());
    auto const value_size =  static_cast<fns::value_size_t>(serde::ser_size(value));
    return sizeof(fns::total_size_t) + sizeof(fns::name_size_t) + name_size + value_size;
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
    data = data.subspan(sizeof(fns::total_size_t), total_size);

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

/// Zwraca liczbę bajtów jaką zajmie serializowany obiekt 'value_t',
/// \remark 1-szy bajt oznacza typy pola.
ssize_t serde::
ser_size(value_t const& v) noexcept {
    switch (v.data_.index()) {
        case MONOSTATE_INDEX:
            // type
            return 1;
        case INTEGER_INDEX:
            // typ + wartość
            return 1 + sizeof(i64);
        case DOUBLE_INDEX:
            // type + wartość
            return 1 + sizeof(f64);
        case STRING_INDEX:
            // typ + rozmiar + wartość
            return 1 + sizeof(u16) + get<STRING_INDEX>(v.data_).size();
        case VECTOR_INDEX:
            // typ + rozmiar + wartość
            return 1 + sizeof(u32) + get<VECTOR_INDEX>(v.data_).size();
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
            i64 const n = std::get<INTEGER_INDEX>(v.data_);
            vec<u8> v{INTEGER_INDEX};
            v.resize(1 + sizeof(i64));
            memcpy(v.data() + 1, reinterpret_cast<void const *>(&n), sizeof(i64));
            return v;
        }
        case DOUBLE_INDEX: {
            // 1 bajt type + 8 baitów wartości
            f64 const n = std::get<DOUBLE_INDEX>(v.data_);
            vec<u8> v{DOUBLE_INDEX};
            v.resize(1 + sizeof(f64));
            memcpy(v.data() + 1, reinterpret_cast<void const *>(&n), sizeof(f64));
            return v;
        }
        case STRING_INDEX: {
            // 1 bajt type + 2 bajty (u16) size + bajty stringa
            // maksymalny rozmiar stringa: 65535 (u16, 2 bajty)
            std::string str = std::get<STRING_INDEX>(v.data_);
            u16 n = static_cast<u16>(str.size());
            vec<u8> v{STRING_INDEX};
            v.resize(1 + sizeof(u16) + n);
            memcpy(v.data() + 1, reinterpret_cast<void const *>(&n), sizeof(u16));
            memcpy(v.data() + 1 + sizeof(u16), str.data(), n);
            return v;
        }
        case VECTOR_INDEX: {
            // 1 bajt type + 4 bajy (u32) size + bajty wektora blob
            // maksymalny rozmiar wektora-blob: 4 294 967 295 (u32, 4 bajty)
            std::vector<u8> vec = std::get<VECTOR_INDEX>(v.data_);
            u32 n = static_cast<u32>(vec.size());
            std::vector<u8> v{VECTOR_INDEX};
            v.resize(1 + sizeof(u32) + n);
            ::memcpy(v.data() + 1, reinterpret_cast<void const *>(&n), sizeof(u32));
            ::memcpy(v.data() + 1 + sizeof(u32), vec.data(), n);
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
            return value_t();
        case INTEGER_INDEX: {
            i64 n{};
            memcpy(&n, v.data() + 1, sizeof(i64));
            return value_t(n);
        }
        case DOUBLE_INDEX: {
            f64 n{};
            memcpy(&n, v.data() + 1, sizeof(f64));
            return value_t(n);
        }
        case STRING_INDEX: {
            u16 n{};
            memcpy(&n, v.data() + 1, sizeof(u16));
            string str{reinterpret_cast<char *>(v.data() + 1 + sizeof(u16)), n};
            return value_t(str);
        }
        case VECTOR_INDEX: {
            u32 n{};
            memcpy(&n, v.data() + 1, sizeof(u32));
            vec<u8> vc;
            vc.resize(n);
            memcpy(vc.data(), v.data() + 1 + sizeof(u32), n);
            return value_t(vc);
        }
    }
    return {};
}