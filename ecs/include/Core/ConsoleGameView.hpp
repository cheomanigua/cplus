#pragma once
#include "Core/Interfaces/IEngineFacade.hpp"
#include "Engine/EngineDriver.hpp"
#include "Core/Contracts/CharacterSheetDto.hpp"
#include <iostream>
#include <vector>
#include "raylib.h"

class ConsoleGameView : public IEngineFacade {
public:
    // Stub implementations: fulfill the IEngineFacade contract
    // They are empty because we are not using them in ConsoleGameView
    void DrawMesh(int32_t id, const Vector2& transform) override;
    bool IsActionPressed(const std::string& actionName) override { return false; }

    // Logic moved from Main.cpp
    void RenderNPCStats(const std::vector<NPCBlueprint>& npcs, EngineDriver& engine);
    
    void DisplayFullCharacterSheet(EntityRegistry& registry, const DataLoader& loader);
    void Render(const CharacterSheetDto& data);
};
