#pragma once

#include <vector>

#include <flecs.h>

#include "domains/movement/movement_commands.hpp"

namespace navalsim::movement
{

class MovementCommandProcessor
{
public:
    std::vector<MovementCommandResult> process(
        flecs::world& world,
        const std::vector<QueuedMovementCommand>& commands
    ) const;
};

} // namespace navalsim::movement

