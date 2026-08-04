#include "InputSystem.h"
#include "InputComponent.h"
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

    // 2. Read keyboard states and pass raw intent to selected entities that have an InputComponent
    bool up = IsKeyDown(KEY_UP) || IsKeyDown(KEY_W);
    bool down = IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S);
    bool left = IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A);
    bool right = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);

    // 3. Apply ONLY to entities that have TagSelectedComponent and InputComponent
    for (Entity e : registry.getEntities())
    {
        if (registry.hasComponent<TagSelectedComponent>(e) && registry.hasComponent<InputComponent>(e))
        {
            auto& input = registry.getComponent<InputComponent>(e);
            input.moveUp = up;
            input.moveDown = down;
            input.moveLeft = left;
            input.moveRight = right;
        }
    }
}
