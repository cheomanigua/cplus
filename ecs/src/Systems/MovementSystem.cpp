// src/Systems/MovementSystem.cpp
#include "Systems/MovementSystem.hpp"
#include "Core/Constants.hpp"
#include "Engine/EntityRegistry.hpp"
#include "raylib.h"
#include "raymath.h"

void MovementSystem::ProcessCommands(CommandQueue& queue, MovementComponent& movComp) {
    // Get the current number of commands so we only cycle through the original set
    size_t commandCount = queue.GetCount(); 

    for (size_t i = 0; i < commandCount; ++i) {
        GameCommand cmd = queue.Dequeue();
        
        if (cmd.Type == CommandType::Move || cmd.Type == CommandType::Stop) {
            // Process movement-specific commands
            if (cmd.EntityId >= 0 && cmd.EntityId < EngineConfig::MaxEntities) {
                if (cmd.Type == CommandType::Move) {
                    movComp.Velocities[cmd.EntityId] = cmd.Velocity;
                    movComp.Speeds[cmd.EntityId] = cmd.Speed;
                    movComp.TargetPositions[cmd.EntityId] = cmd.MoveParams.TargetPosition; 
                    movComp.IsMoving[cmd.EntityId] = true;
                    movComp.Active[cmd.EntityId] = true;
                } else if (cmd.Type == CommandType::Stop) {
                    movComp.Active[cmd.EntityId] = false;
                }
            }
        } else {
            // Re-enqueue non-movement commands immediately
            queue.Enqueue(cmd);
        }
    }
}

void MovementSystem::Update(MovementComponent& moveComp, 
                           PositionComponent& posComp, 
                           const EntityRegistry& registry, 
                           float deltaTime) {
    
    // Maintain your high-performance optimization
    const auto& activeEntities = registry.GetActiveEntities();

    for (int32_t id : activeEntities) {
        // Only process if the entity is moving
        if (!moveComp.IsMoving[id]) continue;

        // Both pieces of data are now in contiguous memory buffers
        Vector2& pos = posComp.Positions[id];
        Vector2& target = moveComp.TargetPositions[id];

        float dist = Vector2Distance(pos, target);

        if (dist < 5.0f) { 
            pos = target; 
            moveComp.IsMoving[id] = false;
        } else {
            Vector2 displacement = Vector2Scale(moveComp.Velocities[id], moveComp.Speeds[id] * deltaTime);
            pos = Vector2Add(pos, displacement);
        }
    }
}
