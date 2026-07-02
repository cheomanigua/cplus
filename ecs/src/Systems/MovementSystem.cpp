// src/Systems/MovementSystem.cpp
#include "Systems/MovementSystem.hpp"
#include <raymath.h>
#include "Core/Constants.hpp"

void MovementSystem::ProcessCommands(CommandQueue& queue, MovementBuffers& buffers) {
    while (queue.HasCommands()) {
        GameCommand cmd = queue.Dequeue();
        
        // Safety Check: Validate EntityId before accessing buffers
        // MaxEntities is defined in Core/Constants.hpp
        if (cmd.EntityId >= 0 && cmd.EntityId < EngineConfig::MaxEntities) {
            
            if (cmd.Type == CommandType::Move) {
                buffers.Velocities[cmd.EntityId] = cmd.Velocity;
                buffers.Speeds[cmd.EntityId] = cmd.Speed;
                buffers.Active[cmd.EntityId] = true;
                
                // If you need to store the target position from the command:
                // buffers.Positions[cmd.EntityId] = cmd.MoveParams.Position;
            } 
            else if (cmd.Type == CommandType::Stop) {
                buffers.Active[cmd.EntityId] = false;
            }
        }
    }
}

void MovementSystem::Update(MovementBuffers& buffers, float deltaTime) {
    for (size_t i = 0; i < EngineConfig::MaxEntities; ++i) {
        // Skip inactive entities to keep the hot path fast
        if (!buffers.Active[i]) continue;

        // Execution: High-performance Euler integration
        // Raylib's Vector2Scale and Vector2Add provide optimized math
        Vector2 displacement = Vector2Scale(buffers.Velocities[i], buffers.Speeds[i] * deltaTime);
        buffers.Positions[i] = Vector2Add(buffers.Positions[i], displacement);
    }
}
