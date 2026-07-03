#pragma once
#include "Core/Interfaces/IEngineFacade.hpp"
#include "raylib.h"
#include <string>

class RaylibGameView : public IEngineFacade {
private:
    // Using underscore prefix for private member
    Vector2 _clickPosition = { -1.0f, -1.0f }; 

public:
    // Implementation of the Engine Contract
    void DrawMesh(int32_t id, const Vector2& transform) override;
    bool IsActionPressed(const std::string& actionName) override;
    
    // New methods for testing
    void SetClickPosition(Vector2 pos) { _clickPosition = pos; }
    Vector2 GetClickPosition() const { return _clickPosition; }
};
