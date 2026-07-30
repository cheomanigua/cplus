/*******************************************************************************************
*
* raylib [textures] example - bunnymark (Refactored)
*
********************************************************************************************/

#include "raylib.h"
#include <vector>

constexpr std::size_t MAX_BUNNIES { 65536 };
constexpr std::size_t MAX_BATCH_ELEMENTS { 8192 };

struct Bunny {
    Vector2 position;
    Vector2 speed;
    Color color;
};

int main(void)
{
    // Initialization
    constexpr int screenWidth { 800 };
    constexpr int screenHeight { 450 };

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - bunnymark");

    Texture2D texBunny { LoadTexture("resources/raybunny.png") };

    // Modernized: Using std::vector instead of raw pointers/RL_MALLOC
    std::vector<Bunny> bunnies;
    bunnies.reserve(MAX_BUNNIES);

    bool paused { false };
    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            for (int i = 0; i < 5; i++)
            {
                if (bunnies.size() < MAX_BUNNIES)
                {
                    // Modernized: Using push_back to add elements
                    bunnies.push_back({
                        GetMousePosition(),
                        {static_cast<float>(GetRandomValue(-250, 250)),
                         static_cast<float>(GetRandomValue(-250, 250))},
                        // Fixed narrowing conversion with explicit casting
                        {static_cast<unsigned char>(GetRandomValue(50, 240)),
                         static_cast<unsigned char>(GetRandomValue(80, 240)),
                         static_cast<unsigned char>(GetRandomValue(100, 240)), 255}
                    });
                }
            }
        }

        if (IsKeyPressed(KEY_P)) paused = !paused;

        if (!paused)
        {
            // Modernized: Range-based for loop for cleaner iteration
            for (auto& b : bunnies)
            {
                b.position.x += b.speed.x * GetFrameTime();
                b.position.y += b.speed.y * GetFrameTime();

                if (((b.position.x + texBunny.width / 2.0f) > GetScreenWidth()) ||
                    ((b.position.x + texBunny.width / 2.0f) < 0)) b.speed.x *= -1;
                if (((b.position.y + texBunny.height / 2.0f) > GetScreenHeight()) ||
                    ((b.position.y + texBunny.height / 2.0f - 40.0f) < 0)) b.speed.y *= -1;
            }
        }

        // Draw
        BeginDrawing();
            ClearBackground(RAYWHITE);

            for (const auto& b : bunnies)
            {
                DrawTexture(texBunny, static_cast<int>(b.position.x), static_cast<int>(b.position.y), b.color);
            }

            DrawRectangle(0, 0, screenWidth, 40, BLACK);
            DrawText(TextFormat("bunnies: %i", static_cast<int>(bunnies.size())), 120, 10, 20, GREEN);
            DrawText(TextFormat("batched draw calls: %i", 1 + static_cast<int>(bunnies.size()) / MAX_BATCH_ELEMENTS), 320, 10, 20, MAROON);

            DrawFPS(10, 10);
        EndDrawing();
    }

    // De-Initialization
    // No manual free() required; vector memory is cleaned up automatically
    UnloadTexture(texBunny);
    CloseWindow();

    return 0;
}
