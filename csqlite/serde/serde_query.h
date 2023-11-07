#pragma once
#include "../query.h"
#include <optional>

namespace serde::query {
    vec<u8> as_bytes(query_t const& query) noexcept;
    std::optional<query_t> from_bytes(std::span<u8> bytes) noexcept;
}
