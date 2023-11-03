#include "field.h"
using namespace std;

using total_size_t = u64;
using name_size_t = u16;
using value_size_t = u32;


/// Konwersja pola na ciąg bajtów.
/// \return ciąg bajtów reprezentujących pole.
vec<u8> field_t::
serialize() const noexcept {
    auto const& [name, value] = data_;
    auto const name_size = static_cast<name_size_t>(name.size());
    auto const value_size = static_cast<value_size_t>(value.size_ext());
    total_size_t const total_size = sizeof(u32) + sizeof(name_size) + name_size + value_size;

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
        auto data = value.serialize();
        copy(begin(data), end(data), back_inserter(buffer));
    }
    return buffer;
}

/// Utworzenie obiektu pola z ciągu bajtów.
field_t field_t::
deserialize(span<u8> data) noexcept {
    // pobierz span odpowiadający polu
    total_size_t total_size;
    memcpy(&total_size, data.data(), sizeof(total_size_t));
    data = data.subspan(sizeof(total_size_t), total_size);

    // pobierz rozmiar 'name'
    name_size_t  name_size;
    memcpy(&name_size, data.data(), sizeof(name_size_t));
    data = data.subspan(sizeof(name_size_t));
    // pobierz 'name'
    string name = share::as_string(data, name_size);
    data = data.subspan(name_size);
    // pobierz 'value'
    auto [value, _] = value_t::deserialize(data);

    return field_t{name, value};
}

