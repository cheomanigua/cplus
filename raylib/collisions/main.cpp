#include "raylib.h"
#include "raymath.h"

struct Circle {
    Vector2 position{};
    float radius{};
};

struct Square {
    Vector2 position{};
    Vector2 size{};

    // Return a Raylib Rectangle type
    Rectangle GetBounds() const
    {
        return {
            position.x,
            position.y,
            size.x,
            size.y
        };
    }
};

int main()
{
    constexpr int screenWidth { 800 };
    constexpr int screenHeight { 450 };
    constexpr float speed { 100.0f };

    InitWindow(screenWidth, screenHeight, "raylib [core] example - input keys");
    Circle circle1 { {400.0f, 215.0f}, 20.0f };
    Circle circle2 { {500.0f, 215.0f}, 40.0f };
    Square square1 { {250.0f, 200.0f}, {30.0f, 30.0f} };
    Square square2 { {300.0f, 200.0f}, {30.0f, 30.0f} };
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        Vector2 direction = {
            static_cast<float>(IsKeyDown(KEY_RIGHT)) - static_cast<float>(IsKeyDown(KEY_LEFT)),
            static_cast<float>(IsKeyDown(KEY_DOWN))  - static_cast<float>(IsKeyDown(KEY_UP))
        };

        if (Vector2Length(direction) > 0.0f)
        {
            direction = Vector2Normalize(direction);
        }


        // Collision Detection
        bool hasCollidedPC = CheckCollisionPointCircle(circle1.position, circle2.position, circle2.radius);
        bool hasCollidedCC = CheckCollisionCircles(circle1.position, circle1.radius, circle2.position, circle2.radius);
        bool hasCollidedRR = CheckCollisionRecs(square1.GetBounds(), square2.GetBounds());
        bool hasCollidedCR = CheckCollisionCircleRec(circle1.position, circle1.radius, square2.GetBounds());

        // Movement
        circle1.position = Vector2Add(circle1.position, Vector2Scale(direction, speed * dt));
        square1.position = Vector2Add(square1.position, Vector2Scale(direction, speed * dt));


        // Draw
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawCircleV(circle1.position, circle1.radius, BLUE);
            DrawCircleV(circle1.position, circle1.radius / 10.0f, GREEN);
            DrawCircleV(circle2.position, circle2.radius, RED);
            DrawRectangleRec(square1.GetBounds(), BLUE);
            DrawRectangleRec(square2.GetBounds(), RED);

            DrawText("move the blue ball and blue square with arrow keys", 10, 10, 20, DARKGRAY);
            DrawText(TextFormat("PointInCircle: %s", hasCollidedPC ? "YES" : "NO"), 10, 30, 20, hasCollidedPC ? RED : DARKGRAY);
            DrawText(TextFormat("CirclevsCircle: %s", hasCollidedCC ? "YES" : "NO"), 10, 50, 20, hasCollidedCC ? RED : DARKGRAY);
            DrawText(TextFormat("RectvsRect: %s", hasCollidedRR ? "YES" : "NO"), 10, 70, 20, hasCollidedRR ? RED : DARKGRAY);
            DrawText(TextFormat("CirclevsRect: %s", hasCollidedCR ? "YES" : "NO"), 10, 90, 20, hasCollidedCR ? RED : DARKGRAY);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
