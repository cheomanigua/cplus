#include "raylib.h"
#include <iostream>
#include <cmath>

namespace SimulationEngine {
    struct PositionComp {
        float x, y;
        PositionComp(float x_val, float y_val) : x(x_val), y(y_val) {}
    };

    struct SensorComp {
        float range;
        float rangeSquared;
        bool isEnabled;
        SensorComp(float r, bool enabled) 
            : range(r), rangeSquared(r * r), isEnabled(enabled) {}
    };

    class RadarSystem {
    public:
        static bool IsWithinRadarRange(const PositionComp& sourcePos, 
                                       const PositionComp& targetPos, 
                                       const SensorComp& radar) {
            if (!radar.isEnabled) return false;
            float deltaX = targetPos.x - sourcePos.x;
            float deltaY = targetPos.y - sourcePos.y;
            float distanceSquared = (deltaX * deltaX) + (deltaY * deltaY);
            return distanceSquared <= radar.rangeSquared;
        }
    };
}

int main() {
    using namespace SimulationEngine;

    InitWindow(800, 600, "Radar - Point and Click Movement");
    SetTargetFPS(60);

    PositionComp targetPos(170.14f, 191.15f);
    PositionComp ussPasadenaPos(520.15f, 330.85f);
    Vector2 destination = { ussPasadenaPos.x, ussPasadenaPos.y }; // Click target
    
    SensorComp passiveRadar(50.0f, true);
    float movementSpeed = 200.0f; 
    bool isMoving = false;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        // 1. Input Handling
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            destination = GetMousePosition();
            isMoving = true;
        }

        // 2. Movement Logic (Moving toward destination)
        if (isMoving) {
            float dx = destination.x - ussPasadenaPos.x;
            float dy = destination.y - ussPasadenaPos.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance > 2.0f) { // Stop if close enough
                // Normalize and move
                ussPasadenaPos.x += (dx / distance) * movementSpeed * deltaTime;
                ussPasadenaPos.y += (dy / distance) * movementSpeed * deltaTime;
            } else {
                isMoving = false;
            }
        }

        // 3. Simulation Logic
        bool detected = RadarSystem::IsWithinRadarRange(ussPasadenaPos, targetPos, passiveRadar);

        // 4. Drawing
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw intended destination
        if (isMoving) DrawCircleLines(destination.x, destination.y, 5, GREEN);

        // Visualizing the radar detection bubble
        DrawCircleLines((int)ussPasadenaPos.x, (int)ussPasadenaPos.y, passiveRadar.range, detected ? RED : BLUE);
        // Draw Target
        DrawCircleV({targetPos.x, targetPos.y}, 10.0f, RED);
        DrawText("Target", (int)targetPos.x + 15, (int)targetPos.y, 20, RED);

        // Draw USS Pasadena
        DrawCircleV({ussPasadenaPos.x, ussPasadenaPos.y}, 10.0f, BLUE);
        DrawText("USS Pasadena", (int)ussPasadenaPos.x + 15, (int)ussPasadenaPos.y, 20, BLUE);

        // UI Information
        DrawText(TextFormat("Detected: %s", detected ? "YES" : "NO"), 10, 10, 20, DARKGRAY);
        DrawText("Click anywhere to move the USS Pasadena", 10, 30, 20, DARKGRAY);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
