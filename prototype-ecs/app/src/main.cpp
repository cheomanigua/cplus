#include <raylib.h>
#include "Registry.h"
#include "PositionComponent.h"
#include "VelocityComponent.h"
#include "SpeedComponent.h"
#include "TagSelectedComponent.h"
#include "SelectionBoundsComponent.h"
#include "InputComponent.h"
#include "ConsoleRenderSystem.h"
#include "GraphicalRenderSystem.h"
#include "InputSystem.h"
#include "MovementSystem.h"
#include <iostream>

int main()
{
    const int screenWidth{800};
    const int screenHeight{600};
    InitWindow(screenWidth, screenHeight, "Prototype ECS Engine");
    SetTargetFPS(60);

    Registry registry{};
    InputSystem inputSystem{};
    MovementSystem movementSystem{};
    ConsoleRenderSystem consoleRenderSystem{};
    GraphicalRenderSystem graphicalRenderSystem{};

    Entity player = registry.spawnEntity(
            PositionComponent{100.0f, 150.0f},
            VelocityComponent{50.0f, 0.0f},
            SpeedComponent{50.0f},
            SelectionBoundsComponent{20.0f},
            InputComponent{});

    Entity enemy = registry.spawnEntity(
            PositionComponent{200.0f, 150.0f},
            VelocityComponent{00.0f, 3.0f},
            SpeedComponent{50.0f},
            SelectionBoundsComponent{20.0f},
            InputComponent{});

    std::cout << "Player entity: " << player << "\n";
    std::cout << "Enemy entity: " << enemy << "\n";

    while (!WindowShouldClose())
    {
        // Update simulation systems
        inputSystem.update(registry);
        movementSystem.update(registry);
        consoleRenderSystem.update(registry);
        graphicalRenderSystem.update(registry);
    }

    CloseWindow();
    return 0;
}
