#include "InputSystem.h"
#include "Input.h"
#include "Position.h"
#include "Tags.h"
#include "SelectionBounds.h"
#include <raylib.h>

void InputSystem::update(entt::registry& registry)
{
    // 1. Handle Mouse Selection using selection bounds
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        Vector2 mousePos = GetMousePosition();
    
        // Remove TagSelected from all entities in a single batch operation
        registry.clear<TagSelected>();
    
        // Query only entities that have both Position and SelectionBounds
        auto view = registry.view<Position, SelectionBounds>();
        for (auto [entity, pos, bounds] : view.each())
        {
            if (CheckCollisionPointCircle(mousePos, pos.position, bounds.radius))
            {
                registry.emplace<TagSelected>(entity);
                break; // Select the first entity hit and stop
            }
        }
    }
    // 2. Read keyboard states using Raylib
    bool up = IsKeyDown(KEY_UP) || IsKeyDown(KEY_W);
    bool down = IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S);
    bool left = IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A);
    bool right = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);

    // 3. Pass raw intent to all entities that have an InputComponent
    auto view  = registry.view<Input, TagSelected>();
    for (auto [entity, input] : view.each())
    {
        input.moveUp = up;
        input.moveDown = down;
        input.moveLeft = left;
        input.moveRight = right;
    }
}
