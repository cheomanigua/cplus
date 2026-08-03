#pragma once
#include <cstdint>
#include <limits>

// A simple integer ID to represent an entity
using Entity = std::uint32_t;
inline constexpr Entity NullEntity{std::numeric_limits<Entity>::max()};
