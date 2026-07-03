#pragma once
#include "Core/Commands/CommandQueue.hpp"
#include <cstdint>

class InputSystem {
public:
    static void PollInput(CommandQueue& queue, int32_t playerId);
};
