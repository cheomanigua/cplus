#include "MovementSystem.h"
#include "PositionComponent.h"
#include "VelocityComponent.h"
#include "SpeedComponent.h" // <----------- Add this
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
        auto& spd = registry.getComponent<SpeedComponent>(e); // <----------- Add this
        Vector2 velocity{0.0f, 0.0f};

        // Case A: Entity is player-controlled via inputs (entity has InputComponent)
        if (registry.hasComponent<InputComponent>(e))
        {
            auto& input = registry.getComponent<InputComponent>(e);

            // velocity.x and velocity.y values are obtained by WASD keys (0.0f, 1.0f or -1.0f)
            velocity.x = static_cast<float>(input.moveRight) - static_cast<float>(input.moveLeft);
            velocity.y = static_cast<float>(input.moveDown) - static_cast<float>(input.moveUp);

            if (Vector2Length(velocity) > 0.0f)
            {
                velocity = Vector2Normalize(velocity);
            }
        }
        // Case B: Entity moves automatically (using its VelocityComponent as a direction vector)
        else if (registry.hasComponent<VelocityComponent>(e))
        {
            auto& vel = registry.getComponent<VelocityComponent>(e);
            velocity = vel.velocity;

            if (Vector2Length(velocity) > 0.0f)
            {
                velocity = Vector2Normalize(velocity);
            }
        }

        // Apply final movement for calculation in one single place
        pos.position.x += velocity.x * spd.speed * deltaTime;
        pos.position.y += velocity.y * spd.speed * deltaTime;
    }
}
