#include "raylib.h"
#include "raymath.h"

int main() {
    // 1. Initialization
    InitWindow(800, 600, "Harpoon Sim - Raylib Direct");
    SetTargetFPS(60);

    Vector2 unknownTarget = { 220.0f, 230.0f };
    Vector2 ussPasadena = { 520.0f, 330.0f };
    Vector2 destination = ussPasadena;
    bool isMoving = false;
    const float speed = 200.0f;

    // 2. Main Simulation Loop
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // INPUT
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            destination = GetMousePosition();
            isMoving = true;
        }

        // MOVEMENT LOGIC
        if (isMoving) {
            ussPasadena = Vector2MoveTowards(ussPasadena, destination, speed * dt);
            if (Vector2Equals(ussPasadena, destination)) {
                isMoving = false;
            }
        }

        // 3. RENDER
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Draw Unknown Target
        DrawCircleV(unknownTarget, 10.0f, RED);
        DrawText("Unknown Target", (int)unknownTarget.x + 15, (int)unknownTarget.y + 20, 20, RED);

        // Draw USS Pasadena
        DrawCircleV(ussPasadena, 10.0f, BLUE);
        DrawText("USS Pasadena", (int)ussPasadena.x + 15, (int)ussPasadena.y + 20, 20, BLUE);

        if (isMoving) {
            DrawCircleLines((int)destination.x, (int)destination.y, 5.0f, GREEN);
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
