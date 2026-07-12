// src/Systems/MovementSystem.cpp
#include "Systems/MovementSystem.hpp"
#include "Core/Constants.hpp"
#include "Engine/EntityRegistry.hpp"
#include "raylib.h"
#include "raymath.h"

void MovementSystem::ProcessCommands(CommandQueue& queue, MovementComponent& buffers) {
    // Get the current number of commands so we only cycle through the original set
    size_t commandCount = queue.GetCount(); 

    for (size_t i = 0; i < commandCount; ++i) {
        GameCommand cmd = queue.Dequeue();
        
        if (cmd.Type == CommandType::Move || cmd.Type == CommandType::Stop) {
            // Process movement-specific commands
            if (cmd.EntityId >= 0 && cmd.EntityId < EngineConfig::MaxEntities) {
                if (cmd.Type == CommandType::Move) {
                    buffers.Velocities[cmd.EntityId] = cmd.Velocity;
                    buffers.Speeds[cmd.EntityId] = cmd.Speed;
                    buffers.TargetPositions[cmd.EntityId] = cmd.MoveParams.TargetPosition; 
                    buffers.IsMoving[cmd.EntityId] = true;
                    buffers.Active[cmd.EntityId] = true;
                } else if (cmd.Type == CommandType::Stop) {
                    buffers.Active[cmd.EntityId] = false;
                }
            }
        } else {
            // Re-enqueue non-movement commands immediately
            queue.Enqueue(cmd);
        }
    }
}

void MovementSystem::Update(MovementComponent& buffers, float deltaTime, EntityRegistry& registry) {
    // Optimization: Iterate only over active entities instead of all possible slots
    const auto& activeEntities = registry.GetActiveEntities();

    for (int32_t id : activeEntities) {
        // Only process movement if this specific entity is flagged as moving
        if (!buffers.IsMoving[id]) continue;

        Vector2& pos = registry.GetPosition(id);

        // 1. Calculate distance to target
        float dist = Vector2Distance(pos, buffers.TargetPositions[id]);

        // 2. Stop if close enough
        if (dist < 5.0f) { 
            pos = buffers.TargetPositions[id]; // Snap to exact position
            buffers.IsMoving[id] = false;       // Stop movement
        } else {
            // 3. Move normally
            Vector2 displacement = Vector2Scale(buffers.Velocities[id], buffers.Speeds[id] * deltaTime);
            pos = Vector2Add(pos, displacement);
        }
    }
}
