#include "RaylibGameView.hpp"

void RaylibGameView::DrawMesh(int32_t id, const Vector2& transform) {
    Vector2 position = { transform.x, transform.y };
    
    // Example logic: Entity 1 is a Circle (Thrall), Entity 2 is a Rectangle (Sergio)
    if (id == 1) {
        DrawCircleV(position, 10.0f, BLUE);
        DrawCircleLinesV(position, 50.0f, DARKBLUE);
    } else if (id == 2) {
        DrawRectangleV(position, { 20.0f, 20.0f }, RED);
    }
}

bool RaylibGameView::IsActionPressed(const std::string& actionName) {
    if (actionName == "Move") {
        return IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    }
    return false;
}
