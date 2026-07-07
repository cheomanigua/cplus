#include "RaylibGameView.hpp"

void RaylibGameView::DrawMesh(int32_t id, const Vector2& transform) {
    bool isSelected = (_registry && _registry->GetSelectedEntity() == id);
    Vector2 position = { transform.x, transform.y };
    
    // Example logic: Entity 1 is a Circle (Thrall), Entity 2 is a Rectangle (Sergio)
    if (id == 1) {
        DrawCircleV(position, 10.0f, BLUE);
        DrawCircleLinesV(position, 50.0f, DARKBLUE);
    } else if (id == 2) {
        DrawRectangleV(position, { 20.0f, 20.0f }, RED);
    }

    // Draw the debug click marker
    if (_clickPosition.x >= 0) {
        DrawCircleV(_clickPosition, 5.0f, GREEN); // Small green circle at click
    }

    // Highlight if selected
    if (isSelected) {
        DrawCircleLinesV(position, 15.0f, BLACK);
    }
}

bool RaylibGameView::IsActionPressed(const std::string& actionName) {
    // if (actionName == "Move") {
    //     return IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    // }
    // return false;

    if (actionName == "Move") {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            _clickPosition = GetMousePosition(); // Store the click
            return true;
        }
    }
    return false;
}
