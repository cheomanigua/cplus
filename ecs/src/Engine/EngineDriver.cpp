#include <iostream>
#include <functional>
#include "Core/FormulaProcessor.hpp"
#include "Engine/EngineDriver.hpp"

EngineDriver::EngineDriver(IEngineFacade* view, DataLoader& loader, std::vector<ItemData> items, std::string dir)
    : _view(view), _dataLoader(loader), _dataDirectory(std::move(dir)) {
    _registry = std::make_unique<EntityRegistry>(std::move(items));
}

void EngineDriver::Tick(float deltaTime) {
    // 1. Process Command Queue
    while (!_queue.empty()) {
        GameCommand cmd = _queue.front();
        _queue.pop();

        switch (cmd.Type) {
            case CommandType::Move:
                // Logic: Access MovementSystem
                break;
            case CommandType::UpdateStats: {
              const auto* bp = _dataLoader.GetBlueprintById(cmd.EntityId);
              EntityStats* stats = _registry->GetEntityStats(cmd.EntityId);
              if (!bp || !stats) break;
          
              // Use .at() for safer access if you are unsure if the key exists
              const auto& classData = _dataLoader.GetClassData(bp->Class);
              const auto& raceData = _dataLoader.GetRaceData(bp->Race);
              
              // Bind data sources
              FormulaProcessor::RegisterSource("ClassStr", [&classData]() { return static_cast<float>(classData.ClassStr); });
              FormulaProcessor::RegisterSource("RaceStr", [&raceData]() { return static_cast<float>(raceData.RaceStr); });
              FormulaProcessor::RegisterSource("ClassInt", [&classData]() { return static_cast<float>(classData.ClassInt); });
              FormulaProcessor::RegisterSource("RaceInt", [&raceData]() { return static_cast<float>(raceData.RaceInt); });
              
              FormulaProcessor::RegisterSource("Strength", [stats]() { return stats->Strength; });
              FormulaProcessor::RegisterSource("Intelligence", [stats]() { return stats->Intelligence; });
          
              // Execute the formula defined in formulas.json
              //std::vector<float> dummy; 
              //stats->Strength = FormulaProcessor::Execute("UpdateStats", dummy); 
              //
              //stats->IsDirty = false;
              //break;



              // 4. Run the formula (Cleaned up)
stats->Strength     = FormulaProcessor::Execute("UpdateStats", "Strength");
stats->Health       = FormulaProcessor::Execute("UpdateStats", "Health");
stats->Intelligence = FormulaProcessor::Execute("UpdateStats", "Intelligence");
stats->Mana         = FormulaProcessor::Execute("UpdateStats", "Mana");

// Log the results
std::cout << "DEBUG: Formula results for ID " << cmd.EntityId 
          << " | Str: " << stats->Strength 
          << " | Health: " << stats->Health << std::endl;

stats->IsDirty = false;
                break;
          }
            case CommandType::EquipItem:
                // Logic: Access EquipmentSystem
                break;
        }
    }

    // 2. Trigger Systems
    // Example: _updateSystem->Update(deltaTime);
    
    // 3. Post-Process
    _registry->ProcessCombat();
}

void EngineDriver::AddCommand(GameCommand cmd) {
    _queue.push(cmd);
}
