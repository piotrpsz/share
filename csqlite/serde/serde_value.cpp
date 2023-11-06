#include "serde_value.h"


namespace serde::value {
    using str_size_t = u16;
    using vec_size_t = u32;

    /// Zwraca liczbę bajtów jaką zajmie serializowany obiekt 'value_t',
    /// \remark 1-szy bajt oznacza typy pola.
    u64 length(value_t const& v) noexcept {
        switch (v().index()) {
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
                return u64(1) + sizeof(str_size_t) + v.str().size();
            case VECTOR_INDEX:
                // typ + rozmiar + wartość
                return u64(1) + sizeof(vec_size_t) + v.vec().size();
        }
        return 0;
    }

    vec<u8> as_bytes(value_t const& v) noexcept {
        switch (v().index()) {
            case MONOSTATE_INDEX:
                return {MONOSTATE_INDEX};
            case INTEGER_INDEX: {
                // 1 bajt type + 8 baitów wartości (64 bity)
                i64 const n = v.int64();
                vec<u8> v{INTEGER_INDEX};
                v.resize(1 + sizeof(i64));
                memcpy(v.data() + 1, reinterpret_cast<void const *>(&n), sizeof(i64));
                return v; }
            case DOUBLE_INDEX: {
                // 1 bajt type + 8 baitów wartości
                f64 const n = v.float64();
                vec<u8> v{DOUBLE_INDEX};
                v.resize(1 + sizeof(f64));
                memcpy(v.data() + 1, reinterpret_cast<void const *>(&n), sizeof(f64));
                return v; }
            case STRING_INDEX: {
                // 1 bajt type + 2 bajty (u16) size + bajty stringa
                // maksymalny rozmiar stringa: 65535 (u16, 2 bajty)
                std::string str = v.str();
                auto n = static_cast<str_size_t>(str.size());
                vec<u8> v{STRING_INDEX};
                v.resize(1 + sizeof(str_size_t) + n);
                memcpy(v.data() + 1, reinterpret_cast<void const *>(&n), sizeof(str_size_t));
                memcpy(v.data() + 1 + sizeof(str_size_t), str.data(), n);
                return v; }
            case VECTOR_INDEX: {
                // 1 bajt type + 4 bajy (u32) size + bajty wektora blob
                // maksymalny rozmiar wektora-blob: 4 294 967 295 (u32, 4 bajty)
                std::vector<u8> vec = v.vec();
                auto n = static_cast<vec_size_t>(vec.size());
                std::vector<u8> v{VECTOR_INDEX};
                v.resize(1 + sizeof(vec_size_t) + n);
                ::memcpy(v.data() + 1, reinterpret_cast<void const *>(&n), sizeof(vec_size_t));
                ::memcpy(v.data() + 1 + sizeof(vec_size_t), vec.data(), n);
                return v; }
        }
        return {};
    }

    std::optional<value_t> from_bytes(std::span<u8> bytes) noexcept {
        // typ wartości rozpoznajemy po pierwszym bajcie
        auto const marker = bytes[0];
        bytes = bytes.subspan(1);

        switch (marker) {
            case INTEGER_INDEX:
                if (bytes.size() >= sizeof(u64)) {
                    u64 n{};
                    memcpy(&n, bytes.data(), sizeof(u64));
                    return value_t{n};
                }
                break;
            case DOUBLE_INDEX:
                if (bytes.size() >= sizeof(f64)) {
                    f64 n{};
                    memcpy(&n, bytes.data(), sizeof(f64));
                    return value_t{n};
                }
                break;
            case STRING_INDEX:
                if (bytes.size() >= sizeof(str_size_t)) {
                    str_size_t n{};
                    memcpy(&n, bytes.data(), sizeof(str_size_t));
                    bytes = bytes.subspan(sizeof(str_size_t));
                    if (bytes.size() >= n) {
                        std::string str{reinterpret_cast<char *>(bytes.data()), n};
                        return value_t{str};
                    }
                }
                break;
            case VECTOR_INDEX:
                if (bytes.size() >= sizeof(vec_size_t)) {
                    vec_size_t n{};
                    memcpy(&n, bytes.data(), sizeof(vec_size_t));
                    bytes = bytes.subspan(sizeof(vec_size_t));
                    if (bytes.size() >= n) {
                        vec<u8> vc;
                        vc.resize(n);
                        memcpy(vc.data(), bytes.data() + sizeof(vec_size_t), n);
                        return value_t{vc};
                    }
                }
                break;
            default:
                // MONOSTATE_INDEX
                return value_t{};
        }
        return {};
    }
}
