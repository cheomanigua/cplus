#include "InputSystem.h"
#include "Input.h"
#include "Position.h"
#include "Tags.h"
#include "SelectionBounds.h"
#include <raylib.h>

void InputSystem::update(flecs::world& world)
{
    // 1. Handle Mouse Selection using selection bounds
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        Vector2 mousePos = GetMousePosition();
    
        // Remove TagSelected from all entities in a single batch operation
        world.remove_all<TagSelected>();
    
        // Query only entities that have both Position and SelectionBounds
        auto query = world.query<Position, SelectionBounds>();
        auto entity = query.find([&](Position& position, SelectionBounds& bounds)
        {
            return CheckCollisionPointCircle(mousePos, position.position, bounds.radius);
        });

        if (entity)
            entity.add<TagSelected>();
    }
    // 2. Read keyboard states using Raylib
    bool up = IsKeyDown(KEY_UP) || IsKeyDown(KEY_W);
    bool down = IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S);
    bool left = IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A);
    bool right = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);

    // 3. Pass raw intent to all entities that have an InputComponent
    auto query = world.query<Input, TagSelected>();
    query.each([&](Input& input, TagSelected)
    {
        input.moveUp = up;
        input.moveDown = down;
        input.moveLeft = left;
        input.moveRight = right;
    });
}
