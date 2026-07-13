#pragma once
#include "Core/Interfaces/IEngineFacade.hpp"
#include "Engine/EntityRegistry.hpp"
#include "Components/MovementComponent.hpp"
#include "raylib.h"

class RaylibGameView : public IEngineFacade {
private:
    const EntityRegistry* _registry = nullptr;

public:
    void SetRegistry(const EntityRegistry* reg) { _registry = reg; }
    
    // Updated to accept MovementComponent for decoupled rendering
    void DrawMesh(int32_t id, const Vector2& transform, const MovementComponent& moveComp) override;

    bool IsActionPressed(const std::string& actionName) override;
};
