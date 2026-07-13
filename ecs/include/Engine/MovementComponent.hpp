#pragma once
#include <vector>
#include <raylib.h>
#include "Core/Constants.hpp"

struct MovementComponent {
    std::vector<Vector2> Positions;
    std::vector<Vector2> Velocities;
    std::vector<float> Speeds;
    std::vector<bool> Active;
    Vector2 TargetPositions[EngineConfig::MaxEntities];
    bool IsMoving[EngineConfig::MaxEntities];

    MovementComponent() {
        Positions.resize(EngineConfig::MaxEntities);
        Velocities.resize(EngineConfig::MaxEntities);
        Speeds.resize(EngineConfig::MaxEntities);
        Active.resize(EngineConfig::MaxEntities, false);

        // Explicitly initialize fixed-size arrays
        std::fill(std::begin(IsMoving), std::end(IsMoving), false);
        for(auto& pos : TargetPositions) { pos = {0, 0}; }
    }
};
