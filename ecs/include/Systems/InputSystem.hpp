#pragma once
#include "Core/Commands/CommandQueue.hpp"
#include "Engine/EntityRegistry.hpp"
#include "Systems/SpatialSystem.hpp"
#include <cstdint>

class InputSystem {
public:
    // Update this signature to match your .cpp file
    static void PollInput(CommandQueue& queue, EntityRegistry& registry, const SpatialSystem& spatialSystem);
};
