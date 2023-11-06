#include "serde_value.h"
#include "serde_field.h"

namespace serde {
    namespace field {
        using name_size_t = u16;
        using value_size_t = u32;

        static const u8 FIELD_MARKER = 'F';


        int length(field_t const& f) noexcept {
            auto const& [name, value] = f();
            auto const name_size = static_cast<name_size_t>(name.size());
            auto const value_size = static_cast<value_size_t>(serde::value::length(value));
            return 1 + int(sizeof(name_size_t) + name_size + value_size);
        }

        vec<u8> as_bytes(field_t const& f) noexcept {
            auto const& [name, value] = f();
            auto const name_size = static_cast<name_size_t>(name.size());
            auto const value_size = static_cast<value_size_t>(serde::value::length(value));
            auto const total_size = sizeof(name_size_t) + name_size + value_size;

            vec<u8> buffer;
            buffer.reserve(total_size);
            buffer.push_back(FIELD_MARKER);
            {   // zapamiętaj rozmiar 'name' w postaci bajtów
                auto const ptr = reinterpret_cast<u8 const *>(&name_size);
                copy(ptr, ptr + sizeof(name_size), back_inserter(buffer));
            }
            {   // zapamiętaj 'name'
                copy(begin(name), end(name), back_inserter(buffer));
            }
            {   // zapamiętaj 'value'
                auto data = serde::value::as_bytes(value);
                copy(begin(data), end(data), back_inserter(buffer));
            }

            // tak na wszelki wypadek (nie powinno się zdarzyć)
            buffer.shrink_to_fit();
            return buffer;
        }

        std::optional<field_t> from_bytes(std::span<u8> bytes) noexcept {
            // pobierz marker pola i sprawdź czy jest właściwy
            if (bytes.empty() || bytes[0] != FIELD_MARKER) return {};
            bytes = bytes.subspan(1);

            // pobierz długość nazwy pola
            if (bytes.size() < sizeof(name_size_t)) return {};
            name_size_t  name_size;
            memcpy(&name_size, bytes.data(), sizeof(name_size_t));
            bytes = bytes.subspan(sizeof(name_size_t));

            // pobierz nazwę pola
            if (bytes.size() < name_size) return {};
            std::string name{share::as_string(bytes, name_size)};
            bytes = bytes.subspan(name_size);

            if ( bytes.size() < name_size) return {};
            auto value = serde::value::from_bytes(bytes);

            if (value)
                return field_t{name, *value};
            return {};
        } // end of from_bytes
    }
}
