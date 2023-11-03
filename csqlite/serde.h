#pragma once
#include <span>
#include <optional>
#include "../share.h"
#include "value.h"
#include "field.h"

class serde final {
public:
    // field_t
    static ssize_t ser_size(field_t const& v) noexcept;
    static vec<u8> serialize(field_t const& v) noexcept;
    static field_t deserialize2field(std::span<u8> data) noexcept;

    // value_t
    static ssize_t ser_size(value_t const& v) noexcept;
    static vec<u8> serialize(value_t const& v) noexcept;
    static value_t deserialize2value(std::span<u8> data) noexcept;
};
