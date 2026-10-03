#include <iostream>
#include <raylib.h>
#include <flecs.h>

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
    constexpr int screenWidth{800};
    constexpr int screenHeight{600};
    InitWindow(screenWidth, screenHeight, "Prototype Engine");
    SetTargetFPS(60);

    flecs::world world{};
    GraphicalRenderSystem graphicalRenderSystem{};
    InputSystem inputSystem{};
    MovementSystem movementSystem{};
    flecs::entity player = world.entity();
    flecs::entity enemy = world.entity();
    //std::cout << "Player entity: " << entt::to_integral(player) << "\n";
    //std::cout << "Enemy entity: "  << entt::to_integral(enemy) << "\n";

    player.set<Position>({60.0f, 60.0f});
    player.set<Input>({});
    player.set<Speed>({150.0f});
    player.set<SelectionBounds>({20.0f});
    //player.set<TagSelected>({});

    enemy.set<Position>({190.0f, 60.0f});
    enemy.set<Input>({});
    enemy.set<Speed>({50.0f});
    enemy.set<SelectionBounds>({20.0f});
    enemy.set<Direction>({0.0f, 30.f});

    for (std::size_t i = 0; i < 200; ++i)
    {
        flecs::entity enemy  = world.entity();
        float x = static_cast<float>((i % 16) * 50.0f);
        float y = 1.0f + static_cast<float>(i / 16) * 50.0f;
        enemy.set<Position>({x, y});
        enemy.set<Input>({});
        enemy.set<Speed>({50.0f});
        enemy.set<SelectionBounds>({20.0f});
    }
        
    while (!WindowShouldClose())
    {
        inputSystem.update(world);
        movementSystem.update(world);
        graphicalRenderSystem.update(world);
    }

    CloseWindow();
    return 0;
}
