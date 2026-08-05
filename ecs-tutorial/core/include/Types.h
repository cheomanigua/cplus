#pragma once
#include <cstdint>
#include <limits>

using Entity = std::uint32_t;
inline constexpr Entity NullEntity{std::numeric_limits<Entity>::max()};
