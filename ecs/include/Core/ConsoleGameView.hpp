#pragma once
#include "Engine/EngineDriver.hpp"
#include "Engine/MovementComponent.hpp"
#include "Engine/PositionComponent.hpp"
#include "Core/Interfaces/IEngineFacade.hpp"
#include "Core/Contracts/CharacterSheetDto.hpp"
#include <iostream>
#include <vector>
#include "raylib.h"

struct StatsComponent;

class ConsoleGameView : public IEngineFacade {
public:
    // Stub implementations: fulfill the IEngineFacade contract
    // They are empty because we are not using them in ConsoleGameView
    void DrawMesh(int32_t id, const Vector2& transform, const MovementComponent& moveComp) override;
    bool IsActionPressed(const std::string& actionName) override;

    // Logic moved from Main.cpp
    void RenderNPCStats(const std::vector<NPCBlueprint>& npcs, EngineDriver& engine);
    
    void DisplayFullCharacterSheet(EntityRegistry& registry, 
                               const PositionComponent& posComp, 
                               const StatsComponent& statsComp, 
                               const DataLoader& loader);
    void Render(const CharacterSheetDto& data);
};
