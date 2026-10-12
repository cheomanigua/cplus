#include "GraphicalRenderSystem.h"
#include <raylib.h>

GraphicalRenderSystem::GraphicalRenderSystem(entt::registry& registry)
    : positionView{registry.view<Position>()}
{
}

void GraphicalRenderSystem::update()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    for (auto [entity, pos] : positionView.each())
    {
        // Draw the entities as a circles
        DrawCircle(static_cast<int>(pos.position.x), static_cast<int>(pos.position.y), 20.0f, BLUE);
    }

    DrawText("Graphical Render System implemented!", 150, 240, 20, DARKGRAY);

    EndDrawing();
}

