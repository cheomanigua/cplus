#pragma once
#include <cstdint>

namespace EngineConfig {
    // Limits
    constexpr int32_t MaxItemCapacity = 1024;
    constexpr int32_t MaxEntities = 1024;
    constexpr int32_t MaxEntityCapacity = 1024;

    // Grid settings
    constexpr int32_t CellSize = 64;
    constexpr int32_t ScreenWidth = 1920;
    constexpr int32_t ScreenHeight = 1080;
    
    constexpr int32_t GridWidth = ScreenWidth / CellSize;
    constexpr int32_t GridHeight = ScreenHeight / CellSize;
}

namespace EntityMasks {
    // These define the bit-flags for type identification
    constexpr int32_t ITEM_MASK       = 0x000;
    constexpr int32_t NPC_MASK        = 0x100;
    constexpr int32_t PROJECTILE_MASK = 0x200;
    
    constexpr int32_t TYPE_MASK       = ITEM_MASK | NPC_MASK | PROJECTILE_MASK;
}
