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
    for (size_t i = 0; i < EngineConfig::MaxEntities; ++i) {
        if (!buffers.IsMoving[i]) continue;

        Vector2* pos = registry.GetPosition(i);
        if (!pos) continue;

        // 1. Calculate distance to target
        float dist = Vector2Distance(*pos, buffers.TargetPositions[i]);

        // 2. Stop if close enough
        if (dist < 5.0f) { 
            *pos = buffers.TargetPositions[i]; // Snap to exact position
            buffers.IsMoving[i] = false;       // Stop movement
        } else {
            // 3. Move normally
            Vector2 displacement = Vector2Scale(buffers.Velocities[i], buffers.Speeds[i] * deltaTime);
            *pos = Vector2Add(*pos, displacement);
        }
    }
}
