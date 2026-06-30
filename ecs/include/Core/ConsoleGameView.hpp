#pragma once
#include "Core/Interfaces/IEngineFacade.hpp"
#include "Core/Contracts/CharacterSheetDto.hpp"
#include <iostream>

class ConsoleGameView : public IEngineFacade {
public:
    void DrawMesh(int32_t id, const Transform2D& transform) override;
    bool IsActionPressed(const std::string& actionName) override;
    void Render(const CharacterSheetDto& data);
};
