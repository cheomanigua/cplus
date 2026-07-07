#pragma once
#include "Core/Commands/CommandQueue.hpp"
#include "Engine/EntityRegistry.hpp" // Required for the reference parameter
#include <cstdint>

class InputSystem {
public:
    // Update this signature to match your .cpp file
    static void PollInput(CommandQueue& queue, EntityRegistry& registry);
};
