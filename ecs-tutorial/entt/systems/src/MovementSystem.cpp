#include "MovementSystem.h"
#include "Position.h"
#include "Direction.h"
#include "Speed.h"
#include "Input.h"
#include <raylib.h>
#include <raymath.h>

void MovementSystem::update(entt::registry& registry)
{
    float deltaTime = GetFrameTime();

    // 1. Process player/input-controlled entities
    // Requires: Position, Speed, Input
    auto inputView = registry.view<Position, Speed, Input>();
    for (auto [entity, pos, spd, input] : inputView.each())
    {
        Vector2 direction{
            static_cast<float>(input.moveRight) - static_cast<float>(input.moveLeft),
            static_cast<float>(input.moveDown) - static_cast<float>(input.moveUp)
        };

        if (Vector2Length(direction) > 0.0f)
        {
            direction = Vector2Normalize(direction);
        }

        pos.position = Vector2Add(pos.position, Vector2Scale(direction, spd.speed * deltaTime));
    }

    // 2. Process autonomous entities (e.g., enemies)
    // Requires: Position, Speed, Direction
    // Excludes: Input
    auto autoView = registry.view<Position, Speed, Direction>(entt::exclude<Input>);
    for (auto [entity, pos, spd, dir] : autoView.each())
    {
        Vector2 direction = dir.direction;

        if (Vector2Length(direction) > 0.0f)
        {
            direction = Vector2Normalize(direction);
        }

        pos.position = Vector2Add(pos.position, Vector2Scale(direction, spd.speed * deltaTime));
    }
}
