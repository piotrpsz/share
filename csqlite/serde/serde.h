#pragma once
#include <optional>
#include <any>
#include "serde_value.h"
#include "serde_field.h"
#include "serde_row.h"
#include "serde_result.h"
#include "serde_query.h"




namespace serde {
    // VALUE
    u64 length(value_t const& value) noexcept {
        return serde::value::length(value);
    }
    vec<u8> as_bytes(value_t const& value) noexcept {
        return serde::value::as_bytes(value);
    }

    // FIELD
    u64 length(field_t const& field) noexcept {
        return serde::field::length(field);
    }
    vec<u8> as_bytes(field_t const& field) noexcept {
        return serde::field::as_bytes(field);
    }

    // ROW
    u64 length(row_t const& row) noexcept {
        return serde::row::length(row);
    }
    vec<u8> as_bytes(row_t const& row) noexcept {
        return serde::row::as_bytes(row);
    }

    // RESULT
    u64 length(result_t const& result) noexcept {
        return serde::result::length(result);
    }
    vec<u8> as_bytes(result_t const& result) noexcept {
        return serde::result::as_bytes(result);
    }

    // QUERY
    vec<u8> as_bytes(query_t const& result) noexcept {
        return serde::query::as_bytes(result);
    }

    template<typename T>
     std::any from_bytes(std::span<u8> data) noexcept {
        if (std::is_same_v<T, value_t>)
            if (auto v = serde::value::from_bytes(data); v)
                return *v;
         if (std::is_same_v<T, field_t>)
             if (auto f = serde::field::from_bytes(data); f)
                 return *f;
        if (std::is_same_v<T, row_t>)
            if (auto r = serde::row::from_bytes(data); r)
                return *r;

        if(std::is_same_v<T, result_t>)
            if (auto r = serde::result::from_bytes(data); r)
                return *r;
        if (std::is_same_v<T, query_t>)
            if (auto q = serde::query::from_bytes(data); q)
                return *q;
        return {};
    }
}
