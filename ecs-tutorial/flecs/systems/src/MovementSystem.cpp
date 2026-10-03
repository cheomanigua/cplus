#include "MovementSystem.h"
#include "Position.h"
#include "Direction.h"
#include "Speed.h"
#include "Input.h"
#include <raylib.h>
#include <raymath.h>

void MovementSystem::update(flecs::world& world)
{
    float deltaTime = GetFrameTime();

    // 1. Process player/input-controlled entities
    auto inputQuery = world.query<Position, Speed, Input>();
    inputQuery.each([&](Position& pos, Speed& spd, Input& input)
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
    });

    // 2. Process autonomous entities
    auto autoQuery = world.query_builder<Position, Speed, Direction>().without<Input>().build();
    autoQuery.each([&](Position& pos, Speed& spd, Direction& dir)
    {
        Vector2 direction = dir.direction;

        if (Vector2Length(direction) > 0.0f)
        {
            direction = Vector2Normalize(direction);
        }

        pos.position = Vector2Add(pos.position, Vector2Scale(direction, spd.speed * deltaTime));
    });
}
