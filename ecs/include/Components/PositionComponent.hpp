#pragma once
#include <vector>
#include <raylib.h>
#include "Core/Constants.hpp"

struct PositionComponent {
    // Flat buffer for cache efficiency, indexed by EntityID
    std::vector<Vector2> Positions;

    PositionComponent() {
        Positions.resize(EngineConfig::MaxEntities);
    }
};
