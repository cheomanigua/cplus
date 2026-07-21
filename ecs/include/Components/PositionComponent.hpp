#pragma once
#include <array>
#include <raylib.h>
#include "Core/Constants.hpp"

struct PositionComponent {
    // Flat buffer for cache efficiency, indexed by EntityID
    std::array<Vector2, EngineConfig::MaxEntities> Positions{};
};
