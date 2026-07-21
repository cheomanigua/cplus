// src/Systems/MovementSystem.cpp
#include "Systems/MovementSystem.hpp"
#include "Core/Constants.hpp"
#include "raylib.h"
#include "raymath.h"

void MovementSystem::ProcessCommands(CommandQueue& queue, MovementComponent& moveComp) {
    // Get the current number of commands so we only cycle through the original set
    size_t commandCount = queue.GetCount(); 

    for (size_t i = 0; i < commandCount; ++i) {
        GameCommand cmd = queue.Dequeue();
        
        if (cmd.Type == CommandType::Move || cmd.Type == CommandType::Stop) {
            // Process movement-specific commands
            if (cmd.EntityId >= 0 && cmd.EntityId < EngineConfig::MaxEntities) {
                if (cmd.Type == CommandType::Move) {
                    moveComp.Velocities[cmd.EntityId] = cmd.Velocity;
                    moveComp.Speeds[cmd.EntityId] = cmd.Speed;
                    moveComp.TargetPositions[cmd.EntityId] = cmd.MoveParams.TargetPosition; 
                    moveComp.IsMoving[cmd.EntityId] = true;
                    moveComp.IsActive[cmd.EntityId] = true;
                } else if (cmd.Type == CommandType::Stop) {
                    moveComp.IsActive[cmd.EntityId] = false;
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
                           const std::vector<int32_t>& activeEntities, 
                           float deltaTime) {
    
    for (int32_t id : activeEntities) {
        // Only process if the entity is moving
        if (!moveComp.IsMoving[id]) continue;

        // Both pieces of data are now in contiguous memory buffers
        Vector2& pos = posComp.Positions[id];
        Vector2& target = moveComp.TargetPositions[id];

        pos = Vector2MoveTowards(pos, target, moveComp.Speeds[id] * deltaTime);

        if (Vector2Equals(pos, target)) {
            moveComp.IsMoving[id] = false;
        }
    }
}
