#include <iostream>
#include <functional>
#include "Core/FormulaProcessor.hpp"
#include "Engine/EngineDriver.hpp"
#include "Systems/MovementSystem.hpp"

EngineDriver::EngineDriver(IEngineFacade* view, 
                           DataLoader& loader, 
                           const std::unordered_map<int32_t, ItemData>& items, 
                           std::string dataPath, 
                           EntityRegistry* registry, 
                           StatsComponent& statsComp, 
                           PositionComponent& posComp, 
                           MovementComponent& moveComp)
    : _registry(registry), 
      _dataLoader(loader), 
      _items(items), 
      _dataDirectory(dataPath), 
      _view(view), 
      _statsComp(statsComp), 
      _posComp(posComp),
      _moveComp(moveComp)
{
    // Constructor body must be opened with {
}

void EngineDriver::Tick(float deltaTime) {
    // 1. Update Spatial Grid (Fresh state for the start of the frame)
    _spatialSystem.Update(*_registry, _posComp);

    // 2. Process Movement "Intent"
    MovementSystem::ProcessCommands(_commandQueue, _moveComp);

    // 3. Perform Movement "Execution"
    MovementSystem::Update(_moveComp, _posComp, _registry->GetActiveEntities(), deltaTime);

    // 4. Process Other Commands (Stats/Combat)
    while (_commandQueue.HasCommands()) {
        GameCommand cmd = _commandQueue.Dequeue();
        switch (cmd.Type) {
            case CommandType::UpdateStats: {
                EntityStats& stats = _statsComp.Data[cmd.EntityId];
                
                // Retrieve metadata directly from the registry
                const auto& meta = _registry->GetMetadata(cmd.EntityId);
                
                // Retrieve template data from the loader
                const auto& cls = _dataLoader.GetClassData(meta.Class);
                const auto& race = _dataLoader.GetRaceData(meta.Race);
                
                stats.IsDirty = true; // Flag for formula processing
                
                // Execute formulas using the metadata
                FormulaProcessor::ExecuteUpdateStats("UpdateStats", stats, cls, race);
                
                stats.IsDirty = false;
                std::cout << "[DEBUG] Processed stats for " << cmd.EntityId 
                              << " (" << meta.Class << "/" << meta.Race << ")" << std::endl;
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
