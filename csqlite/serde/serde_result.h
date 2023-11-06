#pragma once
#include "../result.h"
#include <optional>

namespace serde::result {
    int length(result_t const& f) noexcept;
    vec<u8> as_bytes(result_t const& f) noexcept;
    std::optional<result_t> from_bytes(std::span<u8> data) noexcept;
}
