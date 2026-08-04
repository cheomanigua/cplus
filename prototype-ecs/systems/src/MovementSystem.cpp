#include "MovementSystem.h"
#include "PositionComponent.h"
#include "VelocityComponent.h"
#include "SpeedComponent.h"
#include "InputComponent.h"
#include "TagSelectedComponent.h"
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
        Vector2 movementDirection{0.0f, 0.0f};

        // Case A: Entity is player-controlled via inputs
        if (registry.hasComponent<TagSelectedComponent>(e) && registry.hasComponent<InputComponent>(e))
        {
            auto& input = registry.getComponent<InputComponent>(e);
            
            movementDirection.x = static_cast<float>(input.moveRight) - static_cast<float>(input.moveLeft);
            movementDirection.y = static_cast<float>(input.moveDown) - static_cast<float>(input.moveUp);

            if (Vector2Length(movementDirection) > 0.0f)
            {
                movementDirection = Vector2Normalize(movementDirection);
            }
        }
        // Case B: Entity moves automatically (using its VelocityComponent as a direction vector)
        else if (registry.hasComponent<VelocityComponent>(e))
        {
            auto& vel = registry.getComponent<VelocityComponent>(e);
            movementDirection = vel.velocity;

            if (Vector2Length(movementDirection) > 0.0f)
            {
                movementDirection = Vector2Normalize(movementDirection);
            }
        }

        // Apply final movement calculation in one single place
        pos.position.x += movementDirection.x * spd.speed * deltaTime;
        pos.position.y += movementDirection.y * spd.speed * deltaTime;
    }
}
