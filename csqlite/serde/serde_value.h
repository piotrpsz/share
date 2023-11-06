#pragma once
#include "../value.h"
#include <optional>

namespace serde::value {
    u64 length(value_t const& value) noexcept;
    vec<u8> as_bytes(value_t const& v) noexcept;
    std::optional<value_t> from_bytes(std::span<u8> bytes) noexcept;
}
