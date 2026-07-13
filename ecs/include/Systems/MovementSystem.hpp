#pragma once

#include "Core/Commands/GameCommand.hpp"
#include "Core/Commands/CommandQueue.hpp"
#include "Engine/MovementComponent.hpp"
#include "Engine/PositionComponent.hpp"
#include "Engine/EntityRegistry.hpp"

class MovementSystem {
public:
    /**
     * @brief Processes the intent (Commands) from the queue 
     * and updates the MovementComponent state.
     */
    static void ProcessCommands(CommandQueue& queue, MovementComponent& moveComp);

    /**
     * @brief Performs the "Hot Path" execution logic: 
     * applies velocity and speed to positions based on time.
     */
    static void Update(MovementComponent& moveComp, 
                       PositionComponent& posComp, 
                       const EntityRegistry& registry, 
                       float deltaTime);
};
