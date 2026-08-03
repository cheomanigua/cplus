#include "InputSystem.h"
#include "VelocityComponent.h"
#include "TagSelectedComponent.h"
#include <raylib.h>
#include <raymath.h>

void InputSystem::update(Registry& registry, float speed)
{
    // 1. Read keyboard state using Raylib vector logic
    Vector2 movement = {
        static_cast<float>(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) - static_cast<float>(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)),
        static_cast<float>(IsKeyDown(KEY_DOWN)  || IsKeyDown(KEY_S)) - static_cast<float>(IsKeyDown(KEY_UP)   || IsKeyDown(KEY_W))
    };

    // 2. Normalize to prevent faster diagonal movement
    if (Vector2Length(movement) > 0.0f)
    {
        movement = Vector2Normalize(movement);
    }

    // 3. Apply the velocity to all entities with a VelocityComponent
    for (Entity e : registry.getEntities())
    {
        if (registry.hasComponent<TagSelectedComponent>(e) && 
            registry.hasComponent<VelocityComponent>(e))
        {
            auto& vel = registry.getComponent<VelocityComponent>(e);
            
            // Set the velocity based on the input vector and speed
            vel.velocity = Vector2Scale(movement, speed);
        }
    }
}
