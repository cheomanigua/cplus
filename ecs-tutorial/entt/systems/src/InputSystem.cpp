#include "InputSystem.h"
#include <raylib.h>

InputSystem::InputSystem(entt::registry& registry)
    : registry{registry},
      selectionView{registry.view<Position, SelectionBounds>()},
      inputView{registry.view<Input, TagSelected>()}
{
}

void InputSystem::update()
{
    // 1. Handle mouse selection
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        Vector2 mousePos = GetMousePosition();

        // Deselect all entities
        registry.clear<TagSelected>();

        // Select the first entity under the mouse
        for (auto [entity, pos, bounds] : selectionView.each())
        {
            if (CheckCollisionPointCircle(mousePos, pos.position, bounds.radius))
            {
                registry.emplace<TagSelected>(entity);
                break;
            }
        }
    }

    // 2. Read keyboard states
    const bool up = IsKeyDown(KEY_UP) || IsKeyDown(KEY_W);
    const bool down = IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S);
    const bool left = IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A);
    const bool right = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);

    // 3. Apply input to selected entities
    for (auto [entity, input] : inputView.each())
    {
        input.moveUp = up;
        input.moveDown = down;
        input.moveLeft = left;
        input.moveRight = right;
    }
}
