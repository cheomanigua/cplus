#pragma once
#include "Core/Commands/CommandQueue.hpp"
#include "Engine/EntityRegistry.hpp"
#include "Engine/PositionComponent.hpp"
#include "Systems/SpatialSystem.hpp"
#include <cstdint>

class InputSystem {
public:
    static void PollInput(CommandQueue& queue,
                          EntityRegistry& registry,
                          PositionComponent& posComp,
                          const SpatialSystem& spatialSystem);
};
