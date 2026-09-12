#include "PositionComponent.h"
#include "DirectionComponent.h"
#include "SpeedComponent.h"
#include "InputComponent.h"
#include "SelectionBoundsComponent.h"
//#include "ConsoleRenderSystem.h"
#include "Registry.h"
#include "InputSystem.h"
#include "MovementSystem.h"
#include "GraphicalRenderSystem.h"
#include <iostream>

int main()
{
    const int screenWidth{800};
    const int screenHeight{600};
    InitWindow(screenWidth, screenHeight, "Prototype Engine");
    SetTargetFPS(60);

    Registry registry{};
    InputSystem inputSystem{};
    MovementSystem movementSystem{};
    //ConsoleRenderSystem consoleRenderSystem{};
    GraphicalRenderSystem graphicalRenderSystem{};

    Entity entity1 = registry.spawnEntity(
            PositionComponent{100.0f, 150.0f},
            DirectionComponent{50.0f, 0.0f},
            SpeedComponent{50.0f},
            SelectionBoundsComponent{20.0f},
            InputComponent{});

    Entity entity2 = registry.spawnEntity(
            PositionComponent{200.0f, 150.0f},
            DirectionComponent{00.0f, 30.0f},
            SpeedComponent{50.0f},
            SelectionBoundsComponent{20.0f},
            InputComponent{});

    std::cout << "Entity One: " << entity1 << "\n";
    std::cout << "Entity Two: " << entity2 << "\n";

    while (!WindowShouldClose())
    {
        // Update simulation systems
        inputSystem.update(registry);
        movementSystem.update(registry);
        //consoleRenderSystem.update(registry);
        graphicalRenderSystem.update(registry);
    }

    CloseWindow();
    return 0;
}
