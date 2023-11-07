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
                i64 const n = v.int64();
                vec<u8> buffer; buffer.reserve(1 + sizeof(i64));
                buffer.push_back(INTEGER_INDEX);
                auto ptr = reinterpret_cast<u8 const*>(&n);
                copy(ptr, ptr + sizeof(i64), std::back_inserter(buffer));
                return buffer; }

            case DOUBLE_INDEX: {
                f64 const n = v.float64();
                vec<u8> buffer; buffer.reserve(1 + sizeof(f64));
                buffer.push_back(DOUBLE_INDEX);
                auto ptr = reinterpret_cast<u8 const*>(&n);
                copy(ptr, ptr + sizeof(f64), std::back_inserter(buffer));
                return buffer; }

            case STRING_INDEX: {
                std::string str = v.str();
                auto n = static_cast<str_size_t>(str.size());
                vec<u8> buffer; buffer.reserve(1 + sizeof(str_size_t) + n);
                buffer.push_back(STRING_INDEX);
                auto ptr = reinterpret_cast<u8 const*>(&n);
                copy(ptr, ptr + sizeof(str_size_t), std::back_inserter(buffer));
                copy(std::begin(str), std::end(str), std::back_inserter(buffer));
                return buffer; }

            case VECTOR_INDEX: {
                auto data = v.vec();
                auto n = static_cast<vec_size_t>(data.size());
                vec<u8> buffer;  buffer.reserve(1 + sizeof(vec_size_t) + n);
                buffer.push_back(VECTOR_INDEX);
                auto ptr = reinterpret_cast<u8 const*>(&n);
                copy(ptr, ptr + sizeof(vec_size_t), std::back_inserter(buffer));
                copy(std::begin(data), std::end(data), std::back_inserter(buffer));
                return buffer; }
        }
        return {};
    }

    std::optional<value_t> from_bytes(std::span<u8> bytes) noexcept {
        // typ wartości rozpoznajemy po pierwszym bajcie
        auto const marker = bytes[0];
        bytes = bytes.subspan(1);

        switch (marker) {
            case MONOSTATE_INDEX:
                return value_t{};

            case INTEGER_INDEX: {
                if (bytes.size() < sizeof(u64)) return {};
                u64 n{};
                memcpy(&n, bytes.data(), sizeof(u64));
                return value_t{n}; }

            case DOUBLE_INDEX: {
                if (bytes.size() < sizeof(f64)) return {};
                f64 n{};
                memcpy(&n, bytes.data(), sizeof(f64));
                return value_t{n}; }

            case STRING_INDEX: {
                if (bytes.size() < sizeof(str_size_t)) return {};
                str_size_t n{};
                memcpy(&n, bytes.data(), sizeof(str_size_t));
                bytes = bytes.subspan(sizeof(str_size_t));

                if (bytes.size() < n) return {};
                std::string str{reinterpret_cast<char *>(bytes.data()), n};
                return value_t{str}; }

            case VECTOR_INDEX: {
                if (bytes.size() < sizeof(vec_size_t)) return {};
                vec_size_t n{};
                memcpy(&n, bytes.data(), sizeof(vec_size_t));
                bytes = bytes.subspan(sizeof(vec_size_t));

                if (bytes.size() < n) return {};
                vec<u8> vc; vc.resize(n);
                memcpy(vc.data(), bytes.data(), n);
                return value_t{vc}; }

            default:
                return {};
        }
    }
}
