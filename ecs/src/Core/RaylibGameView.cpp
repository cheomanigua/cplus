#include "Core/RaylibGameView.hpp"
#include "Engine/EntityRegistry.hpp"
#include "Components/MovementComponent.hpp"
#include <raylib.h>

void RaylibGameView::DrawMesh(int32_t id, const Vector2& transform, const MovementComponent& moveComp) {
    // 1. Determine selection state
    bool isSelected = (_registry && _registry->GetSelectedEntity() == id);

    // 2. Render the entity based on its ID
    if (id == 1) {
        DrawCircleV(transform, 10.0f, BLUE);
        DrawCircleLinesV(transform, 50.0f, DARKBLUE);
    } else if (id == 2) {
        DrawRectangleV(transform, { 20.0f, 20.0f }, RED);
    }

    // 3. Render the target marker ONLY if the MovementSystem has marked this entity as moving
    // This reads state directly from the component rather than storing it locally
    if (moveComp.IsMoving[id]) {
        DrawCircleV(moveComp.TargetPositions[id], 5.0f, GREEN);
        DrawLineV(transform, moveComp.TargetPositions[id], GREEN);
    }

    // 4. Render selection highlight
    if (isSelected) {
        DrawCircleLinesV(transform, 15.0f, BLACK);
    }
}


bool RaylibGameView::IsActionPressed(const std::string& actionName) {
    return false; // Or implement your input logic here
}
