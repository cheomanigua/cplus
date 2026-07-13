#include "Systems/InputSystem.hpp"
#include "Systems/SpatialSystem.hpp"
#include "Core/RaylibGameView.hpp"
#include "Core/Interfaces/IEngineFacade.hpp"
#include "raylib.h"
#include "raymath.h"

void InputSystem::PollInput(CommandQueue& queue, EntityRegistry& registry, PositionComponent& posComp, const SpatialSystem& spatialSystem) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        int32_t clickedId = -1;

        int cellX = (int)mousePos.x / EngineConfig::CellSize;
        int cellY = (int)mousePos.y / EngineConfig::CellSize;

        const auto& entitiesInCell = spatialSystem.GetEntitiesInCell(cellX, cellY);
        
        for (int32_t id : entitiesInCell) {
            Vector2& pos = posComp.Positions[id];
            if (CheckCollisionPointCircle(mousePos, pos, 20.0f)) {
                clickedId = id;
                break;
            }
        }
        // ONLY update the registry if an entity was actually hit.
        // If clickedId is -1 (empty space), we decide whether to clear
        // or keep existing selection.
        if (clickedId != -1) {
            registry.SetSelectedEntity(clickedId);
            std::cout << "Entity " << clickedId << " selected" << std::endl;
        } else {
            // // Explicitly clear the selection if you click on empty space
            // registry.SetSelectedEntity(-1);
            // std::cout << "Empty space clicked - Selection cleared" << std::endl;

            int32_t selectedId = registry.GetSelectedEntity();
            if (selectedId != -1) {
                Vector2& startPos = posComp.Positions[selectedId];
                GameCommand cmd;
                cmd.Type = CommandType::Move;
                cmd.EntityId = selectedId;
                // Calculate direction
                Vector2 dir = Vector2Normalize(Vector2Subtract(mousePos, startPos));
                cmd.Velocity = dir;
                cmd.Speed = 100.0f; // Adjust as needed
                cmd.MoveParams.TargetPosition = mousePos;
                queue.Enqueue(cmd);
            }
        }
    }
}
