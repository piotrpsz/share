#pragma once
#include "../row.h"
#include <optional>

namespace serde::row {
    int length(row_t const& row) noexcept;
    vec<u8> as_bytes(row_t const& row) noexcept;
    std::optional<row_t> from_bytes(std::span<u8> bytes) noexcept;
}

