#include "raylib.h"
#include "raymath.h"

struct Torpedo {
    Vector2 pos{};
    Vector2 velocity{};
    bool active{};
};

int main() {
    // 1. Initialization
    InitWindow(800, 600, "Harpoon Sim - Native Raylib");
    SetTargetFPS(60);

    Vector2 ship2Pos { 220.0f, 230.0f }; // Shooter
    Vector2 ship1Pos { 520.0f, 330.0f }; // Target
    Vector2 destination { ship1Pos };
    
    bool isMoving { false };
    constexpr float speed { 200.0f };
    constexpr float radarRadius { 100.0f };

    // Time and Torpedo system
    float timeScale { 1.0f };
    float gameClock { 0.0f };
    float lastFireTime { -5.0f }; // Fire immediately at start
    constexpr float torpedoSpeed { 250.0f };
    Torpedo torpedo { {0,0}, {0,0}, false };

    while (!WindowShouldClose()) {
        float dt { GetFrameTime() };
        float scaledDt { dt * timeScale };
        gameClock += scaledDt;

        // INPUT (Real-time, unaffected by timeScale)
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            destination = GetMousePosition();
            isMoving = true;
        }
        if (IsKeyPressed(KEY_UP)) timeScale += 0.5f;
        if (IsKeyPressed(KEY_DOWN)) timeScale = fmaxf(0.1f, timeScale - 0.5f);

        // MOVEMENT LOGIC & TARGET VELOCITY CALCULATION
        Vector2 targetVelocity { 0.0f, 0.0f };
        if (isMoving) {
            Vector2 toDestination = Vector2Subtract(destination, ship1Pos);
            if (Vector2Length(toDestination) > 0.0f) {
                Vector2 moveDir = Vector2Normalize(toDestination);
                targetVelocity = Vector2Scale(moveDir, speed);
                ship1Pos = Vector2MoveTowards(ship1Pos, destination, speed * scaledDt);
                if (Vector2Equals(ship1Pos, destination)) isMoving = false;
            }
        }

        // TORPEDO LOGIC (Fires every 5 game-seconds with Lead Pursuit)
        if (gameClock - lastFireTime >= 5.0f) {
            lastFireTime = gameClock;

            // --- LEAD PURSUIT INTERCEPTION MATH ---
            Vector2 R = Vector2Subtract(ship1Pos, ship2Pos); // Relative position vector
            Vector2 V = targetVelocity;                     // Target velocity vector
            float s_p = torpedoSpeed;                       // Projectile speed

            // Quadratic equation coefficients: At^2 + Bt + C = 0
            float A = Vector2DotProduct(V, V) - (s_p * s_p);
            float B = 2.0f * Vector2DotProduct(R, V);
            float C = Vector2DotProduct(R, R);

            Vector2 aimPosition = ship1Pos; // Fallback to current position if no solution

            if (fabsf(A) < 0.0001f) {
                // Linear case if speeds match closely
                if (fabsf(B) > 0.0001f) {
                    float t = -C / B;
                    if (t > 0.0f) {
                        aimPosition = Vector2Add(ship1Pos, Vector2Scale(V, t));
                    }
                }
            } else {
                // Quadratic formula discriminant
                float discriminant = (B * B) - (4.0f * A * C);
                if (discriminant >= 0.0f) {
                    float sqrtDisc = sqrtf(discriminant);
                    float t1 = (-B - sqrtDisc) / (2.0f * A);
                    float t2 = (-B + sqrtDisc) / (2.0f * A);

                    // Find the smallest positive time of impact
                    float t = -1.0f;
                    if (t1 > 0.0f && t2 > 0.0f) t = fminf(t1, t2);
                    else if (t1 > 0.0f) t = t1;
                    else if (t2 > 0.0f) t = t2;

                    if (t > 0.0f) {
                        aimPosition = Vector2Add(ship1Pos, Vector2Scale(V, t));
                    }
                }
            }

            // Calculate final trajectory vector toward predicted intercept point
            Vector2 direction = Vector2Normalize(Vector2Subtract(aimPosition, ship2Pos));
            torpedo.pos = ship2Pos;
            torpedo.velocity = Vector2Scale(direction, torpedoSpeed);
            torpedo.active = true;
        }

        if (torpedo.active) {
            torpedo.pos = Vector2Add(torpedo.pos, Vector2Scale(torpedo.velocity, scaledDt));
            if (torpedo.pos.x < 0 || torpedo.pos.x > 800 || torpedo.pos.y < 0 || torpedo.pos.y > 600) 
                torpedo.active = false;
        }

        // 3. RENDER
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Radar
        DrawCircleLinesV(ship1Pos, radarRadius, GRAY);
        bool detected { (Vector2Distance(ship1Pos, ship2Pos) <= radarRadius) };
        DrawText(TextFormat("Detected: %s", detected ? "YES" : "NO"), 10, 10, 20, detected ? RED : DARKGRAY);

        // Clock (Top Right)
        int minutes { static_cast<int>(gameClock) / 60 % 60 };
        int seconds { static_cast<int>(gameClock) % 60 };
        DrawText(TextFormat("Time: %02d:%02d", minutes, seconds), 680, 10, 20, DARKGRAY);
        DrawText(TextFormat("Speed x%.1f", timeScale), 680, 35, 20, MAROON);

        // Entities
        DrawCircleV(ship2Pos, 10.0f, RED);
        DrawCircleV(ship1Pos, 10.0f, BLUE);
        
        if (torpedo.active) DrawCircleV(torpedo.pos, 5.0f, MAROON);
        if (isMoving) DrawCircleLinesV(destination, 5.0f, GREEN);
        
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
