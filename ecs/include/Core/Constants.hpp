#pragma once
#include <cstddef>
#include <cstdint>

namespace EngineConfig {
    // Limits
    constexpr std::size_t MaxItemCapacity{1024};
    constexpr std::size_t MaxEntities{1024};

    // Grid settings
    constexpr int16_t CellSize{64};
    constexpr int16_t ScreenWidth{1920};
    constexpr int16_t ScreenHeight{1080};

    constexpr int16_t GridWidth{ScreenWidth / CellSize};
    constexpr int16_t GridHeight{ScreenHeight / CellSize};
}

namespace EntityMasks {
    constexpr uint16_t ITEM_MASK{0x000};
    constexpr uint16_t NPC_MASK{0x100};
    constexpr uint16_t PROJECTILE_MASK{0x200};

    constexpr uint16_t TYPE_MASK{ITEM_MASK | NPC_MASK | PROJECTILE_MASK};
}
