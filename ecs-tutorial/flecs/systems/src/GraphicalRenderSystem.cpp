#include "GraphicalRenderSystem.h"
#include "Position.h"
#include <raylib.h>

void GraphicalRenderSystem::update(flecs::world& world)
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    auto query = world.query<Position>();
    query.each([](Position& pos)
    {
        // Draw the entities as a circles
        DrawCircle(static_cast<int>(pos.position.x), static_cast<int>(pos.position.y), 20.0f, BLUE);
    });

    DrawText("Graphical Render System implemented!", 150, 240, 20, DARKGRAY);

    EndDrawing();
}

