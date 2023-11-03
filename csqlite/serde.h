#pragma once
#include <span>
#include <optional>
#include "../share.h"
#include "value.h"
#include "field.h"
#include "row.h"
#include "result.h"

class serde final {
public:
    // result_t
    static u64 ser_size(result_t const& v) noexcept;
    static vec<u8> serialize(result_t const& v) noexcept;
    static result_t deserialize2result(std::span<u8> data) noexcept;

    // row_t
    static u64 ser_size(row_t const& v) noexcept;
    static vec<u8> serialize(row_t const& v) noexcept;
    static row_t deserialize2row(std::span<u8> data) noexcept;

    // field_t
    static u64 ser_size(field_t const& v) noexcept;
    static vec<u8> serialize(field_t const& v) noexcept;
    static field_t deserialize2field(std::span<u8> data) noexcept;

    // value_t
    static u64 ser_size(value_t const& v) noexcept;
    static vec<u8> serialize(value_t const& v) noexcept;
    static value_t deserialize2value(std::span<u8> data) noexcept;
};
