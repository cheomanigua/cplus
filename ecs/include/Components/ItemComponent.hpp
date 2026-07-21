#pragma once
#include <array>
#include <cstdint>
#include "Core/Constants.hpp"

struct ItemComponent {
    // Stores the ID of the item equipped by the entity.
    // Using -1 to indicate that no item is currently equipped.
    std::array<int32_t, EngineConfig::MaxEntities> EquippedItemId{-1};
};
