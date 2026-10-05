#pragma once
#include <string>
#include <iostream>
#include "raylib.h"
#include "Components/MovementComponent.hpp"


class IEngineFacade {
public:
    virtual ~IEngineFacade() = default;
    virtual bool IsActionPressed(const std::string& actionName) = 0;
    virtual void DrawMesh(int32_t id, const Vector2& transform, const MovementComponent& moveComp) = 0;
};

namespace EngineFacade {
    inline IEngineFacade* Implementation = nullptr;
}

namespace EngineService {
    inline bool IsActionPressed(const std::string& actionName) {
        if (!EngineFacade::Implementation) {
            std::cerr << "[CRITICAL] EngineFacade::Implementation is NULL!" << std::endl;
            return false;
        }
        return EngineFacade::Implementation->IsActionPressed(actionName);
    }

    inline void DrawMesh(int32_t id, const Vector2& transform, const MovementComponent& moveComp) {
        if (!EngineFacade::Implementation) {
            std::cerr << "[CRITICAL] EngineFacade::Implementation is NULL!" << std::endl;
            return;
        }
        EngineFacade::Implementation->DrawMesh(id, transform, moveComp);
    }
}
