#include <iostream>
#include <functional>
#include "Core/FormulaProcessor.hpp"
#include "Engine/EngineDriver.hpp"
#include "Systems/MovementSystem.hpp"

EngineDriver::EngineDriver(IEngineFacade* view, DataLoader& loader, std::vector<ItemData> items, 
                           std::string dir, EntityRegistry* sharedRegistry)
    : _view(view), _dataLoader(loader), _dataDirectory(std::move(dir)), _registry(sharedRegistry) {
}

void EngineDriver::Tick(float deltaTime) {
    // 1. Process Movement "Intent"
    MovementSystem::ProcessCommands(_commandQueue, _movementBuffers);

    // 2. Perform Movement "Execution"
    MovementSystem::Update(_movementBuffers, deltaTime);

    // 3. Process Other Commands (Stats/Combat)
    while (_commandQueue.HasCommands()) {
        GameCommand cmd = _commandQueue.Dequeue();
        switch (cmd.Type) {
            case CommandType::UpdateStats: {
                EntityStats* stats = _registry->GetEntityStats(cmd.EntityId);
                const auto* bp = _dataLoader.GetBlueprintById(cmd.EntityId);
                
                // TEMPORARY DEBUG HACK: Force the flag to verify the pipeline
                if (stats) {
                    stats->IsDirty = true; 
                }
            
                // Now the check will pass regardless of what happened earlier in the frame
                if (stats && stats->IsDirty && bp) {
                    const auto& cls = _dataLoader.GetClassData(bp->Class);
                    const auto& race = _dataLoader.GetRaceData(bp->Race);
                    
                    FormulaProcessor::Execute("UpdateStats", *stats, cls, race);
                    
                    stats->IsDirty = false; // Mark as clean
                }
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
    _commandQueue.Enqueue(cmd);
}

void EngineDriver::InitializeEntities(DataLoader& loader) {
    for (const auto& npc : loader.GetNPCs()) {
        EntityStats stats;
        stats.IsDirty = true; // Tell the system: "I need to run the formulas!"
        
        _registry->RegisterStats(npc.EntityId, stats);
        
        Vector2* pos = _registry->GetPosition(npc.EntityId);
        if (pos) *pos = npc.SpawnPosition;
    }
}
