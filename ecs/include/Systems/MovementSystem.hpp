#pragma once

#include "Core/Commands/GameCommand.hpp"
#include "Core/Commands/CommandQueue.hpp"
#include "Engine/MovementBuffers.hpp"
#include "Engine/EntityRegistry.hpp"

class MovementSystem {
public:
    /**
     * @brief Processes the intent (Commands) from the queue 
     * and updates the MovementBuffers state.
     */
    static void ProcessCommands(CommandQueue& queue, MovementBuffers& buffers);

    /**
     * @brief Performs the "Hot Path" execution logic: 
     * applies velocity and speed to positions based on time.
     */
    static void Update(MovementBuffers& buffers, float deltaTime, EntityRegistry& registry);
};
