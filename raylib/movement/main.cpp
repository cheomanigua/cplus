#include "raylib.h"
#include "raymath.h"

struct Circle {
    Vector2 position;
    float radius;
};

int main()
{
    constexpr int screenWidth { 800 };
    constexpr int screenHeight { 450 };
    constexpr float speed { 100.0f };

    InitWindow(screenWidth, screenHeight, "Movement");
    Circle circle {{400.0f, 215.0f}, 20.0f};
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();

        Vector2 direction = {
            static_cast<float>(IsKeyDown(KEY_D)) - static_cast<float>(IsKeyDown(KEY_A)),
            static_cast<float>(IsKeyDown(KEY_S))  - static_cast<float>(IsKeyDown(KEY_W))
        };

        if (Vector2Length(direction) > 0.0f)
        {
            direction = Vector2Normalize(direction);
        }

        // Movement
        circle.position = Vector2Add(circle.position, Vector2Scale(direction, speed * deltaTime));


        // Draw
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawCircleV(circle.position, circle.radius, BLUE);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
