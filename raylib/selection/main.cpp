#include "raylib.h"
#include "raymath.h"
#include <iostream>
#include <vector>

struct Circle {
    Vector2 position{};
    float radius{};
    int id{};
};

int main()
{
    constexpr int screenWidth { 800 };
    constexpr int screenHeight { 450 };

    InitWindow(screenWidth, screenHeight, "raylib [core] example - dynamic selection");
    SetTargetFPS(60);

    // 1. Store circles dynamically in a vector
    std::vector<Circle> circles;
    
    // Spawn 100 circles dynamically (e.g., in a grid or random positions)
    for (int i = 0; i < 100; ++i)
    {
        float x = static_cast<float>(50 + (i % 10) * 70);
        float y = static_cast<float>(50 + (i / 10) * 40);
        circles.push_back(Circle{ {x, y}, 15.0f, i + 1 });
    }

    // Keep track of the currently selected circle ID (-1 means none)
    int selectedId = -1;

    while (!WindowShouldClose())
    {
        // Update
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            Vector2 mousePosition = GetMousePosition();
            bool clickedAny = false;

            // 2. Loop through all circles dynamically to check collisions
            for (const auto& circle : circles)
            {
                if (CheckCollisionPointCircle(mousePosition, circle.position, circle.radius))
                {
                    selectedId = circle.id;
                    std::cout << "clicked " << selectedId << "\n";
                    clickedAny = true;
                    break; // Stop checking once we find the clicked circle
                }
            }

            // Optional: Deselect if clicking on empty space
            if (!clickedAny)
            {
                std::cout << "No selection" << "\n";
                selectedId = -1;
            }
        }

        // Draw
        BeginDrawing();
            ClearBackground(RAYWHITE);

            // 3. Loop through all circles to draw them
            for (const auto& circle : circles)
            {
                // Highlight the selected circle differently
                if (circle.id == selectedId)
                {
                    DrawCircleV(circle.position, circle.radius, RED);
                }
                else
                {
                    DrawCircleV(circle.position, circle.radius, BLUE);
                }
            }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
