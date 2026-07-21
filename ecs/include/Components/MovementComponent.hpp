#pragma once
#include <array>
#include <cstdint>
#include <raylib.h>
#include "Core/Constants.hpp"

struct MovementComponent {
    std::array<Vector2, EngineConfig::MaxEntities> Velocities{};
    std::array<Vector2, EngineConfig::MaxEntities> TargetPositions{};
    std::array<float, EngineConfig::MaxEntities> Speeds{};
    std::array<uint8_t, EngineConfig::MaxEntities> IsActive{};
    std::array<uint8_t, EngineConfig::MaxEntities> IsMoving{};
};
