#pragma once
#include <vector>
#include <raylib.h>
#include "Core/Constants.hpp"

struct MovementBuffers {
    std::vector<Vector2> Positions;
    std::vector<Vector2> Velocities;
    std::vector<float> Speeds;
    std::vector<bool> Active;
    Vector2 TargetPositions[EngineConfig::MaxEntities];
    bool IsMoving[EngineConfig::MaxEntities];

    MovementBuffers() {
        Positions.resize(EngineConfig::MaxEntities);
        Velocities.resize(EngineConfig::MaxEntities);
        Speeds.resize(EngineConfig::MaxEntities);
        Active.resize(EngineConfig::MaxEntities, false);
    }
};
