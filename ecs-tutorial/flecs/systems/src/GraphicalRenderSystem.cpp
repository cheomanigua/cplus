#include "GraphicalRenderSystem.h"
#include <raylib.h>

GraphicalRenderSystem::GraphicalRenderSystem(flecs::world& world)
    : positionQuery{world.query_builder<Position>().cached().build()}
{
}

void GraphicalRenderSystem::update()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    positionQuery.each([](Position& pos)
    {
        // Draw the entities as a circles
        DrawCircle(static_cast<int>(pos.position.x), static_cast<int>(pos.position.y), 20.0f, BLUE);
    });

    DrawText("Graphical Render System implemented!", 150, 240, 20, DARKGRAY);

    EndDrawing();
}

