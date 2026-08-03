#include "MovementSystem.h"
#include "PositionComponent.h"
#include "VelocityComponent.h"
#include <raylib.h>

void MovementSystem::update(Registry& registry)
{
    float deltaTime = GetFrameTime();

    // Iterate through entities and update positions for those with both Position and Velocity
    for (Entity e : registry.getEntities())
    {
        if (registry.hasComponent<PositionComponent>(e) && registry.hasComponent<VelocityComponent>(e))
        {
            auto& pos = registry.getComponent<PositionComponent>(e);
            auto& vel = registry.getComponent<VelocityComponent>(e);

            pos.position.x += vel.velocity.x * deltaTime;
            pos.position.y += vel.velocity.y * deltaTime;
        }
    }
}
