#include "raylib.h"
#include "raymath.h"

struct Torpedo {
    Vector2 pos;
    Vector2 velocity;
    bool active;
};

int main() {
    // 1. Initialization
    InitWindow(800, 600, "Harpoon Sim - Native Raylib");
    SetTargetFPS(60);

    Vector2 unknownTarget = { 220.0f, 230.0f };
    Vector2 ussPasadena = { 520.0f, 330.0f };
    Vector2 destination = ussPasadena;
    
    bool isMoving = false;
    const float speed = 200.0f;
    const float radarRadius = 100.0f;

    // Time and Torpedo system
    float timeScale = 1.0f;
    float gameClock = 0.0f;
    float lastFireTime = -5.0f; // Fire immediately at start
    const float torpedoSpeed = 250.0f;
    Torpedo myTorpedo = { {0,0}, {0,0}, false };

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        float scaledDt = dt * timeScale;
        gameClock += scaledDt;

        // INPUT (Real-time, unaffected by timeScale)
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            destination = GetMousePosition();
            isMoving = true;
        }
        if (IsKeyPressed(KEY_UP)) timeScale += 0.5f;
        if (IsKeyPressed(KEY_DOWN)) timeScale = fmax(0.1f, timeScale - 0.5f);

        // MOVEMENT LOGIC
        if (isMoving) {
            ussPasadena = Vector2MoveTowards(ussPasadena, destination, speed * scaledDt);
            if (Vector2Equals(ussPasadena, destination)) isMoving = false;
        }

        // TORPEDO LOGIC (Fires every 5 game-seconds)
        if (gameClock - lastFireTime >= 5.0f) {
            lastFireTime = gameClock;
            Vector2 direction = Vector2Normalize(Vector2Subtract(ussPasadena, unknownTarget));
            myTorpedo.pos = unknownTarget;
            myTorpedo.velocity = Vector2Scale(direction, torpedoSpeed);
            myTorpedo.active = true;
        }

        if (myTorpedo.active) {
            myTorpedo.pos = Vector2Add(myTorpedo.pos, Vector2Scale(myTorpedo.velocity, scaledDt));
            if (myTorpedo.pos.x < 0 || myTorpedo.pos.x > 800 || myTorpedo.pos.y < 0 || myTorpedo.pos.y > 600) 
                myTorpedo.active = false;
        }

        // 3. RENDER
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Radar
        DrawCircleLinesV(ussPasadena, radarRadius, GRAY);
        bool detected = (Vector2Distance(ussPasadena, unknownTarget) <= radarRadius);
        DrawText(TextFormat("Detected: %s", detected ? "YES" : "NO"), 10, 10, 20, detected ? RED : DARKGRAY);

        // Clock (Top Right)
        int minutes = (int)(gameClock / 60) % 60;
        int seconds = (int)gameClock % 60;
        DrawText(TextFormat("Time: %02d:%02d", minutes, seconds), 680, 10, 20, DARKGRAY);
        DrawText(TextFormat("Speed x%.1f", timeScale), 680, 35, 20, MAROON);

        // Entities
        DrawCircleV(unknownTarget, 10.0f, RED);
        DrawCircleV(ussPasadena, 10.0f, BLUE);
        
        if (myTorpedo.active) DrawCircleV(myTorpedo.pos, 5.0f, MAROON);
        if (isMoving) DrawCircleLinesV(destination, 5.0f, GREEN);
        
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
