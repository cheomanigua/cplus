#include <iostream>
#include <functional>
#include "Core/FormulaProcessor.hpp"
#include "Engine/EngineDriver.hpp"
#include "Systems/MovementSystem.hpp"

EngineDriver::EngineDriver(IEngineFacade* view, 
                           DataLoader& loader, 
                           const std::unordered_map<int32_t, ItemData>& items, 
                           std::string dataPath, 
                           EntityRegistry* registry)
    : _registry(registry), 
      _dataLoader(loader), 
      _items(items), 
      _dataDirectory(dataPath), 
      _view(view) 
{
    // Constructor body must be opened with {
}

void EngineDriver::Tick(float deltaTime) {
    // 1. Process Movement "Intent"
    MovementSystem::ProcessCommands(_commandQueue, _movementComponent);

    // 2. Perform Movement "Execution"
    MovementSystem::Update(_movementComponent, deltaTime, *_registry);

    // 3. Process Other Commands (Stats/Combat)
    while (_commandQueue.HasCommands()) {
        GameCommand cmd = _commandQueue.Dequeue();
        switch (cmd.Type) {
          // Inside EngineDriver::Tick(float deltaTime) -> case CommandType::UpdateStats:
            case CommandType::UpdateStats: {
                EntityStats* stats = _registry->GetEntityStats(cmd.EntityId);
                
                // Retrieve metadata directly from the registry
                const auto& meta = _registry->GetMetadata(cmd.EntityId);
                
                // Retrieve template data from the loader
                const auto& cls = _dataLoader.GetClassData(meta.Class);
                const auto& race = _dataLoader.GetRaceData(meta.Race);
                
                if (stats) {
                    stats->IsDirty = true; // Flag for formula processing
                    
                    // Execute formulas using the metadata
                    FormulaProcessor::ExecuteUpdateStats("UpdateStats", *stats, cls, race);
                    
                    stats->IsDirty = false;
                    std::cout << "[DEBUG] Processed stats for " << cmd.EntityId 
                              << " (" << meta.Class << "/" << meta.Race << ")" << std::endl;
                }
                break;
            }
            case CommandType::EquipItem:
                break;
            default:
                std::cout << "[DEBUG] Unrecognized command type: " << (int)cmd.Type << std::endl;
                break;
        }
    }
}

void EngineDriver::AddCommand(GameCommand cmd) {
    _commandQueue.Enqueue(cmd);
}

void EngineDriver::InitializeEntities(DataLoader& loader) {
    // Logic moved to Main.cpp using sharedRegistry.SpawnNPC(templateData)
}
