#pragma once
#include <vector>
#include "Core/Constants.hpp"

struct ItemComponent {
    // Stores the ID of the item equipped by the entity.
    // Using -1 to indicate that no item is currently equipped.
    std::vector<int32_t> EquippedItemId;

    ItemComponent() {
        // Pre-allocate memory to match your engine's capacity
        EquippedItemId.resize(EngineConfig::MaxEntities, -1);
    }
};
