#pragma once
#include "../field.h"

namespace serde::field {
    int length(field_t const&) noexcept;
    vec<u8> as_bytes(field_t const&) noexcept;
    std::optional<field_t> from_bytes(std::span<u8>) noexcept;
}

