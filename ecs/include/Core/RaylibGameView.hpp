#pragma once
#include "Core/Interfaces/IEngineFacade.hpp"
#include "raylib.h"
#include <string>

class RaylibGameView : public IEngineFacade {
public:
    // Implementation of the Engine Contract
    void DrawMesh(int32_t id, const Vector2& transform) override;
    bool IsActionPressed(const std::string& actionName) override;
};
