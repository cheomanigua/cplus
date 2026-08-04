#include "GraphicalRenderSystem.h"
#include "PositionComponent.h"
#include <raylib.h>

void GraphicalRenderSystem::update(Registry& registry)
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    for (Entity e : registry.getEntities())
    {
        if (registry.hasComponent<PositionComponent>(e))
        {
            auto& pos = registry.getComponent<PositionComponent>(e);
            
            // Draw the entity as a circle
            DrawCircle(static_cast<int>(pos.position.x), static_cast<int>(pos.position.y), 20.0f, BLUE);
        }
    }

    DrawText("Click on any entity and move it with WASD/cursor keys", 20, 20, 20, DARKGRAY);

    EndDrawing();
}
