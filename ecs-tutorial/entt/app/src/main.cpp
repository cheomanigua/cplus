#include <iostream>
#include <raylib.h>
#include "entt.hpp"

#include "Position.h"
#include "Direction.h"
#include "Speed.h"
#include "Input.h"
#include "Tags.h"
#include "SelectionBounds.h"

#include "GraphicalRenderSystem.h"
#include "InputSystem.h"
#include "MovementSystem.h"

int main()
{
    const int screenWidth{800};
    const int screenHeight{600};
    InitWindow(screenWidth, screenHeight, "Prototype Engine");
    SetTargetFPS(60);

    entt::registry registry;
    GraphicalRenderSystem graphicalRenderSystem{};
    InputSystem inputSystem{};
    MovementSystem movementSystem{};
    entt::entity player = registry.create();
    entt::entity enemy = registry.create();
    std::cout << "Player entity: " << entt::to_integral(player) << "\n";
    std::cout << "Enemy entity: "  << entt::to_integral(enemy) << "\n";

    registry.emplace<Position>(player, 60.0f, 60.0f);
    registry.emplace<Input>(player);
    registry.emplace<Speed>(player, 150.0f);
    registry.emplace<TagSelected>(player);
    registry.emplace<SelectionBounds>(player, 20.0f);

    registry.emplace<Position>(enemy, 190.0f, 60.0f);
    registry.emplace<Input>(enemy);
    registry.emplace<Direction>(enemy, 0.0f, 30.0f);
    registry.emplace<Speed>(enemy, 50.0f);
    registry.emplace<SelectionBounds>(enemy, 20.0f);

    while (!WindowShouldClose())
    {
        inputSystem.update(registry);
        movementSystem.update(registry);
        graphicalRenderSystem.update(registry);
    }

    CloseWindow();
    return 0;
}

