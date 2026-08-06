#include "MovementSystem.h"
#include "PositionComponent.h"
#include "DirectionComponent.h"
#include "SpeedComponent.h"
#include "InputComponent.h"
#include <raylib.h>
#include <raymath.h>

void MovementSystem::update(Registry& registry)
{
    float deltaTime = GetFrameTime();

    for (Entity e : registry.getEntities())
    {
        if (!registry.hasComponent<PositionComponent>(e) || !registry.hasComponent<SpeedComponent>(e))
            continue;

        auto& pos = registry.getComponent<PositionComponent>(e);
        auto& spd = registry.getComponent<SpeedComponent>(e);
        Vector2 direction{0.0f, 0.0f};

        // Case A: Entity is player-controlled via inputs (entity has InputComponent)
        if (registry.hasComponent<InputComponent>(e))
        {
            auto& input = registry.getComponent<InputComponent>(e);

            // direction values are obtained by WASD keys (0.0f, 1.0f or -1.0f)
            direction = Vector2{ 
                static_cast<float>(input.moveRight) - static_cast<float>(input.moveLeft),
                static_cast<float>(input.moveDown) - static_cast<float>(input.moveUp),
            };

            if (Vector2Length(direction) > 0.0f)
            {
                direction = Vector2Normalize(direction);
            }
        }
        // Case B: Entity moves automatically (using its DirectionComponent as a direction vector)
        else if (registry.hasComponent<DirectionComponent>(e))
        {
            auto& dir = registry.getComponent<DirectionComponent>(e);
            direction = dir.direction;

            if (Vector2Length(direction) > 0.0f)
            {
                direction = Vector2Normalize(direction);
            }
        }

        // Apply final movement for calculation in one single place
        pos.position = Vector2Add(pos.position, Vector2Scale(direction, spd.speed * deltaTime));
    }
}
