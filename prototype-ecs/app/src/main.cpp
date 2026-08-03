#include <raylib.h>
#include "Registry.h"
#include "PositionComponent.h"
#include "VelocityComponent.h"
#include "TagSelectedComponent.h"
#include "ConsoleRenderSystem.h"
#include "GraphicalRenderSystem.h"
#include "InputSystem.h"
#include "MovementSystem.h"
#include <iostream>

int main()
{
    const int screenWidth{800};
    const int screenHeight{600};
    InitWindow(screenWidth, screenHeight, "Prototype Engine - Step 6");
    SetTargetFPS(60);

    Registry registry{};
    InputSystem inputSystem{};
    MovementSystem movementSystem{};
    ConsoleRenderSystem consoleRenderSystem{};
    GraphicalRenderSystem graphicalRenderSystem{};

    Entity player{registry.createEntity()};
    Entity enemy{registry.createEntity()};

    std::cout << "Player entity: " << player << "\n";
    std::cout << "Enemy entity: " << enemy << "\n";

    registry.addComponent<PositionComponent>(player, Vector2{100.0f, 150.0f});
    registry.addComponent<VelocityComponent>(player, Vector2{50.0f, 0.0f});
    registry.addComponent<TagSelectedComponent>(player);  // <---------------- Add this. Now respond to WASD

    registry.addComponent<PositionComponent>(enemy, Vector2{200.0f, 150.0f});
    registry.addComponent<VelocityComponent>(enemy, Vector2{0.0f, 30.0f});


    const float playerSpeed = 200.0f;

    while (!WindowShouldClose())
    {
        // Update simulation systems
        inputSystem.update(registry, playerSpeed);
        movementSystem.update(registry);
        consoleRenderSystem.update(registry);
        graphicalRenderSystem.update(registry);
    }

    CloseWindow();
    return 0;
}
