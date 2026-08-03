#include "InputSystem.h"
#include "VelocityComponent.h"
#include "PositionComponent.h"
#include "SpeedComponent.h"
#include "TagSelectedComponent.h"
#include "SelectionBoundsComponent.h" // Updated component name
#include <raylib.h>
#include <raymath.h>

void InputSystem::update(Registry& registry)
{
    // 1. Handle Mouse Selection using selection bounds
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        Vector2 mousePos = GetMousePosition();

        // First, clear selection from all entities (single-selection model)
        for (Entity e : registry.getEntities())
        {
            if (registry.hasComponent<TagSelectedComponent>(e))
            {
                registry.removeComponent<TagSelectedComponent>(e);
            }
        }

        // Check which entity was clicked based on its specific SelectionBoundsComponent
        for (Entity e : registry.getEntities())
        {
            if (registry.hasComponent<PositionComponent>(e) && registry.hasComponent<SelectionBoundsComponent>(e))
            {
                auto& pos = registry.getComponent<PositionComponent>(e);
                auto& bounds = registry.getComponent<SelectionBoundsComponent>(e);

                // Uses each entity's individual bounding size dynamically
                if (CheckCollisionPointCircle(mousePos, pos.position, bounds.radius))
                {
                    registry.addComponent<TagSelectedComponent>(e);
                    break; // Select the first entity hit and stop
                }
            }
        }
    }

    // 2. Read keyboard state for movement
    Vector2 movement = {
        static_cast<float>(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) - static_cast<float>(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)),
        static_cast<float>(IsKeyDown(KEY_DOWN)  || IsKeyDown(KEY_S)) - static_cast<float>(IsKeyDown(KEY_UP)   || IsKeyDown(KEY_W))
    };

    if (Vector2Length(movement) > 0.0f)
    {
        movement = Vector2Normalize(movement);
    }

    // 3. Apply velocity ONLY to entities that have TagSelectedComponent and VelocityComponent
    for (Entity e : registry.getEntities())
    {
        if (registry.hasComponent<TagSelectedComponent>(e) && 
            registry.hasComponent<VelocityComponent>(e) && 
            registry.hasComponent<SpeedComponent>(e))
        {
            auto& vel = registry.getComponent<VelocityComponent>(e);
            auto& spd = registry.getComponent<SpeedComponent>(e);
            vel.velocity = Vector2Scale(movement, spd.speed);
        }
    }
}
