#include "raylib.h"
#include "reasings.h" // Include the easings header

int main() {
    InitWindow(800, 450, "Raylib Easing Example");
    SetTargetFPS(60);

    int currentTime {};
    constexpr int duration { 30 };

    constexpr float startPositionX { 130.0f };
    constexpr float finalPositionX { 190.0f };

    while (!WindowShouldClose()) {
        float currentPositionX { startPositionX };

        if (currentPositionX < finalPositionX)
        {
            currentPositionX = EaseSineIn(currentTime, startPositionX, finalPositionX - startPositionX, duration);
            currentTime++;
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);
            // Draw a circle oscillating smoothly in the screen
            DrawCircle(static_cast<int>(currentPositionX), 225, 30, BLUE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}


