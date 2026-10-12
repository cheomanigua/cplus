#include "InputSystem.h"
#include <raylib.h>

InputSystem::InputSystem(flecs::world& world)
    : world{world},
      selectionQuery{world.query<Position, SelectionBounds>()},
      inputQuery{world.query<Input, TagSelected>()}
{
}

void InputSystem::update()
{
    // 1. Handle mouse selection
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        Vector2 mousePos = GetMousePosition();

        // Deselect all entities
        world.remove_all<TagSelected>();

        // Select the first entity under the mouse
        auto entity = selectionQuery.find(
            [&](Position& position, SelectionBounds& bounds)
            {
                return CheckCollisionPointCircle(mousePos, position.position, bounds.radius);
            });

        if (entity)
            entity.add<TagSelected>();
    }

    // 2. Read keyboard states
    const bool up = IsKeyDown(KEY_UP) || IsKeyDown(KEY_W);
    const bool down = IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S);
    const bool left = IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A);
    const bool right = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);

    // 3. Update input for selected entities
    inputQuery.each([&](Input& input, TagSelected)
    {
        input.moveUp = up;
        input.moveDown = down;
        input.moveLeft = left;
        input.moveRight = right;
    });
}
