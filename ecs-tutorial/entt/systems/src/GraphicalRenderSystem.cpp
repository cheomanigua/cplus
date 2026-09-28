#include "GraphicalRenderSystem.h"
#include "Position.h"
#include <raylib.h>

void GraphicalRenderSystem::update(entt::registry& registry)
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    auto view = registry.view<Position>();
    for (auto [entity, pos] : view.each())
    {
        // Draw the entities as a circles
        DrawCircle(static_cast<int>(pos.position.x), static_cast<int>(pos.position.y), 20.0f, BLUE);
    }

    DrawText("Graphical Render System implemented!", 150, 240, 20, DARKGRAY);

    EndDrawing();
}

