#include <iostream>
#include <functional>
#include "Core/FormulaProcessor.hpp"
#include "Engine/EngineDriver.hpp"
#include "Systems/MovementSystem.hpp"

EngineDriver::EngineDriver(IEngineFacade* view, DataLoader& loader, std::vector<ItemData> items, std::string dir)
    : _view(view), _dataLoader(loader), _dataDirectory(std::move(dir)) {
    _registry = std::make_unique<EntityRegistry>(std::move(items));
}

void EngineDriver::Tick(float deltaTime) {
    // 1. Process Movement "Intent"
    MovementSystem::ProcessCommands(_commandQueue, _movementBuffers);

    // 2. Perform Movement "Execution"
    MovementSystem::Update(_movementBuffers, deltaTime);

    // 3. Process Other Commands (Stats/Combat)
    std::cout << "[DEBUG] Starting command processing." << std::endl;
    
    while (_commandQueue.HasCommands()) {
        GameCommand cmd = _commandQueue.Dequeue();

        std::cout << "[DEBUG] Processing command type: " << (int)cmd.Type << " for Entity: " << cmd.EntityId << std::endl;

        switch (cmd.Type) {
            case CommandType::UpdateStats: {
                const auto* bp = _dataLoader.GetBlueprintById(cmd.EntityId);
                EntityStats* stats = _registry->GetEntityStats(cmd.EntityId);
                if (!bp || !stats) {
                    std::cout << "[DEBUG] UpdateStats failed: Blueprint or Stats not found for ID " << cmd.EntityId << std::endl;
                    break;
                }
          
                // Bind data sources for the formula processor
                const auto& classData = _dataLoader.GetClassData(bp->Class);
                const auto& raceData = _dataLoader.GetRaceData(bp->Race);
                
                std::cout << "[DEBUG] Binding Data for ID " << cmd.EntityId 
                          << " | ClassStr: " << classData.ClassStr 
                          << " | RaceStr: " << raceData.RaceStr << std::endl;
                
                FormulaProcessor::RegisterSource("ClassStr", [&classData]() { return static_cast<float>(classData.ClassStr); });
                FormulaProcessor::RegisterSource("RaceStr", [&raceData]() { return static_cast<float>(raceData.RaceStr); });
                FormulaProcessor::RegisterSource("ClassInt", [&classData]() { return static_cast<float>(classData.ClassInt); });
                FormulaProcessor::RegisterSource("RaceInt", [&raceData]() { return static_cast<float>(raceData.RaceInt); });
                FormulaProcessor::RegisterSource("Strength", [stats]() { return stats->Strength; });
                FormulaProcessor::RegisterSource("Intelligence", [stats]() { return stats->Intelligence; });
          
                // Execute formulas
                stats->Strength     = FormulaProcessor::Execute("UpdateStats", "Strength");
                stats->Health       = FormulaProcessor::Execute("UpdateStats", "Health");
                stats->Intelligence = FormulaProcessor::Execute("UpdateStats", "Intelligence");
                stats->Mana         = FormulaProcessor::Execute("UpdateStats", "Mana");

                std::cout << "[DEBUG] Formula results for ID " << cmd.EntityId 
                          << " | Str: " << stats->Strength 
                          << " | Health: " << stats->Health << std::endl;

                stats->IsDirty = false;
                break;
            }
            case CommandType::EquipItem:
                break;
            default:
                std::cout << "[DEBUG] Unrecognized command type received: " << (int)cmd.Type << std::endl;
                break;
        }
    }

    // 4. Post-Process
    _registry->ProcessCombat();
}

void EngineDriver::AddCommand(GameCommand cmd) {
  std::cout << "[DEBUG] Enqueueing command for Entity: " << cmd.EntityId << std::endl;
    _commandQueue.Enqueue(cmd);
}
