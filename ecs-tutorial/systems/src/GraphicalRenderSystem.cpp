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
            
            // Draw the entities as a circles
            DrawCircle(static_cast<int>(pos.position.x), static_cast<int>(pos.position.y), 20.0f, BLUE);
        }
    }

    DrawText("ECS Component Pipeline Active!", 150, 240, 20, DARKGRAY);

    EndDrawing();
}
