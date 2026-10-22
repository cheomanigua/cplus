#include <cstddef>

#include "flecs.h"
#include "raylib.h"

constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 720;
constexpr std::size_t ENTITY_COUNT = 200;
constexpr int SPRITE_SIZE = 16;

struct Position {
    float x{}, y{};
};

struct Velocity {
    float x{}, y{};
};

void updateEntities(
    flecs::query<Position, Velocity>& query,
    float deltaTime,
    float maxX,
    float maxY)
{
    query.each([=](Position& pos, Velocity& vel) {
        pos.x += vel.x * deltaTime;
        pos.y += vel.y * deltaTime;

        if (pos.x < 0.0f) {
            pos.x = 0.0f;
            vel.x = -vel.x;
        } else if (pos.x > maxX) {
            pos.x = maxX;
            vel.x = -vel.x;
        }

        if (pos.y < 0.0f) {
            pos.y = 0.0f;
            vel.y = -vel.y;
        } else if (pos.y > maxY) {
            pos.y = maxY;
            vel.y = -vel.y;
        }
    });
}

void drawEntities(
    flecs::query<Position>& query,
    Texture2D texture)
{
    query.each([&](const Position& pos) {
        DrawTexture(
            texture,
            static_cast<int>(pos.x),
            static_cast<int>(pos.y),
            WHITE
        );
    });
}

int main()
{
    // Initialize raylib.
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Flecs + raylib");
    SetTargetFPS(60);

    // Generate a blue texture in memory.
    Image image = GenImageColor(
        SPRITE_SIZE,
        SPRITE_SIZE,
        SKYBLUE
    );

    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);

    if (texture.id == 0) {
        CloseWindow();
        return 1;
    }

    // Initialize Flecs.
    flecs::world world;

    // Create 1000 entities with positions and velocities.
    for (std::size_t i = 0; i < ENTITY_COUNT; ++i) {
        const float x = static_cast<float>(
            GetRandomValue(0, SCREEN_WIDTH - SPRITE_SIZE)
        );

        const float y = static_cast<float>(
            GetRandomValue(0, SCREEN_HEIGHT - SPRITE_SIZE)
        );

        const float vx = static_cast<float>(
            GetRandomValue(-200, 200)
        );

        const float vy = static_cast<float>(
            GetRandomValue(-200, 200)
        );

        world.entity()
            .set<Position>({x, y})
            .set<Velocity>({vx, vy});
    }

    // Build cached queries once.
    auto movementQuery = world
        .query_builder<Position, Velocity>()
        .cached()
        .build();

    auto renderQuery = world
        .query_builder<Position>()
        .cached()
        .build();

    // Main game loop.
    while (!WindowShouldClose()) {
        const float deltaTime = GetFrameTime();

        // Update ECS entities.
        updateEntities(
            movementQuery,
            deltaTime,
            static_cast<float>(SCREEN_WIDTH - SPRITE_SIZE),
            static_cast<float>(SCREEN_HEIGHT - SPRITE_SIZE)
        );

        // Render.
        BeginDrawing();
        ClearBackground(BLACK);

        drawEntities(renderQuery, texture);

        DrawFPS(10, 10);
        DrawText(
            "1000 Flecs entities",
            10, 35, 20, GREEN
        );

        EndDrawing();
    }

    // Release resources.
    UnloadTexture(texture);
    CloseWindow();

    return 0;
}
