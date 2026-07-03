// src/Systems/InputSystem.cpp
#include "Systems/InputSystem.hpp"
#include "Core/RaylibGameView.hpp"
#include "Core/Interfaces/IEngineFacade.hpp"
#include "raylib.h"

void InputSystem::PollInput(CommandQueue& queue, int32_t playerId) {
    // 1. Check for player input
    // We use the EngineService to keep the input check engine-agnostic
    if (EngineService::IsActionPressed("Move")) {
        Vector2 mousePos = GetMousePosition();

        // 2. Visual Feedback (optional but requested for testing)
        // Access the Raylib implementation to update the debug marker
        auto* view = dynamic_cast<RaylibGameView*>(EngineFacade::Implementation);
        if (view) {
            view->SetClickPosition(mousePos);
        }

        // 3. Command Generation
        GameCommand moveCmd;
        moveCmd.Type = CommandType::Move;
        moveCmd.EntityId = playerId;
        moveCmd.MoveParams.Position = mousePos; 
        
        // Note: You will need to calculate the velocity based on player position
        // inside the MovementSystem or here. For now, we set the target.
        // If your MovementSystem requires explicit Velocity, 
        // you would calculate it here: Vector2Normalize(mousePos - playerPos).

        moveCmd.Speed = 200.0f; // Default speed
        
        // 4. Queue the command
        queue.Enqueue(moveCmd);
    }
}
